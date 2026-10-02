#include<any>
#include<cstdint>

#include<Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

std::any ExpressionEvalVisitor::eval(ParseObject& a, AsmContext& c) 
{
    return a.accept(*this, c);
}

std::any ExpressionEvalVisitor::visit(Special& a, AsmContext& c) 
{
    return a.value;
}

std::any ExpressionEvalVisitor::visit(Symbol& a, AsmContext& c)
{
    return a.value;
}

std::any ExpressionEvalVisitor::visit(Number& a, AsmContext& c)
{
    return a.value;
}

std::any ExpressionEvalVisitor::visit(Binary& a, AsmContext& c)
{
    uint32_t left = std::any_cast<uint32_t>(eval(*a.left, c));
    uint32_t right = std::any_cast<uint32_t>(eval(*a.right, c));

    switch(a.oper.subtype)
    {
        case SubType::OPER_ADD: return left + right;
        case SubType::OPER_SUB: return left - right;
        case SubType::OPER_MUL: return left * right;
        case SubType::OPER_DIV: return left / right;
        default: return 0;
    }
}

std::any ExpressionEvalVisitor::visit(Unary& a, AsmContext& c) 
{
    uint32_t right = std::any_cast<uint32_t>(eval(*a.right, c));

    switch(a.oper.subtype)
    {
        case SubType::OPER_ADD: return +right;
        case SubType::OPER_SUB: return -right;
        default: return 0;
    }
}

std::any ExpressionEvalVisitor::visit(Grouping& a, AsmContext& c) 
{
    return eval(*a.expr, c);
}

