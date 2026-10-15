#include<any>
#include<stdexcept>
#include<string>
#include<vector>

#include<ArchInfo.hpp>
#include<ErrorHandler/ErrorHandler.hpp>
#include<Parser/Parser.hpp>
#include<Parser/Unicode.hpp>
#include<Visitors/Visitors.hpp>

namespace {
int expectedOperandCount(uint8_t opcode)
{
    switch (opcode) {
        case 0xFF:
        case 0x01:
        case 0x11:
        case 0x2E:
        case 0x2F:
        case 0x34:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x4E:
            return 0;

        case 0x03:
        case 0x09:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0F:
        case 0x10:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x29:
        case 0x2B:
        case 0x2D:
        case 0x30:
        case 0x31:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4F:
            return 1;

        case 0x02:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x0A:
        case 0x0E:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x2A:
        case 0x2C:
        case 0x32:
        case 0x33:
            return 2;
        default:
            throw std::runtime_error("instruction opcode has no declared operand count");
    }
}
}

std::any SemanticAnalysisVisitor::analyze(ParseResult& result, AsmContext& c)
{
    for (const auto& node : result.parsedLines) {
        if (!node) throw std::runtime_error("parser produced an empty node");
        try {
            node->accept(*this, c);
        }
        catch (const std::exception& error) {
            if (!ErrorBucket::hasErrorAt(c.sourceFile, node->location.row)) {
                ErrorBucket::addError({}, node->location, c.sourceFile, error.what(), node->sourceLine);
            }
        }
    }
    return !ErrorBucket::hasFailed;
}

std::any SemanticAnalysisVisitor::visit(Binary& a, AsmContext& c)
{
    exprAnalyzer.analyze(a, c);
    return true;
}

std::any SemanticAnalysisVisitor::visit(Unary& a, AsmContext& c)
{
    exprAnalyzer.analyze(a, c);
    return true;
}

std::any SemanticAnalysisVisitor::visit(Grouping& a, AsmContext& c)
{
    exprAnalyzer.analyze(a, c);
    return true;
}

