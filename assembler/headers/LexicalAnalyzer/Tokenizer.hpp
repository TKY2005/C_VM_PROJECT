#pragma once

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<cstdint>
#include<sstream>
#include<optional>

#include "./Lexer.hpp"
#include "../ArchInfo.hpp"

class Token {

    public:
        std::string tokenstr;
        MainType maintype;
        SubType subtype;
        int row, column;
        std::string toStringDebug() {
            std::ostringstream oss;
            oss << 
            "[" << "token: " << tokenstr << ", " 
            << "maintype: " << Lexer::mainTypeToStr(maintype) << ", "
            << "subtype: " << Lexer::subTypeToStr(subtype) << ", "
            << "position: " << "row: " << row << ", col: " << column
            << "]";

            return oss.str();
        }
        std::string toString() {
            return tokenstr;
        }
    
    Token(std::string tokstr, MainType main, SubType sub, int r, int c) {
        this->tokenstr = tokstr;
        this->maintype = main;
        this->subtype = sub;
        this->row = r;
        this->column = c;
    }

};

class Tokenizer {

    public:
    std::string preProcessCode(const std::string& input);
    std::vector<std::string> split_find_substr(std::string s, const std::string& delimiter);
    std::vector<Token> tokenize(std::string source);
    void resetGlobalState();
    void consume();
    void skip();
    char getCurChar();
    bool isAtEnd();
    private:
    std::string src;
    std::vector<Token> toklist;
    std::string current;
    int counter;
    int row, col, colstart;

    std::string commentPrefix = ";";
    std::string hexPrefix = "0x";
    std::string binPrefix = "0b";
    std::string charPrefix = "'";
};

#endif
