#include<map>
#include<vector>
#include<cstdint>
#include<string>

#include "../../headers/LexicalAnalyzer/Tokenizer.hpp"
#include "../../headers/Parser/Parser.hpp"
#include "../../headers/LexicalAnalyzer/Lexer.hpp"
#include "../../../shared/ISA_encoding_info.h"

using std::vector, std::map, std::string;


ParseResult* Parser::parseTokens(vector<Token> tokens) {
    
    string section = "";

    ParseResult* result = new ParseResult();
    result->symmap["$"] = program_offset;

    vector<ProgLine> lines = extractLines(tokens);

    vector<Token> currentLine;
    for(int i = 0; i < lines.size(); i++) {

        currentLine = lines[i].tokens;
        if (currentLine.size() == 0) continue;

        if (currentLine[0].maintype == MainType::DECL) {
            result->symmap[currentLine[0].tokenstr.substr(1, string::npos)] = program_offset;
        }

        else if (currentLine[0].maintype == MainType::SYM) {
            if (section == "DATA") {
                if (currentLine[1].subtype == SubType::DIR_EQU) {
                    uint32_t lhs = result->symmap[currentLine[2].tokenstr];
                    SubType oper = currentLine[3].subtype;
                    uint32_t rhs = result->symmap[currentLine[4].tokenstr];
                    uint32_t operresult;

                    // temporary.
                    switch(oper) {
                        case SubType::OPER_ADD:
                        operresult = lhs + rhs;
                        break;
                        case SubType::OPER_SUB:
                        operresult = lhs - rhs;
                        break;
                        case SubType::OPER_MUL:
                        operresult = lhs * rhs;
                        break;
                        case SubType::OPER_DIV:
                        operresult = lhs / rhs;
                        break;
                        default:
                        operresult = 0;
                        break;
                    }
                    ProgData* d = new ProgData(currentLine, 4, program_offset, operresult);
                    result->symmap[currentLine[0].tokenstr] = program_offset;
                    program_offset += d->len;
                    result->program_data.push_back(d);
                }
                else {
                    ProgData* d = parseData(currentLine);
                    result->program_data.push_back(d);
                    result->symmap[currentLine[0].tokenstr] = d->addr;
                }
            }
        }

        else if (currentLine[0].maintype == MainType::DIR) {

            if (currentLine[0].subtype == SubType::DIR_ORG) {
                
                if (currentLine[1].maintype == MainType::NUM) {
                    program_offset = (uint32_t) std::stoi(currentLine[1].tokenstr);
                }

                else if (currentLine[1].maintype == MainType::SYM) {
                    uint32_t base = result->symmap[currentLine[1].tokenstr];
                    if (currentLine[2].maintype == MainType::SEMICOLON) {
                        uint32_t offset = (uint32_t) std::stoi(currentLine[3].tokenstr);
                        program_offset = base + offset;
                    }
                    else program_offset = base;
                }
            }

            else if (currentLine[0].subtype == SubType::DIR_SECTION) {
                section = currentLine[1].tokenstr;
                result->symmap[section] = program_offset;
                result->sections[section] = program_offset;
            }

            else if (matchSubTypes(currentLine[0].subtype, {SubType::DIR_RESB, SubType::DIR_RESW}))
            {
                int scale = 0;
                if (currentLine[0].subtype == SubType::DIR_RESB) scale = 1;
                else if (currentLine[0].subtype == SubType::DIR_RESW) scale = 2;

                uint32_t amount = (uint32_t) std::stoi(currentLine[1].tokenstr);
                program_offset += amount * scale;
            }
        }

        else if (currentLine[0].maintype == MainType::INS) {
            if (section != "DATA"){
                ProgIns* ins = parseInstruction(currentLine);
                result->program_instructions.push_back(ins);
            }
        }
        result->symmap["$"] = program_offset;
    }
    result->code_section_addr = result->sections["CODE"];
    result->data_section_addr = result->sections["DATA"];
    return result;
}

ProgIns* Parser::parseInstruction(vector<Token> insparts) {
    ProgIns* insobj = new ProgIns(insparts);
    ProgIns& ins = *insobj;
    int length = 0;
    int& len = length;
    len += 1; // opcode byte
    
    if (insparts[0].maintype != MainType::INS)  {
        // todo : add error reporting mechanism
    }
    else {
        insobj->encoding_info->opcode = ArchInfo::insmap[insparts[0].tokenstr];
        vector<vector<Token>> operands = extractOperands(insparts);
        evaluateOperands(operands, ins, len);
    }
    ins.len = len;
    ins.addr = program_offset;
    program_offset += len;
    return insobj;
}

vector<vector<Token>> Parser::extractOperands(vector<Token> insparts) {
    vector<vector<Token>> operands;
    vector<Token> current_operand;

    for(int i = 1; i < insparts.size(); i++) {
        if (insparts[i].maintype == MainType::COMMA){
            operands.push_back(current_operand);
            current_operand.clear();
        }
        else current_operand.push_back(insparts[i]);
    }
    if (!current_operand.empty()) operands.push_back(current_operand);
    return operands;
}

