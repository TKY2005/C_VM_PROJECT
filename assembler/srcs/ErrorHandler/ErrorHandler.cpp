#include<string>
#include<vector>

#include<ErrorHandler/ErrorHandler.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>

std::vector<Error> ErrorBucket::errlist;
bool ErrorBucket::hasFailed = false;

void ErrorBucket::addError(std::vector<Token> errline, Token t, std::string sourceFile, std::string errMsg)
{
    errlist.emplace_back(errline, t, sourceFile, errMsg, ERR_CRITICAL);
    hasFailed = true;
}

void ErrorBucket::addWarning(std::vector<Token> errline, Token t, std::string sourceFile, std::string errMsg)
{
    errlist.emplace_back(errline, t, sourceFile, errMsg, ERR_WARNING);
}

void ErrorBucket::addWarning(std::string sourceFile, std::string errMsg) {
    std::vector<Token> dummy;
    Token dummy2;
    errlist.emplace_back(sourceFile, errMsg);
}

std::vector<Error> ErrorBucket::getErrorList() {
    return errlist;
}
std::vector<Error> ErrorBucket::getCriticalErrors() {
    std::vector<Error> errs;
    for(int i = 0; i < errlist.size(); i++) {
        if (errlist[i].severity == ERR_CRITICAL) errs.push_back(errlist[i]);
    }
    return errs;
}

std::vector<Error> ErrorBucket::getWarnings() {
    std::vector<Error> warns;
    for(int i = 0; i < errlist.size(); i++) {
        if (errlist[i].severity == ERR_WARNING) warns.push_back(errlist[i]);
    }
    return warns;
}