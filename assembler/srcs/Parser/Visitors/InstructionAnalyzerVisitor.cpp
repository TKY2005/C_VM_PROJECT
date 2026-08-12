#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

void InstructionAnalyzer::analyze(ParseObject& a) {
    a.accept(*this);
}

void InstructionAnalyzer::visit(Instruction& a) {
    
    for(int i = 0; i < a.operands.size(); i++) {

        if (isExprNode(a.operands[i].get())) {
            if (!a.hasLiteral){
                a.hasLiteral = true;
            }
            else return; // TODO: Throw semantic error: can't have two literals as operands.
        }
        
        else {
            analyze(*a.operands[i]);
            
            if (prevType == MainType::REG) a.hasRegister = true;
            else if (prevWasMemory) {
                if (a.hasMemExpr) return; // TODO: throw semantic error: can't have two memory operands
                else a.hasMemExpr = true;
            }
            else if (prevType == MainType::SPECIAL) a.hasSpecial = true;
            else if (prevType == MainType::SYM){
                if (a.hasSymbol) return; // TODO: throw semantic error: can't have two symbols
                a.hasSymbol = true;
            }
        }
    }
}

void InstructionAnalyzer::visit(Register& a) {
    prevType = MainType::REG;
    prevSubType = a.size;
}

void InstructionAnalyzer::visit(MemExpr& a) {
    
    if (!isBaseReg(a.basereg.get())) // TODO: throw a semantic error.
    if (!isIdxReg(a.indexreg.get())) // TODO: throw a semantic error.
    
    prevWasMemory = true;
}

void InstructionAnalyzer::visit(Binary& a) {

    analyze(*a.left);
    MainType leftType = prevType;
    SubType leftsub = prevSubType;

    if (!compareTypes(leftType, allowedExprTypes)) {
        // TODO: throw a semantic error: Illegal expression type
    }

    analyze(*a.right);
    MainType rightType = prevType;
    SubType rightsub = prevSubType;

    if (!compareTypes(rightType, allowedExprTypes)) {
        // TODO: throw a semantic error: Illegal expression type
    }
    
}

void InstructionAnalyzer::visit(Unary& a) {

    analyze(*a.right);
    MainType rightType = prevType;
    SubType rightsub = prevSubType;

    if (!compareTypes(rightType, allowedExprTypes)) {
        // TODO: throw a semantic error: Illegal expression type
    }
}

void InstructionAnalyzer::visit(Grouping& a) {

    analyze(*a.expr);
}

void InstructionAnalyzer::visit(Special& a) {
    prevType = MainType::SPECIAL;
    prevSubType = a.type;
}

void InstructionAnalyzer::visit(Literal& a) {
    prevType = MainType::NUM;
    prevSubType = SubType::NONE;
}

void InstructionAnalyzer::visit(Symbol& a) {
    prevType = MainType::SYM;
    prevSubType = SubType::NONE;
}