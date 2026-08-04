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

class Parser {
    public:

    std::unique_ptr<ParseObject> parse();

    std::unique_ptr<ParseObject> line();
    std::unique_ptr<ParseObject> instruction();
    std::unique_ptr<ParseObject> directive();
    std::unique_ptr<ParseObject> dataEmmiter();
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

    int currentToken = 0;
    std::vector<Token> tokens;

    uint32_t programOffset = 0;
    uint32_t sectionOffset = 0;

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

    bool isAtEnd() {
        return currentToken >= tokens.size() || tokens[currentToken].maintype == MainType::ENOF;
    }

    Token advance() {
        if(!isAtEnd()) return tokens[currentToken++];
        return Token{};
    }

    Token previous() {
        return tokens[currentToken - 1];
    }

    Token peek() {
        if (!isAtEnd())
            return tokens[currentToken + 1];
        return Token{};
    }

    Token peekForward(int lookahead) {
        if (!isAtEndLookahead(lookahead)) return tokens[currentToken + lookahead];
        return Token{};
    }

    bool check(MainType type) {
        if (!isAtEnd())
            return (tokens[currentToken].maintype == type);
        else return false;
    }

    bool checkSubType(SubType t) {
        if (!isAtEnd()) {
            return tokens[currentToken].subtype == t;
        }
        else return false;
    }

    bool isAtEndLookahead(int lookahead) {
        return (currentToken + lookahead <= tokens.size());
    }

    bool checkNext(MainType type, int lookahead) {
        if (!isAtEndLookahead(lookahead)) {
            for(int i = 0; i < lookahead; i++) {
                if (tokens[currentToken + i].maintype == type) return true;
            }
        }
        return false;
    }
    bool checkNextSubType(SubType t, int lookahead) {
        if (!isAtEndLookahead(lookahead)) {
            for(int i = 0; i < lookahead; i++) {
                if (tokens[currentToken + i].subtype == t) return true;
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
            if (tokens[currentToken].maintype == type) return true;
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
            if (tokens[currentToken].subtype == type) return true;
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
        return tokens[currentToken];
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
    void accept(NodeVisitor& v) override;
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

    uint32_t value = 0;

    Special(SubType type) {this->type = type;}
    void accept(NodeVisitor& v) override;
};

class Instruction : public ParseObject {
    public:
    uint8_t opcode;
    std::vector<std::unique_ptr<ParseObject>> operands;

    Instruction(uint8_t opcode, std::vector<std::unique_ptr<ParseObject>> operands) {
        this->opcode = opcode;
        this->operands = std::move(operands);
    }

    void accept(NodeVisitor& v) override;
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
    }

    void accept(NodeVisitor& v) override;
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
    }

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