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

class ProgIns {
    public:
    std::vector<Token> parts;
    int len;
    int operand_count;
    int immediateSize = 0;
    bool hasOperands = false;
    bool hasImmediate = false;
    bool hasDisplacement = false;
    bool hasMemory = false;
    bool hasSymbol = false;
    std::string symbol;

    ins_encoding* encoding_info;
    std::vector<std::vector<Token>> operands;
    uint32_t addr;

    ProgIns(std::vector<Token> parts, int len, uint32_t addr) {
        this->parts = parts;
        this->len = len;
        this->addr = addr;
        operand_count = 0;
        encoding_info = (ins_encoding*) malloc(sizeof(ins_encoding));
        setDefaultValues();
    }
    ProgIns(std::vector<Token> parts) {
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
};
class ProgData {
    public:
    std::vector<Token> parts;
    int len;
    int data_store_mode;
    bool isConst = false;
    bool isRes = false;
    uint32_t constval = 0;
    uint32_t addr;

    ProgData(std::vector<Token> parts, int len, int data_mode, uint32_t addr) {
        this->parts = parts;
        this->len = len;
        this->data_store_mode = data_mode;
        this->addr = addr;
    }
    ProgData(std::vector<Token> parts) {
        this->parts = parts;
    }
    ProgData(std::vector<Token> parts, int len, uint32_t addr, uint32_t constval) {
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
    std::vector<ProgIns&> program_instructions;
    std::vector<ProgData&> program_data;

    uint32_t code_section_addr = 0;
    uint32_t data_section_addr = 0;
};

class Parser {
    public:
    virtual ParseResult* parseTokens(std::vector<Token> tokens);
    virtual ProgIns& parseInstruction(std::vector<Token> insparts);
    virtual ProgData& parseData(std::vector<Token> dparts);
    virtual void evaluateOperands(std::vector<std::vector<Token>> operands, ProgIns& result, int& length);
    virtual void evaluateDestinationOperand(std::vector<Token> operand, ProgIns& result, int& length);
    virtual void evaluateSourceOperand(std::vector<Token> operand, ProgIns& result, int& length);
    virtual void evaluateMemoryExpression(std::vector<Token> expr, ProgIns& result, int& length);
    virtual ~Parser() = default;

    std::vector<ProgLine> extractLines(std::vector<Token> tokens) {
        std::vector<ProgLine> lines;
        std::vector<Token> lineparts;
        int index = 0;
        while (tokens[index].maintype != MainType::ENOF) {
            if (tokens[index].maintype == MainType::NEWLINE) {
                lines.push_back(lineparts);
                lineparts.clear();
            }
            else lineparts.push_back(tokens[index]);
            index++;
        }
        return lines;
    }

    std::vector<Token> extractData(std::vector<Token> dparts) {
        std::vector<Token> data;

        for(int i = 2; i < dparts.size(); i++) {
            if (dparts[i].maintype != MainType::COMMA) data.push_back(dparts[i]);
        }
        return data;
    }

    std::vector<std::vector<Token>> extractOperands(std::vector<Token> insparts) {
        std::vector<std::vector<Token>> operands;
        std::vector<Token> current_operand;

        for(int i = 1; i < insparts.size(); i++) {
            if (insparts[i].maintype == MainType::COMMA){
                operands.push_back(current_operand);
                current_operand.clear();
            }
            else current_operand.push_back(insparts[i]);
        }
        if (!current_operand.empty()) operands.push_back(current_operand);
        return operands;
    }

    
    bool matchTypes(MainType t, std::initializer_list<MainType> a) {
        for(MainType type : a) {
            if (t == type) return true;
        }
        return false;
    }

    
    bool matchSubTypes(SubType t, std::initializer_list<SubType> a) {

        for(SubType type : a) {
            if (t == type) return true;
        }
        return false;
    }

    bool isMemoryOperand(std::vector<Token> operand) {
    return (operand[0].maintype == MainType::OPEN_BRACE || 
            ( matchSubTypes(operand[0].subtype, {SubType::DIR_BYTE, SubType::DIR_WORD}) 
            && operand[1].maintype == MainType::OPEN_BRACE ));
    }

    std::vector<ProgLine> progLines;

    std::vector<Token> extractMemoryExpression(std::vector<Token> operand){
        std::vector<Token> memexpr;
        bool add = false;
        for(int i = 0; i < operand.size(); i++) {

            if (operand[i].maintype == MainType::OPEN_BRACE) add = true;

            if (add) memexpr.push_back(operand[i]);

            if (operand[i].maintype == MainType::CLOSE_BRACE) return memexpr;
        }
        return memexpr;
    }

    std::vector<Token> getLine(int linenum) {
        return progLines[linenum].tokens;
    }

    void setRegister8bit(REG_SELECT* reg, int operselect, uint8_t lhselect, uint8_t regcode) {
        uint8_t r = 0;
        if (operselect == DEST) r = reg->dest;
        else if (operselect == SRC) r = reg->src;
        r = (r << 4) | lhselect;
        r = (r << 3) | regcode;
        if (operselect == DEST) reg->dest = r;
        else if (operselect == SRC) reg->src = r;
    }

    bool isDestReg(uint8_t dest) {
        return (dest >= REG8 && dest <= REG32);
    }

    bool isSourceBiggerThanDest(ins_encoding* ins) {
        uint8_t source = getSrcSize(&ins->opertype);
        uint8_t dest = getDestSize(&ins->opertype);

        return source > dest;
    }
    uint8_t getSrcSize(OPER_TYPE* t) {
        switch(t->src_type) {
            case REG8 : case MEM8 :
            return 8;
            break;
            case REG16 : case MEM16 :
            return 16;
            break;
            case REG32 : MEM32 :
            return 32;
            break;

            default:
            return 0;
            break;
        }
        return 0;
    }
    uint8_t getDestSize(OPER_TYPE* t);

    uint8_t getDestSize(OPER_TYPE* t) {
        switch(t->dest_type) {
            case REG8 : case MEM8 :
            return 8;
            break;
            case REG16 : case MEM16 :
            return 16;
            break;
            case REG32 : MEM32 :
            return 32;
            break;

            default:
            return 0;
            break;
        }
        return 0;
    }

    protected:
    uint32_t program_offset = 0;
    uint32_t section_begin = 0;
    uint32_t section_offset = 0;
};

#endif