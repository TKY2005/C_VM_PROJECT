#include<Parser/Parser.hpp>
#include<Visitors/Visitors.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<ArchInfo.hpp>

#include<memory>
#include<string>
#include<vector>
#include<cstdint>
#include<stdexcept>
#include<utility>

std::any Binary::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Unary::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Number::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Grouping::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Instruction::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any DataNode::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any DirORG::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any DirSection::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any DirTimes::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any MemExpr::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any StringNode::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Register::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Symbol::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any ResNode::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Special::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any Declaration::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}
std::any EquNode::accept(NodeVisitor& v, AsmContext& c) {
    return v.visit(*this, c);
}


std::unique_ptr<ParseResult> Parser::parse() {

    setProgramLines(extractLines());

    std::unique_ptr<ParseResult> result = std::unique_ptr<ParseResult>(new ParseResult());

    if (numLines() == 0) return result;
    setStartLine();
    for(int i = 0; i < numLines(); i++) {
        try {
            std::unique_ptr<ParseObject> l = line();
            if (!l) {
                Token errorToken = isAtEndOfLine() ? current() : operationLine[numToken];
                const std::string sourceLine =
                    errorToken.row >= 0 && static_cast<size_t>(errorToken.row) < sourceLines.size()
                        ? sourceLines[errorToken.row] : "";
                if (!ErrorBucket::hasErrorAt(sourceFile, errorToken.row)) {
                    ErrorBucket::addError(
                        currentLine(), errorToken, sourceFile,
                        "could not parse source line", sourceLine
                    );
                }
            }
            else {
                if (!operationLine.empty()) l->location = operationLine.front();
                if (l->location.row >= 0 && static_cast<size_t>(l->location.row) < sourceLines.size()) {
                    l->sourceLine = sourceLines[l->location.row];
                }
                result->parsedLines.push_back(std::move(l));
            }
        }
        catch (const std::exception& error) {
            const Token errorToken = isAtEndOfLine() ? current() : operationLine[numToken];
            const std::string sourceLine =
                errorToken.row >= 0 && static_cast<size_t>(errorToken.row) < sourceLines.size()
                    ? sourceLines[errorToken.row] : "";
            if (!ErrorBucket::hasErrorAt(sourceFile, errorToken.row)) {
                ErrorBucket::addError(
                    currentLine(), errorToken, sourceFile, error.what(), sourceLine
                );
            }
        }
        if (i < numLines() - 1) nextLine();
    }

    return result;
}

std::unique_ptr<ParseObject> Parser::line() {
    
    if (check(MainType::INS)) return instruction();

    else if (
        compareSubTypes({SubType::DIR_ORG, SubType::DIR_SECTION, SubType::DIR_TIMES})
    ) {return directive();}

    else if (check(MainType::DECL)) {
        std::string symbol = advance().tokenstr;
        if (checkSubType(SubType::DIR_EQU) ||
            checkSubType(SubType::DIR_DEFB) ||
            checkSubType(SubType::DIR_DEFW) ||
            checkSubType(SubType::DIR_DEFDW) ||
            checkSubType(SubType::DIR_RESB) ||
            checkSubType(SubType::DIR_RESW) ||
            checkSubType(SubType::DIR_RESDW)) {
            return dataDelcaration(symbol);
        }
        return std::unique_ptr<ParseObject>(new Declaration(symbol));
    }

    else return dataDelcaration();
}

std::unique_ptr<ParseObject> Parser::instruction() {

    uint8_t opcode = ArchInfo::insmap[advance().tokenstr];

    if (isAtEndOfLine()) return std::unique_ptr<ParseObject>(new Instruction(opcode));

    else {
        std::vector<std::unique_ptr<ParseObject>> operands;
        auto parseLocatedOperand = [this]() {
            const Token location = current();
            std::unique_ptr<ParseObject> parsed = operand();
            if (parsed) {
                parsed->location = location;
                if (location.row >= 0 && static_cast<size_t>(location.row) < sourceLines.size()) {
                    parsed->sourceLine = sourceLines[location.row];
                }
            }
            return parsed;
        };
        operands.push_back(parseLocatedOperand());
        while (matchAndAdvance({MainType::COMMA})) {
            operands.push_back(parseLocatedOperand());
        }
        return std::unique_ptr<ParseObject>(new Instruction(opcode, std::move(operands)));
    }
}

