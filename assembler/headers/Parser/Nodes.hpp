#ifndef NODES_HPP
#define NODES_HPP

#include<cstdint>
#include<string>

#include "./Visitors.hpp"


class ProgramNode {
    public:
    virtual ~ProgramNode() = default;

    ProgramNode* child = nullptr;
    ProgramNode* sibling = nullptr;

    void setAddress(uint32_t address) {this->address = address;}
    void setLength(uint32_t len) {this->length = len;}
    void setStr(std::string s) {this->nodestr = s;}

    uint32_t getAddress() {return address;}
    uint32_t getLength() {return length;}
    std::string getStr() {return nodestr;}

    ProgramNode() {}
    ProgramNode(std::string s) {this->nodestr = s;}
    ProgramNode(std::string s, uint32_t addr, uint32_t len) {
        nodestr = s;
        address = addr;
        length = len;
    }
    ProgramNode(std::string s, uint32_t addr) {
        nodestr = s;
        address = addr;
    }

    protected:
    virtual void accept(NodeVisitor& v) = 0;

    std::string nodestr;
    uint32_t address = 0;
    uint32_t length = 0;
};

class InstructionNode : protected ProgramNode {
    public:
    uint8_t operandCount = 0;

    InstructionNode(uint8_t operandCount) {this->operandCount = operandCount;}

    void accept(NodeVisitor& v) override {
        v.visit(*this);
    }
};

class OperandNode;
class DirectiveNode;
class ExpressionNode;
class LabelNode;

#endif