#include<any>
#include<cstdint>
#include<stdexcept>

#include<Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

std::any ExpressionEvalVisitor::eval(ParseObject& a, AsmContext& c) 
{
    return a.accept(*this, c);
}

std::any ExpressionEvalVisitor::evalMemory(ParseObject& a, AsmContext& c)
{
    bool wasEvaluatingMemory = evaluatingMemory;
    evaluatingMemory = true;
    try {
        std::any value = eval(a, c);
        evaluatingMemory = wasEvaluatingMemory;
        return value;
    }
    catch (...) {
        evaluatingMemory = wasEvaluatingMemory;
        throw;
    }
}

std::any ExpressionEvalVisitor::visit(Special& a, AsmContext& c) 
{
    if (a.type == SubType::SPECIAL_PROG_OFFSET) return c.program_counter;
    if (a.type == SubType::SPECIAL_SECT_OFFSET) return c.section_offset;
    throw std::runtime_error("unsupported special expression");
}

std::any ExpressionEvalVisitor::visit(Symbol& a, AsmContext& c)
{
    auto value = c.symMap.find(a.name);
    if (value == c.symMap.end()) {
        if (c.allowUnresolvedSymbols) return uint32_t(0);
        throw std::runtime_error("undefined symbol '" + a.name + "'");
    }
    return value->second;
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
        case SubType::OPER_DIV:
            if (right == 0) throw std::runtime_error("division by zero in expression");
            return left / right;
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

std::any ExpressionEvalVisitor::visit(StringNode& a, AsmContext& c)
{
    throw std::runtime_error("strings cannot be evaluated as arithmetic expressions");
}

std::any ExpressionEvalVisitor::visit(Register& a, AsmContext& c)
{
    if (evaluatingMemory) return uint32_t(0);
    throw std::runtime_error("registers cannot be evaluated as arithmetic expressions");
}