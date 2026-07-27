#include<Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

void SymbolCollectorVisitor::evalSize(ParseObject& a) {
    a.accept(*this);
}

void SymbolCollectorVisitor::visit(Literal& a) {
    if (addDisplacement){
        incProgramOffset(4);
        addDisplacement = false;
    }
    else {
        currentsize = typeToSize(prevSubType);
        incProgramOffset(currentsize);
    }
}

void SymbolCollectorVisitor::visit(Register& a) {
    if (prevType == MainType::REG && addRegCode && !parsingMemory){
        incProgramOffset(1); // regtype
        addRegCode = false;
    }
    currentsize = typeToSize(a.size);
}

void SymbolCollectorVisitor::visit(Symbol& a) {
    incProgramOffset(4);
}

void SymbolCollectorVisitor::visit(Declaration& a) {
    symMap[a.name] = program_offset;
}

void SymbolCollectorVisitor::visit(DirSection& a) {
    symMap[a.name] = program_offset;
    setProgramAndSectionsOffsets(program_offset);
}

void SymbolCollectorVisitor::visit(StringNode& a) {
    for(int i = 0; i < a.str.size(); i++) {
        incProgramOffset(currentsize);
    }
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
    
    incProgramOffset(1); // opcode
    if (a.operands.size() > 0) incProgramOffset(1); // operand types
    for(int i = 0; i < a.operands.size(); i++) evalSize(*a.operands[i]);
    addRegCode = true;
    addDispInfo = true;
}

void SymbolCollectorVisitor::visit(DirORG& a) {
    evalExpr(*a.expr);
    setProgramOffset(exprResult);
}

void SymbolCollectorVisitor::visit(DirSection& a) {
    symMap[a.name] = program_offset;
    setProgramAndSectionsOffsets(program_offset);
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

    for(int i = 0; i < a.data.size(); i++) {
        evalSize(*a.data[i]);
    }
}

void SymbolCollectorVisitor::visit(ResNode& a) {
    currentsize = a.size;
    evalExpr(*a.expr);
    incProgramOffset(currentsize * exprResult);
}

void SymbolCollectorVisitor::visit(MemExpr& a) {
    parsingMemory = true;
    addDisplacement = true;
    if (addDispInfo){
        incProgramOffset(1); // disp_info byte
        addDispInfo = false;
    }
    parsingMemory = false;
    addDisplacement = false;
}