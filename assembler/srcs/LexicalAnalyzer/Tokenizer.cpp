#include<iostream>
#include<string>
#include<cstdint>
#include<map>
#include<vector>
#include<optional>
#include<sstream>
#include<regex>
#include<cctype>
#include<unordered_map>

#include "../../headers/LexicalAnalyzer/Tokenizer.hpp"

using std::string, std::vector, std::optional;

vector<Token> Tokenizer::tokenize(string source) {

    resetGlobalState();

    src = source;

    while (!isAtEnd()) {
        // single char tokens
        colstart = col;
        
        char cur = src[counter];

        // handle single character tokens
        if (cur == ' ' || cur == '\t') skip();

        else if (cur == '\n') {
            consume();
            row++;
            col = 0;
            colstart = 0;
            toklist.emplace_back(current, MainType::NEWLINE, SubType::NONE, row, colstart);
        }

        else if (cur == '[') {
            consume();
            toklist.emplace_back(current, MainType::OPEN_BRACE, SubType::NONE, row, colstart);
        }
        
        else if (cur == ']') {
            consume();
            toklist.emplace_back(current, MainType::CLOSE_BRACE, SubType::NONE, row, colstart);
        }

        else if (cur == ',') {
            consume();
            toklist.emplace_back(current, MainType::COMMA, SubType::NONE, row, colstart);
        }

        else if (cur == '$') {
            consume();
            toklist.emplace_back(current, MainType::SYM, SubType::NONE, row, colstart);
        }
        else if (cur == ':') {
            consume();
            toklist.emplace_back(current, MainType::SEMICOLON, SubType::NONE, row, colstart);
        }

        else if (Lexer::isOperator(cur)) {
            consume();
            SubType s = Lexer::decideSubType(current, MainType::OPER);
            toklist.emplace_back(current, MainType::OPER, s, row, colstart);
        }

        // handle multi character tokens
        else if (isdigit(cur)) {
            while ( isdigit( getCurChar() ) && !isAtEnd() ) consume();
            toklist.emplace_back(current, MainType::NUM, SubType::NONE, row, colstart);
        }

        else if (cur == '"') {
            skip();
            do {
                consume();
                if (getCurChar() == '"') break;
            } while (getCurChar() != '"' && !isAtEnd());
            skip();
            toklist.emplace_back(current, MainType::STR, SubType::NONE, row, colstart);
        }

        else if (cur == '.') {
            do {
                consume();
            } while ( ( isalnum(getCurChar()) || Lexer::isSeperator(getCurChar()) ) && !isAtEnd());
            toklist.emplace_back(current, MainType::DECL, SubType::NONE, row, colstart);
        }

        else if (Lexer::isAlpha(cur)) {
            while (((isalnum(getCurChar())) || Lexer::isSeperator(getCurChar())) && !isAtEnd()) consume();
            MainType m = Lexer::decideMainType(current);
            SubType s = Lexer::decideSubType(current, m);
            toklist.emplace_back(current, m, s, row, colstart);
        }

        else {
            consume();
            toklist.emplace_back(current, MainType::UNDEFINED, SubType::NONE, row, colstart);
        }

        current.clear();
    }

    toklist.emplace_back("", MainType::ENOF, SubType::NONE, row, col);
    return toklist;
}

bool Tokenizer::isAtEnd() {
    return counter >= src.length();
}

void Tokenizer::consume() {
    current.push_back(src[counter++]);
    col++;
}

void Tokenizer::skip() {
    counter++;
    col++;
}

char Tokenizer::getCurChar() {
    return src[counter];
}

void Tokenizer::resetGlobalState() {
    counter = 0;
    row = 0;
    col = 0;
    colstart = 0;
    toklist.clear();
    src.clear();
    current.clear();
}

// Convert binary string (without 0b) to decimal
long long binToDec(const std::string& bin) {
    long long value = 0;
    for (char c : bin) {
        value = value * 2 + (c - '0');
    }
    return value;
}

// Convert hex string (without 0x) to decimal
long long hexToDec(const std::string& hex) {
    return std::stoll(hex, nullptr, 16);
}

