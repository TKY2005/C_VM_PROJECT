#include<iostream>
#include<string>
#include<vector>
#include<cctype>
#include<cstdint>
#include<limits>

#include<Assembler.hpp>

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
}

int main(int argc, char** argv)
{
    bool printListing = false;
    uint64_t maximumImageSize = std::numeric_limits<uint64_t>::max();
    std::vector<std::string> positionalArguments;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--list" || argument == "--listing" || argument == "-l") {
            printListing = true;
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
        else if (!argument.empty() && argument[0] == '-') {
            std::cerr << "Unknown option: " << argument << '\n'
                      << "Usage: tkyc [--list] [--max size] source.asm [output.tky]\n";
            return ERR_ASM_FAIL;
        }
        else positionalArguments.push_back(argument);
    }

    if (positionalArguments.empty() || positionalArguments.size() > 2) {
        std::cerr << "Usage: tkyc [--list] [--max size] source.asm [output.tky]\n";
        return ERR_ASM_FAIL;
    }

    const std::string& sourceFileName = positionalArguments[0];
    std::string outputFileName = positionalArguments.size() == 2
        ? positionalArguments[1] : "./out.tky";
    Assembler assembler;
    return assembler.assembleSource(
        sourceFileName, outputFileName, printListing, maximumImageSize
    );
}
