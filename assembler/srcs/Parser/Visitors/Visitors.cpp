#include<cstdint>
#include<map>
#include<string>
#include<initializer_list>

#include<Parser/Visitors/Visitors.hpp>
#include<Parser/Parser.hpp>

#include<LexicalAnalyzer/Tokenizer.hpp>

bool NodeVisitor::isExprNode(ParseObject* p) {
        return (
            typeid(*p) == typeid(Binary) || 
            typeid(*p) == typeid(Unary) ||
            typeid(*p) == typeid(Grouping) ||
            typeid(*p) == typeid(Literal) || 
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