#ifndef VISITORS_HPP
#define VISITORS_HPP

#include<cstdint>
#include<map>
#include<string>
#include<initializer_list>
#include<memory>
#include<typeindex>
#include<any>

#include<LexicalAnalyzer/Tokenizer.hpp>
#include<ISA_encoding_info.h>

#define CURRENT_OPERAND_DEST 0b0
#define CURRENT_OPERAND_SRC 0b1

#define OPERAND_TYPE_UNDEF 0b00
#define OPERAND_TYPE_EXPR 0b01
#define OPERAND_TYPE_MEM 0b10
#define OPERAND_TYPE_REG 0b11

// base class for every node
class ParseObject;

// T = terminal node, NT = non-terminal node

// Expression classes
class Binary; // NT
class Unary; // NT
class Grouping; // NT

class Number; // T


class Register; // T
class Symbol; // T
class Declaration; // T
class Special; // T
class Instruction; // NT
class DirORG; // NT
class DirSection; // T
class DirTimes; // NT
class DataNode; // NT
class ResNode; // NT
class StringNode; // T
class MemExpr; // NT

typedef union instruction_context {
    struct {
        uint8_t has_operands : 1;
        uint8_t single_operand : 1;
        uint8_t is_jmp_or_int : 1;
        uint8_t current_operand : 1;
        uint8_t dest_type : 2;
        uint8_t src_type : 2;
        uint8_t parsing_expr : 1;
        uint8_t parsing_mem : 1;
    };
    uint16_t ctxt;
} instruction_context;

typedef struct instruction_semantics {
    uint8_t opcode;
    union {
        struct {
            uint8_t num_operands : 2;
            uint8_t add_opertypes : 1;
            uint8_t add_reginfo : 1;
            uint8_t add_meminfo : 1;
        };
    };
};

typedef struct TypeInfo {
    MainType main;
    SubType sub;
} TypeInfo;

class AsmContext {
    public:

    uint32_t program_counter = 0x00;
    uint32_t section_offset = 0x00;
    uint32_t section_begin = 0x00;

    instruction_context* current_context;
    ins_encoding* current_encoding;

    std::map<Instruction*, std::unique_ptr<instruction_context>> instructionSemantics;
;
    std::map<Instruction*, std::unique_ptr<ins_encoding>> analyzedInstructions;

    std::map<std::string, uint32_t> symMap;

    void incProgramOffset(uint32_t val) {
        program_counter += val;
        section_offset = program_counter - section_begin;
    }

    void setProgramOffset(uint32_t val) {
        program_counter = val;
        section_offset = program_counter - section_begin;
    }
    void updateSections() {
        section_begin = program_counter;
        section_offset = 0x00;
    }
    void setProgramOffsetAndSections(uint32_t val) {
        program_counter = val;
        updateSections();
    }

    AsmContext() {
        program_counter = 0x00;
        section_offset = 0x00;
        section_begin = 0x00;
        current_context = new instruction_context();
        current_context->dest_type = OPERAND_TYPE_UNDEF;
        current_context->src_type = OPERAND_TYPE_UNDEF;

        current_encoding = new ins_encoding();
        current_encoding->operand_types = OPERTYPE_DEFAULT;
        current_encoding->register_select = REGSELECT_DEFAULT;
        current_encoding->displacement_info = DISP_DEFAULT;

        current_context->ctxt = 0x00;
    }

    ~AsmContext() {
        delete current_context;
    }
};


typedef struct Value {
    void* val;
} Value;

class NodeVisitor {
    public:

    virtual ~NodeVisitor() = default;

