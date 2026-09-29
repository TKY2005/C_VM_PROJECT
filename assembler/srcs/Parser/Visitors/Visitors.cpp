#include<cstdint>
#include<map>
#include<initializer_list>
#include<typeindex>
#include<iostream>
#include<cstring>

#include<Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

void NodeVisitor::undefinedNode() {
    std::cout << "This type is not allowed here." << std::endl;
}

bool NodeVisitor::isExprNode(ParseObject* p) {
        return (
            typeid(*p) == typeid(Binary) || 
            typeid(*p) == typeid(Unary) ||
            typeid(*p) == typeid(Grouping) ||
            typeid(*p) == typeid(Number) || 
            typeid(*p) == typeid(Symbol) ||
            typeid(*p) == typeid(Special)
        );
}

bool NodeVisitor::compareTypes(MainType t, std::initializer_list<MainType> types)  {
        for(MainType type : types) {
            if (type == t) return true;
        }
        return false;
}

bool NodeVisitor::compareNodeTypes(std::type_index t, std::initializer_list<std::type_index> types) {
    for(std::type_index type : types) {
        if (t == type) return true;
    }
    return false;
}

bool NodeVisitor::isBaseReg(Register* a) {
        if (a) {
            try{
                ArchInfo::base_select_map.at(a->name);
                return true;
            }
            catch(const std::runtime_error& e) {
                return false;
            }
        }
        else return false;
}

bool NodeVisitor::isIdxReg(Register* a) {
        if (a) {
            try{
                ArchInfo::index_select_map.at(a->name);
                return true;
            }
            catch(const std::runtime_error& e) {
                return false;
            }
        }
        else return false;
}

int NodeVisitor::typeToSize(SubType t) {
        switch(t) {
            case SubType::REG8 : case SubType::DIR_BYTE : case SubType::DIR_DEFB : case SubType::DIR_RESB : 
            {
                return 1;
            }
            case SubType::REG16 : case SubType::DIR_WORD : case SubType::DIR_DEFW : case SubType::DIR_RESW : 
            {
                return 2;
            }
            case SubType::REG32 : case SubType::DIR_DWORD : case SubType::DIR_DEFDW : case SubType::DIR_RESDW :
            {
                return 4;
            }
            default:
            return -1;
        }
        return -1;
}

std::vector<uint8_t> NodeVisitor::sliceVal(uint32_t v, int bits) {
    std::vector<uint8_t> b;

    for(int i = bits - 8; i >= 0; i -= 8) {
        b.push_back(v >> i);
    }
    return b;
}

uint32_t NodeVisitor::vptouint(void* v) {

    if (!v) return 0;
    uint32_t i;
    memcpy(&i, v, sizeof(uint32_t));
    return i;
}

int NodeVisitor::findOperandType(ParseObject& a) 
{
    if (typeid(a) == typeid(MemExpr)) return OPERAND_TYPE_MEM;
                else if (typeid(a) == typeid(Register)) return OPERAND_TYPE_REG;
                else return OPERAND_TYPE_EXPR;
}