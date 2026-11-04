#include<string>
#include<vector>
#include<iostream>
#include<sstream>
#include<algorithm>
#include<utility>

#include<ErrorHandler/ErrorHandler.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>

std::vector<Error> ErrorBucket::errlist;
bool ErrorBucket::hasFailed = false;

std::string Error::errmsg() const
{
    std::ostringstream output;
    output << sourceFile << ':' << errorToken.row + 1 << ':' << errorToken.column + 1
           << ": " << (severity == ERR_WARNING ? "warning: " : "error: ") << errorMsg;
    if (!sourceLine.empty()) {
        output << '\n' << sourceLine << '\n'
               << std::string(static_cast<size_t>(std::max(0, errorToken.column)), ' ')
               << '^';
        if (!errorToken.tokenstr.empty()) output << " token '" << errorToken.tokenstr << '\'';
    }
    return output.str();
}

void ErrorBucket::addError(std::vector<Token> errline, Token t, std::string sourceFile,
                           std::string errMsg, std::string sourceLine)
{
    errlist.emplace_back(errline, t, sourceFile, errMsg, ERR_CRITICAL, std::move(sourceLine));
    hasFailed = true;
}

void ErrorBucket::addWarning(std::vector<Token> errline, Token t, std::string sourceFile,
                             std::string errMsg, std::string sourceLine)
{
    errlist.emplace_back(errline, t, sourceFile, errMsg, ERR_WARNING, std::move(sourceLine));
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

void ErrorBucket::reportErrors(std::ostream& output)
{
    for (const Error& error : errlist) output << error.errmsg() << '\n';
}

void ErrorBucket::clear()
{
    errlist.clear();
    hasFailed = false;
}

bool ErrorBucket::hasErrorAt(const std::string& sourceFile, int row)
{
    for (const Error& error : errlist) {
        if (error.severity == ERR_CRITICAL && error.sourceFile == sourceFile &&
            error.errorToken.row == row) {
            return true;
        }
    }
    return false;
}