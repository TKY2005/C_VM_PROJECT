#ifndef VISITORS_HPP
#define VISITORS_HPP

#include<cstdint>

// base class for every node
class ParseObject;

// Expression classes
class Binary;
class Unary;
class Grouping;
class Literal;


class Register;
class Symbol;
class Declaration;
class Special;
class Instruction;
class DirORG;
class DirSection;
class DirTimes;
class DataNodes;
class ResNode;
class StringNode;
class MemExpr;

class NodeVisitor {
    public:
    virtual ~NodeVisitor() = default;

    virtual void visit(Binary& a) = 0;
    virtual void visit(Unary& a) = 0;
    virtual void visit(Grouping& a) = 0;
    virtual void visit(Literal& a) = 0;

    virtual void visit(Register& a) = 0;
};

class ExpressionVisitor : public NodeVisitor {
    public:
    uint32_t value;

    virtual ~ExpressionVisitor() = default;

    virtual void visit(Binary& a) = 0;
    virtual void visit(Unary& a) = 0;
    virtual void visit(Grouping& a) = 0;
    virtual void visit(Literal& a) = 0;
};

#endif