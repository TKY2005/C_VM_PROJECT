#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include<stdint.h>

#include<ISA_encoding_info.h>

typedef struct CPU CPU;
typedef struct memory memory;

typedef void (*instruction)(CPU*, struct memory*);

instruction* create_ins_table();
void free_ins_table(instruction* table);

#define X(mnemonic, opcode, funcname) void funcname(CPU* cpu, memory* mem);

INSTRUCTION_LIST(X)

#undef X

instruction* setup_instruction_set();

#endif