#include<any>
#include<algorithm>
#include<cstdint>
#include<limits>
#include<stdexcept>
#include<string>
#include<vector>

#include<ArchInfo.hpp>
#include<ErrorHandler/ErrorHandler.hpp>
#include<Parser/Parser.hpp>
#include<Visitors/Visitors.hpp>
#include<Parser/Unicode.hpp>

namespace {
struct EncodedOperand {
    uint8_t type = UNKNOWN;
    uint8_t registerCode = REG_UNSELECTED;
    uint32_t value = 0;
    bool isMemory = false;
    bool displacementEnabled = false;
    uint8_t base = BASE_UNSELECTED;
    uint8_t index = INDEX_UNSELECTED;
    uint8_t scale = 1;
};

std::string normalizedName(std::string name)
{
    if (!name.empty() && name[0] == '.') name.erase(0, 1);
    return name;
}

uint8_t memoryType(uint8_t size)
{
    if (size == 1) return MEM8;
    if (size == 2) return MEM16;
    if (size == 4) return MEM32;
    throw std::runtime_error("unsupported memory operand size");
}

uint8_t registerType(SubType size)
{
    if (size == SubType::REG8) return REG8;
    if (size == SubType::REG16) return REG16;
    if (size == SubType::REG32) return REG32;
    throw std::runtime_error("unsupported register size");
}

uint8_t immediateSize(uint8_t destinationType)
{
    if (destinationType == REG8 || destinationType == MEM8) return 1;
    if (destinationType == REG16 || destinationType == MEM16) return 2;
    return 4;
}
}

std::any CodeGenerationVisitor::generate(ParseResult& result, AsmContext& c)
{
    c.generatedBytes.clear();
    c.generatedBaseSet = false;
    c.generatedBaseAddress = 0;
    addressIndices.clear();
    for (const auto& node : result.parsedLines) {
        if (!node) throw std::runtime_error("parser produced an empty node");
        try {
            node->accept(*this, c);
        }
        catch (const std::exception& error) {
            if (!ErrorBucket::hasErrorAt(c.sourceFile, node->location.row)) {
                ErrorBucket::addError({}, node->location, c.sourceFile, error.what(), node->sourceLine);
            }
        }
    }
    return c.generatedBytes;
}

uint32_t CodeGenerationVisitor::nextAddress(ParseObject& a, AsmContext& c)
{
    auto addresses = c.nodeAddresses.find(&a);
    if (addresses == c.nodeAddresses.end()) {
        throw std::runtime_error("layout stage did not record a node address");
    }
    size_t& index = addressIndices[&a];
    if (index >= addresses->second.size()) {
        throw std::runtime_error("code generation visited a node more times than layout");
    }
    return addresses->second[index++];
}

uint32_t CodeGenerationVisitor::nextLength(ParseObject& a, AsmContext& c)
{
    auto lengths = c.nodeLengths.find(&a);
    if (lengths == c.nodeLengths.end()) {
        throw std::runtime_error("layout stage did not record a node length");
    }
    size_t index = addressIndices[&a] - 1;
    if (index >= lengths->second.size()) {
        throw std::runtime_error("layout stage did not record this node occurrence");
    }
    return lengths->second[index];
}

void CodeGenerationVisitor::ensureOutputRange(uint32_t address, uint32_t length, AsmContext& c)
{
    if (length > std::numeric_limits<uint32_t>::max() - address) {
        throw std::runtime_error("generated output exceeds the 32-bit address space");
    }
    if (!c.generatedBaseSet) {
        c.generatedBaseAddress = address;
        c.generatedBaseSet = true;
    }
    if (address < c.generatedBaseAddress) {
        const uint64_t prefix = static_cast<uint64_t>(c.generatedBaseAddress) - address;
        if (prefix + c.generatedBytes.size() > c.generatedBytes.max_size()) {
            throw std::runtime_error("generated output exceeds the available address space");
        }
        c.generatedBytes.insert(c.generatedBytes.begin(), static_cast<size_t>(prefix), 0);
        c.generatedBaseAddress = address;
    }

    const uint64_t end = static_cast<uint64_t>(address) + length;
    const uint64_t offset = static_cast<uint64_t>(address) - c.generatedBaseAddress;
    const uint64_t outputEnd = offset + length;
    if (end > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1 ||
        outputEnd > c.generatedBytes.max_size()) {
        throw std::runtime_error("generated output exceeds the available address space");
    }
    if (c.generatedBytes.size() < outputEnd) {
        c.generatedBytes.resize(static_cast<size_t>(outputEnd), 0);
    }
}

