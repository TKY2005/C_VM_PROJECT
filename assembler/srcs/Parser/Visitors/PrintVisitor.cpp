#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

#include<any>
#include<string>

std::any PrintVisitor::traverse(ParseObject& a, AsmContext& c) {
    return a.accept(*this, c);
}

std::any PrintVisitor::visit(Binary& a, AsmContext& c) {

    std::string s(4 * indentLevel, ' ');

    indentLevel++;

    std::string left = std::any_cast<std::string>(traverse(*a.left, c));
    std::string right = std::any_cast<std::string>(traverse(*a.right, c));

    indentLevel--;

    return "Binary: " + a.oper.tokenstr + "\n" +
           s + "├── " + left + "\n" +
           s + "└── " + right;
}

std::any PrintVisitor::visit(Unary &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');

    indentLevel++;

    std::string right = std::any_cast<std::string>(traverse(*a.right, c));

    indentLevel--;

    return "Unary: " + a.oper.tokenstr + "\n" +
    s +     "└── " + right;
}

std::any PrintVisitor::visit(Grouping &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');

    indentLevel++;
    std::string expr = std::any_cast<std::string>(traverse(*a.expr, c));
    indentLevel--;

    return "Grouping:\n" +
            s + "└── " + expr;
}

std::any PrintVisitor::visit(Number &a, AsmContext &c)
{
    return "Number: " + std::to_string(a.value);
}


