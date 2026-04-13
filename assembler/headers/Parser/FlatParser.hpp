#ifndef FLAT_PARSER_HPP
#define FLAT_PARSER_HPP

#include<vector>
#include<map>
#include<string>

#include<Parser/Parser.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<LexicalAnalyzer/Lexer.hpp>

class FlatParser : public Parser {
    
    public:
    ParseResult* parseTokens(std::vector<Token> tokens) override;
    ProgIns& parseInstruction(std::vector<Token> insparts) override;
    ProgData& parseData(std::vector<Token> dparts) override;
    void evaluateOperands(std::vector<std::vector<Token>> operands, ProgIns& result, int& length) override;
    void evaluateDestinationOperand(std::vector<Token> operand, ProgIns& result, int& length) override;
    void evaluateSourceOperand(std::vector<Token> operand, ProgIns& result, int& length) override;
    void evaluateMemoryExpression(std::vector<Token> expr, ProgIns& result, int& length) override;
};

#endif