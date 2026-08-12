#ifndef PARSER_HPP
#define PARSER_HPP

#include<memory>
#include<vector>
#include<cstdint>
#include<string>
#include<initializer_list>

#include<LexicalAnalyzer/Tokenizer.hpp>
#include<ErrorHandler/ErrorHandler.hpp>

class ParseObject;
class NodeVisitor;

class MemExpr;

class ParseResult {
    public:

    std::vector<std::unique_ptr<ParseObject>> parsedLines;

    private:

    uint32_t program_counter = 0x00;
    uint32_t section_offset = 0x00;
    uint32_t section_begin = 0x00;

};

class Parser {
    public:

    std::unique_ptr<ParseResult> parse();

    std::unique_ptr<ParseObject> line();
    std::unique_ptr<ParseObject> instruction();
    std::unique_ptr<ParseObject> directive();
    std::unique_ptr<ParseObject> dataDelcaration();
    std::vector<std::unique_ptr<ParseObject>> dataList();
    std::unique_ptr<ParseObject> data();
    std::unique_ptr<ParseObject> operand();
    std::unique_ptr<ParseObject> memexpr();
    std::unique_ptr<ParseObject> memoryaddr();
    void indexexpr(MemExpr* expr);

    std::unique_ptr<ParseObject> expression();
    std::unique_ptr<ParseObject> addition();
    std::unique_ptr<ParseObject> multiplication();
    std::unique_ptr<ParseObject> unary();
    std::unique_ptr<ParseObject> primary();

    std::unique_ptr<ParseObject> parseOrg();
    std::unique_ptr<ParseObject> parseSection();
    std::unique_ptr<ParseObject> parseTimes();

    private:

    int numLine = 0;
    int numToken = 0;
    std::vector<Token> tokens;

    std::vector<Token> operationLine;
    std::vector<std::vector<Token>> programLines;

    std::vector<std::vector<Token>> extractLines() {

        std::vector<std::vector<Token>> lines;
        std::vector<Token> currentLine;

        for(int i = 0; i < tokens.size(); i++) {
            if (currentLine[i].maintype == MainType::ENOF) break;
            if (currentLine[i].maintype == MainType::NEWLINE) {
                lines.push_back(currentLine);
                currentLine.clear();
            }
            else currentLine.push_back(currentLine[i]);
        }
        return lines;
    }

    uint8_t getDirectiveSize(SubType t) {
        switch(t){
            case SubType::DIR_BYTE : case SubType::DIR_DEFB : case SubType::DIR_RESB: {
                return 1;
            }
            case SubType::DIR_WORD : case SubType::DIR_DEFW : case SubType::DIR_RESW: {
                return 2;
            }
            case SubType::DIR_DWORD : case SubType::DIR_DEFDW : case SubType::DIR_RESDW: {
                return 4;
            }
            default:
            return 0;
        }
        return 0;
    }

    bool isBaseReg(std::string regname) {
        try {
            ArchInfo::base_select_map.at(regname);
            return true;
        }
        catch(std::exception& e) {
            return false;
        }
    }
    bool isIdxReg(std::string regname) {
        try {
            ArchInfo::index_select_map.at(regname);
            return true;
        }
        catch(std::exception& e) {
            return false;
        }
    }

    void nextLine() {
        operationLine = programLines[++numLine];
        numToken = 0;
    }

    bool matchAndAdvance(std::initializer_list<MainType> types) {
        for(MainType t : types) {
            if (check(t)) {
                advance();
                return true;
            }
        }
        return false;
    }
    bool matchNextAndAdvance(std::initializer_list<MainType> types, int lookahead) {
        for(MainType t : types) {
            if (checkNext(t, lookahead)) {
                advance();
                return true;
            }
        }
        return false;
    }

    bool matchSubTypeAndAdvance(std::initializer_list<SubType> types) {
        for(SubType t : types) {
            if (checkSubType(t)) {
                advance();
                return true;
            }
        }
        return false;
    }
    bool matchNextSubTypeAndAdvance(std::initializer_list<SubType> types, int lookahead) {
        for(SubType t : types) {
            if (checkNextSubType(t, lookahead)) {
                advance();
                return true;
            }
        }
        return false;
    }

    bool isAtEndOfLine() {
        return numToken >= operationLine.size();
    }

    bool isAtEndOfProgram() {
        return numLine >= programLines.size();
    }

