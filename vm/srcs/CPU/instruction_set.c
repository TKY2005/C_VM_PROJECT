#include<stdint.h>
#include<stdlib.h>

#include<Memory/memory.h>
#include<CPU/instruction_set.h>
#include<CPU/CPU.h>
#include<VM/vm.h>
#include<VM/vm_input.h>
#include<ISA_encoding_info.h>
#include<Timing/clock.h>
#include<VM/vm_io.h>
#include<math.h>
#include<time.h>

/*TODO: 
implement the new branch instructions
*/

instruction* create_ins_table() {
    instruction* ins_table = (instruction*) calloc(0x100, sizeof(instruction*));
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
    reg_clear_flags(cpu->registers, FLG_I);
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
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_ADD);
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
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_SUB);
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
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
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

        if (src == 0) CPU_interrupt(cpu, mem, CPU_EXCEPTION_DIVZERO);
        else {
            uint32_t val = dest / src;
            CPU_write_dest(cpu, mem, ins, val);
            CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
        }
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
        CPU_update_flags(cpu, ins, val - 1, 1, val, CPU_FLAG_OP_ADD);
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
        CPU_update_flags(cpu, ins, val + 1, 1, val, CPU_FLAG_OP_SUB);
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
void _outs(CPU* cpu, memory* mem) {

    uint32_t straddr = cpu->registers->SI;
    uint8_t chr;
    uint32_t offset = 0;
    int read;
    while ( (read = mem_read_byte(mem, straddr + offset, &chr)) != MEM_READ_FAILURE ) {
        if (chr == '\0') break;
        printf("%c", chr);
        clock_simulate_delay(cpu->clock_delay_ms);
        offset++;
    }
}
void _inp(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    encoding.resolve_sym = 1;

    int read = CPU_read_operand_bytes(cpu, mem, &encoding);

    uint32_t port = CPU_decode_src(cpu, mem, &encoding);

    uint32_t result = 0;
    int r = io_read((uint8_t) port, &result);

    CPU_write_dest(cpu, mem, &encoding, result);
}
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
    ins_encoding ins = {0};
    ins.single_oper = 1;
    int read = CPU_read_operand_bytes(cpu, mem, &ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
        return;
    }
    uint32_t calladdr = CPU_decode_dest(cpu, mem, &ins);
    CPU_call_addr(cpu, mem, calladdr);
}
typedef enum {
    BRANCH_ALWAYS, BRANCH_EQUAL, BRANCH_NOT_EQUAL, BRANCH_LESS,
    BRANCH_LESS_EQUAL, BRANCH_GREATER, BRANCH_GREATER_EQUAL,
    BRANCH_BELOW, BRANCH_CARRY, BRANCH_OVERFLOW, BRANCH_NOT_CARRY,
    BRANCH_NOT_OVERFLOW, BRANCH_ABOVE, BRANCH_ABOVE_EQUAL, BRANCH_BELOW_EQUAL
} branch_condition;

static int CPU_branch_condition(CPU* cpu, branch_condition condition) {
    int z = reg_check_flag(cpu->registers, FLG_Z) != 0;
    int n = reg_check_flag(cpu->registers, FLG_N) != 0;
    int c = reg_check_flag(cpu->registers, FLG_C) != 0;
    int o = reg_check_flag(cpu->registers, FLG_O) != 0;
    switch (condition) {
        case BRANCH_ALWAYS: return 1;
        case BRANCH_EQUAL: return z;
        case BRANCH_NOT_EQUAL: return !z;
        case BRANCH_LESS: return n != o;
        case BRANCH_LESS_EQUAL: return z || (n != o);
        case BRANCH_GREATER: return !z && (n == o);
        case BRANCH_GREATER_EQUAL: return n == o;
        case BRANCH_BELOW:
        case BRANCH_CARRY: return c;
        case BRANCH_NOT_CARRY: return !c;
        case BRANCH_OVERFLOW: return o;
        case BRANCH_NOT_OVERFLOW: return !o;
        case BRANCH_ABOVE: return !c && !z;
        case BRANCH_ABOVE_EQUAL: return !c;
        case BRANCH_BELOW_EQUAL: return c || z;
        default: return 0;
    }
}

static void CPU_conditional_transfer(CPU* cpu, memory* mem, branch_condition condition, int is_call) {
    if (CPU_branch_condition(cpu, condition)) {
        if (is_call) _call(cpu, mem);
        else _jmp(cpu, mem);
    } else {
        ins_encoding skipped = {.single_oper = 1};
        if (CPU_read_operand_bytes(cpu, mem, &skipped) == MEM_READ_FAILURE)
            CPU_fail(cpu, "Invalid branch operand @ 0x%08X.", cpu->registers->PC);
    }
}

void _cale(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_EQUAL, 1);
}
void _calne(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_NOT_EQUAL, 1);
}
void _cal(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_LESS, 1);
}
void _calle(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_LESS_EQUAL, 1);
}
void _calg(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_GREATER, 1);
}
void _calge(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_GREATER_EQUAL, 1);
}
void _calb(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_BELOW, 1);
}

void _jmp(CPU* cpu, memory* mem) {

    ins_encoding ins = {0};
    ins.single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, &ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid branch operand @ 0x%08X.", cpu->registers->PC);
        return;
    }
    uint32_t jmpval = CPU_decode_dest(cpu, mem, &ins);
    CPU_jump_addr(cpu, jmpval);

}
void _je(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_EQUAL, 0);
}
void _jne(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_NOT_EQUAL, 0);
}
void _jl(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_LESS, 0);
}
void _jle(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_LESS_EQUAL, 0);
}
void _jg(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_GREATER, 0);
}
void _jge(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_GREATER_EQUAL, 0);
}

void _jb(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_BELOW, 0);
}

