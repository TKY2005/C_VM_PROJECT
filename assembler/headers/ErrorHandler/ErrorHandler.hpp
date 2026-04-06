#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include<vector>
#include<string>

#include<LexicalAnalyzer/Tokenizer.hpp>

#define ERR_WARNING 0
#define ERR_CRITICAL 1

class Error {
    public:
    std::vector<Token> line;
    Token errorToken;
    std::string sourceFile;
    std::string errorMsg;
    int severity = ERR_WARNING;

    Error(std::vector<Token> line, Token t, std::string source, std::string errMsg, int severity) {
        this->line = line;
        this->errorToken = t;
        this->sourceFile = source;
        this->errorMsg = errMsg;
        this->severity = severity;
    }
    Error(std::string source, std::string errMsg) {
        this->sourceFile = source;
        this->errorMsg = errMsg;
    }
    std::string errmsg() {
        std::string s;
        std::string tokenstr;
        for(int i = 0; i < line.size(); i++) tokenstr.append(line[i].toString()).append(" ");
        s.append( (severity == ERR_WARNING) ? "Warning " : "Error " ).append("in file: ").append(sourceFile).append(" @ ")
        .append(std::to_string(errorToken.row))
        .append(":")
        .append(std::to_string(errorToken.column))
        .append("(").append(tokenstr).append(")").append(" ")
        .append("{").append(errorToken.toString()).append("}").append(" ")
        .append(": ").append(errorMsg);

        return s;
    }
};

class ErrorBucket {
    public:
    static std::vector<Error> errlist;
    static bool hasFailed;

    ErrorBucket() = default;

    static void addError(std::vector<Token> errline, Token t, std::string sourceFile, std::string errMsg);
    static void addWarning(std::vector<Token> errline, Token t, std::string sourceFile, std::string errMsg);
    static void addWarning(std::string sourceFile, std::string errMsg);
    static std::vector<Error> getErrorList();
    static std::vector<Error> getCriticalErrors();
    static std::vector<Error> getWarnings();
};

#endif