#include<stdint.h>
#include<stdlib.h>

#include<Memory/memory.h>
#include<CPU/instruction_set.h>
#include<CPU/CPU.h>
#include<ISA_encoding_info.h>
#include<math.h>
#include<time.h>


instruction* create_ins_table() {
    instruction* ins_table = (instruction*) calloc(0x100, sizeof(void*));
    return ins_table;
}

void free_ins_table(instruction* table) {
    free(table);
}

#define X(mnemonic, opcode, funcname) t[INS_##mnemonic] = funcname;

instruction* setup_instruction_set() {

    instruction* t = create_ins_table();

    INSTRUCTION_LIST(X)
    return t;
}

#undef X

void _ext(CPU* cpu, memory* mem) {
    cpu->state->CPU_RUNNING = 0;
}

void _nop(CPU* cpu, memory* mem) {
    printf("nop.\n");
}

void _set(CPU* cpu, memory* mem) {
    
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_src(cpu, mem, ins);
        CPU_write_dest(cpu, mem, ins, val);
    }
}

void _out(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);

    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        printf("%u", val);
    }

}

void _add(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t dest = CPU_decode_dest(cpu, mem, ins);
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        uint32_t val = dest + src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val);
    }
}
void _sub(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t dest = CPU_decode_dest(cpu, mem, ins);
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        uint32_t val = dest - src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val);
    }
}
void _mul(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t dest = CPU_decode_dest(cpu, mem, ins);
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        uint32_t val = dest * src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val);
    }
}
void _div(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t dest = CPU_decode_dest(cpu, mem, ins);
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        uint32_t val = dest / src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val);
    }
}
void _pow(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t dest = CPU_decode_dest(cpu, mem, ins);
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        uint32_t val = pow(dest, src);
        CPU_write_dest(cpu, mem, ins, val);
    }
}
void _sqrt(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        val = sqrt(val);
        CPU_write_dest(cpu, mem, ins, val);
    }
}

void _rnd(CPU* cpu, memory* mem) { // src is the upper bound whereas the lower bound is 0

    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        srand(time(NULL));
        uint32_t val = rand() % (src - 0 + 1) + 0;
        CPU_write_dest(cpu, mem, ins, val);
    }
}
void _cti(CPU* cpu, memory* mem) { }

void _inc(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        val++;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, val - 1, 1, val);
    }
}
void _dec(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        val--;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, val + 1, 1, val);
    }
}

void _la(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_src(cpu, mem, ins);
        CPU_write_dest(cpu, mem, ins, val);
    }
}
void _len(CPU* cpu, memory* mem) { }
void _str(CPU* cpu, memory* mem) { }
void _outs(CPU* cpu, memory* mem) { }
void _inp(CPU* cpu, memory* mem) { }
void _inps(CPU* cpu, memory* mem) { }

void _push(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        CPU_stack_push(cpu, mem, val);
    }
}

void _pop(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t val = 0;
        CPU_stack_pop(cpu, mem, &val);
        CPU_write_dest(cpu, mem, ins, val);
    }
}