void _and_(CPU* cpu, memory* mem) {
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
        uint32_t val = dest & src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
    }
}

void _or_(CPU* cpu, memory* mem) {
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
        uint32_t val = dest | src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
    }
}

void _xor(CPU* cpu, memory* mem) {
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
        uint32_t val = dest ^ src;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
    }
}

void _not_(CPU* cpu, memory* mem) {
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
        val = ~val;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, ~val, 1, val, CPU_FLAG_OP_BITWISE);
    }
}
void _nand(CPU* cpu, memory* mem) {
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
        uint32_t val = ~(dest & src);
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
    }
}

void _nor(CPU* cpu, memory* mem) {
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
        uint32_t val = ~(dest | src);
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
    }
}

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
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_SUB);
    }
}
void _loop(CPU* cpu, memory* mem) {
    
    cpu->registers->C--;
    if (cpu->registers->C != 0){
        _jmp(cpu, mem);
    }
    else {
        CPU_read_operand_bytes(cpu, mem, &(ins_encoding){.single_oper = 1});
    }
}

void _ret(CPU* cpu, memory* mem) {
    CPU_stack_pop(cpu, mem, &cpu->registers->PC);
}

void _end(CPU* cpu, memory* mem) { }

void _int_(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;
    ins->single_oper = 1;

    int read = CPU_read_operand_bytes(cpu, mem, ins);
    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X.", cpu->registers->PC);
    }
    else {
        uint32_t intcode = CPU_decode_dest(cpu, mem, ins);
        CPU_interrupt(cpu, mem, intcode);
    }
}

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
        printf("%c", vmchr_to_platform(val));
    }
}

void _shl(CPU* cpu, memory* mem) {
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
        uint8_t width_bits = CPU_destsize_8(ins->opertype.dest_type) ? 8
            : CPU_destsize_16(ins->opertype.dest_type) ? 16 : 32;
        uint32_t width_mask = width_bits == 8 ? 0xffu : width_bits == 16 ? 0xffffu : 0xffffffffu;
        uint32_t val = src >= width_bits ? 0u : (dest << src) & width_mask;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_SHIFT_LEFT);
    }
}
void _shr(CPU* cpu, memory* mem) {
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
        uint8_t width_bits = CPU_destsize_8(ins->opertype.dest_type) ? 8
            : CPU_destsize_16(ins->opertype.dest_type) ? 16 : 32;
        uint32_t width_mask = width_bits == 8 ? 0xffu : width_bits == 16 ? 0xffffu : 0xffffffffu;
        uint32_t val = src >= width_bits ? 0u : (dest >> src) & width_mask;
        CPU_write_dest(cpu, mem, ins, val);
        CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_SHIFT_RIGHT);
    }
}
void _outsw(CPU* cpu, memory* mem) {
    uint32_t straddr = cpu->registers->SI;
    uint16_t chr;
    uint32_t offset = 0;
    int read;
    while ( (read = mem_read_word(mem, straddr + offset, &chr)) != MEM_READ_FAILURE ) {
        if (chr == '\0') break;
        printf("%c", chr);
        clock_simulate_delay(cpu->clock_delay_ms);
        offset += 2;
    }
}

void _lenw(CPU* cpu, memory* mem) { }

void _jc(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_CARRY, 0);
}
void _jo(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_OVERFLOW, 0);
}
void _jnc(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_NOT_CARRY, 0);
}
void _jno(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_NOT_OVERFLOW, 0);
}
void _calc(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_CARRY, 1);
}
void _calo(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_OVERFLOW, 1);
}
void _calnc(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_NOT_CARRY, 1);
}
void _calno(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_NOT_OVERFLOW, 1);
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

void _ja(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_ABOVE, 0);
}

void _jae(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_ABOVE_EQUAL, 0);
}

void _jbe(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_BELOW_EQUAL, 0);
}

void _cala(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_ABOVE, 1);
}

void _calae(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_ABOVE_EQUAL, 1);
}

void _calbe(CPU* cpu, memory* mem) {
    CPU_conditional_transfer(cpu, mem, BRANCH_BELOW_EQUAL, 1);
}

void _iret(CPU* cpu, memory* mem) {
    CPU_pop_flags(cpu, mem);
    CPU_stack_pop(cpu, mem, &cpu->registers->CS);
    CPU_stack_pop(cpu, mem, &cpu->registers->PC);
}

void _livt(CPU* cpu, memory* mem) {
    ins_encoding encoding = {.single_oper = 1};
    CPU_read_operand_bytes(cpu, mem, &encoding);

    uint32_t ivtaddr = CPU_decode_dest(cpu, mem, &encoding);
    cpu->registers->IVTR = ivtaddr;
}

void _outp(CPU* cpu, memory* mem) {
    ins_encoding encoding = {.single_oper = 1};
    encoding.resolve_sym = 1;

    int read = CPU_read_operand_bytes(cpu, mem, &encoding);

    uint32_t port = CPU_decode_dest(cpu, mem, &encoding);
    uint32_t val = CPU_decode_src(cpu, mem, &encoding);

    io_write((uint8_t) port, val);
}

void _hlt(CPU* cpu, memory* mem) {
    cpu->state->CPU_HALTED = 1;
}

void _dbg(CPU* cpu, memory* mem) {
    reg_set_flags(cpu->registers, FLG_T);
    vm_debug_shell();
}

void _rem(CPU* cpu, memory* mem) {
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

        if (src == 0) CPU_interrupt(cpu, mem, CPU_EXCEPTION_DIVZERO);
        else {
            uint32_t val = dest % src;
            CPU_write_dest(cpu, mem, ins, val);
            CPU_update_flags(cpu, ins, dest, src, val, CPU_FLAG_OP_BITWISE);
        }
    }
}
