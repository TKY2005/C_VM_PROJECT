#include<any>
#include<cstdint>
#include<limits>
#include<map>
#include<stdexcept>
#include<string>
#include<set>

#include<ErrorHandler/ErrorHandler.hpp>
#include<Parser/Parser.hpp>
#include<Visitors/Visitors.hpp>
#include<Parser/Unicode.hpp>

namespace {
std::string symbolName(std::string name)
{
    if (!name.empty() && name[0] == '.') name.erase(0, 1);
    return name;
}

uint32_t checkedLength(uint64_t length)
{
    if (length > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("assembled address space exceeds 32 bits");
    }
    return static_cast<uint32_t>(length);
}
}

std::any SymbolLengthVisitor::calculate(ParseResult& result, AsmContext& c)
{
    std::map<std::string, uint32_t> previousSymbols;

    for (int pass = 0; pass < 32; ++pass) {
        c.program_counter = 0;
        c.section_offset = 0;
        c.section_begin = 0;
        c.symMap = previousSymbols;
        c.sectionAddresses.clear();
        c.nodeAddresses.clear();
        c.nodeLengths.clear();
        definedSymbols.clear();
        c.allowUnresolvedSymbols = true;

        for (const auto& node : result.parsedLines) {
            if (!node) throw std::runtime_error("parser produced an empty node");
            try {
                layout(*node, c);
            }
            catch (const std::exception& error) {
                if (!ErrorBucket::hasErrorAt(c.sourceFile, node->location.row)) {
                    ErrorBucket::addError({}, node->location, c.sourceFile, error.what(), node->sourceLine);
                }
            }
        }
        if (ErrorBucket::hasFailed) throw std::runtime_error("symbol layout failed");

        if (c.symMap == previousSymbols) {
            previousSymbols = c.symMap;
            break;
        }
        previousSymbols = c.symMap;

        if (pass == 31) {
            c.allowUnresolvedSymbols = false;
            throw std::runtime_error("symbol-dependent layout did not converge after 32 passes");
        }
    }

    c.program_counter = 0;
    c.section_offset = 0;
    c.section_begin = 0;
    c.symMap = previousSymbols;
    c.sectionAddresses.clear();
    c.nodeAddresses.clear();
    c.nodeLengths.clear();
    definedSymbols.clear();
    c.allowUnresolvedSymbols = false;
    for (const auto& node : result.parsedLines) {
        try {
            layout(*node, c);
        }
        catch (const std::exception& error) {
            if (!ErrorBucket::hasErrorAt(c.sourceFile, node->location.row)) {
                ErrorBucket::addError({}, node->location, c.sourceFile, error.what(), node->sourceLine);
            }
        }
    }
    if (ErrorBucket::hasFailed) throw std::runtime_error("symbol layout failed");

    return true;
}

void SymbolLengthVisitor::layout(ParseObject& a, AsmContext& c)
{
    a.accept(*this, c);
}

void SymbolLengthVisitor::record(ParseObject& a, AsmContext& c, uint32_t address, uint32_t length)
{
    c.nodeAddresses[&a].push_back(address);
    c.nodeLengths[&a].push_back(length);
}

void SymbolLengthVisitor::defineSymbol(const std::string& name, uint32_t value, AsmContext& c)
{
    const std::string normalizedName = symbolName(name);
    if (normalizedName.empty()) throw std::runtime_error("symbol name cannot be empty");
    if (!definedSymbols.insert(normalizedName).second) {
        throw std::runtime_error("duplicate symbol '" + normalizedName + "'");
    }
    c.symMap[normalizedName] = value;
}

uint32_t SymbolLengthVisitor::currentValue(ParseObject& a, AsmContext& c)
{
    return std::any_cast<uint32_t>(evaluator.eval(a, c));
}