std::unique_ptr<ParseObject> Parser::directive() {
    Token cur = advance();
    if (cur.maintype == MainType::SYM) return parseTimes();
    else{
        switch(cur.subtype) {

            case SubType::DIR_ORG:
            {
                return parseOrg();
            }

            case SubType::DIR_SECTION:
            {
                return parseSection();
            }
            case SubType::DIR_TIMES:
            {
                return parseTimes();
            }
            default:
            return nullptr;
        }
    }
}

std::unique_ptr<ParseObject> Parser::parseOrg() {
    std::unique_ptr<ParseObject> expr = expression();
    return std::unique_ptr<ParseObject>(new DirORG(std::move(expr)));
}

std::unique_ptr<ParseObject> Parser::parseSection() {
    Token name = advance();
    if (name.maintype != MainType::SYM) return nullptr; // TODO: throw a parse error.
    return std::unique_ptr<ParseObject>(new DirSection(name.tokenstr));
}

std::unique_ptr<ParseObject> Parser::parseTimes() {
    std::string sym;
    if (matchAndAdvance({MainType::SYM})) sym = previous().tokenstr;
    std::unique_ptr<ParseObject> expr = expression();
    std::unique_ptr<ParseObject> l = line();

    return std::unique_ptr<ParseObject>(new DirTimes(sym, std::move(expr), std::move(l)));
}

std::unique_ptr<ParseObject> Parser::dataDelcaration(std::string symbol) {
    
    std::string sym = std::move(symbol);
    uint8_t size = 0;
    if (sym.empty() && matchAndAdvance({MainType::SYM})) sym = previous().tokenstr;

    if (!sym.empty() && matchSubTypeAndAdvance({SubType::DIR_EQU})) {
        return std::unique_ptr<ParseObject>(new EquNode(sym, expression()));
    }

    if (matchSubTypeAndAdvance({
        SubType::DIR_DEFB, SubType::DIR_DEFW, SubType::DIR_DEFDW})) {

        size = getDirectiveSize(previous().subtype);

        std::vector<std::unique_ptr<ParseObject>> list = dataList();

        return std::unique_ptr<ParseObject>(new DataNode(sym, std::move(list), size));
    }

    else if (matchSubTypeAndAdvance({SubType::DIR_RESB, SubType::DIR_RESW, SubType::DIR_RESDW})) {
        size = getDirectiveSize(previous().subtype);
        std::unique_ptr<ParseObject> expr = expression();

        return std::unique_ptr<ParseObject>(new ResNode(sym, std::move(expr), size));
    }
    else return nullptr;
}

std::vector<std::unique_ptr<ParseObject>> Parser::dataList() {

    std::vector<std::unique_ptr<ParseObject>> list;

    list.push_back(data());

    while (matchAndAdvance({MainType::COMMA})) list.push_back(data());

    return list;
}

std::unique_ptr<ParseObject> Parser::data() {
    
    if (matchAndAdvance({MainType::STR})) {
        Token token = previous();
        std::unique_ptr<ParseObject> string(new StringNode(token.tokenstr));
        string->location = token;
        if (token.row >= 0 && static_cast<size_t>(token.row) < sourceLines.size()) {
            string->sourceLine = sourceLines[token.row];
        }
        return string;
    }

    else return expression();
}

std::unique_ptr<ParseObject> Parser::operand() {
    
    if (compareTypes({MainType::OPEN_BRACE}) || compareSubTypes({SubType::DIR_BYTE, SubType::DIR_WORD, SubType::DIR_DWORD}))
        return memexpr();
    else return expression();
}