    Token advance() {
        if(!isAtEndOfLine()) return operationLine[numToken++];
        return Token{};
    }

    Token previous() {
        return operationLine[numToken - 1];
    }

    Token peek() {
        if (!isAtEndOfLine())
            return operationLine[numToken + 1];
        return Token{};
    }

    Token peekForward(int lookahead) {
        if (!isAtEndLookahead(lookahead)) return operationLine[numToken + lookahead];
        return Token{};
    }

    bool check(MainType type) {
        if (!isAtEndOfLine())
            return (operationLine[numToken].maintype == type);
        else return false;
    }

    bool checkSubType(SubType t) {
        if (!isAtEndOfLine()) {
            return operationLine[numToken].subtype == t;
        }
        else return false;
    }

    bool isAtEndLookahead(int lookahead) {
        return (numToken + lookahead <= operationLine.size());
    }

    bool checkNext(MainType type, int lookahead) {
        if (!isAtEndLookahead(lookahead)) {
            for(int i = 0; i < lookahead; i++) {
                if (operationLine[numToken + i].maintype == type) return true;
            }
        }
        return false;
    }
    bool checkNextSubType(SubType t, int lookahead) {
        if (!isAtEndLookahead(lookahead)) {
            for(int i = 0; i < lookahead; i++) {
                if (operationLine[numToken + i].subtype == t) return true;
            }
        }
        return false;
    }

    bool compareTypes(MainType t, std::initializer_list<MainType> types) {
        for(MainType type : types) {
            if (t == type) return true;
        }
        return false;
    }
    bool compareTypes(std::initializer_list<MainType> types) {
        for(MainType type : types) {
            if (operationLine[numToken].maintype == type) return true;
        }
        return false;
    }

    bool compareSubTypes(SubType t, std::initializer_list<SubType> types) {
        for(SubType type : types) {
            if (t == type) return true;
        }
        return false;
    }
    bool compareSubTypes(std::initializer_list<SubType> types) {
        for(SubType type : types) {
            if (operationLine[numToken].subtype == type) return true;
        }
        return false;
    }

    void skipNewLine() {
        if (current().maintype == MainType::NEWLINE) advance();
    }
    void skipNewLineUntilNextToken() {
        while (current().maintype == MainType::NEWLINE) advance();
    }

    Token consume(MainType type, std::string failMsg) {
        if (check(type)) return advance();
        std::cout << failMsg << std::endl;
        return Token{};
    }

    Token current() {
        return operationLine[numToken];
    }

    std::vector<Token> currentLine() {
        return programLines[numLine];
    }
};

class ParseObject {
    public:
    virtual ~ParseObject() = default;

    ParseObject() = default;
    
    virtual void accept(NodeVisitor& v) = 0;
};

class Binary : public ParseObject {
    public:
    std::unique_ptr<ParseObject> left = nullptr;
    std::unique_ptr<ParseObject> right = nullptr;
    Token oper;

    Binary(std::unique_ptr<ParseObject> left, Token oper, std::unique_ptr<ParseObject> right) {
        this->oper = oper;
        this->left = std::move(left);
        this->right = std::move(right);
    }
    void accept(NodeVisitor& override);
};

class Unary : public ParseObject {
    public:
    std::unique_ptr<ParseObject> right = nullptr;
    Token oper;

    Unary(Token oper, std::unique_ptr<ParseObject> right) {
        this->oper = oper;
        this->right = std::move(right);
    }

    void accept(NodeVisitor& v) override;
};

class Literal : public ParseObject {
    public:
    uint32_t value;

    Literal(uint32_t value) {this->value = value;}

    void accept(NodeVisitor& v) override;
};

class Grouping : public ParseObject {
    public:
    std::unique_ptr<ParseObject> expr = nullptr;

    Grouping(std::unique_ptr<ParseObject> expr) {
        this->expr = std::move(expr);
    }

    void accept(NodeVisitor& v) override;
};

class Register : public ParseObject {
    public:
    std::string name;
    SubType size;
    int regcode;

    Register(std::string name, SubType size, int regcode) {
        this->name = name;
        this->size = size;
        this->regcode = regcode;
    }

    void accept(NodeVisitor& v) override;
};

class Symbol : public ParseObject {
    public:
    std::string name;

    Symbol(std::string name) 
    {
        this->name = name;
    }

    void accept(NodeVisitor& v) override;
};

class Declaration : public ParseObject {
    public:
    std::string name;
    Declaration(std::string name) {this->name = name;}

