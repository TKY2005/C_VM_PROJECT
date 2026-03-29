#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<CPU/registers.h>
#include<CPU/instruction_set.h>
#include<ISA_encoding_info.h>
#include<debuglogger.h>
#include<VM/vm.h>

#include<stdarg.h>
#include<stdlib.h>
#include<stdint.h>

CPU* mkCPU(union registerfile* regfile, instruction* ins_set) {
    CPU* cpu = malloc(sizeof(CPU));

    cpu->arch = 32;
    if(regfile == NULL) cpu->registers = calloc(1, sizeof(union registerfile));
    else cpu->registers = regfile;

    cpu->state = calloc(1, sizeof(CPU_STATE));

    cpu->instruction_set = ins_set;

    return cpu;
}

void CPU_reset_state(CPU* cpu) {
    free(cpu->registers);
    free(cpu->state);
    cpu->registers = calloc(1, sizeof(union registerfile));
    cpu->state = calloc(1, sizeof(CPU_STATE));
}

void CPU_destroy(CPU* cpu) {
    if (cpu){
        free(cpu->instruction_set);
        free(cpu->registers);
        free(cpu->state);
        free(cpu);
    }
}

int CPU_run(CPU* cpu, memory* mem) {
    
    while (cpu->state->CPU_RUNNING) {
        uint8_t opcode = 0;
        int read = mem_read_byte(mem, cpu->registers->PC, &opcode);

        if (read == MEM_READ_FAILURE) {
            // error handling
        }
        else {
            CPU_exec_ins(cpu, mem, opcode);
        }
    }
    return CPU_EXEC_SUCCESS;
}

void CPU_exec_ins(CPU* cpu, memory* mem, uint8_t opcode) {
    
    instruction ins = cpu->instruction_set[opcode];

    if (ins == NULL) {
        char buff[2048] = {0};
        sprintf(buff, "The CPU requested instruction with opcode: 0x%02X but it isn't defined.\n", opcode);
        CPU_fail(cpu, buff);
    }
    else {
        ins(cpu, mem);
        CPU_step(cpu);
    }
}

uint32_t CPU_step(CPU* cpu) {
    cpu->registers->PC++;
    // Simulate delay...
    return cpu->registers->PC;
}
void CPU_step_e(void** arg) {
    CPU* cpu = (CPU*) arg[0];

    cpu->registers->PC++;
    // Simulate delay
    return;
}

int CPU_read_operand_bytes(CPU* cpu, memory* mem, ins_encoding* ins) {

    int oper = CPU_read_opertype(cpu, mem, ins);
    if (oper == MEM_READ_FAILURE) return oper;

    else {
        uint8_t dest = ins->opertype.dest_type; uint8_t src = ins->opertype.src_type;

        if ( CPU_dest_is_reg(dest) || CPU_src_is_reg(src) ) {
            int reg = CPU_read_regselect(cpu, mem, ins);
            if (reg == MEM_READ_FAILURE) return reg;
        }
        if ( CPU_dest_is_mem(dest) || CPU_src_is_mem(src) ) {

            int disp = CPU_read_dispinfo(cpu, mem, ins);
    
            if (disp == MEM_READ_FAILURE) return disp;
            else {
                if (ins->dispinfo.disp_enable) {
                    CPU_step(cpu);
                    CPU_read_displacement_value(cpu, mem, ins);
                    cpu->registers->PC--; // we have to sub 1 because PC gets automatically incremented after each instruction
                }
            }
        }
        if ( CPU_dest_is_num(dest) || CPU_src_is_num(src) ) {
            CPU_step(cpu);
            if (CPU_dest_is_num(dest) || CPU_destsize_32(dest)) CPU_read_imm32(cpu, mem, ins);
            else if (CPU_destsize_16(dest)) CPU_read_imm16(cpu, mem, ins);
            else if (CPU_destsize_8(dest)) CPU_read_imm8(cpu, mem, ins);
            else CPU_read_imm32(cpu, mem, ins);
            cpu->registers->PC--;
        }

        // our last assumption is there is an symbol and read the next 4 bytes
        if ((dest == UNKNOWN && ins->single_oper) || (src == UNKNOWN && dest != UNKNOWN && !ins->single_oper)) { 
            ins->symflg = 1;
            CPU_step(cpu);
            CPU_read_sym(cpu, mem, ins);
            cpu->registers->PC--;
        }
    }
}

