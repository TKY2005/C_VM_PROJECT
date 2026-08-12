#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

void DirectiveAnalyzer::analyze(ParseObject& a) {
    a.accept(*this);
}

void DirectiveAnalyzer::visit(DirORG& a) {
    
    if (!isExprNode(a.expr.get())) return; // TODO: Throw a semantic error.
    analyze(*a.expr);
}

void DirectiveAnalyzer::visit(DirSection& a) {
    return;
}


void DirectiveAnalyzer::visit(DirTimes& a) {
    analyze(*a.repeated);
}

void DirectiveAnalyzer::visit(Binary& a) {
    
    analyze(*a.left);

    MainType left = prevtype;
    SubType lefts = prevsub;

    if (!compareTypes(left, allowedExprTypes)) return; // TODO: throw semantic error

    analyze(*a.right);

    MainType right = prevtype;
    SubType rights = prevsub;

    if (!compareTypes(right, allowedExprTypes)) return; // TODO: throw semantic error    
}

void DirectiveAnalyzer::visit(Unary& a) {
    
    analyze(*a.right);

    MainType right = prevtype;
    SubType rights = prevsub;

    if (!compareTypes(right, allowedExprTypes)) return; // TODO: throw semantic error
}

void DirectiveAnalyzer::visit(Grouping& a) {
    
    analyze(*a.expr);
}
void DirectiveAnalyzer::visit(Literal& a) {
    prevtype = MainType::NUM;
    prevsub = SubType::NONE;
}
void DirectiveAnalyzer::visit(Special& a) {
    prevtype = MainType::SPECIAL;
    prevsub = a.type;
}
void DirectiveAnalyzer::visit(Symbol& a) {
    prevtype = MainType::SYM;
    prevsub = SubType::NONE;
}