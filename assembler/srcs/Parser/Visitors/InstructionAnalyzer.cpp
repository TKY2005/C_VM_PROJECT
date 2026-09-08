#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

std::unique_ptr<Value> InstructionAnalyzerVisitor::analyze(ParseObject& a, AsmContext& c) {
    if (isExprNode(&a)) return exprAnalyzer.get()->analyze(a, c);
    return a.accept(*this, c);
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Register& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{(void*) new TypeInfo{MainType::REG, a.size}});
}
std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Symbol& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{(void*) new TypeInfo{MainType::SYM, SubType::NONE}});
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Special& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{(void*) new TypeInfo{MainType::SPECIAL, a.type}});
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Number& a, AsmContext& c) {
    return std::unique_ptr<Value>(new Value{(void*) new TypeInfo{MainType::NUM, SubType::NONE}});
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Instruction& a, AsmContext& c) {

    c.current_context = new instruction_context({0});

    if (a.operands.size() == 0) c.current_context->has_operands = 0;
    else {
        c.current_context->has_operands = 1;
        if (a.operands.size() == 1){

            c.current_context->is_jmp_or_int =
            ((a.opcode >= INS_CALL && a.opcode <= INS_CALB) ||
            (a.opcode >= INS_JMP && a.opcode <= INS_JB) ||
            (a.opcode >= INS_JC && a.opcode <= INS_JNO) ||
            (a.opcode >= INS_JZ && a.opcode <= INS_JNZ) ||
            a.opcode == INS_LOOP ||
            a.opcode == INS_INT);
            
            c.current_context->single_operand = 1;
            c.current_context->dest_type = findOperandType(*a.operands[0]);
            analyze(*a.operands[0], c);
        }
        else{
            c.current_context->single_operand = 0;

            c.current_context->dest_type = findOperandType(*a.operands[0]);
            analyze(*a.operands[0], c);
            c.current_context->src_type = findOperandType(*a.operands[1]);
            analyze(*a.operands[1], c);
        }
    }
    
    c.instructionSemantics[&a] = std::unique_ptr<instruction_context>(c.current_context);

    return std::unique_ptr<Value>(new Value{(void*) new TypeInfo{MainType::INS, SubType::NONE}});
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(MemExpr& a, AsmContext& c) {
    
    c.current_context->parsing_mem = 1;
    if (!isBaseReg(a.basereg.get())) return nullptr; // TODO: throw a semantic error
    if (!isIdxReg(a.indexreg.get())) return nullptr; // TODO: throw a semantic error

    if (a.scale > 4) return nullptr; // TODO: throw a semantic error

    analyze(*a.displacement, c);

    c.current_context->parsing_mem = 0;
    return std::unique_ptr<Value>(new Value{(void*) new TypeInfo{MainType::OPEN_BRACE, SubType::NONE}});
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Binary& a, AsmContext& c) {

    c.current_context->parsing_expr = 1;
    
    MainType left = static_cast<TypeInfo*>(analyze(*a.left, c).get()->val)->main;
    c.current_context->parsing_expr = 1;
    MainType right = static_cast<TypeInfo*>(analyze(*a.right, c).get()->val)->main;
    c.current_context->parsing_expr = 1;

    if (!compareTypes(left, allowedExprTypes) || !compareTypes(right, allowedExprTypes)){
        return nullptr; // TODO: throw a semantic error
    }

    c.current_context->parsing_expr = 0;
    return std::unique_ptr<Value>(new Value({(void*) new TypeInfo{MainType::NUM, SubType::NONE}}));
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Unary& a, AsmContext& c) {

    c.current_context->parsing_expr = 1;
    
    MainType right = static_cast<MainType>(vptouint(analyze(*a.right, c).get()->val));
    c.current_context->parsing_expr = 1;

    if (!compareTypes(right, allowedExprTypes)) return nullptr; // TODO: throw a semantic error

    c.current_context->parsing_expr = 0;
    return std::unique_ptr<Value>(new Value({(void*) new TypeInfo{MainType::NUM, SubType::NONE}}));
}

std::unique_ptr<Value> InstructionAnalyzerVisitor::visit(Grouping& a, AsmContext& c) {
    c.current_context->parsing_expr = 1;
    return analyze(*a.expr, c);
}