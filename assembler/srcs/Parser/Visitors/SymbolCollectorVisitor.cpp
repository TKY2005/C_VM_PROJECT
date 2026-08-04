#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

void SymbolCollectorVisitor::evalSize(ParseObject& a) {
    a.accept(*this);
}

void SymbolCollectorVisitor::visit(Special& a) {

    if (collectingInstruction) incProgramOffset(currentsize);

    prevType = MainType::SPECIAL;
    prevSubtype = a.type;
}

void SymbolCollectorVisitor::visit(Literal& a) {
    
    currentsize = typeToSize(prevSubtype);

    if (collectingInstruction || collectingData) { // literals shouldn't count if they're part of an expression
        incProgramOffset(currentsize);
    }
    prevType = MainType::NUM;
}

void SymbolCollectorVisitor::visit(Register& a) {
    if (collectingOperand && prevType != MainType::REG) {
        incProgramOffset(1); // register select byte
    }
    prevSubtype = a.size;
    prevType = MainType::REG;
}

void SymbolCollectorVisitor::visit(Symbol& a) {

    if (collectingOperand) incProgramOffset(4); // add size only when the symbol is an operand

    prevType = MainType::SYM;
    prevSubtype = SubType::NONE;
    
}

void SymbolCollectorVisitor::visit(Declaration& a) {
    symMap[a.name] = program_offset;

    prevType = MainType::DECL;
    prevSubtype = SubType::NONE;
}

void SymbolCollectorVisitor::visit(DirSection& a) {
    symMap[a.name] = program_offset;
    setProgramAndSectionsOffsets(program_offset);

    prevType = MainType::STR;
    prevSubtype = SubType::NONE;
}

void SymbolCollectorVisitor::visit(StringNode& a) {
    for(int i = 0; i < a.str.size(); i++) {
        incProgramOffset(currentsize);
    }

    prevType = MainType::STR;
    prevSubtype = SubType::NONE;
}

void SymbolCollectorVisitor::visit(Binary& a) {
    evalSize(*a.left);
    evalSize(*a.right);
}

void SymbolCollectorVisitor::visit(Unary& a) {
    evalSize(*a.right);
}

void SymbolCollectorVisitor::visit(Grouping& a) {
    evalSize(*a.expr);
}

void SymbolCollectorVisitor::visit(Instruction& a) {
    
    collectingInstruction = true;
    incProgramOffset(1); // operand types byte

    if (a.operands.size() == 0) return;

    else {
        collectingOperand = true;

        for(int i = 0; i < a.operands.size(); i++) evalSize(*a.operands[i]);
    }

    collectingInstruction = false;
    collectingOperand = false;
}

void SymbolCollectorVisitor::visit(DirORG& a) {
    evalExpr(*a.expr);
    setProgramOffset(exprResult);
}

void SymbolCollectorVisitor::visit(DirTimes& a) {
    evalExpr(*a.repeated);
    for(int i = 0; i < exprResult; i++) {
        evalSize(*a.expr);
    }
}

void SymbolCollectorVisitor::visit(DataNode& a) {
    currentsize = a.size;
    symMap[a.symbol] = program_offset;

    collectingData = true;

    for(int i = 0; i < a.data.size(); i++) {
        evalSize(*a.data[i]);
    }

    collectingData = false;
}

void SymbolCollectorVisitor::visit(ResNode& a) {
    currentsize = a.size;
    evalExpr(*a.expr);
    incProgramOffset(currentsize * exprResult);
}

void SymbolCollectorVisitor::visit(MemExpr& a) {

    collectingMemory = true;

    if (a.displacement != nullptr) incProgramOffset(4); // displacement size
    if (a.basereg == nullptr && a.indexreg == nullptr) goto done;

    incProgramOffset(1); // displacement info byte

    done:
        prevType = MainType::CLOSE_BRACE;
        prevSubtype = SubType::NONE;
    
    collectingMemory = false;
}