uint32_t SymbolLengthVisitor::instructionLength(Instruction& a, AsmContext& c)
{
    if (a.operands.empty()) return 1;

    bool hasRegister = false;
    MemExpr* memory = nullptr;
    bool hasImmediate = false;
    bool hasSymbol = false;
    for (const auto& operand : a.operands) {
        hasRegister = hasRegister || dynamic_cast<Register*>(operand.get()) != nullptr;
        if (MemExpr* memExpr = dynamic_cast<MemExpr*>(operand.get())) memory = memExpr;
        Symbol* symbol = dynamic_cast<Symbol*>(operand.get());
        hasSymbol = hasSymbol || symbol != nullptr;
        if (!symbol && !dynamic_cast<Register*>(operand.get()) &&
            !dynamic_cast<MemExpr*>(operand.get()) &&
            !dynamic_cast<Symbol*>(operand.get())) {
            hasImmediate = true;
        }
    }

    uint64_t length = 2;
    if (hasRegister) ++length;
    if (memory) length += 1 + (memory->displacementEnabled ? 4 : 0);
    if (hasSymbol) length += 4;
    if (hasImmediate) {
        if (a.operands.size() == 1) return checkedLength(length + 4);
        ParseObject& destination = *a.operands[0];
        uint8_t size = 4;
        if (Register* reg = dynamic_cast<Register*>(&destination)) {
            if (reg->size == SubType::REG8) size = 1;
            else if (reg->size == SubType::REG16) size = 2;
        }
        else if (MemExpr* memory = dynamic_cast<MemExpr*>(&destination)) {
            size = memory->size;
        }
        length += size;
    }
    return checkedLength(length);
}

std::any SymbolLengthVisitor::visit(Binary& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any SymbolLengthVisitor::visit(Unary& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any SymbolLengthVisitor::visit(Grouping& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any SymbolLengthVisitor::visit(Number& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any SymbolLengthVisitor::visit(Special& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any SymbolLengthVisitor::visit(StringNode& a, AsmContext& c)
{
    return a.str;
}

std::any SymbolLengthVisitor::visit(Symbol& a, AsmContext& c)
{
    return evaluator.eval(a, c);
}

std::any SymbolLengthVisitor::visit(DataNode& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    if (a.hasSymbol) defineSymbol(a.symbol, address, c);

    uint64_t length = 0;
    for (const auto& datum : a.data) {
        if (StringNode* string = dynamic_cast<StringNode*>(datum.get())) {
            length += static_cast<uint64_t>(decodeUtf8(string->str).size()) * a.size;
        }
        else length += a.size;
    }
    a.address = address;
    a.len = checkedLength(length);
    record(a, c, address, a.len);
    c.incProgramOffset(a.len);
    return a.len;
}

std::any SymbolLengthVisitor::visit(ResNode& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    if (a.hasSymbol) defineSymbol(a.symbol, address, c);
    const uint32_t count = currentValue(*a.expr, c);
    a.exprVal = count;
    a.address = address;
    a.len = checkedLength(static_cast<uint64_t>(count) * a.size);
    record(a, c, address, a.len);
    c.incProgramOffset(a.len);
    return a.len;
}

std::any SymbolLengthVisitor::visit(DirSection& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    const std::string name = symbolName(a.name);
    if (c.sectionAddresses.find(name) == c.sectionAddresses.end()) {
        c.sectionAddresses[name] = address;
        defineSymbol(name, address, c);
    }
    record(a, c, address, 0);
    c.updateSections();
    return address;
}

std::any SymbolLengthVisitor::visit(DirORG& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    record(a, c, address, 0);
    a.val = currentValue(*a.expr, c);
    c.setProgramOffset(a.val);
    return a.val;
}

std::any SymbolLengthVisitor::visit(DirTimes& a, AsmContext& c)
{
    const uint32_t start = c.program_counter;
    if (!a.symbol.empty()) defineSymbol(a.symbol, start, c);
    a.exprVal = currentValue(*a.expr, c);
    record(a, c, start, 0);
    for (uint32_t i = 0; i < a.exprVal; ++i) layout(*a.repeated, c);
    const uint32_t length = checkedLength(static_cast<uint64_t>(c.program_counter) - start);
    c.nodeLengths[&a].back() = length;
    return length;
}

std::any SymbolLengthVisitor::visit(Instruction& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    const uint32_t length = instructionLength(a, c);
    a.address = address;
    a.len = length;
    record(a, c, address, length);
    c.incProgramOffset(length);
    return length;
}

std::any SymbolLengthVisitor::visit(MemExpr& a, AsmContext& c)
{
    return evaluator.evalMemory(*a.memexpr, c);
}

std::any SymbolLengthVisitor::visit(Register& a, AsmContext& c)
{
    return a.name;
}

std::any SymbolLengthVisitor::visit(Declaration& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    defineSymbol(a.name, address, c);
    record(a, c, address, 0);
    return address;
}

std::any SymbolLengthVisitor::visit(EquNode& a, AsmContext& c)
{
    const uint32_t address = c.program_counter;
    a.value = currentValue(*a.expr, c);
    defineSymbol(a.symbol, a.value, c);
    record(a, c, address, 0);
    return a.value;
}
