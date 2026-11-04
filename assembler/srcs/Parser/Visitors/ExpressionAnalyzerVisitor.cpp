#include<any>
#include<cstdint>
#include<stdexcept>

#include<Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

std::any ExpressionAnalyzerVisitor::analyze(ParseObject& a, AsmContext& c) 
{
    return a.accept(*this, c);
}

std::any ExpressionAnalyzerVisitor::visit(Binary &a, AsmContext &c)
{
    analyze(*a.left, c);
    analyze(*a.right, c);
    return true;
}

std::any ExpressionAnalyzerVisitor::visit(Unary& a, AsmContext& c)
{
    analyze(*a.right, c);
    return true;
}

std::any ExpressionAnalyzerVisitor::visit(Grouping& a, AsmContext& c)
{
    analyze(*a.expr, c);
    return true;
}

std::any ExpressionAnalyzerVisitor::visit(Number& a, AsmContext& c)
{
    return true;
}

std::any ExpressionAnalyzerVisitor::visit(Special& a, AsmContext& c)
{
    return true;
}

std::any ExpressionAnalyzerVisitor::visit(Symbol& a, AsmContext& c)
{
    return true;
}

std::any ExpressionAnalyzerVisitor::visit(StringNode& a, AsmContext& c)
{
    throw std::runtime_error("strings are not valid arithmetic expressions");
}

std::any ExpressionAnalyzerVisitor::visit(Register& a, AsmContext& c)
{
    throw std::runtime_error("registers are not valid arithmetic expressions");
}
