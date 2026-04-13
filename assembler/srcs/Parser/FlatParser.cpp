#include<vector>
#include<map>
#include<string>

#include<Parser/FlatParser.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<LexicalAnalyzer/Lexer.hpp>

ParseResult* FlatParser::parseTokens(std::vector<Token> tokens) {

    ParseResult* result = new ParseResult();

    result->symmap["$"] = program_offset;
    result->symmap["$$"] = section_offset;

    return result;
}