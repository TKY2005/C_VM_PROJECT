#ifndef PARSER_HPP
#define PARSER_HPP

#include<map>
#include<vector>
#include<cstdint>
#include<iomanip>

#include<LexicalAnalyzer/Tokenizer.hpp>
#include<utils/strbuilder/strbuilder.h>
#include<ISA_encoding_info.h>

#define MODE_BYTE 1
#define MODE_WORD 2
#define MODE_DWORD 4

class ProgLine {
    public:
    std::vector<Token> tokens;
    
    ProgLine(std::vector<Token> tokens) {
        this->tokens = tokens;
    }

    ProgLine() : tokens() {

    }
};

class ProgramInstruction {
    public:
    std::vector<Token> parts;
    int len;
    int operand_count;
    int immediateSize = 0;
    bool hasOperands = false;
    bool hasImmediate = false;
    bool hasDisplacement = false;
    bool hasMemory = false;
    bool hasReg = false;
    bool hasSymbol = false;
    std::string symbol;

    ins_encoding* encoding_info;
    std::vector<std::vector<Token>> operands;
    uint32_t addr;

    ProgramInstruction(std::vector<Token> parts, int len, uint32_t addr) {
        this->parts = parts;
        this->len = len;
        this->addr = addr;
        operand_count = 0;
        encoding_info = (ins_encoding*) malloc(sizeof(ins_encoding));
        setDefaultValues();
    }
    ProgramInstruction(std::vector<Token> parts) {
        this->parts = parts;
        operand_count = 0;
        len = 0;
        addr = 0;
        encoding_info = (ins_encoding*) malloc(sizeof(ins_encoding));
        setDefaultValues();
    }

    std::string toStringDebug() {
        std::string insstr;
        for(int i = 0; i < parts.size(); i++) insstr.append(parts[i].tokenstr).append(" ");
        strbuilder sb = strbuilder_init();

        strbuilder_append(&sb,  (char*)"[INSTRUCTION]\n");
        strbuilder_appendf(&sb, (char*) "   instruction string: %s\n", insstr.c_str());
        strbuilder_appendf(&sb, (char*) "   opcode: 0x%02X -> %d\n", encoding_info->opcode, encoding_info->opcode);
        strbuilder_appendf(&sb, (char*) "   operand count: %d\n", operand_count);
        strbuilder_appendf(&sb, (char*) "   length: %d\n", len);
        strbuilder_appendf(&sb, (char*) "   address: 0x%08X\n", addr);
        strbuilder_appendf(&sb, (char*) "   operand_types: %02X\n", encoding_info->operand_types);
        strbuilder_appendf(&sb, (char*) "   register select: %02X\n", encoding_info->register_select);
        strbuilder_appendf(&sb, (char*) "   displacement info: %02X\n", encoding_info->displacement_info);
        strbuilder_appendf(&sb, (char*) "   displacement_value: %02X\n", encoding_info->disp_val);
        strbuilder_appendf(&sb, (char*) "   immediate value: %02X\n", encoding_info->imm_val);
        strbuilder_append(&sb,  (char*) "[END INSTRUCTION]");

        return std::string(strbuilder_getstr(&sb));
    }
    std::string toString() {
        std::string insstr;
        for(int i = 0; i < parts.size(); i++) insstr.append(parts[i].tokenstr).append(" ");
        return insstr;
    }
    private:
    void setDefaultValues() {
        encoding_info->operand_types = OPERTYPE_DEFAULT;
        encoding_info->register_select = REGSELECT_DEFAULT;
        encoding_info->displacement_info = DISP_DEFAULT;
        encoding_info->imm_val = 0;
        encoding_info->disp_val = 0;
    }

    ~ProgramInstruction() = default;
};
class ProgramData {
    public:
    std::vector<Token> parts;
    int len;
    int data_store_mode;
    bool isConst = false;
    bool isRes = false;
    uint32_t constval = 0;
    uint32_t addr;

    ProgramData(std::vector<Token> parts, int len, int data_mode, uint32_t addr) {
        this->parts = parts;
        this->len = len;
        this->data_store_mode = data_mode;
        this->addr = addr;
    }
    ProgramData(std::vector<Token> parts) {
        this->parts = parts;
    }
    ProgramData(std::vector<Token> parts, int len, uint32_t addr, uint32_t constval) {
        this->parts = parts;
        this->len = len;
        this->addr = addr;
        this->constval = constval;
        isConst = true;
    }

    std::string toStringDebug() {
        std::string dstr;
        for(int i = 0; i < parts.size(); i++) dstr.append(parts[i].tokenstr).append(" ");

        strbuilder sb = strbuilder_init();
        strbuilder_append(&sb,  (char*)"[DATA ENTRY]\n");
        strbuilder_appendf(&sb, (char*) "   data: %s\n", dstr.c_str());
        strbuilder_appendf(&sb, (char*) "   length: %d\n", len);
        strbuilder_appendf(&sb, (char*) "   store mode: %s\n", (data_store_mode == 1) ? "byte" : "word");
        strbuilder_appendf(&sb, (char*) "   value: 0x%08X -> %d\n", constval, constval);
        strbuilder_appendf(&sb, (char*) "   address: 0x%08X\n", addr);
        strbuilder_append(&sb,  (char*) "[END DATA]");

        return std::string(strbuilder_getstr(&sb));
    }
};

class ParseResult {

    public:
    std::vector<Token> tokens;
    std::map<std::string, uint32_t> symmap;
    std::map<std::string, uint32_t> sections;
    std::vector<ProgramInstruction&> program_instructions;
    std::vector<ProgramData&> program_data;

    uint32_t code_section_addr = 0;
    uint32_t data_section_addr = 0;
};

#endif