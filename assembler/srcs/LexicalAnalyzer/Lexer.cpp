#include<string>
#include<iostream>
#include<vector>

#include "../../headers/LexicalAnalyzer/Lexer.hpp"
#include "../../headers/ArchInfo.hpp"

using std::string;

string Lexer::delimeters = ",[]_\n ";

MainType Lexer::decideMainType(string tok) {

    if (isIns(tok)) return MainType::INS;
    else if (isReg(tok)) return MainType::REG;
    else if (isOperator(tok[0])) return MainType::OPER;
    else if (tok[0] == '.') return MainType::DECL;
    else if (isDir(tok)) return MainType::DIR;

    else if (isAlpha(tok[0]) || isSeperator(tok[0])) {
        bool sym = true;
        for(int i = 0; i < tok.length(); i++) {
                if (!isalnum(tok[i]) && !isSeperator(tok[i])) {
                sym = false;
                break;
            }
        }
        if (sym) return MainType::SYM;
    }
    else if (isDigit(tok[0])) {

        bool digit = true;
        for(int i = 0; i < tok.length(); i++) {
            if (!isDigit(tok[i])) {
                digit = false;
                break;
            }
        }
        if (digit) return MainType::NUM;
    }
    return MainType::UNDEFINED;
}
SubType Lexer::decideSubType(string tok, MainType t) {

    tok = toLowerCase(tok);
    if (t == MainType::REG) {

        if (tok[0] >= 'a' && tok[0] <= 'f') {
            if (tok[1] == 'l' || tok[1] == 'h') return SubType::REG8;
            else if (tok[1] == 'x') return SubType::REG16;
            else return SubType::NONE;

        } else {
            for(int i = ArchInfo::dwrstart; i < ArchInfo::reglist.size(); i++) if (tok == ArchInfo::reglist[i]) return SubType::REG32;
            return SubType::NONE;
        }
    }
    else if (t == MainType::DIR) {
        
        if (tok == "org") return SubType::DIR_ORG;
        else if (tok == "section") return SubType::DIR_SECTION;
        else if (tok == "equ") return SubType::DIR_EQU;
        else if (tok == "byte") return SubType::DIR_BYTE;
        else if (tok == "word") return SubType::DIR_WORD;
        else if (tok == "defb") return SubType::DIR_DEFB;
        else if (tok == "defw") return SubType::DIR_DEFW;
        else if (tok == "resb") return SubType::DIR_RESB;
        else if (tok == "resw") return SubType::DIR_RESW;
        else return SubType::NONE;
    }
    else if (t == MainType::OPER) {
        if (tok[0] == '+') return SubType::OPER_ADD;
        if (tok[0] == '-') return SubType::OPER_SUB;
        if (tok[0] == '*') return SubType::OPER_MUL;
        if (tok[0] == '/') return SubType::OPER_DIV;
        else return SubType::NONE;
    }
    return SubType::NONE;
}

string Lexer::toLowerCase(string s) {
    
    for(int i = 0; i < s.length(); i++) {
        s[i] = tolower(s[i]);
    }
    return s;
}

bool Lexer::isDir(string tok) {
	tok = toLowerCase(tok);
	for(int i = 0; i < ArchInfo::dirList.size(); i++) if (tok == ArchInfo::dirList[i]) return true;
	return false;
}
bool Lexer::isDigit(char c) {
    return (c >= '0' && c <= '9');
}
bool Lexer::isPrintable(char c) {
    return (c >= 0x20 && c <= 0x7e);
}
bool Lexer::isAlpha(char c) {
    return ( ( c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') );
}
bool Lexer::isSeperator(char c) {
    return c == '_';
}
bool Lexer::isDelim(char c) {
    for(int i = 0; i < Lexer::delimeters.length(); i++) if (c == Lexer::delimeters[i]) return true;
    return false;
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isIns(string tok) {
    
    tok = Lexer::toLowerCase(tok);
    for(const auto& pair : ArchInfo::insmap) {
        if (pair.first == tok) return true;
    }
    return false;
}
bool Lexer::isReg(string tok) {

    tok = toLowerCase(tok);
    
    for(int i = 0; i < ArchInfo::reglist.size(); i++) {
        if (tok == ArchInfo::reglist[i])
             return true;
    }
    return false;
}
bool Lexer::isOperator(char c) {

    for(int i = 0; i < ArchInfo::operators.length(); i++) if (c == ArchInfo::operators[i]) return true;
    return false;
}

string Lexer::mainTypeToStr(MainType t) {
    switch (t)
    {
        case MainType::NUM:
        return "NUMBER";
        break;

        case MainType::STR:
        return "STRING";
        break;

        case MainType::INS:
        return "INSTRUCTION";
        break;
        
        case MainType::REG:
        return "REGISTER";
        break;
        
        case MainType::DIR:
        return "DIRECTIVE";
        break;
        
        case MainType::SYM:
        return "SYMBOL";
        break;
        
        case MainType::DECL:
        return "SYMBOL DECLARATION";
        break;
        
        case MainType::OPER:
        return "OPERATOR";
        break;
        
        case MainType::COMMA:
        return "COMMA";
        break;
        
        case MainType::NEWLINE:
        return "NEWLINE";
        break;

        case MainType::OPEN_BRACE:
        return "OPER BRACE";
        break;
        
        case MainType::CLOSE_BRACE:
        return "CLOSE BRACE";
        break;

        case MainType::SEMICOLON:
        return "SEMI COLON";
        break;
        
        case MainType::UNDEFINED:
        return "UNDEFINED";
        break;

        case MainType::ENOF:
        return "END OF FILE";
        break;

        default:
        return "UNKOWN TOKEN TYPE!";
        break;
    }
}
string Lexer::subTypeToStr(SubType t) {

    switch(t) {
        case SubType::REG8:
        return "8-BIT REGISTER";
        break;

        case SubType::REG16:
        return "16-BIT REGISTER";
        break;
        
        case SubType::REG32:
        return "32-BIT REGISTER";
        break;

        case SubType::DIR_ORG:
        return "ORG DIRECTIVE";
        break;
        
        case SubType::DIR_BYTE:
        return "BYTE DIRECTIVE";
        break;
        
        case SubType::DIR_WORD:
        return "WORD DIRECTIVE";
        break;

        case SubType::DIR_DEFB:
        return "DEFB DIRECTIVE";
        break;

        case SubType::DIR_DEFW:
        return "DEFW DIRECTIVE";
        break;

        case SubType::DIR_RESB:
        return "RESB DIRECTIVE";
        break;
        
        case SubType::DIR_RESW:
        return "RESW DIRECTIVE";
        break;

        case SubType::DIR_SECTION:
        return "SECTION DIRECTIVE";
        break;      

        case SubType::OPER_ADD:
        return "ADD OPERATOR";
        break;

        case SubType::OPER_SUB:
        return "SUB OPERATOR";
        break;

        case SubType::OPER_MUL:
        return "MUL OPERATOR";
        break;

        case SubType::OPER_DIV:
        return "DIV OPERATOR";
        break;

        case SubType::NONE:
        return "NONE";
        break;

        default:
        return "UNKOWN SUBTYPE!";
        break;
    }
}