// Generic regex replace with custom transformer
template<typename Transformer>
std::string regexReplaceWith(const std::string& s, const std::regex& r, Transformer transform) {
    std::string result;
    std::sregex_iterator it(s.begin(), s.end(), r);
    std::sregex_iterator end;

    size_t last = 0;
    for (; it != end; ++it) {
        const std::smatch& m = *it;
        result.append(s, last, m.position() - last);
        result += transform(m);
        last = m.position() + m.length();
    }
    result.append(s, last, std::string::npos);
    return result;
}

std::string Tokenizer::preProcessCode(const std::string& input) {
    std::stringstream ss(input);
    std::string line;
    std::string output;

    std::regex charRegex("'(.)'");
    std::regex hexRegex("0x([0-9A-Fa-f]+)");
    std::regex binRegex("0b([01]+)");

    std::unordered_map<std::string, std::string> defines;

    // First pass: preprocess everything and collect macros
    std::vector<std::string> processedLines;

    while (std::getline(ss, line)) {

        // Trim for checks
        std::string trimmed = line;
        trimmed.erase(trimmed.begin(),
                      std::find_if(trimmed.begin(), trimmed.end(),
                                   [](unsigned char c){ return !std::isspace(c); }));

        // Skip full-line comments
        if (!trimmed.empty() && trimmed[0] == ';') {
            processedLines.push_back("");
            continue;
        }

        // Handle define (case-insensitive)
        {
            std::string lowered = trimmed;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

            if (lowered.rfind("define ", 0) == 0) {
                std::stringstream defStream(trimmed);
                std::string keyword, symbol, value;

                defStream >> keyword >> symbol;
                std::getline(defStream, value);

                // Trim leading spaces
                value.erase(value.begin(),
                            std::find_if(value.begin(), value.end(),
                                         [](unsigned char c){ return !std::isspace(c); }));

                // Preprocess the macro value BEFORE storing it
                value = regexReplaceWith(value, charRegex,
                    [](const std::smatch& m){
                        return std::to_string((unsigned char)m.str(1)[0]);
                    }
                );

                value = regexReplaceWith(value, hexRegex,
                    [](const std::smatch& m){
                        return std::to_string(hexToDec(m.str(1)));
                    }
                );

                value = regexReplaceWith(value, binRegex,
                    [](const std::smatch& m){
                        return std::to_string(binToDec(m.str(1)));
                    }
                );

                defines[symbol] = value;
                processedLines.push_back("");
                continue;
            }
        }

        // Remove leading whitespace
        line.erase(line.begin(),
                   std::find_if(line.begin(), line.end(),
                                [](unsigned char c){ return !std::isspace(c); }));

        // Remove trailing whitespace
        line.erase(
            std::find_if(line.rbegin(), line.rend(),
                        [](unsigned char c){ return !std::isspace(c); }).base(),
            line.end()
        );
                        

        // Remove inline comments
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Preprocess literals
        line = regexReplaceWith(line, charRegex,
            [](const std::smatch& m){
                return std::to_string((unsigned char)m.str(1)[0]);
            }
        );

        line = regexReplaceWith(line, hexRegex,
            [](const std::smatch& m){
                return std::to_string(hexToDec(m.str(1)));
            }
        );

        line = regexReplaceWith(line, binRegex,
            [](const std::smatch& m){
                return std::to_string(binToDec(m.str(1)));
            }
        );

        processedLines.push_back(line);
    }

    // Second pass: macro expansion
    for (std::string& ln : processedLines) {

        std::string rebuilt;
        size_t i = 0;

        while (i < ln.size()) {

            // Identifier token
            if (std::isalnum(ln[i]) || ln[i] == '_') {
                size_t start = i;
                while (i < ln.size() &&
                       (std::isalnum(ln[i]) || ln[i] == '_'))
                {
                    i++;
                }

                std::string token = ln.substr(start, i - start);

                if (defines.count(token)) {
                    rebuilt += defines[token];
                } else {
                    rebuilt += token;
                }
            }
            else {
                rebuilt += ln[i];
                i++;
            }
        }

        output += rebuilt + "\n";
    }

    return output;
}
