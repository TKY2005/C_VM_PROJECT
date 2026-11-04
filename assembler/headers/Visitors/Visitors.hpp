#ifndef VISITORS_HPP
#define VISITORS_HPP

#include<cstdint>
#include<map>
#include<set>
#include<string>
#include<any>
#include<cstddef>
#include<limits>
#include<stdexcept>

#include<LexicalAnalyzer/Tokenizer.hpp>
#include<ISA_encoding_info.h>

// base class for every node
class ParseObject;
class ParseResult;

// T = terminal node, NT = non-terminal node

// Expression classes
class Binary; // NT
class Unary; // NT
class Grouping; // NT

class Number; // T


class Register; // T
class Symbol; // T
class EquNode; // T
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

class AsmContext {
    public:

    uint32_t program_counter = 0x00;
    uint32_t section_offset = 0x00;
    uint32_t section_begin = 0x00;
    std::string sourceFile;
    bool allowUnresolvedSymbols = false;
    bool generatedBaseSet = false;
    uint32_t generatedBaseAddress = 0;

    std::map<std::string, uint32_t> symMap;
    std::map<std::string, uint32_t> sectionAddresses;
    std::map<const ParseObject*, std::vector<uint32_t>> nodeAddresses;
    std::map<const ParseObject*, std::vector<uint32_t>> nodeLengths;
    std::vector<uint8_t> generatedBytes;

    void incProgramOffset(uint32_t val) {
        if (val > std::numeric_limits<uint32_t>::max() - program_counter) {
            throw std::runtime_error("assembled address space exceeds 32 bits");
        }
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
};

class NodeVisitor {
    public:

    virtual ~NodeVisitor() = default;

    virtual std::any visit(Binary& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Unary& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Grouping& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Number& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Special& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(StringNode& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Symbol& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(DataNode& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(ResNode& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(DirSection& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(DirORG& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(DirTimes& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Instruction& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(MemExpr& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Register& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(Declaration& a, AsmContext& c) {undefinedNode();};
    virtual std::any visit(EquNode& a, AsmContext& c) {undefinedNode();};

    protected:
    [[noreturn]] void undefinedNode();

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
    std::any visit(EquNode& a, AsmContext& c) override;

};

class ExpressionVisitor : public NodeVisitor {
    public:

    virtual std::any visit(Binary& a, AsmContext& c) = 0;
    virtual std::any visit(Unary& a, AsmContext& c) = 0;
    virtual std::any visit(Grouping& a, AsmContext& c) = 0;
    virtual std::any visit(Number& a, AsmContext& c) = 0;
    virtual std::any visit(Special& a, AsmContext& c) = 0;
    virtual std::any visit(Symbol& a, AsmContext& c) = 0;

    virtual std::any visit(StringNode& a, AsmContext& c) = 0;
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
    std::any visit(StringNode& a, AsmContext& c) override;
    std::any visit(Register& a, AsmContext& c) override;

};

class ExpressionEvalVisitor : public ExpressionVisitor {
    public:

    std::any eval(ParseObject& a, AsmContext& c);
    std::any evalMemory(ParseObject& a, AsmContext& c);

    std::any visit(Binary& a, AsmContext& c) override;
    std::any visit(Unary& a, AsmContext& c) override;
    std::any visit(Grouping& a, AsmContext& c) override;
    std::any visit(Number& a, AsmContext& c) override;
    std::any visit(Special& a, AsmContext& c) override;
    std::any visit(Symbol& a, AsmContext& c) override;

    std::any visit(StringNode& a, AsmContext& c) override;
    std::any visit(Register& a, AsmContext& c) override;

    private:
    bool evaluatingMemory = false;
};

class SemanticAnalysisVisitor : public NodeVisitor {
    public:
    std::any analyze(ParseResult& result, AsmContext& c);

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
    std::any visit(EquNode& a, AsmContext& c) override;

    private:
    ExpressionAnalyzerVisitor exprAnalyzer;

    void collectMemoryTerms(ParseObject& a, int sign, MemExpr& memory, AsmContext& c);
    bool containsRegister(ParseObject& a);
    int operandSize(ParseObject& a);
};

class SymbolLengthVisitor : public NodeVisitor {
    public:
    std::any calculate(ParseResult& result, AsmContext& c);

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
    std::any visit(EquNode& a, AsmContext& c) override;

    private:
    ExpressionEvalVisitor evaluator;
    std::set<std::string> definedSymbols;

    void layout(ParseObject& a, AsmContext& c);
    void record(ParseObject& a, AsmContext& c, uint32_t address, uint32_t length);
    void defineSymbol(const std::string& name, uint32_t value, AsmContext& c);
    uint32_t instructionLength(Instruction& a, AsmContext& c);
    uint32_t currentValue(ParseObject& a, AsmContext& c);
};

class CodeGenerationVisitor : public NodeVisitor {
    public:
    std::any generate(ParseResult& result, AsmContext& c);

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
    std::any visit(EquNode& a, AsmContext& c) override;

    private:
    ExpressionEvalVisitor evaluator;
    std::map<const ParseObject*, size_t> addressIndices;

    uint32_t nextAddress(ParseObject& a, AsmContext& c);
    uint32_t nextLength(ParseObject& a, AsmContext& c);
    void emitByte(uint32_t& address, uint8_t value, AsmContext& c);
    void emitValue(uint32_t& address, uint32_t value, uint8_t size, AsmContext& c);
    void ensureOutputRange(uint32_t address, uint32_t length, AsmContext& c);
    uint8_t valueSize(uint8_t operandType);
};

class AssemblerPipeline {
    public:
    std::any assemble(ParseResult& result, AsmContext& c);
};

#endif