int CPU_read_opertype(CPU* cpu, memory* mem, ins_encoding* ins) {

    int read = mem_read_byte(mem, CPU_step(cpu), &ins->operand_types);
    return read;
}
int CPU_read_regselect(CPU* cpu, memory* mem, ins_encoding* ins) {

    int read = mem_read_byte(mem, CPU_step(cpu), &ins->register_select);
    return read;
}
int CPU_read_dispinfo(CPU* cpu, memory* mem, ins_encoding* ins) {

    int read = mem_read_byte(mem, CPU_step(cpu), &ins->displacement_info);
    return read;
}

int CPU_read_imm8(CPU* cpu, memory* mem, ins_encoding* ins) {
    return mem_read_byte_e(mem, cpu->registers->PC, (uint8_t*) &ins->imm_val, CPU_step_e, (void*) &cpu);
}
int CPU_read_imm16(CPU* cpu, memory* mem, ins_encoding* ins) {
    return mem_read_word_e(mem, cpu->registers->PC, (uint16_t*) &ins->imm_val, CPU_step_e, (void*) &cpu);
}
int CPU_read_imm32(CPU* cpu, memory* mem, ins_encoding* ins) {
    return mem_read_dword_e(mem, cpu->registers->PC, &ins->imm_val, CPU_step_e, (void*) &cpu);
}

int CPU_deref_sym(CPU* cpu, memory* mem, ins_encoding* ins, uint32_t* result) {
    if (CPU_destsize_8(ins->opertype.dest_type)) return mem_read_byte(mem, ins->sym, (uint8_t*) result);
    if (CPU_destsize_16(ins->opertype.dest_type)) return mem_read_word(mem, ins->sym, (uint16_t*) result);
    if (CPU_destsize_32(ins->opertype.dest_type)) return mem_read_dword(mem, ins->sym, result);
    else return mem_read_dword(mem, ins->sym, result);
}

int CPU_stack_push(CPU* cpu, memory* mem, uint32_t val) {
    cpu->registers->SP -= 4;
    int wr = mem_write_dword(mem, cpu->registers->SP, val);
    return wr;
}
int CPU_stack_pop(CPU* cpu, memory* mem, uint32_t* result) {
    int rd = mem_read_dword(mem, cpu->registers->SP, result);
    cpu->registers->SP += 4;
    return rd;
}
void CPU_jump_addr(CPU* cpu, uint32_t jmpaddr) {
    cpu->registers->PC = jmpaddr;
    cpu->registers->PC--; // because the CPU auto increments PC after each instruction cycle.
}
void CPU_call_addr(CPU* cpu, memory* mem, uint32_t calladdr) {

    CPU_stack_push(cpu, mem, cpu->registers->PC - 1);
    CPU_jump_addr(cpu, calladdr);
}

void CPU_update_flags8(CPU* cpu, uint8_t dest, uint8_t src, uint8_t val) {
    int8_t flag_setter = (int8_t) val;
    if (flag_setter == 0) reg_set_flags(cpu->registers, FLG_Z);
    if (flag_setter < 0) reg_set_flags(cpu->registers, FLG_N);

    if ((uint16_t) dest + (uint16_t) src > 0xff) reg_set_flags(cpu->registers, FLG_C);
    if ( (dest ^ val) & ((src ^ val) & 0x80) != 0 ) reg_set_flags(cpu->registers, FLG_O);
}
void CPU_update_flags16(CPU* cpu, uint16_t dest, uint16_t src, uint16_t val) {
    
    if ( (int16_t) val == 0 ) reg_set_flags(cpu->registers, FLG_Z);
    if ( (int16_t) val < 0) reg_set_flags(cpu->registers, FLG_N);

    if ( (uint32_t) dest + (uint32_t) src > 0xffff ) reg_set_flags(cpu->registers, FLG_C);
    if ( (dest ^ val) & ( (src ^ val) & 0x8000 ) != 0 ) reg_set_flags(cpu->registers, FLG_O);
}
void CPU_update_flags32(CPU* cpu, uint32_t dest, uint32_t src, uint32_t val) {

    if ((int32_t) val == 0) reg_set_flags(cpu->registers, FLG_Z);
    if ((int32_t) val < 0) reg_set_flags(cpu->registers, FLG_N);

    if ( (uint64_t) dest + (uint64_t) src > 0xffffffff ) reg_set_flags(cpu->registers, FLG_C);
    if ( (dest ^ val) & ( (src ^ val) & 0x80000000 ) != 0 ) reg_set_flags(cpu->registers, FLG_O);
}

