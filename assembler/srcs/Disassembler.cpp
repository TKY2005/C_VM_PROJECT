#include<Disassembler.hpp>

#include<ISA_encoding_info.h>

#include<algorithm>
#include<array>
#include<cctype>
#include<cstdint>
#include<fstream>
#include<iomanip>
#include<iostream>
#include<iterator>
#include<limits>
#include<map>
#include<optional>
#include<queue>
#include<sstream>
#include<string>
#include<utility>
#include<vector>

namespace {
struct DecodedInstruction {
    size_t offset = 0;
    size_t length = 0;
    uint8_t opcode = 0;
    std::string mnemonic;
    std::string operands;
    std::vector<uint8_t> bytes;
    bool hasTarget = false;
    uint32_t target = 0;
};

struct DecodedOperand {
    uint8_t type = UNKNOWN;
    uint8_t registerCode = REG_UNSELECTED;
    uint32_t value = 0;
    bool isMemory = false;
};

const std::array<std::string, 256>& instructionNames()
{
    static const std::array<std::string, 256> names = [] {
        std::array<std::string, 256> result{};
#define X(mnemonic, opcode, funcname) result[opcode] = #mnemonic;
        INSTRUCTION_LIST(X)
#undef X
        for (std::string& name : result) {
            std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
        }
        result[INS_SET] = "mov";
        return result;
    }();
    return names;
}

bool readValue(
    const std::vector<uint8_t>& bytes,
    size_t& cursor,
    size_t size,
    uint32_t& value
)
{
    if (size > bytes.size() - std::min(cursor, bytes.size())) return false;
    value = 0;
    for (size_t i = 0; i < size; ++i) {
        value = (value << 8) | bytes[cursor++];
    }
    return true;
}

std::string hexValue(uint32_t value)
{
    std::ostringstream result;
    result << "0x" << std::uppercase << std::hex << value;
    return result.str();
}

std::string registerName(uint8_t type, uint8_t code)
{
    static const char* const names32[] = {
        "eax", "ebx", "ecx", "edx", "eex", "efx", "si", "di",
        "bp", "sp", "ss", "ds", "cs"
    };
    static const char* const names16[] = {
        "ax", "bx", "cx", "dx", "ex", "fx", "si", "di",
        "bp", "sp", "ss", "ds", "cs"
    };
    static const char* const names8[] = {
        "al", "bl", "cl", "dl", "el", "fl"
    };

    if (type == REG8) {
        const uint8_t index = code & 0x07;
        if (index >= sizeof(names8) / sizeof(names8[0])) return "?";
        std::string result = names8[index];
        if ((code & 0x08) != 0) result[1] = 'h';
        return result;
    }
    if (code >= sizeof(names32) / sizeof(names32[0])) return "?";
    if (type == REG16) return names16[code];
    if (type == REG32) return names32[code];
    return "?";
}

std::string operandName(const DecodedOperand& operand)
{
    if (operand.type == UNKNOWN) return hexValue(operand.value);
    if (operand.type == NUMBER) return hexValue(operand.value);
    if (operand.type == REG8 || operand.type == REG16 || operand.type == REG32) {
        return registerName(operand.type, operand.registerCode);
    }
    if (operand.isMemory) return hexValue(operand.value);
    return "?";
}

bool isRegisterType(uint8_t type)
{
    return type <= REG32;
}

bool isMemoryType(uint8_t type)
{
    return type >= MEM8 && type <= MEM32;
}

bool isValueType(uint8_t type)
{
    return type == NUMBER || type == UNKNOWN;
}

bool isSingleOperand(uint8_t opcode)
{
    return opcode == INS_OUT || opcode == INS_SQRT || opcode == INS_INC ||
        opcode == INS_DEC || opcode == INS_PUSH || opcode == INS_POP ||
        opcode == INS_LOOP ||
        (opcode >= INS_CALL && opcode <= INS_CALB) ||
        (opcode >= INS_JMP && opcode <= INS_JB) ||
        opcode == INS_NOT || opcode == INS_LOOP || opcode == INS_INT ||
        opcode == INS_OUTC || (opcode >= INS_JC && opcode <= INS_JNO) ||
        (opcode >= INS_CALC && opcode <= INS_CALNO) ||
        (opcode >= INS_JA && opcode <= INS_JBE) ||
        (opcode >= INS_CALA && opcode <= INS_CALBE) ||
        opcode == INS_LIVT;
}

bool isOperandless(uint8_t opcode)
{
    return opcode == INS_NOP || opcode == INS_RET || opcode == INS_END ||
        opcode == INS_EXT || opcode == INS_IRET || opcode == INS_LEN ||
        opcode == INS_STR || opcode == INS_OUTS || opcode == INS_INP ||
        opcode == INS_INPS || opcode == INS_OUTSW || opcode == INS_LENW ||
        (opcode >= INS_CLC && opcode <= INS_SLI);
}

bool isUnconditionalTerminator(uint8_t opcode)
{
    return opcode == INS_JMP || opcode == INS_RET || opcode == INS_END ||
        opcode == INS_EXT || opcode == INS_IRET;
}

size_t loopBodyPenalty(
    const std::map<size_t, DecodedInstruction>& instructions,
    size_t targetOffset,
    size_t loopOffset
)
{
    size_t cursor = targetOffset;
    size_t instructionCount = 0;
    while (cursor < loopOffset && instructionCount < 64) {
        const auto instruction = instructions.find(cursor);
        if (instruction == instructions.end()) return 64;
        cursor += instruction->second.length;
        ++instructionCount;
    }
    if (cursor != loopOffset) return 64;
    return instructionCount > 3 ? instructionCount - 3 : 3 - instructionCount;
}

bool hasControlTarget(uint8_t opcode)
{
    return (opcode >= INS_CALL && opcode <= INS_CALB) ||
        (opcode >= INS_JMP && opcode <= INS_JB) ||
        opcode == INS_LOOP ||
        (opcode >= INS_JC && opcode <= INS_JNO) ||
        (opcode >= INS_CALC && opcode <= INS_CALNO) ||
        (opcode >= INS_JA && opcode <= INS_JBE) ||
        (opcode >= INS_CALA && opcode <= INS_CALBE);
}

bool decodeInstruction(
    const std::vector<uint8_t>& bytes,
    size_t offset,
    DecodedInstruction& decoded
)
{
    if (offset >= bytes.size() || instructionNames()[bytes[offset]].empty()) return false;

    const uint8_t opcode = bytes[offset];
    size_t cursor = offset + 1;
    decoded.offset = offset;
    decoded.opcode = opcode;
    decoded.mnemonic = instructionNames()[opcode];
    if (isOperandless(opcode)) {
        decoded.length = 1;
        decoded.bytes.assign(bytes.begin() + offset, bytes.begin() + cursor);
        return true;
    }

    if (cursor >= bytes.size()) return false;
    const uint8_t operandTypes = bytes[cursor++];
    const uint8_t sourceType = static_cast<uint8_t>(operandTypes >> 5);
    const uint8_t destinationType = static_cast<uint8_t>((operandTypes >> 2) & 0x07);
    const uint8_t memoryMode = operandTypes & 0x03;
    const bool singleOperand = isSingleOperand(opcode);
    const bool hasSource = sourceType != UNKNOWN ||
        (!singleOperand && destinationType != UNKNOWN && sourceType == UNKNOWN);
    const bool hasDestination = destinationType != UNKNOWN ||
        (singleOperand && destinationType == UNKNOWN);
    const bool hasSourceRegister = isRegisterType(sourceType);
    const bool hasDestinationRegister = isRegisterType(destinationType);
    const bool hasSourceMemory = isMemoryType(sourceType);
    const bool hasDestinationMemory = isMemoryType(destinationType);
    const bool hasMemory = hasSourceMemory || hasDestinationMemory;

    if ((!hasSource && !hasDestination) ||
        (hasSource && !hasSourceRegister && !hasSourceMemory && !isValueType(sourceType)) ||
        (hasDestination && !hasDestinationRegister && !hasDestinationMemory &&
         !isValueType(destinationType)) ||
        (hasSourceMemory && hasDestinationMemory) ||
        (!hasMemory && memoryMode != NOMEM) ||
        (hasMemory && memoryMode == NOMEM)) {
        return false;
    }
    if (singleOperand && hasSource) return false;

    uint8_t registerSelection = REGSELECT_DEFAULT;
    if (hasSourceRegister || hasDestinationRegister) {
        if (cursor >= bytes.size()) return false;
        registerSelection = bytes[cursor++];
    }
    uint8_t displacementInfo = DISP_DEFAULT;
    if (hasMemory) {
        if (cursor >= bytes.size()) return false;
        displacementInfo = bytes[cursor++];
    }

    DecodedOperand destination;
    destination.type = destinationType;
    destination.isMemory = hasDestinationMemory;
    if (hasDestinationRegister) destination.registerCode = registerSelection & 0x0f;
    DecodedOperand source;
    source.type = sourceType;
    source.isMemory = hasSourceMemory;
    if (hasSourceRegister) source.registerCode = (registerSelection >> 4) & 0x0f;

    if (hasMemory) {
        const uint8_t baseCode = displacementInfo & 0x07;
        const uint8_t indexCode = (displacementInfo >> 3) & 0x07;
        const bool displacementEnabled = (displacementInfo & 0x40) != 0;
        const bool registersEnabled = (displacementInfo & 0x80) != 0;
        std::string address;
        static const char* const bases[] = {"sp", "bp", "si", "di", "cs", "ss", "ds"};
        static const char* const indexes[] = {"eax", "ebx", "ecx", "edx", "eex", "efx"};
        if (registersEnabled) {
            if (baseCode < sizeof(bases) / sizeof(bases[0])) address = bases[baseCode];
            if (indexCode < sizeof(indexes) / sizeof(indexes[0])) {
                if (!address.empty()) address += " + ";
                address += indexes[indexCode];
                if (memoryMode != X1SCALE) {
                    address += " * " + std::to_string(memoryMode == X2SCALE ? 2 : 4);
                }
            }
        }

        uint32_t displacement = 0;
        if (displacementEnabled && !readValue(bytes, cursor, 4, displacement)) return false;
        if (displacementEnabled) {
            const int32_t signedDisplacement = static_cast<int32_t>(displacement);
            if (address.empty()) address = hexValue(displacement);
            else if (signedDisplacement > 0) address += " + " + hexValue(displacement);
            else if (signedDisplacement < 0 &&
                     -static_cast<int64_t>(signedDisplacement) <= 0x100000) {
                const uint32_t magnitude = static_cast<uint32_t>(-static_cast<int64_t>(signedDisplacement));
                address += " - " + hexValue(magnitude);
            }
            else if (signedDisplacement < 0) {
                address += " + " + hexValue(displacement);
            }
        }
        if (address.empty()) address = "0x0";

        const uint8_t memoryType = hasDestinationMemory ? destination.type : source.type;
        const char* sizeName = memoryType == MEM8 ? "byte" :
            memoryType == MEM16 ? "word" : "dword";
        const std::string rendered = std::string(sizeName) + "[" + address + "]";
        decoded.operands = rendered;
    }

    if (destinationType == NUMBER || sourceType == NUMBER) {
        size_t immediateSize = 4;
        if (hasSource && hasDestination) {
            const uint8_t inheritedType = destinationType;
            if (inheritedType == REG8 || inheritedType == MEM8) immediateSize = 1;
            else if (inheritedType == REG16 || inheritedType == MEM16) immediateSize = 2;
        }
        uint32_t immediate = 0;
        if (!readValue(bytes, cursor, immediateSize, immediate)) return false;
        (destinationType == NUMBER ? destination : source).value = immediate;
    }
    const bool hasSymbol = (singleOperand && destinationType == UNKNOWN) ||
        (!singleOperand && destinationType != UNKNOWN && sourceType == UNKNOWN);
    if (hasSymbol) {
        uint32_t symbol = 0;
        if (!readValue(bytes, cursor, 4, symbol)) return false;
        (singleOperand ? destination : source).value = symbol;
        decoded.hasTarget = hasControlTarget(opcode);
        decoded.target = symbol;
    }

    std::vector<std::string> operands;
    if (hasDestination) {
        operands.push_back(destination.isMemory ? decoded.operands : operandName(destination));
    }
    if (hasSource) operands.push_back(operandName(source));
    if (hasSourceMemory) {
        operands.back() = decoded.operands;
    }
    decoded.operands.clear();
    for (size_t i = 0; i < operands.size(); ++i) {
        if (i != 0) decoded.operands += ", ";
        decoded.operands += operands[i];
    }

    decoded.length = cursor - offset;
    decoded.bytes.assign(bytes.begin() + offset, bytes.begin() + cursor);
    return true;
}

std::string addressText(size_t offset)
{
    std::ostringstream result;
    result << std::uppercase << std::hex << std::setw(8) << std::setfill('0')
           << static_cast<uint32_t>(offset);
    return result.str();
}

bool isStringByte(uint8_t byte)
{
    return byte >= 0x20 && byte <= 0x7e;
}

std::string quoteString(const std::vector<uint8_t>& bytes, size_t begin, size_t end)
{
    std::string result = "\"";
    for (size_t i = begin; i < end; ++i) {
        const char value = static_cast<char>(bytes[i]);
        if (value == '"' || value == '\\') result += '\\';
        result += value;
    }
    return result + '"';
}

std::string formatData(
    const std::vector<uint8_t>& bytes,
    size_t begin,
    size_t end,
    uint32_t imageBase
)
{
    size_t width = 1;
    if ((end - begin) % 4 == 0) {
        bool looksLikeDwordData = true;
        for (size_t i = begin; i < end; i += 4) {
            uint32_t value = 0;
            for (size_t j = 0; j < 4; ++j) value = (value << 8) | bytes[i + j];
            const uint64_t imageEnd = static_cast<uint64_t>(imageBase) + bytes.size();
            if (value != 0 && value > bytes.size() &&
                (value < imageBase || value >= imageEnd)) {
                looksLikeDwordData = false;
                break;
            }
        }
        if (looksLikeDwordData) width = 4;
    }
    if (width == 1) {
        if ((begin % 4) == 0 && (end - begin) % 4 == 0) width = 4;
        else if ((begin % 2) == 0 && (end - begin) % 2 == 0) width = 2;
    }

    bool hasString = false;
    for (size_t i = begin; i < end;) {
        size_t runEnd = i;
        while (runEnd < end && isStringByte(bytes[runEnd])) ++runEnd;
        if (runEnd - i >= 4) hasString = true;
        i = runEnd == i ? i + 1 : runEnd;
    }
    if (hasString) width = 1;

    const char* directive = width == 1 ? "b" : width == 2 ? "w" : "dw";
    std::string result = "def(" + std::string(directive) + ") ";
    bool first = true;
    for (size_t i = begin; i < end;) {
        size_t runEnd = i;
        while (runEnd < end && isStringByte(bytes[runEnd])) ++runEnd;
        if (runEnd - i >= 4) {
            if (!first) result += ", ";
            result += quoteString(bytes, i, runEnd);
            first = false;
            i = runEnd;
            continue;
        }

        size_t valueEnd = std::min(end, i + width);
        uint32_t value = 0;
        for (size_t j = i; j < valueEnd; ++j) value = (value << 8) | bytes[j];
        if (!first) result += ", ";
        result += hexValue(value);
        first = false;
        i = valueEnd;
    }
    return result;
}

bool isDwordDataValue(
    const std::vector<uint8_t>& bytes,
    size_t offset,
    uint32_t imageBase
)
{
    uint32_t value = 0;
    for (size_t i = 0; i < 4; ++i) value = (value << 8) | bytes[offset + i];
    const uint64_t imageEnd = static_cast<uint64_t>(imageBase) + bytes.size();
    return value == 0 || value <= bytes.size() ||
        (value >= imageBase && value < imageEnd);
}

void writeDataSegment(
    const std::vector<uint8_t>& bytes,
    size_t begin,
    size_t end,
    uint32_t imageBase,
    size_t& dataIndex,
    std::ostream& output
)
{
    if (begin == end) return;
    size_t dwordEnd = begin;
    while (dwordEnd + 4 <= end &&
           isDwordDataValue(bytes, dwordEnd, imageBase)) {
        dwordEnd += 4;
    }
    if (dwordEnd > begin && dwordEnd < end) {
        writeDataSegment(bytes, begin, dwordEnd, imageBase, dataIndex, output);
        writeDataSegment(bytes, dwordEnd, end, imageBase, dataIndex, output);
        return;
    }

    output << "<data_" << dataIndex++ << " @0x"
           << addressText(imageBase + static_cast<uint32_t>(begin)) << "> "
           << formatData(bytes, begin, end, imageBase) << '\n';
}

void decodeReachable(
    const std::vector<uint8_t>& bytes,
    uint32_t imageBase,
    std::map<size_t, DecodedInstruction>& instructions,
    std::vector<bool>& codeBytes,
    std::vector<uint32_t>& discoveredTargets,
    const std::vector<uint32_t>& entryTargets = {}
)
{
    std::queue<size_t> pending;
    if (!bytes.empty()) pending.push(0);
    for (uint32_t target : entryTargets) {
        if (target >= imageBase &&
            static_cast<uint64_t>(target) - imageBase < bytes.size()) {
            pending.push(static_cast<size_t>(target - imageBase));
        }
    }

    while (!pending.empty()) {
        size_t offset = pending.front();
        pending.pop();
        while (offset < bytes.size() && instructions.find(offset) == instructions.end()) {
            if (codeBytes[offset]) break;

            DecodedInstruction decoded;
            if (!decodeInstruction(bytes, offset, decoded)) break;
            instructions.emplace(offset, decoded);
            for (size_t i = offset; i < offset + decoded.length; ++i) codeBytes[i] = true;

            if (decoded.hasTarget) {
                discoveredTargets.push_back(decoded.target);
                if (decoded.target >= imageBase &&
                    static_cast<uint64_t>(decoded.target) - imageBase < bytes.size()) {
                    pending.push(static_cast<size_t>(decoded.target - imageBase));
                }
            }
            if (isUnconditionalTerminator(bytes[offset])) break;
            offset += decoded.length;
        }
    }
}

std::vector<size_t> linearInstructionScores(
    const std::vector<uint8_t>& bytes,
    std::vector<bool>& stringBytes
)
{
    std::vector<size_t> scores(bytes.size(), 0);
    stringBytes.assign(bytes.size(), false);
    for (size_t offset = 0; offset < bytes.size();) {
        size_t end = offset;
        while (end < bytes.size() && isStringByte(bytes[end])) ++end;
        if (end - offset >= 4) {
            std::fill(stringBytes.begin() + offset, stringBytes.begin() + end, true);
            size_t terminatorEnd = end;
            while (terminatorEnd < bytes.size() && terminatorEnd - end < 2 &&
                   (bytes[terminatorEnd] == 0 || bytes[terminatorEnd] == '\n' ||
                    bytes[terminatorEnd] == '\r')) {
                stringBytes[terminatorEnd++] = true;
            }
        }
        offset = end == offset ? offset + 1 : end;
    }

    for (size_t offset = bytes.size(); offset > 0; --offset) {
        const size_t current = offset - 1;
        if (stringBytes[current]) continue;
        DecodedInstruction decoded;
        if (!decodeInstruction(bytes, current, decoded)) continue;
        scores[current] = decoded.length;
        if (!isUnconditionalTerminator(bytes[current])) {
            const size_t next = current + decoded.length;
            if (next < bytes.size() && !stringBytes[next]) {
                scores[current] += std::min<size_t>(scores[next], 2048 - decoded.length);
            }
        }
    }
    return scores;
}

size_t printableStringEnd(const std::vector<uint8_t>& bytes, size_t offset)
{
    size_t end = offset;
    while (end < bytes.size() && isStringByte(bytes[end])) ++end;
    return end - offset >= 4 ? end : offset;
}

std::vector<size_t> promoteLinearIslands(
    const std::vector<uint8_t>& bytes,
    std::map<size_t, DecodedInstruction>& instructions,
    std::vector<bool>& codeBytes
)
{
    std::vector<size_t> islandStarts;
    std::vector<bool> stringBytes;
    linearInstructionScores(bytes, stringBytes);
    size_t offset = 0;
    while (offset < bytes.size()) {
        if (codeBytes[offset] || stringBytes[offset]) {
            ++offset;
            continue;
        }
        const size_t stringEnd = printableStringEnd(bytes, offset);
        if (stringEnd != offset) {
            offset = stringEnd;
            continue;
        }

        std::vector<DecodedInstruction> candidate;
        size_t cursor = offset;
        size_t candidateBytes = 0;
        while (cursor < bytes.size() && !codeBytes[cursor] && !stringBytes[cursor] &&
               printableStringEnd(bytes, cursor) == cursor) {
            DecodedInstruction decoded;
            if (!decodeInstruction(bytes, cursor, decoded)) break;
            candidateBytes += decoded.length;
            candidate.push_back(std::move(decoded));
            cursor += candidate.back().length;
            if (isUnconditionalTerminator(bytes[cursor - candidate.back().length])) break;
        }

        const bool trailingReturn = candidate.size() == 1 &&
            candidate.front().mnemonic == "ret" && offset > 0 && codeBytes[offset - 1];
        if ((candidate.size() >= 2 && candidateBytes >= 6) || trailingReturn) {
            islandStarts.push_back(offset);
            for (const DecodedInstruction& decoded : candidate) {
                instructions.emplace(decoded.offset, decoded);
                for (size_t i = decoded.offset; i < decoded.offset + decoded.length; ++i) {
                    codeBytes[i] = true;
                }
            }
            offset = cursor;
        }
        else {
            ++offset;
        }
    }
    return islandStarts;
}

int writeDisassembly(
    const std::vector<uint8_t>& bytes,
    std::ostream& output,
    const std::optional<uint32_t>& requestedBase
)
{
    std::map<size_t, DecodedInstruction> instructions;
    std::vector<bool> codeBytes(bytes.size(), false);
    std::vector<uint32_t> discoveredTargets;
    uint32_t imageBase = requestedBase.value_or(0);
    decodeReachable(bytes, imageBase, instructions, codeBytes, discoveredTargets);

    const std::map<size_t, DecodedInstruction> seedInstructions = instructions;
    const std::vector<uint32_t> seedTargets = discoveredTargets;
    std::vector<uint32_t> uniqueSeedTargets = seedTargets;
    std::sort(uniqueSeedTargets.begin(), uniqueSeedTargets.end());
    uniqueSeedTargets.erase(
        std::unique(uniqueSeedTargets.begin(), uniqueSeedTargets.end()),
        uniqueSeedTargets.end()
    );
    std::vector<uint32_t> inferredBases;
    std::vector<bool> stringBytes;
    const std::vector<size_t> candidateScores = linearInstructionScores(bytes, stringBytes);
    for (uint32_t target : discoveredTargets) {
        if (requestedBase.has_value()) break;
        if (target < bytes.size() && instructions.find(target) != instructions.end()) continue;
        for (const auto& instruction : seedInstructions) {
            if (instruction.first > target) continue;
            const uint32_t base = target - static_cast<uint32_t>(instruction.first);
            if (base != 0 &&
                std::find(inferredBases.begin(), inferredBases.end(), base) == inferredBases.end()) {
                inferredBases.push_back(base);
            }
        }
        std::vector<std::pair<size_t, uint32_t>> candidates;
        for (size_t candidateOffset = 1; candidateOffset < bytes.size(); ++candidateOffset) {
            if (candidateOffset > target || stringBytes[candidateOffset]) continue;
            DecodedInstruction candidate;
            if (!decodeInstruction(bytes, candidateOffset, candidate)) continue;
            const uint32_t base = target - static_cast<uint32_t>(candidateOffset);
            if (base != 0) {
                candidates.emplace_back(candidateScores[candidateOffset], base);
            }
        }
        std::sort(candidates.begin(), candidates.end(),
            [](const auto& left, const auto& right) {
                return left.first > right.first;
            });
        for (size_t i = 0; i < candidates.size() && i < 16; ++i) {
            if (candidates[i].first != 0 &&
                std::find(inferredBases.begin(), inferredBases.end(), candidates[i].second) ==
                    inferredBases.end()) {
                inferredBases.push_back(candidates[i].second);
            }
        }
    }

    size_t bestScore = static_cast<size_t>(
        std::count(codeBytes.begin(), codeBytes.end(), true)
    );
    size_t bestSeedMatches = 0;
    size_t bestLoopMatches = 0;
    size_t bestLoopPenalty = std::numeric_limits<size_t>::max();
    size_t bestTargetMatches = 0;
    size_t bestTargetEvidence = 0;
    for (uint32_t candidateBase : inferredBases) {
        std::map<size_t, DecodedInstruction> candidateInstructions;
        std::vector<bool> candidateCodeBytes(bytes.size(), false);
        std::vector<uint32_t> candidateTargets;
        decodeReachable(
            bytes, candidateBase, candidateInstructions, candidateCodeBytes, candidateTargets
        );
        const size_t candidateScore = static_cast<size_t>(
            std::count(candidateCodeBytes.begin(), candidateCodeBytes.end(), true)
        );
        size_t seedMatches = 0;
        for (uint32_t target : uniqueSeedTargets) {
            if (target < candidateBase ||
                static_cast<uint64_t>(target) - candidateBase >= bytes.size()) {
                continue;
            }
            const size_t targetOffset = target - candidateBase;
            if (seedInstructions.find(targetOffset) != seedInstructions.end()) {
                ++seedMatches;
            }
        }
        size_t targetMatches = 0;
        size_t targetEvidence = 0;
        size_t loopMatches = 0;
        size_t loopPenalty = 0;
        for (const auto& instruction : candidateInstructions) {
            if (!instruction.second.hasTarget) continue;
            const uint32_t target = instruction.second.target;
            if (target < candidateBase ||
                static_cast<uint64_t>(target) - candidateBase >= bytes.size()) {
                continue;
            }
            const size_t targetOffset = target - candidateBase;
            if (candidateInstructions.find(targetOffset) != candidateInstructions.end() &&
                (instruction.second.opcode != INS_LOOP ||
                 targetOffset < instruction.first)) {
                ++targetMatches;
                targetEvidence += candidateScores[targetOffset];
                if (instruction.second.opcode == INS_LOOP) {
                    ++loopMatches;
                    loopPenalty += loopBodyPenalty(
                        candidateInstructions, targetOffset, instruction.first
                    );
                }
            }
        }
        if (loopMatches == 0 && seedMatches < 2) continue;
        if (seedMatches > bestSeedMatches ||
            (seedMatches == bestSeedMatches && loopMatches > bestLoopMatches) ||
            (seedMatches == bestSeedMatches && loopMatches == bestLoopMatches &&
             loopPenalty < bestLoopPenalty) ||
            (seedMatches == bestSeedMatches && loopMatches == bestLoopMatches &&
             loopPenalty == bestLoopPenalty && targetMatches > bestTargetMatches) ||
            (seedMatches == bestSeedMatches && loopMatches == bestLoopMatches &&
             loopPenalty == bestLoopPenalty && targetMatches == bestTargetMatches &&
             targetEvidence > bestTargetEvidence) ||
            (seedMatches == bestSeedMatches && loopMatches == bestLoopMatches &&
             loopPenalty == bestLoopPenalty && targetMatches == bestTargetMatches &&
             targetEvidence == bestTargetEvidence && candidateScore > bestScore)) {
            bestSeedMatches = seedMatches;
            bestLoopMatches = loopMatches;
            bestLoopPenalty = loopPenalty;
            bestTargetMatches = targetMatches;
            bestTargetEvidence = targetEvidence;
            bestScore = candidateScore;
            imageBase = candidateBase;
            instructions = std::move(candidateInstructions);
            codeBytes = std::move(candidateCodeBytes);
            discoveredTargets = std::move(candidateTargets);
        }
    }

    std::vector<size_t> inferredFunctions =
        promoteLinearIslands(bytes, instructions, codeBytes);
    std::vector<uint32_t> islandTargets;
    for (const auto& instruction : instructions) {
        if (instruction.second.hasTarget) islandTargets.push_back(instruction.second.target);
    }
    decodeReachable(
        bytes, imageBase, instructions, codeBytes, discoveredTargets, islandTargets
    );
    for (const auto& instruction : instructions) {
        if (instruction.second.hasTarget) discoveredTargets.push_back(instruction.second.target);
    }

    std::vector<size_t> functionOffsets{0};
    for (size_t functionOffset : inferredFunctions) {
        if (functionOffset != 0) functionOffsets.push_back(functionOffset);
    }
    std::sort(discoveredTargets.begin(), discoveredTargets.end());
    discoveredTargets.erase(
        std::unique(discoveredTargets.begin(), discoveredTargets.end()),
        discoveredTargets.end()
    );
    for (uint32_t target : discoveredTargets) {
        if (target >= imageBase &&
            static_cast<uint64_t>(target) - imageBase < bytes.size()) {
            const size_t targetOffset = target - imageBase;
            if (instructions.find(targetOffset) != instructions.end()) {
                functionOffsets.push_back(targetOffset);
            }
        }
    }
    std::sort(functionOffsets.begin(), functionOffsets.end());
    functionOffsets.erase(
        std::unique(functionOffsets.begin(), functionOffsets.end()),
        functionOffsets.end()
    );
    std::map<size_t, size_t> functionLabels;
    for (size_t i = 0; i < functionOffsets.size(); ++i) {
        functionLabels.emplace(functionOffsets[i], i);
    }

    size_t maxByteColumn = 0;
    for (const auto& entry : instructions) {
        maxByteColumn = std::max(maxByteColumn, entry.second.bytes.size() * 3 - 1);
    }
    std::map<size_t, std::string> labelsAt;
    for (const auto& label : functionLabels) {
        labelsAt[label.first] =
            "<func_" + std::to_string(label.second) + " @0x" +
            addressText(imageBase + static_cast<uint32_t>(label.first)) + ">";
    }

    size_t offset = 0;
    size_t dataIndex = 0;
    while (offset < bytes.size()) {
        const auto label = labelsAt.find(offset);
        if (label != labelsAt.end()) {
            output << label->second << '\n';
        }

        const auto instruction = instructions.find(offset);
        if (instruction != instructions.end()) {
            const DecodedInstruction& decoded = instruction->second;
            std::ostringstream byteColumn;
            for (size_t i = 0; i < decoded.bytes.size(); ++i) {
                if (i != 0) byteColumn << ' ';
                byteColumn << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                           << static_cast<unsigned int>(decoded.bytes[i]);
            }
            const std::string byteText = byteColumn.str();
            output << addressText(imageBase + static_cast<uint32_t>(offset)) << ":\t" << byteText
                   << std::string(maxByteColumn - byteText.size() + 3, ' ')
                   << decoded.mnemonic;
            if (!decoded.operands.empty()) {
                std::string operands = decoded.operands;
                if (decoded.hasTarget) {
                    const size_t targetOffset = decoded.target >= imageBase
                        ? static_cast<size_t>(decoded.target - imageBase) : bytes.size();
                    const auto targetLabel = labelsAt.find(targetOffset);
                    if (targetLabel != labelsAt.end()) {
                        const uint32_t value = decoded.target;
                        const std::string raw = hexValue(static_cast<uint32_t>(value));
                        const size_t position = operands.find(raw);
                        if (position != std::string::npos) {
                            operands.replace(position, raw.size(), targetLabel->second);
                        }
                    }
                }
                output << ' ' << operands;
            }
            output << '\n';
            offset += decoded.length;
            continue;
        }

        const size_t dataBegin = offset;
        while (offset < bytes.size() && !codeBytes[offset]) ++offset;
        if (offset == dataBegin) {
            ++offset;
            continue;
        }
        const auto emitData = [&](size_t begin, size_t end) {
            writeDataSegment(bytes, begin, end, imageBase, dataIndex, output);
        };
        size_t segmentStart = dataBegin;
        size_t cursor = dataBegin;
        while (cursor < offset) {
            if (!isStringByte(bytes[cursor])) {
                ++cursor;
                continue;
            }
            size_t stringEnd = cursor;
            while (stringEnd < offset && isStringByte(bytes[stringEnd])) ++stringEnd;
            if (stringEnd - cursor < 4) {
                cursor = stringEnd;
                continue;
            }
            emitData(segmentStart, cursor);
            size_t dataStringEnd = stringEnd;
            while (dataStringEnd < offset && dataStringEnd - stringEnd < 2 &&
                   (bytes[dataStringEnd] == 0 || bytes[dataStringEnd] == '\n' ||
                    bytes[dataStringEnd] == '\r')) {
                ++dataStringEnd;
            }
            emitData(cursor, dataStringEnd);
            cursor = dataStringEnd;
            segmentStart = cursor;
        }
        emitData(segmentStart, offset);
    }
    return output ? 0 : -1;
}
}

int disassembleBinary(
    const std::string& sourceFile,
    const std::string& destinationFile,
    const std::optional<uint32_t>& imageBase
)
{
    std::ifstream source(sourceFile, std::ios::binary);
    if (!source.is_open()) {
        std::cerr << "Unable to open binary input file: " << sourceFile << '\n';
        return -1;
    }
    std::istreambuf_iterator<char> sourceBegin(source);
    std::istreambuf_iterator<char> sourceEnd;
    std::vector<uint8_t> bytes(sourceBegin, sourceEnd);

    std::ofstream destination(destinationFile);
    if (!destination.is_open()) {
        std::cerr << "Unable to open disassembly output file: " << destinationFile << '\n';
        return -1;
    }
    if (writeDisassembly(bytes, destination, imageBase) != 0 || !destination) {
        std::cerr << "Failed while writing disassembly output file: "
                  << destinationFile << '\n';
        return -1;
    }
    return 0;
}