std::unique_ptr<ParseObject> Parser::memexpr() {

    uint8_t size = 4;
    std::unique_ptr<ParseObject> expr = nullptr;
    if (matchSubTypeAndAdvance({SubType::DIR_BYTE, SubType::DIR_WORD, SubType::DIR_DWORD})) {
        size = getDirectiveSize(previous().subtype);
    }

    consume(MainType::OPEN_BRACE, "expected an opening bracket for a memory expression");
    expr = expression();
    consume(MainType::CLOSE_BRACE, "expected a closing bracket for a memory expression");
    return std::unique_ptr<ParseObject>(new MemExpr(std::move(expr), size));
}


std::unique_ptr<ParseObject> Parser::expression() {
    return addition();
}

std::unique_ptr<ParseObject> Parser::addition() {
    std::unique_ptr<ParseObject> expr = multiplication();

    while (matchSubTypeAndAdvance({SubType::OPER_ADD, SubType::OPER_SUB})) {
        Token oper = previous();
        std::unique_ptr<ParseObject> right = multiplication();
        expr = std::unique_ptr<ParseObject>(new Binary(std::move(expr), oper, std::move(right))); 
    }
    return expr;
}

std::unique_ptr<ParseObject> Parser::multiplication() {
    std::unique_ptr<ParseObject> expr = unary();
    while (matchSubTypeAndAdvance({SubType::OPER_MUL, SubType::OPER_DIV})) {
        Token oper = previous();
        std::unique_ptr<ParseObject> right = unary();
        expr = std::unique_ptr<ParseObject>(new Binary(std::move(expr), oper, std::move(right)));
    }
    return expr;
}

std::unique_ptr<ParseObject> Parser::unary() {
    if (matchSubTypeAndAdvance({SubType::OPER_ADD, SubType::OPER_SUB})) {
        Token oper = previous();
        std::unique_ptr<ParseObject> right = unary();
        return std::unique_ptr<ParseObject>(new Unary(oper, std::move(right)));
    }
    else return primary();
}

std::unique_ptr<ParseObject> Parser::primary() {
    if (matchAndAdvance({MainType::NUM})) return std::unique_ptr<ParseObject>(new Number(std::stoul(previous().tokenstr)));
    else if (matchAndAdvance( {MainType::SYM} )) return std::unique_ptr<ParseObject>(new Symbol(previous().tokenstr));
    else if (matchAndAdvance( {MainType::SPECIAL} )) return std::unique_ptr<ParseObject>(new Special(previous().subtype));
    else if (matchAndAdvance({MainType::REG})) {
        const Token token = previous();
        std::unique_ptr<ParseObject> reg(new Register(
            previous().tokenstr,
            previous().subtype,
            ArchInfo::reg_select_map[previous().tokenstr]
        ));
        reg->location = token;
        if (token.row >= 0 && static_cast<size_t>(token.row) < sourceLines.size()) {
            reg->sourceLine = sourceLines[token.row];
        }
        return reg;
    }
    
    else if (matchAndAdvance({MainType::OPEN_PAREN})) {
        std::unique_ptr<ParseObject> expr = expression();
        consume(MainType::CLOSE_PAREN, "expected a closing parenthesis");

        return std::unique_ptr<ParseObject>(new Grouping(std::move(expr)));
    }

    else {
        parseError("expected an expression");
    }
}

[[noreturn]] void Parser::parseError(const std::string& message)
{
    const Token errorToken = current();
    const std::string sourceLine =
        errorToken.row >= 0 && static_cast<size_t>(errorToken.row) < sourceLines.size()
            ? sourceLines[errorToken.row] : "";
    ErrorBucket::addError(currentLine(), errorToken, sourceFile, message, sourceLine);
    throw std::runtime_error(message);
}