void _call(CPU* cpu, memory* mem) {
    ins_encoding dummy = {0};
    CPU_step(cpu); // position the PC at the beginning of the address.
    CPU_read_sym(cpu, mem, &dummy);
    CPU_call_addr(cpu, mem, dummy.sym);
}
void _cale(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calne(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _cal(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calle(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calg(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calge(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calb(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}

void _jmp(CPU* cpu, memory* mem) {

    ins_encoding dummy = {0};
    CPU_step(cpu); // position the PC at the beginning of the address.
    CPU_read_sym(cpu, mem, &dummy);
    CPU_jump_addr(cpu, dummy.sym);
}
void _je(CPU* cpu, memory* mem) {

    if (reg_check_flag(cpu->registers, FLG_Z)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jne(CPU* cpu, memory* mem) {

    if (!reg_check_flag(cpu->registers, FLG_Z)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jl(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_N)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jle(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_Z) || reg_check_flag(cpu->registers, FLG_N)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jg(CPU* cpu, memory* mem) {
    if ( !reg_check_flag(cpu->registers, FLG_N) ) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jge(CPU* cpu, memory* mem) {
    if ( !reg_check_flag(cpu->registers, FLG_N) || reg_check_flag(cpu->registers, FLG_Z) ) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jb(CPU* cpu, memory* mem) {
    if ( reg_check_flag(cpu->registers, FLG_O) && reg_check_flag(cpu->registers, FLG_N) ) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _and_(CPU* cpu, memory* mem) { }
void _or_(CPU* cpu, memory* mem) { }
void _xor(CPU* cpu, memory* mem) { }
void _not_(CPU* cpu, memory* mem) { }
void _nand(CPU* cpu, memory* mem) { }
void _nor(CPU* cpu, memory* mem) { }

void _cmp(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->resolve_sym = 1;
    ins->flg_mod = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t dest = CPU_decode_dest(cpu, mem, ins);
        uint32_t src = CPU_decode_src(cpu, mem, ins);
        uint32_t val = dest - src;
        CPU_update_flags(cpu, ins, dest, src, val);
    }
}
void _loop(CPU* cpu, memory* mem) {
    
    cpu->registers->C--;
    if (cpu->registers->C > 0){
        _jmp(cpu, mem);
    }
    else {
        cpu->registers->PC += 4;
    }
}

void _ret(CPU* cpu, memory* mem) {
    CPU_stack_pop(cpu, mem, &cpu->registers->PC);
}

void _end(CPU* cpu, memory* mem) { }
void _int_(CPU* cpu, memory* mem) { }

void _outc(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);

    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X. CPU shutting down.\n", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        printf("%c", val);
    }
}

void _shl(CPU* cpu, memory* mem) { }
void _shr(CPU* cpu, memory* mem) { }
void _outsw(CPU* cpu, memory* mem) { }
void _lenw(CPU* cpu, memory* mem) { }
void _jc(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_C)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jo(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_O)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jnc(CPU* cpu, memory* mem) {
    if (!reg_check_flag(cpu->registers, FLG_C)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _jno(CPU* cpu, memory* mem) {
    if (!reg_check_flag(cpu->registers, FLG_O)) _jmp(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calc(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_C)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calo(CPU* cpu, memory* mem) {
    if (reg_check_flag(cpu->registers, FLG_O)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calnc(CPU* cpu, memory* mem) {
    if (!reg_check_flag(cpu->registers, FLG_C)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _calno(CPU* cpu, memory* mem) {
    if (!reg_check_flag(cpu->registers, FLG_O)) _call(cpu, mem);
    else {
        cpu->registers->PC += 4;
    }
}
void _clc(CPU* cpu, memory* mem) {
    reg_clear_flags(cpu->registers, FLG_C);
}
void _clo(CPU* cpu, memory* mem) {
    reg_clear_flags(cpu->registers, FLG_O);
}
void _clz(CPU* cpu, memory* mem) {
    reg_clear_flags(cpu->registers, FLG_Z);
}
void _cln(CPU* cpu, memory* mem) {
    reg_clear_flags(cpu->registers, FLG_N);
}
void _cli(CPU* cpu, memory* mem) {
    reg_clear_flags(cpu->registers, FLG_I);
}
void _slc(CPU* cpu, memory* mem) {
    reg_set_flags(cpu->registers, FLG_C);
}
void _slo(CPU* cpu, memory* mem) {
    reg_set_flags(cpu->registers, FLG_O);
}
void _slz(CPU* cpu, memory* mem) {
    reg_set_flags(cpu->registers, FLG_Z);
}
void _sln(CPU* cpu, memory* mem) {
    reg_set_flags(cpu->registers, FLG_N);
}
void _sli(CPU* cpu, memory* mem) {
    reg_set_flags(cpu->registers, FLG_I);
}