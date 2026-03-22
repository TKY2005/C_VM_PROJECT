#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include<cstdint>
#include<fstream>
#include<vector>
#include<map>

#include "../Parser/Parser.hpp"

typedef struct {
    uint8_t size;
    uint8_t* bindata;
} bin_form;

class CodeGenerator {

    public:
    std::ofstream makeBinaryFile(std::string outputFilePath, ParseResult* parseResult);
    bin_form* instructionToByteCode(ProgIns* ins);
    bin_form* instructionToByteCode(ProgIns* ins, std::map<std::string, uint32_t> symmap);
    bin_form* dataToBin(ProgData* data);
    //bin_form* dataToBin(ProgData* data, std::map<std::string, uint32_t> symmap);

    std::vector<uint8_t> dwordToBytes(uint32_t val);
    std::vector<uint8_t> wordToBytes(uint16_t val);

    CodeGenerator() = default;

    private: 
    std::string signature = "Made by T.K.Y\0";
    int siglen = signature.length() + 1;
    std::string date = "3/22/2026\0";
    int datelen = date.length() + 1;
    const uint32_t version = 0xC1000001;
};

#endif