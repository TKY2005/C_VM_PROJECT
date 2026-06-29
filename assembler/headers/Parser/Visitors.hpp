#ifndef VISITORS_HPP
#define VISITORS_HPP

#include "./Nodes.hpp"

class NodeVisitor {
    public:
    virtual ~NodeVisitor() = default;

    virtual void visit(InstructionNode& n) = 0;
    virtual void visit(OperandNode& n) = 0;
    virtual void visit(DirectiveNode& n) = 0;
    virtual void visit(ExpressionNode& n) = 0;
    virtual void visit(LabelNode& n) = 0;
};

// TODO: Add visitor classes //

#endif