#ifndef VISITORS_HPP
#define VISITORS_HPP

#include<cstdint>
#include<map>
#include<string>

// base class for every node
class ParseObject;

// T = terminal node, NT = non-terminal node

// Expression classes
class Binary; // NT
class Unary; // NT
class Grouping; // NT
class Literal; // T


class Register; // T
class Symbol; // T
class Declaration; // T
class Special; // T
class Instruction; // NT
class DirORG; // NT
class DirSection; // T
class DirTimes; // NT
class DataNode; // NT
class ResNode; // NT
class StringNode; // T
class MemExpr; // NT

class NodeVisitor {
    public:
    virtual ~NodeVisitor() = default;

    virtual void visit(Binary& a) = 0;
    virtual void visit(Unary& a) = 0;
    virtual void visit(Grouping& a) = 0;
    virtual void visit(Literal& a) = 0;

    virtual void visit(Register& a) = 0;
    virtual void visit(Symbol& a) = 0;
    virtual void visit(Declaration& a) = 0;
    virtual void visit(Special& a) = 0;
    virtual void visit(Instruction& a) = 0;
    virtual void visit(DirORG& a) = 0;
    virtual void visit(DirSection& a) = 0;
    virtual void visit(DirTimes& a) = 0;
    virtual void visit(DataNode& a) = 0;
    virtual void visit(ResNode& a) = 0;
    virtual void visit(StringNode& a) = 0;
    virtual void visit(MemExpr& a) = 0;

    int typeToSize(SubType t) {
        switch(t) {
            case SubType::REG8 : case SubType::DIR_BYTE : case SubType::DIR_DEFB : case SubType::DIR_RESB : 
            {
                return 1;
            }
            case SubType::REG16 : case SubType::DIR_WORD : case SubType::DIR_DEFW : case SubType::DIR_RESW : 
            {
                return 2;
            }
            case SubType::REG32 : case SubType::DIR_DWORD : case SubType::DIR_DEFDW : case SubType::DIR_RESDW :
            {
                return 4;
            }
            default:
            return -1;
        }
        return -1;
    }
};

class ExpressionEvalVisitor : virtual public NodeVisitor {
    public:
    uint32_t exprResult;

    virtual ~ExpressionEvalVisitor() = default;

    void evalExpr(ParseObject& o);

    virtual void visit(Binary& a) = 0;
    virtual void visit(Unary& a) = 0;
    virtual void visit(Grouping& a) = 0;
    virtual void visit(Literal& a) = 0;
};

class StatementVisitor : virtual public NodeVisitor {
    public:
    virtual ~StatementVisitor() = default;

    void evalStmt(ParseObject& a);

    std::map<std::string, uint32_t> symMap;

    virtual void visit(Register& a) = 0;
    virtual void visit(Symbol& a) = 0;
    virtual void visit(Declaration& a) = 0;
    virtual void visit(Special& a) = 0;
    virtual void visit(Instruction& a) = 0;
    virtual void visit(DirORG& a) = 0;
    virtual void visit(DirSection& a) = 0;
    virtual void visit(DirTimes& a) = 0;
    virtual void visit(DataNode& a) = 0;
    virtual void visit(ResNode& a) = 0;
    virtual void visit(StringNode& a) = 0;
    virtual void visit(MemExpr& a) = 0;
};

class ExpressionVisitor : public ExpressionEvalVisitor {
    public:
    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;

    void evalExpr(ParseObject& a);
};

class SymbolCollectorVisitor : public StatementVisitor, public ExpressionEvalVisitor {
    public:
    uint32_t program_offset = 0x0;
    uint32_t section_offset = 0x0;
    uint32_t section_begin = 0x0;
    int currentsize = 0;

    MainType prevType;
    SubType prevSubType;

    bool addRegCode = false;
    bool addDispInfo = false;
    bool parsingMemory = false;
    bool addDisplacement = false;

    void evalSize(ParseObject& a);

    void setProgramAndSectionsOffsets(uint32_t new_val) {
        program_offset = new_val;
        section_begin = program_offset;
        section_offset = 0;
    }
    void setProgramOffset(uint32_t new_val) {
        program_offset = new_val;
        section_offset = program_offset - section_begin;
    }

    void incProgramAndSetSectionOffsets(uint32_t inc_val) {
        program_offset += inc_val;
        section_begin = program_offset;
        section_offset = 0;
    }

    void incProgramOffset(uint32_t inc_val) {
        program_offset += inc_val;
        section_offset = program_offset - section_begin;
    }

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;
    void visit(Register& a) override;
    void visit(Symbol& a) override;
    void visit(Declaration& a) override;
    void visit(Special& a) override;
    void visit(Instruction& a) override;
    void visit(DirORG& a) override;
    void visit(DirSection& a) override;
    void visit(DirTimes& a) override;
    void visit(DataNode& a) override;
    void visit(ResNode& a) override;
    void visit(StringNode& a) override;
    void visit(MemExpr& a) override;
};

class SemanticAnalyzerVisitor : public ExpressionEvalVisitor, public StatementVisitor {
    public:
    MainType maintype;
    SubType subtype;

    void evalStmt(ParseObject& a);

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;
    void visit(Register& a) override;
    void visit(Symbol& a) override;
    void visit(Declaration& a) override;
    void visit(Special& a) override;
    void visit(Instruction& a) override;
    void visit(DirORG& a) override;
    void visit(DirSection& a) override;
    void visit(DirTimes& a) override;
    void visit(DataNode& a) override;
    void visit(ResNode& a) override;
    void visit(StringNode& a) override;
    void visit(MemExpr& a) override;
};



#endif