#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

std::unique_ptr<Value> ExpressionVisitor::eval(ParseObject& a, AsmContext& c) {
    return a.accept(*this, c);
}

std::unique_ptr<Value> ExpressionVisitor::visit(Special& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{.val = (void*) a.value});
}

std::unique_ptr<Value> ExpressionVisitor::visit(Symbol& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{.val = (void*) c.symMap[a.name]});
}

std::unique_ptr<Value> ExpressionVisitor::visit(Number& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{.val = (void*) a.value});
}

std::unique_ptr<Value> ExpressionVisitor::visit(Binary& a, AsmContext& c) {
    
    uint32_t left = vptouint(eval(*a.left, c).get()->val);
    uint32_t right = vptouint(eval(*a.right, c).get()->val);

    std::unique_ptr<Value> result = std::unique_ptr<Value>(new Value());

    uint32_t res;

    switch(a.oper.subtype) {

        case SubType::OPER_ADD:
        res = left + right;
        break;

        case SubType::OPER_SUB:
        res = left - right;
        break;

        case SubType::OPER_MUL:
        res = left * right;
        break;

        case SubType::OPER_DIV:
        res = left / right;
        break;

        default:
        res = 0;
        break;
    }

    result.get()->val = (void*) res;

    return result;
}

std::unique_ptr<Value> ExpressionVisitor::visit(Unary& a, AsmContext& c) {
    
    uint32_t right = vptouint(eval(*a.right, c).get()->val);

    std::unique_ptr<Value> result = std::unique_ptr<Value>(new Value());

    uint32_t res;

    switch(a.oper.subtype) {

        case SubType::OPER_ADD:
        res = right;
        break;

        case SubType::OPER_SUB:
        res = -right;
        break;

        default:
        res = 0;
        break;
    }

    result.get()->val = (void*) res;

    return result;
}

std::unique_ptr<Value> ExpressionVisitor::visit(Grouping& a, AsmContext& c) {
    return eval(*a.expr, c);
}