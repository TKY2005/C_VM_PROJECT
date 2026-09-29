#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include<string>
#include<vector>

#include<LexicalAnalyzer/Tokenizer.hpp>
#include<Parser/Parser.hpp>

#define ERR_FILE_NOT_OPEN -1
#define ERR_ASM_FAIL -2
#define ERR_ASM_WARN -3

#define ASM_SUCCESS 0

class Assembler {
    private:
    Tokenizer tokenizer;
    Parser parser;

    public:
    static std::string filename;

    Assembler() = default;

    int assembleSource(std::string sourcefile, std::string destinationfile);
};

#endif