vector<Token> extractData(vector<Token> dparts) {
    vector<Token> data;

    for(int i = 2; i < dparts.size(); i++) {
        if (dparts[i].maintype != MainType::COMMA) data.push_back(dparts[i]);
    }
    return data;
}

ProgData* Parser::parseData(vector<Token> dparts) {
    
    ProgData* d = new ProgData(dparts);
    int scale = 0;
    int length = 0;
    if (matchSubTypes(dparts[1].subtype, {SubType::DIR_DEFB, SubType::DIR_RESB})) scale = 1;
    else if (matchSubTypes(dparts[1].subtype, {SubType::DIR_DEFW, SubType::DIR_RESW})) scale = 2;

    if (dparts[1].subtype == SubType::DIR_RESB || dparts[1].subtype == SubType::DIR_RESW) {
        d->isRes = true;
        if (dparts[2].maintype != MainType::NUM) {
            // error reporting
        }
        else length += std::stoi(dparts[2].tokenstr) * scale;
    }
    else {
        vector<Token> data = extractData(dparts);

        for(int i = 0; i < data.size(); i++) {
            if (data[i].maintype == MainType::STR) {
                for(int j = 0; j < data[i].tokenstr.length(); j++) length += scale;
            }
            else if (data[i].maintype == MainType::NUM) length += scale;
        }
    }
    d = new ProgData(dparts, length, scale, program_offset);
    program_offset += length;
    return d;
}

vector<ProgLine> Parser::extractLines(vector<Token> tokens) {
    vector<ProgLine> lines;
    vector<Token> lineparts;
    int index = 0;
    while (tokens[index].maintype != MainType::ENOF) {
        if (tokens[index].maintype == MainType::NEWLINE) {
            lines.push_back(lineparts);
            lineparts.clear();
        }
        else lineparts.push_back(tokens[index]);
        index++;
    }
    return lines;
}

vector<Token> Parser::extractMemoryExpression(vector<Token> operand){
    vector<Token> memexpr;
    bool add = false;
    for(int i = 0; i < operand.size(); i++) {

        if (operand[i].maintype == MainType::OPEN_BRACE) add = true;

        if (add) memexpr.push_back(operand[i]);

        if (operand[i].maintype == MainType::CLOSE_BRACE) return memexpr;
    }
    return memexpr;
}

void Parser::evaluateOperands(vector<vector<Token>> operands, ProgIns& result, int& length) {
    
    if (operands.size() == 0) {
        result.hasOperands = false;
        result.operand_count = 0;
        result.len = length;
        result.addr = program_offset;
        return;
    }
    else if (operands.size() == 1) {
        result.hasOperands = true;
        result.operand_count = 1;
        length += 1; // operand types byte //
        evaluateDestinationOperand(operands[0], result, length);
    }
    else if (operands.size() == 2) {
        result.hasOperands = true;
        result.operand_count = 2;
        length += 1; // operand types byte //
        evaluateDestinationOperand(operands[0], result, length);
        evaluateSourceOperand(operands[1], result, length);
    }
}

void Parser::evaluateDestinationOperand(vector<Token> operand, ProgIns& result, int& length) {

    if (operand[0].maintype == MainType::SYM)  {
        if (result.operand_count == 1){
            result.hasSymbol = true;
            result.symbol = operand[0].tokenstr;
            length += 4; // symbols are allowed as destinations for single operand instructions //
            length--; // operand types byte won't be present //
            // TODO: lookahead to check if there's any displacement.
        }
        else {
            // TODO: Error reporting
        }
    }
    else if (operand[0].maintype == MainType::NUM){
        length += 4; // only allowed for instructions that may take an immediate for operand, no opertype byte needed.
        result.hasImmediate = true;
        result.immediateSize = 4;
        result.encoding_info->imm_val = (uint32_t) std::stoi(operand[0].tokenstr);
        length--; // operand types byte also won't be present
    }
    else if (operand[0].maintype == MainType::REG) {
        length += 1; // register select byte //
        
        switch(operand[0].subtype) {
            case SubType::REG8:
            result.encoding_info->opertype.dest_type = REG8;
            break;
            case SubType::REG16:
            result.encoding_info->opertype.dest_type = REG16;
            break;
            default:
            result.encoding_info->opertype.dest_type = REG32;
            break;
        }
        for(auto const& pair : ArchInfo::reg_select_map) {
            if (Lexer::toLowerCase(operand[0].tokenstr) == pair.first){
                result.encoding_info->regselect.dest = pair.second;
                break;
            }
        }
    }
    else if (isMemoryOperand(operand)){
        switch(operand[0].subtype) {
            case SubType::DIR_BYTE:
            result.encoding_info->opertype.dest_type = MEM8;
            break;
            case SubType::DIR_WORD:
            result.encoding_info->opertype.dest_type = MEM16;
            break;
            default:
            result.encoding_info->opertype.dest_type = MEM32;
            break;
        }
        length += 1; // displacement info byte (displacement value may come later.)
        result.encoding_info->opertype.mem_mode = X1SCALE;
        result.hasMemory = true;
        vector<Token> memoryexpr = extractMemoryExpression(operand);
        evaluateMemoryExpression(memoryexpr, result, length);
    }
}

