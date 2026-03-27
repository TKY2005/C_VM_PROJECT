#ifndef CPU_H
#define CPU_H

#include<stdint.h>
#include<stdarg.h>

#include<Memory/memory.h>
#include<CPU/registers.h>
#include<CPU/instruction_set.h>
#include<ISA_encoding_info.h>

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
void CPU_step_e(void** arg);

int CPU_read_operand_bytes(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_opertype(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_regselect(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_dispinfo(CPU* cpu, memory* mem, ins_encoding* ins);

int CPU_read_displacement_value(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_sym(CPU* cpu, memory* mem, ins_encoding* ins);

int CPU_read_imm8(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_imm16(CPU* cpu, memory* mem, ins_encoding* ins);
int CPU_read_imm32(CPU* cpu, memory* mem, ins_encoding* ins);

uint32_t CPU_decode_dest(CPU* cpu, memory* mem, ins_encoding* ins);
uint32_t CPU_decode_src(CPU* cpu, memory* mem, ins_encoding* ins);

int CPU_deref_sym(CPU* cpu, memory* mem, ins_encoding* ins, uint32_t* result);

int CPU_write_dest(CPU* cpu, memory* mem, ins_encoding* ins, uint32_t val);
int CPU_stack_push(CPU* cpu, memory* mem, uint32_t val);
int CPU_stack_pop(CPU* cpu, memory* mem, uint32_t* result);
void CPU_jump_addr(CPU* cpu, uint32_t jmpaddr);
void CPU_call_addr(CPU* cpu, memory* mem, uint32_t calladdr);

void CPU_update_flags8(CPU* cpu, uint8_t dest, uint8_t src, uint8_t val);
void CPU_update_flags16(CPU* cpu, uint16_t dest, uint16_t src, uint16_t val);
void CPU_update_flags32(CPU* cpu, uint32_t dest, uint32_t src, uint32_t val);

void CPU_update_flags(CPU* cpu, ins_encoding* ins, uint32_t dest, uint32_t src, uint32_t val);

uint32_t CPU_calc_addr(CPU* cpu, ins_encoding* ins);

int CPU_get_reg_idx(int idx);
int CPU_get_base_idx(int base);

int CPU_dest_is_reg(uint8_t dest);
int CPU_src_is_reg(uint8_t src);
int CPU_dest_is_mem(uint8_t dest);
int CPU_src_is_mem(uint8_t src);
int CPU_dest_is_num(uint8_t dest);
int CPU_src_is_num(uint8_t src);

int CPU_destsize_32(uint8_t dest);
int CPU_destsize_16(uint8_t dest);
int CPU_destsize_8(uint8_t dest);

int CPU_srcsize_32(uint8_t src);
int CPU_srcsize_16(uint8_t src);
int CPU_srcsize_8(uint8_t src);

void CPU_write_out(CPU* cpu, memory* mem, const char* txt, ...);

void CPU_fail(CPU* cpu, const char* msg, ...);

#endif
