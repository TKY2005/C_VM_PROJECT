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

typedef struct {
    const char* signature;
    const char* date;
    uint16_t ver;
    uint8_t arch;
    uint32_t entrypoint;
} file_metadata;

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
    std::string signature = "Made by T.K.Y";
    int siglen = signature.length();
    std::string date = "3/19/2026";
    int datelen = date.length();
    uint16_t version = 0xC100;
};

#endif