void Parser::evaluateSourceOperand(vector<Token> operand, ProgIns& result, int& length) {

    if (operand[0].maintype == MainType::SYM) {
        length += 4;
        result.hasSymbol = true;
        result.symbol = operand[0].tokenstr;
    }

    else if (operand[0].maintype == MainType::REG) {
        if (result.encoding_info->opertype.dest_type == UNKNOWN)
            length += 1; // we should add the register select byte only if we haven't encountered it yet //
        
        switch(operand[0].subtype) {
            case SubType::REG8:
            result.encoding_info->opertype.src_type = REG8;
            break;
            case SubType::REG16:
            result.encoding_info->opertype.src_type = REG16;
            break;
            default:
            result.encoding_info->opertype.src_type = REG32;
            break;
        }
        // TODO: Check to see if the source register size > destination register size
        for(auto const& pair : ArchInfo::reg_select_map) {
            if (Lexer::toLowerCase(operand[0].tokenstr) == pair.first){
                result.encoding_info->regselect.src = pair.second;
                break;
            }
        }
    }
    else if (isMemoryOperand(operand)) {

        switch(operand[0].subtype) {
            case SubType::DIR_BYTE:
            result.encoding_info->opertype.src_type = MEM8;
            break;
            case SubType::DIR_WORD:
            result.encoding_info->opertype.src_type = MEM16;
            break;
            default:
            result.encoding_info->opertype.src_type = MEM32;
            break;
        }
        if (!result.hasMemory){
            result.hasMemory = true;
            length += 1; // displacement info byte
            result.encoding_info->opertype.mem_mode = X1SCALE;
        }
        vector<Token> memexpr = extractMemoryExpression(operand);
        evaluateMemoryExpression(memexpr, result, length);
    }
    else if (operand[0].maintype == MainType::NUM) {
        result.hasImmediate = true;
        result.encoding_info->opertype.src_type = NUMBER;
        uint32_t num = (uint32_t) std::stoi(operand[0].tokenstr);
        
        switch (result.encoding_info->opertype.dest_type) // immediate size matches the destination type.
        {
        case REG8 : case MEM8:
            length += 1;
            result.immediateSize = 1;
            break;
        case REG16 : case MEM16:
            length += 2;
            result.immediateSize = 2;
            break;
        case REG32 : case MEM32:
            length += 4;
            result.immediateSize = 4;
            break;
        
        default:
            break;
        }
        result.encoding_info->imm_val = num;
    }
}

void Parser::evaluateMemoryExpression(vector<Token> expr, ProgIns& result, int& length) {

    for(int i = 0; i < expr.size(); i++) {

        if (expr[i].maintype == MainType::REG) {
            result.encoding_info->dispinfo.reg_enable = 1;
            string regname = Lexer::toLowerCase(expr[i].tokenstr);
            bool found = false;
            for(auto const& pair : ArchInfo::index_select_map) {
                if (regname == pair.first) {
                    result.encoding_info->dispinfo.index = pair.second;
                    found = true;
                    break;
                }
            }
            if (!found) {
                for(auto const& pair : ArchInfo::base_select_map) {
                    if (regname == pair.first) {
                        result.encoding_info->dispinfo.base = pair.second;
                        break;
                    }
                }
            }
        }
        else if (expr[i].maintype == MainType::NUM) {
            if (!result.hasDisplacement) {
                result.hasDisplacement = true;
                result.encoding_info->dispinfo.disp_enable = 1;
                length += 4; // displacement value
                int val = 0;
                if (expr[i - 1].subtype == SubType::OPER_SUB) val = std::stoi("-" + expr[i].tokenstr);
                else val = std::stoi(expr[i].tokenstr);

                result.encoding_info->disp_val = val;
            }
        }
        else if (expr[i].subtype == SubType::OPER_MUL) {
            switch(std::stoi(expr[i + 1].tokenstr)) {
                case 1:
                result.encoding_info->opertype.mem_mode = X1SCALE;
                break;
                case 2:
                result.encoding_info->opertype.mem_mode = X2SCALE;
                break;
                case 4:
                result.encoding_info->opertype.mem_mode = X4SCALE;
                break;
                default:
                // replace this with error reporting //
                result.encoding_info->opertype.mem_mode = NOMEM;
                break;
            }
        }
    }
}

bool Parser::matchTypes(MainType t, std::initializer_list<MainType> a) {
    for(MainType type : a) {
        if (t == type) return true;
    }
    return false;
}

bool Parser::matchSubTypes(SubType t, std::initializer_list<SubType> a) {

    for(SubType type : a) {
        if (t == type) return true;
    }
    return false;
}

bool Parser::isMemoryOperand(vector<Token> operand) {
    return (operand[0].maintype == MainType::OPEN_BRACE || 
        ( matchSubTypes(operand[0].subtype, {SubType::DIR_BYTE, SubType::DIR_WORD}) 
        && operand[1].maintype == MainType::OPEN_BRACE ));
}
