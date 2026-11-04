#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include<vector>
#include<string>
#include<iosfwd>
#include<utility>

#include<LexicalAnalyzer/Tokenizer.hpp>

#define ERR_WARNING 0
#define ERR_CRITICAL 1

class Error {
    public:
    std::vector<Token> line;
    Token errorToken;
    std::string sourceFile;
    std::string sourceLine;
    std::string errorMsg;
    int severity = ERR_WARNING;

    Error(std::vector<Token> line, Token t, std::string source, std::string errMsg, int severity,
          std::string sourceLine = "") {
        this->line = line;
        this->errorToken = t;
        this->sourceFile = source;
        this->errorMsg = errMsg;
        this->severity = severity;
        this->sourceLine = std::move(sourceLine);
    }
    Error(std::string source, std::string errMsg) {
        this->sourceFile = source;
        this->errorMsg = errMsg;
    }
    std::string errmsg() const;
};

class ErrorBucket {
    public:
    static std::vector<Error> errlist;
    static bool hasFailed;

    ErrorBucket() = default;

    static void addError(std::vector<Token> errline, Token t, std::string sourceFile,
                         std::string errMsg, std::string sourceLine = "");
    static void addWarning(std::vector<Token> errline, Token t, std::string sourceFile,
                           std::string errMsg, std::string sourceLine = "");
    static void addWarning(std::string sourceFile, std::string errMsg);
    static std::vector<Error> getErrorList();
    static std::vector<Error> getCriticalErrors();
    static std::vector<Error> getWarnings();
    static void reportErrors(std::ostream& output);
    static void clear();
    static bool hasErrorAt(const std::string& sourceFile, int row);
};

#endif