std::any SemanticAnalysisVisitor::visit(Number& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(Special& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(StringNode& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(Symbol& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(DataNode& a, AsmContext& c)
{
    if (a.size != 1 && a.size != 2 && a.size != 4) {
        throw std::runtime_error("data declaration has an unsupported element size");
    }

    for (const auto& value : a.data) {
        if (!value) throw std::runtime_error("data declaration contains an empty value");
        if (StringNode* string = dynamic_cast<StringNode*>(value.get())) {
            const std::string& text = string->str;
            const uint64_t maximum = a.size == 1 ? 0xffu :
                                     a.size == 2 ? 0xffffu : 0xffffffffu;
            std::vector<uint32_t> characters;
            try {
                characters = decodeUtf8(text);
            }
            catch (...) {
                a.location = string->location;
                throw;
            }
            for (uint32_t character : characters) {
                if (character > maximum) {
                    a.location = string->location;
                    throw std::runtime_error(
                        "string character does not fit in the declared element width"
                    );
                }
            }
        }
        else {
            exprAnalyzer.analyze(*value, c);
        }
    }
    return true;
}

std::any SemanticAnalysisVisitor::visit(ResNode& a, AsmContext& c)
{
    if (a.size != 1 && a.size != 2 && a.size != 4) {
        throw std::runtime_error("reserve declaration has an unsupported element size");
    }
    if (!a.expr) throw std::runtime_error("reserve declaration has no length expression");
    exprAnalyzer.analyze(*a.expr, c);
    return true;
}

std::any SemanticAnalysisVisitor::visit(DirSection& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(DirORG& a, AsmContext& c)
{
    if (!a.expr) throw std::runtime_error("org directive has no address expression");
    exprAnalyzer.analyze(*a.expr, c);
    return true;
}

std::any SemanticAnalysisVisitor::visit(DirTimes& a, AsmContext& c)
{
    if (!a.expr || !a.repeated) {
        throw std::runtime_error("times directive requires a count and a repeated statement");
    }
    exprAnalyzer.analyze(*a.expr, c);
    a.repeated->accept(*this, c);
    return true;
}

int SemanticAnalysisVisitor::operandSize(ParseObject& a)
{
    if (Register* reg = dynamic_cast<Register*>(&a)) {
        if (reg->size == SubType::REG8) return 1;
        if (reg->size == SubType::REG16) return 2;
        if (reg->size == SubType::REG32) return 4;
    }
    if (MemExpr* memory = dynamic_cast<MemExpr*>(&a)) return memory->size;
    return 0;
}

std::any SemanticAnalysisVisitor::visit(Instruction& a, AsmContext& c)
{
    const int expectedCount = expectedOperandCount(a.opcode);
    if (a.operands.size() != static_cast<size_t>(expectedCount)) {
        throw std::runtime_error(
            "instruction opcode " + std::to_string(a.opcode) + " expects " +
            std::to_string(expectedCount) + " operand(s), got " +
            std::to_string(a.operands.size())
        );
    }

    size_t memoryCount = 0;
    size_t symbolCount = 0;
    size_t expressionCount = 0;
    for (size_t i = 0; i < a.operands.size(); ++i) {
        const auto& operand = a.operands[i];
        if (!operand) throw std::runtime_error("instruction contains an empty operand");
        if (dynamic_cast<MemExpr*>(operand.get())) {
            ++memoryCount;
            try {
                operand->accept(*this, c);
            }
            catch (...) {
                a.location = dynamic_cast<MemExpr*>(operand.get())->location;
                throw;
            }
        }
        else if (dynamic_cast<Register*>(operand.get())) {
            continue;
        }
        else if (dynamic_cast<Symbol*>(operand.get())) {
            if (a.operands.size() == 2 && i == 0) {
                a.location = operand->location;
                throw std::runtime_error(
                    "a direct symbol cannot be an instruction destination; use a sized memory operand"
                );
            }
            ++symbolCount;
        }
        else if (dynamic_cast<StringNode*>(operand.get())) {
            a.location = operand->location;
            throw std::runtime_error("strings are not valid instruction operands");
        }
        else {
            ++expressionCount;
            try {
                exprAnalyzer.analyze(*operand, c);
            }
            catch (...) {
                a.location = operand->location;
                throw;
            }
        }
    }

    if (memoryCount > 1) {
        throw std::runtime_error("an instruction can contain at most one memory operand");
    }
    if (symbolCount > 1) {
        throw std::runtime_error("an instruction can contain at most one direct symbol operand");
    }
    if (expressionCount > 1) {
        if (!a.operands.empty()) a.location = a.operands.back()->location;
        throw std::runtime_error("the instruction encoding supports at most one immediate expression");
    }
    if (a.operands.size() == 2) {
        int destinationSize = operandSize(*a.operands[0]);
        int sourceSize = operandSize(*a.operands[1]);
        if (destinationSize != 0 && sourceSize != 0 && sourceSize > destinationSize) {
            a.location = a.operands[1]->location;
            throw std::runtime_error("source operand cannot be larger than the destination operand");
        }
    }
    return true;
}

bool SemanticAnalysisVisitor::containsRegister(ParseObject& a)
{
    if (dynamic_cast<Register*>(&a)) return true;
    if (Binary* binary = dynamic_cast<Binary*>(&a)) {
        return containsRegister(*binary->left) || containsRegister(*binary->right);
    }
    if (Unary* unary = dynamic_cast<Unary*>(&a)) return containsRegister(*unary->right);
    if (Grouping* grouping = dynamic_cast<Grouping*>(&a)) return containsRegister(*grouping->expr);
    return false;
}

void SemanticAnalysisVisitor::collectMemoryTerms(ParseObject& node, int sign, MemExpr& memory, AsmContext& c)
{
    if (Grouping* grouping = dynamic_cast<Grouping*>(&node)) {
        collectMemoryTerms(*grouping->expr, sign, memory, c);
        return;
    }

    if (Binary* binary = dynamic_cast<Binary*>(&node)) {
        if (binary->oper.subtype == SubType::OPER_ADD) {
            collectMemoryTerms(*binary->left, sign, memory, c);
            collectMemoryTerms(*binary->right, sign, memory, c);
            return;
        }
        if (binary->oper.subtype == SubType::OPER_SUB) {
            collectMemoryTerms(*binary->left, sign, memory, c);
            collectMemoryTerms(*binary->right, -sign, memory, c);
            return;
        }

        if (binary->oper.subtype == SubType::OPER_MUL &&
            (containsRegister(*binary->left) || containsRegister(*binary->right))) {
            Register* reg = dynamic_cast<Register*>(binary->left.get());
            Number* scaleValue = dynamic_cast<Number*>(binary->right.get());
            if (!reg) {
                reg = dynamic_cast<Register*>(binary->right.get());
                scaleValue = dynamic_cast<Number*>(binary->left.get());
            }
            if (!reg || !scaleValue || sign < 0) {
                throw std::runtime_error("memory index must be multiplied by a scale of 1, 2, or 4");
            }
            if (scaleValue->value != 1 && scaleValue->value != 2 && scaleValue->value != 4) {
                throw std::runtime_error("memory index scale must be 1, 2, or 4");
            }
            const bool isBase = ArchInfo::base_select_map.find(reg->name) != ArchInfo::base_select_map.end();
            const bool isIndex = ArchInfo::index_select_map.find(reg->name) != ArchInfo::index_select_map.end();
            if (!isBase && !isIndex) {
                throw std::runtime_error("register cannot be used in a memory expression");
            }
            if (memory.scaleSpecified) {
                throw std::runtime_error("memory expression has more than one scale");
            }
            if (isBase) {
                if (memory.baseReg) {
                    memory.location = reg->location;
                    throw std::runtime_error("memory expression has more than one base register");
                }
                memory.baseReg = std::make_unique<Register>(*reg);
            }
            else {
                if (memory.indexReg) {
                    memory.location = reg->location;
                    throw std::runtime_error("memory expression has more than one index register");
                }
                memory.indexReg = std::make_unique<Register>(*reg);
            }
            memory.scale = static_cast<uint8_t>(scaleValue->value);
            memory.scaleSpecified = true;
            return;
        }
    }

    if (Register* reg = dynamic_cast<Register*>(&node)) {
        if (sign < 0) throw std::runtime_error("registers cannot be subtracted in a memory expression");
        if (ArchInfo::base_select_map.find(reg->name) != ArchInfo::base_select_map.end()) {
            if (memory.baseReg) {
                memory.location = reg->location;
                throw std::runtime_error("memory expression has more than one base register");
            }
            memory.baseReg = std::make_unique<Register>(*reg);
            return;
        }
        if (ArchInfo::index_select_map.find(reg->name) != ArchInfo::index_select_map.end()) {
            if (memory.indexReg) {
                memory.location = reg->location;
                throw std::runtime_error("memory expression has more than one index register");
            }
            memory.indexReg = std::make_unique<Register>(*reg);
            return;
        }
        throw std::runtime_error("register is not allowed in a memory expression");
    }

    if (containsRegister(node)) {
        throw std::runtime_error("registers in memory expressions must be base or index terms");
    }
    memory.displacementEnabled = true;
    exprAnalyzer.analyze(node, c);
}

std::any SemanticAnalysisVisitor::visit(MemExpr& a, AsmContext& c)
{
    if (!a.memexpr) throw std::runtime_error("memory operand has no address expression");
    if (a.size != 1 && a.size != 2 && a.size != 4) {
        throw std::runtime_error("memory operand has an unsupported size");
    }
    a.baseReg.reset();
    a.indexReg.reset();
    a.scale = 1;
    a.scaleSpecified = false;
    a.displacementEnabled = false;
    collectMemoryTerms(*a.memexpr, 1, a, c);
    return true;
}

std::any SemanticAnalysisVisitor::visit(Register& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(Declaration& a, AsmContext& c)
{
    return true;
}

std::any SemanticAnalysisVisitor::visit(EquNode& a, AsmContext& c)
{
    if (!a.expr) throw std::runtime_error("equ directive has no value expression");
    exprAnalyzer.analyze(*a.expr, c);
    return true;
}
