#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<cctype>
#include<cstdint>
#include<limits>
#include<optional>

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

bool parseSymbols(
    const std::string& argument,
    std::map<std::string, uint32_t>& symbols
)
{
    const std::string input = trim(argument);
    if (input.size() < 2 || input.front() != '{' || input.back() != '}') return false;
    const std::string entries = input.substr(1, input.size() - 2);
    size_t position = 0;
    while (position < entries.size()) {
        const size_t comma = entries.find(',', position);
        const std::string entry = trim(entries.substr(
            position, comma == std::string::npos ? std::string::npos : comma - position
        ));
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
        if (!parseUnsigned32(trim(entry.substr(colon + 1)), value) ||
            !symbols.emplace(name, value).second) {
            return false;
        }

        if (comma == std::string::npos) break;
        position = comma + 1;
        if (position == entries.size()) return false;
    }
    return !symbols.empty();
}
}

int main(int argc, char** argv)
{
    bool printListing = false;
    bool disassemble = false;
    std::optional<uint32_t> imageBase;
    uint64_t maximumImageSize = std::numeric_limits<uint64_t>::max();
    std::map<std::string, uint32_t> symbols;
    std::vector<std::string> positionalArguments;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--list" || argument == "--listing" || argument == "-l") {
            printListing = true;
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
            if (!parseSymbols(symbolArgument, symbols)) {
                std::cerr << "Invalid symbol definitions. Use -D or --define "
                             "\"{symbol: value, symbol2: value2}\" with unsigned "
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
        sourceFileName, outputFileName, printListing, maximumImageSize, symbols
    );
}