    void accept(NodeVisitor& v) override;
};

class Special : public ParseObject {
    public:
    SubType type;

    Special(SubType type) {this->type = type;}

    void accept(NodeVisitor& v) override;

    uint32_t value;
};

class Instruction : public ParseObject {
    public:
    uint8_t opcode;
    std::vector<std::unique_ptr<ParseObject>> operands;

    Instruction(uint8_t opcode, std::vector<std::unique_ptr<ParseObject>> operands) {
        this->opcode = opcode;
        this->operands = std::move(operands);
        hasOperands = true;
        numOperands = operands.size();
    }

    Instruction(uint8_t opcode) {
        this->opcode = opcode;
        hasOperands = false;
        numOperands = 0;
    }

    void accept(NodeVisitor& v) override;

    bool hasOperands = false;
    bool hasRegister = false;
    bool hasMemExpr = false;
    bool hasLiteral = false;
    bool hasSpecial = false;
    bool hasSymbol = false;

    uint8_t numOperands;

    uint32_t address;
    uint32_t len;
};

class DirORG : public ParseObject {
    public:
    std::unique_ptr<ParseObject> expr;

    DirORG(std::unique_ptr<ParseObject> expr) {
        this->expr = std::move(expr);
    }

    void accept(NodeVisitor& v) override;
};

class DirSection : public ParseObject {
    public:
    std::string name;
    DirSection(std::string name) {
        this->name = name;
    }

    void accept(NodeVisitor& v) override;
};

class DirTimes : public ParseObject {
    public:
    std::string symbol;
    std::unique_ptr<ParseObject> expr;
    std::unique_ptr<ParseObject> repeated;

    DirTimes(std::string symbol, std::unique_ptr<ParseObject> expr, std::unique_ptr<ParseObject> repeated) {
        this->symbol = symbol;
        this->expr = std::move(expr);
        this->repeated = std::move(repeated);
    }

    void accept(NodeVisitor& v) override;
};

class DataNode : public ParseObject {
    public:
    std::string symbol;
    std::vector<std::unique_ptr<ParseObject>> data;
    uint8_t size;
    DataNode(std::string symbol, std::vector<std::unique_ptr<ParseObject>> data, uint8_t size) {
        this->symbol = symbol;
        this->data = std::move(data);
        this->size = size;
        hasSymbol = true;
    }

    DataNode(std::vector<std::unique_ptr<ParseObject>> data, uint8_t size) {
        this->data = std::move(data);
        this->size = size;
        hasSymbol = false;
    }

    void accept(NodeVisitor& v) override;

    bool hasSymbol;

    uint32_t address;
    uint32_t len;
};

class ResNode : public ParseObject {
    public:
    std::string symbol;
    std::unique_ptr<ParseObject> expr;
    uint8_t size;

    ResNode(std::string symbol, std::unique_ptr<ParseObject> expr, uint8_t size) {
        this->symbol = symbol;
        this->expr = std::move(expr);
        this->size = size;
        hasSymbol = true;
    }

    ResNode(std::unique_ptr<ParseObject> expr, uint8_t size) {
        this->expr = std::move(expr);
        this->size = size;

        hasSymbol = false;
    }

    bool hasSymbol;

    uint32_t address;

    void accept(NodeVisitor& v) override;
};

class StringNode : public ParseObject {
    public:
    std::string str;

    StringNode(std::string str) {
        this->str = str;
    }

    void accept(NodeVisitor& v) override;
};

class MemExpr : public ParseObject {
    public:
    uint8_t size;
    uint8_t scale = 1;
    std::unique_ptr<Register> indexreg = nullptr;
    std::unique_ptr<Register> basereg = nullptr;
    std::unique_ptr<ParseObject> displacement = nullptr;

    MemExpr(uint8_t size, std::string index, std::string base, std::unique_ptr<ParseObject> displacement) {
        this->size = size;
        this->displacement = std::move(displacement);
        if (!index.empty()) {
            indexreg = std::move(
                std::unique_ptr<Register>(new Register(index, SubType::REG32, ArchInfo::index_select_map[index]))
            );
        }
        if (!base.empty()) {
            basereg = std::move(
                std::unique_ptr<Register>(new Register(base, SubType::REG32, ArchInfo::base_select_map[index]))
            );
        } 
    }

    void accept(NodeVisitor& v) override;
};

#endif