void CPU_update_flags(CPU* cpu, ins_encoding* ins, uint32_t dest, uint32_t src, uint32_t val) {

    if (ins->flg_mod) {
        if (CPU_destsize_8(ins->opertype.dest_type)) CPU_update_flags8(cpu, dest, src, val);
        else if (CPU_destsize_16(ins->opertype.dest_type)) CPU_update_flags16(cpu, dest, src, val);
        else if (CPU_destsize_32(ins->opertype.dest_type)) CPU_update_flags32(cpu, dest, src, val);
    }
}

int CPU_read_displacement_value(CPU* cpu, memory* mem, ins_encoding* ins) {
    return mem_read_dword_e(mem, cpu->registers->PC, &ins->disp_val, CPU_step_e, (void*) &cpu);
}

int CPU_read_sym(CPU* cpu, memory* mem, ins_encoding* ins) {
    return mem_read_dword_e(mem, cpu->registers->PC, &ins->sym, CPU_step_e, (void*) &cpu);
}


uint32_t CPU_decode_dest(CPU* cpu, memory* mem, ins_encoding* ins) {

    uint32_t val = 0;
    if ( CPU_dest_is_reg(ins->opertype.dest_type) ) {

        int reg_idx = CPU_get_reg_idx(ins->regselect.dest);

        switch(ins->opertype.dest_type) {
            case REG8:
            val = reg_read_b(cpu->registers, reg_idx);
            break;
            case REG16:
            val = reg_read_w(cpu->registers, reg_idx / 2);
            break;
            case REG32:
            val = reg_read_dw(cpu->registers, reg_idx / 4);
            break;
        }
    }

    else if ( CPU_dest_is_mem(ins->opertype.dest_type) ) {

        uint32_t addr = CPU_calc_addr(cpu, ins);
        switch (ins->opertype.dest_type) {
            case MEM8:
                mem_read_byte(mem, addr, (uint8_t*) &val);
                break;
            case MEM16:
                mem_read_word(mem, addr, (uint16_t*) &val);
                break;
            case MEM32:
                mem_read_dword(mem, addr, &val);
                break;
            
        }
    }

    else if ( CPU_dest_is_num(ins->opertype.dest_type) ) {
        val = ins->imm_val;
    }
    else if (ins->symflg) {
        val = ins->sym;
    }
    else CPU_fail(cpu, "Couldn't decode destination operand: %02X\n", ins->operand_types);
    return val;
}

uint32_t CPU_decode_src(CPU* cpu, memory* mem, ins_encoding* ins) {
    uint32_t val = 0;
    if ( CPU_src_is_reg(ins->opertype.src_type) ) {

        int reg_idx = CPU_get_reg_idx(ins->regselect.src);

        switch(ins->opertype.src_type) {
            case REG8:
            val = reg_read_b(cpu->registers, reg_idx);
            break;
            case REG16:
            val = reg_read_w(cpu->registers, reg_idx / 2);
            break;
            case REG32:
            val = reg_read_dw(cpu->registers, reg_idx / 4);
            break;
        }
    }

    else if ( CPU_src_is_mem(ins->opertype.src_type) ) {

        uint32_t addr = CPU_calc_addr(cpu, ins);
        switch (ins->opertype.src_type) {
            case MEM8:
                mem_read_byte(mem, addr, (uint8_t*) &val);
                break;
            case MEM16:
                mem_read_word(mem, addr, (uint16_t*) &val);
                break;
            case MEM32:
                mem_read_dword(mem, addr, &val);
                break;
            
        }
    }

    else if ( CPU_src_is_num(ins->opertype.src_type) ) {
        val = ins->imm_val;
    }

    else if (ins->symflg) {
        val = ins->sym;

        if (ins->resolve_sym) {
            CPU_deref_sym(cpu, mem, ins, &val);
        }
    }

    else CPU_fail(cpu, "Couldn't decode source operand: %02X\n", ins->operand_types);
    return val;
}

int CPU_write_dest(CPU* cpu, memory* mem, ins_encoding* ins, uint32_t val) {

    if (CPU_dest_is_reg(ins->opertype.dest_type)){

        int idx = CPU_get_reg_idx(ins->regselect.dest);

        switch(ins->opertype.dest_type) {
            case REG8:
            reg_write_b(cpu->registers, idx, (uint8_t) val);
            break;
            case REG16:
            reg_write_w(cpu->registers, idx / 2, (uint16_t) val);
            break;
            case REG32:
            reg_write_dw(cpu->registers, idx / 4, val);
            break;
        }
    }
    else if (CPU_dest_is_mem(ins->opertype.dest_type)) {

        uint32_t addr = CPU_calc_addr(cpu, ins);

        switch(ins->opertype.dest_type) {
            case MEM8:
            mem_write_byte(mem, addr, (uint8_t) val);
            break;
            case MEM16:
            mem_write_word(mem, addr, (uint16_t) val);
            break;
            case MEM32:
            mem_write_dword(mem, addr, val);
            break;
        }
    }
    else {
        CPU_fail(cpu, "Invalid destination type.");
    }
}

