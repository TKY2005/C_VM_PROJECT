#ifndef VISITORS_HPP
#define VISITORS_HPP

#include<cstdint>
#include<map>
#include<string>
#include<initializer_list>

#include<LexicalAnalyzer/Tokenizer.hpp>

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

    virtual void visit(Binary& a) {};
    virtual void visit(Unary& a) {};
    virtual void visit(Grouping& a) {};
    virtual void visit(Literal& a) {};

    virtual void visit(Register& a) {};
    virtual void visit(Symbol& a) {};
    virtual void visit(Declaration& a) {};
    virtual void visit(Special& a) {};
    virtual void visit(Instruction& a) {};
    virtual void visit(DirORG& a) {};
    virtual void visit(DirSection& a) {};
    virtual void visit(DirTimes& a) {};
    virtual void visit(DataNode& a) {};
    virtual void visit(ResNode& a) {};
    virtual void visit(StringNode& a) {};
    virtual void visit(MemExpr& a) {};

    std::initializer_list<MainType> allowedExprTypes = {MainType::NUM, MainType::SYM, MainType::SPECIAL};

    int typeToSize(SubType t);

    bool isExprNode(ParseObject* p);

    bool compareTypes(MainType t, std::initializer_list<MainType> types);

    bool isBaseReg(Register* a);
    bool isIdxReg(Register* a);
};

class ExpressionEvalVisitor : public NodeVisitor {
    public:

    uint32_t exprVal = 0;

    void evalExpr(ParseObject& a);

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;
    void visit(Symbol& a) override;
};

class InstructionVisitor : public NodeVisitor {

    public:

    ExpressionEvalVisitor expressionEvaluator;

    virtual void visit(Instruction& a) override {};
    virtual void visit(Register& a) override {};
    virtual void visit(MemExpr& a) override {};
    virtual void visit(Special& a) override {};
};

class DataVisitor : public NodeVisitor {
    public:
    ExpressionEvalVisitor exprEvaluator;


    virtual void visit(DataNode& a) override {};
    virtual void visit(StringNode& a) override {};
    virtual void visit(ResNode& a) override {};
};

class DirectiveVisitor : public NodeVisitor {
    public:
    ExpressionEvalVisitor exprEvaluator;

    virtual void visit(DirTimes& a) override {};
    virtual void visit(DirSection& a) override {};
    virtual void visit(DirORG& a) override {};
};


class ExpressionAnalyzer : public NodeVisitor {
    public:
    void analyze(ParseObject& a);

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;
    void visit(Special& a) override;
    void visit(Symbol& a) override;
};

class InstructionAnalyzer : public InstructionVisitor {
    public:

    MainType prevType;
    SubType prevSubType;

    bool prevWasMemory = false;

    void visit(Instruction& a) override;
    void visit(Register& a) override;
    void visit(MemExpr& a) override;
    void visit(Special& a) override;

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;
    void visit(Symbol& a) override;

    void analyze(ParseObject& a);
};

class DataAnalyzer : public DataVisitor {
    public:

    MainType prevtype;
    SubType prevsub;

    void analyze(ParseObject& a);

    void visit(DataNode& a) override;
    void visit(StringNode& a) override;
    void visit(ResNode& a) override;

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;

    void visit(Literal& a) override;
    void visit(Special& a) override;
    void visit(Symbol& a) override;
};

class DirectiveAnalyzer : public DirectiveVisitor {

    public:

    MainType prevtype;
    SubType prevsub;
    
    void analyze(ParseObject& a);

    void visit(DirTimes& a) override;
    void visit(DirORG& a) override;
    void visit(DirSection& a) override;

    void visit(Binary& a) override;
    void visit(Unary& a) override;
    void visit(Grouping& a) override;
    void visit(Literal& a) override;
    void visit(Special& a) override;
    void visit(Symbol& a) override;
};

#endif