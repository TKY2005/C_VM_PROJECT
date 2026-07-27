#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

void ExpressionVisitor::evalExpr(ParseObject& o) {
    o.accept(*this);
}

void ExpressionVisitor::visit(Literal& a) {
    exprResult = a.value;
}

void ExpressionVisitor::visit(Grouping& a) {
    evalExpr(*a.expr);
}

void ExpressionVisitor::visit(Binary& a) {
    evalExpr(*a.left);
    uint32_t left = exprResult;
    evalExpr(*a.right);
    uint32_t right = exprResult;

    switch(a.oper.subtype) {
        case SubType::OPER_ADD:
        {
            exprResult = left + right;
            break;
        }
        case SubType::OPER_SUB:
        {
            exprResult = left - right;
            break;
        }
        case SubType::OPER_MUL:
        {
            exprResult = left * right;
            break;
        }
        case SubType::OPER_DIV:
        {
            exprResult = left / right;
            break;
        }
        default:
        break;
    }
}

void ExpressionVisitor::visit(Unary& a) {

    evalExpr(*a.right);
    uint32_t x = exprResult;
    switch(a.oper.subtype) {
        case SubType::OPER_ADD:
        {
            exprResult = +x;
            break;
        }
        case SubType::OPER_SUB:
        {
            exprResult = -x;
            break;
        }
        default:
        break;
    } 
}