#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

void ExpressionEvalVisitor::evalExpr(ParseObject& o) {
    o.accept(*this);
}

void ExpressionEvalVisitor::visit(Literal& a) {
    exprVal = a.value;
}

void ExpressionEvalVisitor::visit(Symbol& a) {
    // TODO: get value from symbol map.
}

void ExpressionEvalVisitor::visit(Unary& a) {
    evalExpr(*a.right);
    uint32_t right = exprVal;

    switch (a.oper.subtype) {

        case SubType::OPER_ADD:
        exprVal = +right;
        break;

        case SubType::OPER_SUB:
        exprVal = -right;
        break;

        default:
        exprVal = right;
        break;
    }
}

void ExpressionEvalVisitor::visit(Binary& a) {
    evalExpr(*a.left);
    uint32_t left = exprVal;

    evalExpr(*a.right);
    uint32_t right = exprVal;

    switch (a.oper.subtype) {

        case SubType::OPER_ADD:
        exprVal = left + right;
        break;

        case SubType::OPER_SUB:
        exprVal = left - right;
        break;

        case SubType::OPER_MUL:
        exprVal = left * right;
        break;

        case SubType::OPER_DIV:
        exprVal = left / right;
        break;

        default:
        break;
    }
}

void ExpressionEvalVisitor::visit(Grouping& a) {
    evalExpr(*a.expr);
}