#include<vector>
#include<map>
#include<string>
#include<cstdint>

#include<Parser/FlatParser.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<LexicalAnalyzer/Lexer.hpp>
#include<ErrorHandler/ErrorHandler.hpp>
#include<Assembler.hpp>

ParseResult* FlatParser::parseTokens(std::vector<Token> tokens) {

    ParseResult* result = new ParseResult();
    ParseResult& r = *result;


    progLines = extractLines(tokens);

    // first pass: calculate symbol offsets and instruction lengths

    for(int i = 0; i < progLines.size(); i++) {
        std::vector<Token> currentLine = progLines[i].tokens;

            for(int j = 0; j < currentLine.size(); j++){

            if (currentLine[j].maintype == MainType::DIR){
                resolveDirective(std::vector<Token>(currentLine.begin() + j, currentLine.end()), r);
                break;
            }
            if (currentLine[j].maintype == MainType::INS){
                program_offset += calculateInstructionLength(currentLine);
                break;
            }
            if (currentLine[j].maintype == MainType::SYM){
                result->symmap[currentLine[j].tokenstr] = program_offset;
            }
            if (currentLine[j].maintype == MainType::DECL){
                result->symmap[currentLine[j].tokenstr.substr(1, std::string::npos)] = program_offset;
            }
        }
        section_offset = program_offset - section_begin;
    }

    return result;
}

uint32_t FlatParser::calculateInstructionLength(std::vector<Token> insParts) {
    
    std::vector<std::vector<Token>> operands = extractOperands(insParts);
    int len = 0;
    len++; // opcode
    if (operands.size() == 0) return len;
    else if (operands.size() == 1) len += calcLenSingleOperand(operands);
    else if (operands.size() == 2) len += calcLenTwoOperand(operands);
    else {
        ErrorBucket::addError(getLine(operands[2][0].row), operands[2][0], Assembler::filename, 
        "Too many instruction operands.");
    }
    return len;
}

void FlatParser::resolveDirective(std::vector<Token> dir, ParseResult& r) {

    // NOTE: all expressions used in ORG, TIMES, and RESX directives must be absolute.
    // and all symbols used in these expressions must be already defined before evaluation time.
    if (dir[0].subtype == SubType::DIR_ORG) {

        if (checkSyntax_DIR_ORG(dir)){
            std::vector<Token> expr = extractExpression(dir);
            program_offset = evalExpr(expr);
        }
    }

    else if(dir[0].subtype == SubType::DIR_SECTION) {

        if(checkSyntax_DIR_SECTION(dir)) {
            r.symmap[dir[1].tokenstr] = program_offset;
            r.sections[dir[1].tokenstr] = program_offset;
            section_offset = 0;
            section_begin = program_offset;
        }
    }

    else if (isDataDefenition(dir[0])) {

        if (checkSyntax_DIR_DATA_DEF(dir)) {
            int scale = 0;

            if (isReservationDirective(dir[0])) {
                if (checkSyntax_DIR_RES(dir)) {
                    std::vector<Token> expr = extractExpression(dir);
                    program_offset += evalExpr(expr);
                }
            }

            else {
                std::vector<Token> data = extractData(dir);
                for(int i = 0; i < data.size(); i++) {
                    if (data[i].maintype == MainType::STR) program_offset += getStrLen(dir, scale);
                    else program_offset += scale;
                }
            }
        }
    }
}