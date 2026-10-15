#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include<string>
#include<vector>
#include<cstdint>
#include<limits>

#include<LexicalAnalyzer/Tokenizer.hpp>

#define ERR_FILE_NOT_OPEN -1
#define ERR_ASM_FAIL -2
#define ERR_ASM_WARN -3

#define ASM_SUCCESS 0

class Assembler {
    private:
    Tokenizer tokenizer;

    public:
    static std::string filename;

    Assembler() = default;

    int assembleSource(
        std::string sourcefile,
        std::string destinationfile,
        bool printListing = false,
        uint64_t maximumImageSize = std::numeric_limits<uint64_t>::max()
    );
};

#endif