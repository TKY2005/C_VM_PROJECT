#ifndef FLAT_PARSER_HPP
#define FLAT_PARSER_HPP

#include<vector>
#include<map>
#include<string>
#include<cstdint>

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

    //std::vector<Token> extractExpression(std::vector<Token> t);
    //uint32_t evalExpr(std::vector<Token> expr);

    uint32_t calculateInstructionLength(std::vector<Token> insTokens);
    void resolveDirective(std::vector<Token> dir, ParseResult& r);

    bool checkSyntax_INS_0_OPER(std::vector<Token> dir);
    bool checkSyntax_INS_1_OPER(std::vector<Token> dir);
    bool checkSyntax_INS_2_OPER(std::vector<Token> dir);
    
    bool checkSyntax_DIR_SECTION(std::vector<Token> dir);
    bool checkSyntax_DIR_DATA_DEF(std::vector<Token> dir);
    bool checkSyntax_DIR_RES(std::vector<Token> dir);
    bool checkSyntax_DIR_ORG(std::vector<Token> dir);


    bool isReservationDirective(Token t);
};

#endif