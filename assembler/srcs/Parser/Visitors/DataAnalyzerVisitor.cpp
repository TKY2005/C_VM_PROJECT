#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

void DataAnalyzer::analyze(ParseObject& a) {
    a.accept(*this);
}

void DataAnalyzer::visit(DataNode& a) {
    
    for(int i = 0; i < a.data.size(); i++) {
        analyze(*a.data[i]);
    }
}

void DataAnalyzer::visit(ResNode& a) {
    analyze(*a.expr);
}

void DataAnalyzer::visit(StringNode& a) {
    prevtype = MainType::STR;
    prevsub = SubType::NONE;
}

void DataAnalyzer::visit(Binary& a) {

    analyze(*a.left);

    MainType lefttype = prevtype;
    SubType leftsub = prevsub;

    analyze(*a.right);

    MainType righttype = prevtype;
    SubType rightsub = prevsub;

    if (!compareTypes(lefttype, allowedExprTypes)) {
        // TODO: throw semantic error: illegal expression types
    }

}

void DataAnalyzer::visit(Unary& a) {

    analyze(*a.right);

    if (!compareTypes(prevtype, allowedExprTypes)) return; // TODO: throw semantic error
}

void DataAnalyzer::visit(Grouping& a) {
    analyze(*a.expr);
}

void DataAnalyzer::visit(Literal& a) {
    prevtype = MainType::NUM;
    prevsub = SubType::NONE;
}

void DataAnalyzer::visit(Symbol& a) {
    prevtype = MainType::SYM;
    prevsub = SubType::NONE;
}

void DataAnalyzer::visit(Special& a) {
    prevtype = MainType::SPECIAL;
    prevsub = a.type;
}