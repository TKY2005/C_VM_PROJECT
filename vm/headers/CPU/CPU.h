#ifndef CPU_H
#define CPU_H

#include<stdint.h>

#include<headers/Memory/memory.h>
#include<headers/CPU/registers.h>
#include<headers/CPU/instruction_set.h>
#include<ISA_encoding_info.h>
#include<stdarg.h>

#define CPU_EXEC_SUCCESS 0

#define CPU_UNDEFINED_OPCODE -1

typedef union {
    struct {
        uint8_t CPU_RUNNING : 1;
        uint8_t CPU_HALTED : 1;
        uint8_t CPU_INTERRUPTED : 1;
    };
    uint8_t state;
} CPU_STATE;

typedef struct CPU {
    uint8_t arch;
	union registerfile* registers;
    CPU_STATE* state;
    instruction* instruction_set;
} CPU;


CPU* mkCPU(union registerfile* regfile, instruction* ins_set);
int CPU_run(CPU* cpu, memory* mem);
void CPU_exec_ins(CPU* cpu, memory* mem, uint8_t opcode);

void CPU_destroy(CPU* cpu);

uint32_t CPU_step(CPU* cpu);
int CPU_read_opertype(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_regselect(CPU* cpu, memory* mem, ins_encoding* ins);

uint32_t CPU_decode_dest(CPU* cpu, memory* mem, ins_encoding* ins);
uint32_t CPU_decode_src(CPU* cpu, memory* mem, ins_encoding* ins);

void CPU_fail(CPU* cpu, const char* msg, ...);

#endif
