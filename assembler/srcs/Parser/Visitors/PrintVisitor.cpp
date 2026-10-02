#include<Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>
#include<LexicalAnalyzer/Lexer.hpp>

#include<any>
#include<string>
#include<sstream>
#include<iomanip>

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

std::any PrintVisitor::visit(Special &a, AsmContext &c)
{
    return "Special: " + Lexer::subTypeToStr(a.type);
}

std::any PrintVisitor::visit(StringNode &a, AsmContext &c)
{
    return "String: " "\"" + a.str + "\"";
}

std::any PrintVisitor::visit(Symbol &a, AsmContext &c)
{
    return "Symbol: " + a.name;
}

std::any PrintVisitor::visit(DataNode &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');

    indentLevel++;
    std::vector<std::string> data;
    for(int i = 0; i < a.data.size(); i++) 
        data.push_back(std::any_cast<std::string>(traverse(*a.data[i], c)));
    indentLevel--; 

    std::string d;
    for(int i = 0; i < data.size(); i++) d.append(s).append("├── ").append(data[i]).append("\n");

    return "Data: size = " + std::to_string(a.size) + " byte(s)\n" + d;
}

std::any PrintVisitor::visit(ResNode &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');
    indentLevel++;
    std::string expr = std::any_cast<std::string>(traverse(*a.expr, c));
    indentLevel--;

    return "ResNode: " "size = " + std::to_string(a.size) + "byte(s)\n" +
            s + "└── " + expr;
}

std::any PrintVisitor::visit(DirSection &a, AsmContext &c)
{
    return "Section: " + a.name;
}

std::any PrintVisitor::visit(DirORG &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');
    indentLevel++;
    std::string expr = std::any_cast<std::string>(traverse(*a.expr, c));
    indentLevel--;

    return "OrgNode: \n" +
        s + "└── " + expr; 
}

std::any PrintVisitor::visit(DirTimes &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');
    indentLevel++;
    std::string expr = std::any_cast<std::string>(traverse(*a.expr, c));
    std::string repeated = std::any_cast<std::string>(traverse(*a.repeated, c));
    indentLevel--;

    return "TimesNode: \n" +
        s + "└── " + expr + "\nRepeated: \n" + s + "└── " + repeated;
}

std::any PrintVisitor::visit(Instruction &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');
    indentLevel++;
    std::stringstream ss;
    ss << "0x" << std::hex << (int) a.opcode;

    std::string operands;
    if (a.operands.size() == 1) operands.append("└── ").append(
        std::any_cast<std::string>(traverse(*a.operands[0], c))
    );
    else if (a.operands.size() == 2) operands.append("├──").append(
        std::any_cast<std::string>(traverse(*a.operands[0], c))
    ).append("\n").append("└── ").append(
        std::any_cast<std::string>(traverse(*a.operands[1], c))
    );
    indentLevel--;
    return "Instruction: " + std::to_string(a.opcode) + " " + ss.str() + '\n' +
        s + operands;
}

std::any PrintVisitor::visit(MemExpr &a, AsmContext &c)
{
    std::string s(4 * indentLevel, ' ');

    indentLevel++;
    std::string disp = 
        (a.displacement != nullptr) ? std::any_cast<std::string>(traverse(*a.displacement, c)) : "None";
    indentLevel--;

    std::string base = "base: " + ((a.basereg != nullptr) ? a.basereg.get()->name : "None");
    std::string index = "index: " + ((a.indexreg != nullptr) ? a.indexreg.get()->name : "None");
    return "MemExpr: " + base + ", " + index + "\n" +
        s + "└── " + disp;
}

std::any PrintVisitor::visit(Register &a, AsmContext &c)
{
    return "Register: " + a.name + " " + std::to_string(a.regcode);
}

std::any PrintVisitor::visit(Declaration &a, AsmContext &c)
{
    return "Declaration: " + a.name;
}
