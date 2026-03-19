#ifndef LEXER_HPP
#define LEXER_HPP

#include<string>

enum class MainType {
    // multi character tokens
    NUM,
    STR,
    INS,
    REG,
    DIR,
    SYM,
    DECL,
    // single character tokens
    OPER,
    COMMA,
    NEWLINE,
    OPEN_BRACE,
    SEMICOLON,
    CLOSE_BRACE,
    ENOF,
    UNDEFINED
};

enum class SubType {
    REG8, REG16, REG32,
    DIR_ORG, DIR_SECTION, DIR_EQU,
    DIR_BYTE, DIR_WORD,
    DIR_DEFB, DIR_DEFW,
    DIR_RESB, DIR_RESW,
    OPER_ADD, OPER_SUB, OPER_MUL, OPER_DIV,
    NONE
};

class Lexer {

    public:
    static std::string delimeters;

    static MainType decideMainType(std::string tok);
    static SubType decideSubType(std::string tok, MainType t);
    static bool isDigit(char c);
    static bool isPrintable(char c);
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
    static bool isSeperator(char c);
    static bool isDelim(char c);

    static std::string toLowerCase(std::string s);

    static bool isOperator(char c);
    static bool isIns(std::string tok);
    static bool isReg(std::string tok);
    static bool isDir(std::string tok);

    static std::string mainTypeToStr(MainType t);
    static std::string subTypeToStr(SubType t);
};


#endif