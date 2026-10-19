#ifndef CPU_H
#define CPU_H

#include<stdint.h>
#include<stdarg.h>

#include<Memory/memory.h>
#include<CPU/registers.h>
#include<CPU/instruction_set.h>
#include<ISA_encoding_info.h>

#define ROM_START_ADDR 0xf9C00000u

#define CPU_EXEC_SUCCESS 0

#define CPU_UNDEFINED_OPCODE -1

typedef union {
    struct {
        uint8_t CPU_RUNNING : 1;
        uint8_t CPU_HALTED : 1;
        uint8_t CPU_INTERRUPTED : 1;
        uint8_t CPU_INTERRUPT_LINE_ENABLE : 1;
        uint8_t CPU_PREVILEGED_MODE : 1;
    };
    uint8_t state;
} CPU_STATE;

typedef struct CPU {
    uint8_t arch;
	union registerfile* registers;
    CPU_STATE* state;
    instruction* instruction_set;
    uint64_t clock_delay_ms;
} CPU;

// Initializes the CPU with the given registers and instructions set
CPU* CPU_init(union registerfile* regfile, instruction* ins_set);
// Resets the CPU state
void CPU_reset_state(CPU* cpu);
// when called the CPU will enter the fetch-decode-execute cycle starting at the memory address in PC register
int CPU_run(CPU* cpu, memory* mem);
// makes the CPU execute the instruction that corresponds to the opcode and increments the PC register
void CPU_exec_ins(CPU* cpu, memory* mem, uint8_t opcode);

void CPU_destroy(CPU* cpu);
// increments the PC register and returns its value
uint32_t CPU_step(CPU* cpu);
// increments the PC register (used as an event)
void CPU_step_e(void** arg);

// Reads the opertypes byte and reads the rest of the instruction operands accordingly. puts the result in 'ins'
int CPU_read_operand_bytes(CPU* cpu, memory* mem, ins_encoding* ins);
// Reads the operand types byte and places the result in 'ins->operand_types'
int CPU_read_opertype(CPU* cpu, memory* mem, ins_encoding* ins);
// Reads the register select byte and places the result in 'ins->register_select'
int CPU_read_regselect(CPU* cpu, memory* mem, ins_encoding* ins);
// Reads the memory info byte and places the result in 'ins->displacement_info'
int CPU_read_dispinfo(CPU* cpu, memory* mem, ins_encoding* ins);

// Reads the memory displacement value, places it in 'ins->disp_val', and increments the PC register
int CPU_read_displacement_value(CPU* cpu, memory* mem, ins_encoding* ins);
// Reads a symbol, places it in 'ins->sym', and incremets the PC register
int CPU_read_sym(CPU* cpu, memory* mem, ins_encoding* ins);

// Reads an 8-bit immediate, places it in 'ins->imm_val', and increments the PC register
int CPU_read_imm8(CPU* cpu, memory* mem, ins_encoding* ins);
// Reads a 16-bit immediate, places it in 'ins->imm_val', and increments the PC register
int CPU_read_imm16(CPU* cpu, memory* mem, ins_encoding* ins);
// Reads a 32-bit immediate, places it in 'ins->imm_val', and increments the PC register
int CPU_read_imm32(CPU* cpu, memory* mem, ins_encoding* ins);

int CPU_interrupt(CPU* cpu, memory* mem, uint8_t icode);

// reads the lh_select bit for 8-bit registers in 'reg' and returns the result
int CPU_check_lh_8bit(REG_SELECT* reg, uint8_t operselect);
// reads the register select bits for 8-bit registers in 'reg' and returns the result
int CPU_get_regcode_8bit(REG_SELECT* reg, uint8_t operselect);

// reads the 'ins->opertype.dest_type' field, fetches the operand value accordingly and returns the result
uint32_t CPU_decode_dest(CPU* cpu, memory* mem, ins_encoding* ins);
// reads the 'ins->opertype.src_type' field, fetches the operand value accordingly and returns the result
uint32_t CPU_decode_src(CPU* cpu, memory* mem, ins_encoding* ins);
// goes to the given address in 'ins->sym' reads a number of bytes and places the value in 'result'
int CPU_deref_sym(CPU* cpu, memory* mem, ins_encoding* ins, uint32_t* result);
// writes the given value to the destination operand accordingly
int CPU_write_dest(CPU* cpu, memory* mem, ins_encoding* ins, uint32_t val);
// decrements the SP register by 4 and writes a 32-bit value to the address pointed to by SP
int CPU_stack_push(CPU* cpu, memory* mem, uint32_t val);
// reads a 32-bit value from the address pointed to by SP and increments SP by 4
int CPU_stack_pop(CPU* cpu, memory* mem, uint32_t* result);
// Changes the value of the PC register to the given jump address
void CPU_jump_addr(CPU* cpu, uint32_t jmpaddr);
// Pushes the current PC register value to the address pointed to by SP and changes the PC value to the given call address
void CPU_call_addr(CPU* cpu, memory* mem, uint32_t calladdr);
// Updates the flags for 8-bit operations
void CPU_update_flags8(CPU* cpu, uint8_t dest, uint8_t src, uint8_t val);
// Updates the flags for 16-bit operations
void CPU_update_flags16(CPU* cpu, uint16_t dest, uint16_t src, uint16_t val);
// Updates the flags for 32-bit operations
void CPU_update_flags32(CPU* cpu, uint32_t dest, uint32_t src, uint32_t val);
// Resets the flags and updates its value according to the destination operand
void CPU_update_flags(CPU* cpu, ins_encoding* ins, uint32_t dest, uint32_t src, uint32_t val);

// returns the effective address using the formula: base + (index * scale) + displacement
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