void CodeGenerationVisitor::emitByte(uint32_t& address, uint8_t value, AsmContext& c)
{
    ensureOutputRange(address, 1, c);
    const size_t offset = static_cast<size_t>(address - c.generatedBaseAddress);
    c.generatedBytes[offset] = value;
    ++address;
}

void CodeGenerationVisitor::emitValue(uint32_t& address, uint32_t value, uint8_t size, AsmContext& c)
{
    if (size != 1 && size != 2 && size != 4) {
        throw std::runtime_error("cannot emit a value with an unsupported size");
    }
    for (int shift = (size - 1) * 8; shift >= 0; shift -= 8) {
        emitByte(address, static_cast<uint8_t>(value >> shift), c);
    }
}

uint8_t CodeGenerationVisitor::valueSize(uint8_t operandType)
{
    return immediateSize(operandType);
}

std::any CodeGenerationVisitor::visit(Binary& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any CodeGenerationVisitor::visit(Unary& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any CodeGenerationVisitor::visit(Grouping& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any CodeGenerationVisitor::visit(Number& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any CodeGenerationVisitor::visit(Special& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any CodeGenerationVisitor::visit(StringNode& a, AsmContext& c)
{
    return a.str;
}

std::any CodeGenerationVisitor::visit(Symbol& a, AsmContext& c)
{
    Symbol normalized(normalizedName(a.name));
    return evaluator.eval(normalized, c);
}

std::any CodeGenerationVisitor::visit(DataNode& a, AsmContext& c)
{
    uint32_t cursor = nextAddress(a, c);
    const uint32_t start = cursor;
    uint32_t length = nextLength(a, c);
    for (const auto& datum : a.data) {
        if (StringNode* string = dynamic_cast<StringNode*>(datum.get())) {
            for (uint32_t character : decodeUtf8(string->str)) {
                emitValue(cursor, character, a.size, c);
            }
        }
        else {
            c.program_counter = cursor;
            c.section_offset = cursor - c.section_begin;
            uint32_t value = std::any_cast<uint32_t>(evaluator.eval(*datum, c));
            emitValue(cursor, value, a.size, c);
        }
    }
    if (cursor - start != length) {
        throw std::runtime_error("generated data length differs from the calculated layout");
    }
    return length;
}

std::any CodeGenerationVisitor::visit(ResNode& a, AsmContext& c)
{
    uint32_t start = nextAddress(a, c);
    uint32_t length = nextLength(a, c);
    ensureOutputRange(start, length, c);
    const size_t offset = static_cast<size_t>(start - c.generatedBaseAddress);
    std::fill(
        c.generatedBytes.begin() + offset,
        c.generatedBytes.begin() + offset + length,
        0
    );
    return length;
}

std::any CodeGenerationVisitor::visit(DirSection& a, AsmContext& c)
{
    nextAddress(a, c);
    nextLength(a, c);
    return true;
}

std::any CodeGenerationVisitor::visit(DirORG& a, AsmContext& c)
{
    nextAddress(a, c);
    nextLength(a, c);
    return a.val;
}

std::any CodeGenerationVisitor::visit(DirTimes& a, AsmContext& c)
{
    nextAddress(a, c);
    nextLength(a, c);
    for (uint32_t i = 0; i < a.exprVal; ++i) a.repeated->accept(*this, c);
    return a.exprVal;
}

std::any CodeGenerationVisitor::visit(Instruction& a, AsmContext& c)
{
    uint32_t cursor = nextAddress(a, c);
    uint32_t expectedLength = nextLength(a, c);
    const uint32_t start = cursor;
    c.program_counter = start;
    c.section_offset = start - c.section_begin;

    std::vector<EncodedOperand> operands;
    operands.reserve(a.operands.size());
    size_t immediateIndex = a.operands.size();
    size_t symbolIndex = a.operands.size();

    for (size_t i = 0; i < a.operands.size(); ++i) {
        ParseObject& operand = *a.operands[i];
        EncodedOperand encoded;

        if (Register* reg = dynamic_cast<Register*>(&operand)) {
            encoded.type = registerType(reg->size);
            encoded.registerCode = reg->regcode & 0x0f;
            if (reg->size == SubType::REG8 && !reg->name.empty() && reg->name.back() == 'h') {
                encoded.registerCode |= LH_CHECK;
            }
        }
        else if (MemExpr* memory = dynamic_cast<MemExpr*>(&operand)) {
            a.location = operand.location;
            encoded.type = memoryType(memory->size);
            encoded.isMemory = true;
            encoded.displacementEnabled = memory->displacementEnabled;
            encoded.scale = memory->scale;
            if (memory->baseReg) {
                encoded.base = ArchInfo::base_select_map.at(memory->baseReg->name);
            }
            if (memory->indexReg) {
                encoded.index = ArchInfo::index_select_map.at(memory->indexReg->name);
            }
            encoded.value = std::any_cast<uint32_t>(evaluator.evalMemory(*memory->memexpr, c));
        }
        else if (Symbol* symbol = dynamic_cast<Symbol*>(&operand)) {
            a.location = operand.location;
            const std::string name = normalizedName(symbol->name);
            Symbol normalized(name);
            encoded.value = std::any_cast<uint32_t>(evaluator.eval(normalized, c));
            encoded.type = UNKNOWN;
            symbolIndex = i;
        }
        else {
            a.location = operand.location;
            encoded.type = NUMBER;
            encoded.value = std::any_cast<uint32_t>(evaluator.eval(operand, c));
            immediateIndex = i;
        }
        operands.push_back(encoded);
    }

    size_t immediateCount = 0;
    for (const EncodedOperand& operand : operands) {
        if (operand.type == NUMBER) ++immediateCount;
    }
    if (immediateCount > 1) {
        throw std::runtime_error("instruction encoding supports only one immediate value");
    }

    if (operands.empty()) {
        emitByte(cursor, a.opcode, c);
    }
    else {
        EncodedOperand dest;
        EncodedOperand src;
        if (operands.size() == 1) dest = operands[0];
        else {
            dest = operands[0];
            src = operands[1];
        }

        uint8_t scaleMode = NOMEM;
        const EncodedOperand* memoryOperand = nullptr;
        bool hasRegister = false;
        for (const EncodedOperand& operand : operands) {
            if (operand.isMemory) {
                memoryOperand = &operand;
                if (operand.scale == 1) scaleMode = X1SCALE;
                else if (operand.scale == 2) scaleMode = X2SCALE;
                else if (operand.scale == 4) scaleMode = X4SCALE;
                else throw std::runtime_error("unsupported memory index scale");
            }
            if (operand.type == REG8 || operand.type == REG16 || operand.type == REG32) {
                hasRegister = true;
            }
        }

        const uint8_t operandTypes = static_cast<uint8_t>(
            ((src.type & 0x07) << 5) |
            ((dest.type & 0x07) << 2) |
            (scaleMode & 0x03)
        );
        emitByte(cursor, a.opcode, c);
        emitByte(cursor, operandTypes, c);

        if (hasRegister) {
            uint8_t registerSelection = REGSELECT_DEFAULT;
            if (dest.type == REG8 || dest.type == REG16 || dest.type == REG32) {
                registerSelection = static_cast<uint8_t>(
                    (registerSelection & 0xf0) | (dest.registerCode & 0x0f)
                );
            }
            if (src.type == REG8 || src.type == REG16 || src.type == REG32) {
                registerSelection = static_cast<uint8_t>(
                    (registerSelection & 0x0f) | ((src.registerCode & 0x0f) << 4)
                );
            }
            emitByte(cursor, registerSelection, c);
        }

        if (memoryOperand) {
            const bool hasAddressRegister =
                memoryOperand->base != BASE_UNSELECTED ||
                memoryOperand->index != INDEX_UNSELECTED;
            uint8_t displacementInfo = static_cast<uint8_t>(
                (memoryOperand->base & 0x07) |
                ((memoryOperand->index & 0x07) << 3) |
                (memoryOperand->displacementEnabled ? 0x40 : 0) |
                (hasAddressRegister ? 0x80 : 0)
            );
            emitByte(cursor, displacementInfo, c);
            if (memoryOperand->displacementEnabled) {
                emitValue(cursor, memoryOperand->value, 4, c);
            }
        }

        if (immediateIndex < operands.size()) {
            const uint8_t size = operands.size() == 1 ? 4 : valueSize(dest.type);
            emitValue(cursor, operands[immediateIndex].value, size, c);
        }
        if (symbolIndex < operands.size()) {
            emitValue(cursor, operands[symbolIndex].value, 4, c);
        }
    }

    if (cursor - start != expectedLength) {
        throw std::runtime_error("generated instruction length differs from the calculated layout");
    }
    return expectedLength;
}

std::any CodeGenerationVisitor::visit(MemExpr& a, AsmContext& c)
{
    return evaluator.evalMemory(*a.memexpr, c);
}

std::any CodeGenerationVisitor::visit(Register& a, AsmContext& c)
{
    return a.name;
}

std::any CodeGenerationVisitor::visit(Declaration& a, AsmContext& c)
{
    nextAddress(a, c);
    nextLength(a, c);
    return true;
}

std::any CodeGenerationVisitor::visit(EquNode& a, AsmContext& c)
{
    nextAddress(a, c);
    nextLength(a, c);
    return a.value;
}