    virtual std::any visit(Binary& a, AsmContext& c) {undefinedNode();return nullptr;};
    virtual std::any visit(Unary& a, AsmContext& c) {undefinedNode();return nullptr;};
    virtual std::any visit(Grouping& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(Number& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(Special& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(StringNode& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(Symbol& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(DataNode& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(ResNode& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(DirSection& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(DirORG& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(DirTimes& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(Instruction& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(MemExpr& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(Register& a, AsmContext& c) {undefinedNode(); return nullptr;};
    virtual std::any visit(Declaration& a, AsmContext& c) {undefinedNode(); return nullptr;};

    std::initializer_list<MainType> allowedExprTypes = {
        MainType::NUM,
        MainType::SYM,
        MainType::SPECIAL
    };

    std::initializer_list<std::type_index> expressionNodes {
        typeid(Binary),
        typeid(Unary),
        typeid(Grouping),
        typeid(Number),
        typeid(Special),
        typeid(Symbol)
    };

    bool isExprNode(ParseObject* p);
    bool compareTypes(MainType t, std::initializer_list<MainType> types);
    bool compareNodeTypes(std::type_index t, std::initializer_list<std::type_index> types);
    bool isBaseReg(Register* a);
    bool isIdxReg(Register* a);
    int typeToSize(SubType t);
    std::vector<uint8_t> sliceVal(uint32_t v, int bits);

    int findOperandType(ParseObject& a);

    uint32_t vptouint(void* v);

    void undefinedNode();

    MainType extractType(Value& v) {
        return *static_cast<MainType*>(v.val);
    }
    SubType extractSubType(Value& v) {
        return *static_cast<SubType*>(v.val);
    }

};

class PrintVisitor : public NodeVisitor {
    public:

    int indentLevel = 0;

    std::any traverse(ParseObject& a, AsmContext& c);

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;
    std::any visit(StringNode& a, AsmContext& c) override;
    std::any visit(Symbol& a, AsmContext& c) override;
    std::any visit(DataNode& a, AsmContext& c) override;
    std::any visit(ResNode& a, AsmContext& c) override;
    std::any visit(DirSection& a, AsmContext& c) override;
    std::any visit(DirORG& a, AsmContext& c) override;
    std::any visit(DirTimes& a, AsmContext& c) override;
    std::any visit(Instruction& a, AsmContext& c) override;
    std::any visit(MemExpr& a, AsmContext& c) override;
    std::any visit(Register& a, AsmContext& c) override;
    std::any visit(Declaration& a, AsmContext& c) override;

};

class ExpressionVisitor : public NodeVisitor {
    public:

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;
    std::any visit(Symbol& a, AsmContext& c) override;

    std::any visit(StringNode& a, AsmContext& c) override;
};

class ExpressionAnalyzerVisitor : public ExpressionVisitor {

    public:
    std::any analyze(ParseObject& a, AsmContext& c);

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;
    std::any visit(Symbol& a, AsmContext& c) override;

};

class ExpressionEvalVisitor : public ExpressionVisitor {

    public:
    std::any eval(ParseObject& a, AsmContext& c);

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;
    std::any visit(Symbol& a, AsmContext& c) override;
};

class InstructionVisitor : public NodeVisitor {
    public:

    std::any visit(Instruction& a, AsmContext& c) override;
    std::any visit(MemExpr& a, AsmContext& c) override;
    std::any visit(Register& a, AsmContext& c) override;

    std::any visit(Symbol& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
};

class DataVisitor : public NodeVisitor {
    public:

    std::any visit(DataNode& a, AsmContext& c) override;
    std::any visit(ResNode& a, AsmContext& c) override;
    std::any visit(StringNode& a, AsmContext& c) override;
};

class DirectiveVisitor : public NodeVisitor {
    public:

    std::any visit(DirORG& a, AsmContext& c) override;
    std::any visit(DirSection& a, AsmContext& c) override;
    std::any visit(DirTimes& a, AsmContext& c) override;
    std::any visit(Declaration& a, AsmContext& c) override;
};

class InstructionAnalyzerVisitor : public InstructionVisitor {
    public:

    std::unique_ptr<ExpressionAnalyzerVisitor> exprAnalyzer = std::unique_ptr<ExpressionAnalyzerVisitor>
    (
        new ExpressionAnalyzerVisitor()
    );

    std::any analyze(ParseObject& a, AsmContext& c);

    std::any visit(Instruction& a, AsmContext& c) override;
    std::any visit(MemExpr& a, AsmContext& c) override;
    std::any visit(Register& a, AsmContext& c) override;

    std::any visit(Symbol& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
};

class DataAnalyzerVisitor : public DataVisitor {
    public:

    std::any analyze(ParseObject& a, AsmContext& c);

    std::any visit(DataNode& a, AsmContext& c) override;
    std::any visit(ResNode& a, AsmContext& c) override;
    std::any visit(StringNode& a, AsmContext& c) override;
};

#endif