uint32_t CPU_calc_addr(CPU* cpu, ins_encoding* ins) {
    uint32_t index = 0, base = 0;
    int scale = 0;
    switch(ins->opertype.mem_mode) {
        case X1SCALE:
        scale = 1;
        break;
        case X2SCALE:
        scale = 2;
        break;
        case X4SCALE:
        scale = 4;
        break;
    }

    if (ins->dispinfo.reg_enable) {
        if (ins->dispinfo.index != INDEX_UNSELECTED)
            index = reg_read_dw(cpu->registers, CPU_get_reg_idx(ins->dispinfo.index) / 4);

        if (ins->dispinfo.base != BASE_UNSELECTED)
            base = reg_read_dw(cpu->registers ,CPU_get_base_idx(ins->dispinfo.base) / 4);
    }
    uint32_t addr = base + (index * scale) + ins->disp_val;
    return addr;
}

int CPU_get_reg_idx(int idx) {
    
    if (idx >= REG_EAX && idx <= REG_EFX) return idx * 4;
    else {
        switch(idx) {

            case REG_SI:
            return REG_SI_IDX * 4;
            break;
            
            case REG_DI:
            return REG_DI_IDX * 4;
            break;
            
            case REG_BP:
            return REG_BP_IDX * 4;
            break;
            
            case REG_SP:
            return REG_SP_IDX * 4;
            break;
            
            case REG_SS:
            return REG_SS_IDX * 4;
            break;
            
            case REG_DS:
            return REG_DS_IDX * 4;
            break;
            
            case REG_CS:
            return REG_CS_IDX * 4;
            break;

            default:
            return -1;
            break;
        }
    }
    return -1;
}

int CPU_get_base_idx(int base) {
    switch(base) {
        case SP:
        return REG_SP_IDX * 4;
        break;
        case BP:
        return REG_BP_IDX * 4;
        break;

        case SI:
        return REG_SI_IDX * 4;
        break;

        case DI:
        return REG_DI_IDX * 4;
        break;

        case CS:
        return REG_CS_IDX * 4;
        break;

        case SS:
        return REG_SS_IDX * 4;
        break;

        case DS:
        return REG_DS_IDX * 4;
        break;

        default:
        return -1;
        break;
    }
    return -1;
}

int CPU_dest_is_reg(uint8_t dest) {
    return ( dest >= REG8 && dest <= REG32 );
}
int CPU_src_is_reg(uint8_t src) {
    return ( src >= REG8 && src <= REG32 );
}
int CPU_dest_is_mem(uint8_t dest) {
    return ( dest >= MEM8 && dest <= MEM32 );
}
int CPU_src_is_mem(uint8_t src) {
    return ( src >= MEM8 && src <= MEM32 );
}
int CPU_dest_is_num(uint8_t dest) {
    return ( dest == NUMBER );
}
int CPU_src_is_num(uint8_t src) {
    return (src == NUMBER);
}

int CPU_destsize_32(uint8_t dest) {
    return ( dest == REG32 || dest == MEM32 );
}
int CPU_destsize_16(uint8_t dest) {
    return (dest == REG16 || dest == MEM16);
}
int CPU_destsize_8(uint8_t dest) {
    return (dest == REG8 || dest == MEM8);
}

int CPU_srcsize_32(uint8_t src) {
    return ( src == REG32 || src == MEM32 );
}
int CPU_srcsize_16(uint8_t src) {
    return ( src == REG16 || src == MEM16 );
}
int CPU_srcsize_8(uint8_t src) {
    return ( src == REG8 || src == MEM8 );
}

void CPU_write_out(CPU* cpu, memory* mem, const char* txt, ...) {

    char buff[4096] = {0};
    va_list args;
    va_start(args, txt);

    vsnprintf(buff, sizeof(buff), txt, args);
    
    va_end(args);

    vm_interrupt(cpu, mem, VM_INTR_WRITE);
}

void CPU_fail(CPU* cpu, const char* msg, ...) {
    
    printd("\n0x%08X: ", cpu->registers->PC);
    printd(msg);
    cpu->state->CPU_RUNNING = 0;
}