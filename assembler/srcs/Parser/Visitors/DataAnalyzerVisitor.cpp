#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

std::unique_ptr<Value> DataAnalyzerVisitor::analyze(ParseObject& a, AsmContext& c) {
    return a.accept(*this, c);
}

std::unique_ptr<Value> DataAnalyzerVisitor::visit(StringNode& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value({(void*) new TypeInfo({MainType::STR, SubType::NONE})}));
}

std::unique_ptr<Value> DataAnalyzerVisitor::visit(DataNode& a, AsmContext& c) {
    
    for(int i = 0; i < a.data.size(); i++) analyze(*a.data[i], c);
}