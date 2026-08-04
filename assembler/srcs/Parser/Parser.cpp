#include<Parser/Parser.hpp>
#include<Parser/Visitors/Visitors.hpp>
#include<LexicalAnalyzer/Tokenizer.hpp>
#include<ArchInfo.hpp>

#include<memory>
#include<string>
#include<vector>
#include<cstdint>

void Binary::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Unary::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Literal::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Grouping::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Register::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Symbol::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Declaration::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Special::accept(NodeVisitor& v) {
    v.visit(*this);
}
void Instruction::accept(NodeVisitor& v) {
    v.visit(*this);
}
void DirORG::accept(NodeVisitor& v) {
    v.visit(*this);
}
void DirSection::accept(NodeVisitor& v) {
    v.visit(*this);
}
void DirTimes::accept(NodeVisitor& v) {
    v.visit(*this);
}
void DataNode::accept(NodeVisitor& v) {
    v.visit(*this);
}
void ResNode::accept(NodeVisitor& v) {
    v.visit(*this);
}
void StringNode::accept(NodeVisitor& v) {
    v.visit(*this);
}
void MemExpr::accept(NodeVisitor& v) {
    v.visit(*this);
}


std::unique_ptr<ParseObject> Parser::line() {
    
    skipNewLineUntilNextToken();

    if (check(MainType::INS)) return instruction();

    else if (
        compareSubTypes({SubType::DIR_ORG, SubType::DIR_SECTION, SubType::DIR_TIMES})
    ) {return directive();}

    else if (check(MainType::DECL)) return std::unique_ptr<ParseObject>(new Declaration(current().tokenstr));

    else return dataEmmiter();
}

std::unique_ptr<ParseObject> Parser::instruction() {

    uint8_t opcode = ArchInfo::insmap[advance().tokenstr];
    std::vector<std::unique_ptr<ParseObject>> operands;

    if (matchAndAdvance({MainType::NEWLINE})) return std::unique_ptr<ParseObject>(new Instruction(opcode, std::move(operands)));

    else {
        operands.push_back(operand());
        while (matchAndAdvance({MainType::COMMA})) {
            operands.push_back(operand());
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
    if (name.maintype != MainType::STR) // TODO: throw a parse error.
    return std::unique_ptr<ParseObject>(new DirSection(name.tokenstr));
}

std::unique_ptr<ParseObject> Parser::parseTimes() {
    std::string sym;
    if (matchAndAdvance({MainType::SYM})) sym = previous().tokenstr;
    std::unique_ptr<ParseObject> expr = expression();
    std::unique_ptr<ParseObject> l = line();

    return std::unique_ptr<ParseObject>(new DirTimes(sym, std::move(expr), std::move(l)));
}

std::unique_ptr<ParseObject> Parser::dataEmmiter() {
    
    std::string sym;
    uint8_t size = 0;
    if (matchAndAdvance({MainType::SYM})) sym = previous().tokenstr;

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
    
    if (matchAndAdvance({MainType::STR})) return std::unique_ptr<ParseObject>(new StringNode(previous().tokenstr));

    else return expression();
}

std::unique_ptr<ParseObject> Parser::operand() {
    
    if (compareTypes({MainType::OPEN_BRACE}) || compareSubTypes({SubType::DIR_BYTE, SubType::DIR_WORD, SubType::DIR_DWORD}))
        return memexpr();
    else return expression();
}

std::unique_ptr<ParseObject> Parser::memexpr() {

    uint8_t size = 0;
    std::unique_ptr<ParseObject> expr;
    if (matchSubTypeAndAdvance({SubType::DIR_BYTE, SubType::DIR_WORD, SubType::DIR_DWORD})) {
        size = getDirectiveSize(previous().subtype);
        ((MemExpr*) expr.get())->size = size;
    }
    if (matchAndAdvance({MainType::OPEN_BRACE})) {
        expr = memoryaddr();
    }
    consume(MainType::CLOSE_BRACE, "Expected a ']' at the end of expression.");
    return expr;
}

std::unique_ptr<ParseObject> Parser::memoryaddr() {
    std::unique_ptr<MemExpr> memexpr = std::unique_ptr<MemExpr>(new MemExpr(0, "", "", nullptr));

    if (matchAndAdvance({MainType::REG})) {
        std::string name = previous().tokenstr;

        if (isBaseReg(name)) { // rule 1: BASEREG (("+" | "-") indexexpr)? (("+" | "-") expression)?
            memexpr.get()->basereg = std::unique_ptr<Register>(new Register(
                name, SubType::REG32, ArchInfo::base_select_map[name])
            );
            if (matchSubTypeAndAdvance({SubType::OPER_ADD, SubType::OPER_SUB})) {
                indexexpr(memexpr.get());
            }
            if (matchSubTypeAndAdvance({SubType::OPER_ADD, SubType::OPER_SUB})) {
                memexpr.get()->displacement = expression();
            }
        }

        else if (isIdxReg(name)) { // rule 2: indexexpr (("+" | "-") expression)?
            memexpr.get()->indexreg = std::unique_ptr<Register>(new Register(
                name, SubType::REG32, ArchInfo::index_select_map[name]
            ));
            if (matchSubTypeAndAdvance({SubType::OPER_ADD, SubType::OPER_SUB})) {
                memexpr.get()->displacement = expression();
            }
        }
    }
    else { // rule 3: expression
        memexpr.get()->displacement = expression();
    }
    return memexpr;
}

void Parser::indexexpr(MemExpr* expr) {

    if (matchSubTypeAndAdvance({SubType::OPER_MUL})){
        std::string n = current().tokenstr;
        if (n != "1" && n != "2" && n != "4") // TODO: throw error.

        expr->scale = std::stoi(n);
    }
    else expr->scale = 1;
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
    if (matchAndAdvance({MainType::NUM})) return std::unique_ptr<ParseObject>(new Literal(std::stoul(previous().tokenstr)));
    else if (matchAndAdvance( {MainType::SYM} )) return std::unique_ptr<ParseObject>(new Symbol(previous().tokenstr));
    else if (matchAndAdvance( {MainType::SPECIAL} )) return std::unique_ptr<ParseObject>(new Special(previous().subtype));
    else if (matchAndAdvance({MainType::REG})) {
        return std::unique_ptr<ParseObject>(new Register(
            previous().tokenstr,
            previous().subtype,
            ArchInfo::reg_select_map[previous().tokenstr]
        ));
    }
    
    else if (matchAndAdvance({MainType::OPEN_PAREN})) {
        std::unique_ptr<ParseObject> expr = expression();
        consume(MainType::CLOSE_PAREN, "Expected a closing parenthises.");

        return std::unique_ptr<ParseObject>(new Grouping(std::move(expr)));
    }

    else {
        // throw a parse error.
        return nullptr;
    }
}