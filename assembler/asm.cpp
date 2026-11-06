#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<cctype>
#include<cstdint>
#include<limits>
#include<optional>
#include<sstream>
#include<iterator>

#include<Assembler.hpp>
#include<Disassembler.hpp>

namespace {
bool parseMaximumSize(const std::string& argument, uint64_t& size)
{
    if (argument.empty()) return false;
    size_t digitCount = argument.size();
    uint64_t multiplier = 1;
    const char suffix = static_cast<char>(
        std::tolower(static_cast<unsigned char>(argument.back()))
    );
    if (!std::isdigit(static_cast<unsigned char>(suffix))) {
        --digitCount;
        switch (suffix) {
            case 'b': multiplier = 1; break;
            case 'k': multiplier = 1024ull; break;
            case 'm': multiplier = 1024ull * 1024ull; break;
            case 'g': multiplier = 1024ull * 1024ull * 1024ull; break;
            default: return false;
        }
    }
    if (digitCount == 0) return false;
    for (size_t i = 0; i < digitCount; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(argument[i]))) return false;
    }

    try {
        const uint64_t value = std::stoull(argument.substr(0, digitCount));
        if (value > std::numeric_limits<uint64_t>::max() / multiplier) return false;
        size = value * multiplier;
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool parseImageBase(const std::string& argument, uint32_t& base)
{
    if (argument.empty() || argument[0] == '-') return false;
    try {
        size_t parsedLength = 0;
        const unsigned long long value = std::stoull(argument, &parsedLength, 0);
        if (parsedLength != argument.size() ||
            value > std::numeric_limits<uint32_t>::max()) {
            return false;
        }
        base = static_cast<uint32_t>(value);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::string trim(const std::string& value)
{
    const size_t begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return "";
    const size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

bool parseUnsigned32(const std::string& argument, uint32_t& value)
{
    if (argument.empty() || argument[0] == '-' || argument[0] == '+') return false;

    int base = 10;
    size_t digitStart = 0;
    if (argument.size() >= 2 && argument[0] == '0') {
        const char prefix = static_cast<char>(
            std::tolower(static_cast<unsigned char>(argument[1]))
        );
        if (prefix == 'x' || prefix == 'b' || prefix == 'o') {
            base = prefix == 'x' ? 16 : prefix == 'b' ? 2 : 8;
            digitStart = 2;
        }
        else if (argument.size() > 1) {
            base = 8;
            digitStart = 1;
        }
    }
    if (digitStart == argument.size()) return false;

    try {
        size_t parsedLength = 0;
        const unsigned long long parsed =
            std::stoull(argument.substr(digitStart), &parsedLength, base);
        if (parsedLength != argument.size() - digitStart ||
            parsed > std::numeric_limits<uint32_t>::max()) {
            return false;
        }
        value = static_cast<uint32_t>(parsed);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool parseSymbolEntry(const std::string& entry, std::map<std::string, uint32_t>& symbols)
{
    const size_t colon = entry.find(':');
    if (entry.empty() || colon == std::string::npos ||
        entry.find(':', colon + 1) != std::string::npos) {
        return false;
    }

    const std::string name = trim(entry.substr(0, colon));
    if (name.empty() ||
        !(std::isalpha(static_cast<unsigned char>(name[0])) || name[0] == '_')) {
        return false;
    }
    for (char character : name) {
        if (!(std::isalnum(static_cast<unsigned char>(character)) ||
              character == '_')) {
            return false;
        }
    }

    uint32_t value = 0;
    return parseUnsigned32(trim(entry.substr(colon + 1)), value) &&
        symbols.emplace(name, value).second;
}

bool decodeSymbolFile(const std::string& bytes, std::string& text)
{
    // Symbol files are often edited in Windows tools that save UTF-16 by
    // default. Decode the BOM-marked encodings here so the normal line parser
    // sees the same ASCII definitions regardless of the host platform.
    if (bytes.size() >= 2 &&
        ((static_cast<unsigned char>(bytes[0]) == 0xFF &&
          static_cast<unsigned char>(bytes[1]) == 0xFE) ||
         (static_cast<unsigned char>(bytes[0]) == 0xFE &&
          static_cast<unsigned char>(bytes[1]) == 0xFF))) {
        const bool littleEndian = static_cast<unsigned char>(bytes[0]) == 0xFF;
        if ((bytes.size() - 2) % 2 != 0) return false;

        for (size_t i = 2; i < bytes.size(); i += 2) {
            const unsigned char first = static_cast<unsigned char>(bytes[i]);
            const unsigned char second = static_cast<unsigned char>(bytes[i + 1]);
            const uint16_t codeUnit = littleEndian
                ? static_cast<uint16_t>(first | (second << 8))
                : static_cast<uint16_t>((first << 8) | second);
            if (codeUnit > 0x7F) return false;
            text.push_back(static_cast<char>(codeUnit));
        }
        return true;
    }

    const size_t utf8BomLength = bytes.size() >= 3 &&
        static_cast<unsigned char>(bytes[0]) == 0xEF &&
        static_cast<unsigned char>(bytes[1]) == 0xBB &&
        static_cast<unsigned char>(bytes[2]) == 0xBF ? 3 : 0;
    text.assign(bytes.begin() + utf8BomLength, bytes.end());
    return true;
}

bool parseSymbolFile(
    const std::string& path,
    std::map<std::string, uint32_t>& symbols,
    std::string& error
)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        error = "Unable to open symbol file: " + path;
        return false;
    }

    const std::string bytes(
        (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()
    );
    if (file.bad()) {
        error = "Failed while reading symbol file: " + path;
        return false;
    }

    std::string contents;
    if (!decodeSymbolFile(bytes, contents)) {
        error = "Unsupported text encoding in symbol file: " + path;
        return false;
    }

    std::istringstream lines(contents);
    std::string line;
    size_t lineNumber = 0;
    while (std::getline(lines, line)) {
        ++lineNumber;
        const std::string entry = trim(line);
        if (entry.empty()) continue;
        if (!parseSymbolEntry(entry, symbols)) {
            error = "Invalid or duplicate symbol definition in " + path + ":" +
                std::to_string(lineNumber);
            return false;
        }
    }
    return true;
}

bool parseSymbols(
    const std::string& argument,
    std::map<std::string, uint32_t>& symbols,
    std::string& error
)
{
    const std::string input = trim(argument);
    if (input.size() < 2 || input.front() != '{' || input.back() != '}') {
        error = "Expected symbol definitions enclosed in braces.";
        return false;
    }

    const std::string entries = input.substr(1, input.size() - 2);
    size_t position = 0;
    bool hasEntry = false;
    while (position < entries.size()) {
        const size_t comma = entries.find(',', position);
        const std::string entry = trim(entries.substr(
            position, comma == std::string::npos ? std::string::npos : comma - position
        ));
        if (entry.empty()) {
            error = "Empty entry in symbol definitions.";
            return false;
        }

        hasEntry = true;
        if (entry.find(':') != std::string::npos) {
            if (!parseSymbolEntry(entry, symbols)) {
                error = "Invalid or duplicate symbol definition: " + entry;
                return false;
            }
        }
        else if (!parseSymbolFile(entry, symbols, error)) {
            return false;
        }

        if (comma == std::string::npos) break;
        position = comma + 1;
        if (position == entries.size()) {
            error = "Empty entry in symbol definitions.";
            return false;
        }
    }
    if (!hasEntry) {
        error = "At least one symbol or symbol file is required.";
        return false;
    }
    return true;
}
}

int main(int argc, char** argv)
{
    bool printListing = false;
    bool printSymbols = false;
    bool disassemble = false;
    std::optional<uint32_t> imageBase;
    uint64_t maximumImageSize = std::numeric_limits<uint64_t>::max();
    std::map<std::string, uint32_t> symbols;
    std::vector<std::string> positionalArguments;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--list" || argument == "--listing" || argument == "-l") {
            printListing = true;
            if (i + 1 < argc && std::string(argv[i + 1]) == "s") {
                printSymbols = true;
                ++i;
            }
        }
        else if (argument == "-ls") {
            printListing = true;
            printSymbols = true;
        }
        else if (argument == "-d" || argument == "--disassemble") {
            disassemble = true;
        }
        else if (argument == "--base") {
            uint32_t base = 0;
            if (i + 1 >= argc || !parseImageBase(argv[++i], base)) {
                std::cerr << "Invalid image base. Use a 32-bit decimal or hexadecimal address.\n"
                          << "Usage: tkyc -d [--base address] input.tky output.txt\n";
                return ERR_ASM_FAIL;
            }
            imageBase = base;
        }
        else if (argument.rfind("--base=", 0) == 0) {
            uint32_t base = 0;
            if (!parseImageBase(argument.substr(7), base)) {
                std::cerr << "Invalid image base. Use a 32-bit decimal or hexadecimal address.\n"
                          << "Usage: tkyc -d [--base address] input.tky output.txt\n";
                return ERR_ASM_FAIL;
            }
            imageBase = base;
        }
        else if (argument == "--max" || argument == "-m") {
            if (i + 1 >= argc || !parseMaximumSize(argv[++i], maximumImageSize)) {
                std::cerr << "Invalid maximum image size. Use a non-negative integer "
                             "with optional b, k, m, or g suffix.\n"
                          << "Usage: tkyc [--list] [--max size] source.asm [output.tky]\n";
                return ERR_ASM_FAIL;
            }
        }
        else if (argument.rfind("--max=", 0) == 0) {
            if (!parseMaximumSize(argument.substr(6), maximumImageSize)) {
                std::cerr << "Invalid maximum image size. Use a non-negative integer "
                             "with optional b, k, m, or g suffix.\n"
                          << "Usage: tkyc [--list] [--max size] source.asm [output.tky]\n";
                return ERR_ASM_FAIL;
            }
        }
        else if (argument == "-D" || argument == "--define") {
            std::string symbolArgument;
            if (i + 1 < argc) {
                symbolArgument = argv[++i];
                while (symbolArgument.find('}') == std::string::npos && i + 1 < argc) {
                    symbolArgument += ' ';
                    symbolArgument += argv[++i];
                }
            }
            std::string error;
            if (!parseSymbols(symbolArgument, symbols, error)) {
                std::cerr << error << '\n'
                          << "Use -D or --define "
                             "\"{symbols-file, symbol: value}\" with unsigned "
                             "32-bit values.\n";
                return ERR_ASM_FAIL;
            }
        }
        else if (!argument.empty() && argument[0] == '-') {
            std::cerr << "Unknown option: " << argument << '\n'
                      << "Usage: tkyc [--list] [--max size] "
                         "[-D|--define \"{symbol: value}\"] source.asm [output.tky]\n"
                      << "       tkyc -d [--base address] input.tky output.txt\n";
            return ERR_ASM_FAIL;
        }
        else positionalArguments.push_back(argument);
    }

    if (positionalArguments.empty() || positionalArguments.size() > 2) {
        std::cerr << "Usage: tkyc [--list] [--max size] "
                     "[-D|--define \"{symbol: value}\"] source.asm [output.tky]\n"
                  << "       tkyc -d [--base address] input.tky output.txt\n";
        return ERR_ASM_FAIL;
    }

    if (disassemble) {
        if (positionalArguments.size() != 2 || printListing ||
            maximumImageSize != std::numeric_limits<uint64_t>::max() ||
            !symbols.empty()) {
            std::cerr << "Usage: tkyc -d [--base address] input.tky output.txt\n";
            return ERR_ASM_FAIL;
        }
        return disassembleBinary(
            positionalArguments[0], positionalArguments[1], imageBase
        );
    }
    if (imageBase.has_value()) {
        std::cerr << "--base is only valid with -d.\n"
                  << "Usage: tkyc -d [--base address] input.tky output.txt\n";
        return ERR_ASM_FAIL;
    }

    const std::string& sourceFileName = positionalArguments[0];
    std::string outputFileName = positionalArguments.size() == 2
        ? positionalArguments[1] : "./out.tky";
    Assembler assembler;
    return assembler.assembleSource(
        sourceFileName, outputFileName, printListing, maximumImageSize, symbols,
        printSymbols
    );
}
