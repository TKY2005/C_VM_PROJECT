#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>

void SemanticAnalyzerVisitor::evalStmt(ParseObject& a) {
    a.accept(*this);
}

void SemanticAnalyzerVisitor::visit(Literal& a) {
    maintype = MainType::NUM;
    subtype = SubType::NONE;
}

void SemanticAnalyzerVisitor::visit(Register& a) {
    maintype = MainType::REG;
    subtype = a.size;
}

void SemanticAnalyzerVisitor::visit(Symbol& a) {
    maintype = MainType::SYM;
    subtype = SubType::NONE;
}

void SemanticAnalyzerVisitor::visit(Declaration& a) {
    maintype = MainType::DECL;
    subtype = SubType::NONE;
}

void SemanticAnalyzerVisitor::visit(Special& a) {
    maintype = MainType::SPECIAL;
    subtype = a.type;
}

void SemanticAnalyzerVisitor::visit(DirSection& a) {
    maintype = MainType::DIR;
    subtype = SubType::DIR_SECTION;
}

void SemanticAnalyzerVisitor::visit(StringNode& a) {
    maintype = MainType::STR;
    subtype = SubType::NONE;
}

void SemanticAnalyzerVisitor::visit(Binary& a) {

    evalStmt(*a.left);
    MainType leftMain = maintype;
    SubType leftSub = subtype;

    evalStmt(*a.right);
    MainType rightMain = maintype;
    SubType rightSub = subtype;
}