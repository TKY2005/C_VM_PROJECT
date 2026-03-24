#include<stdint.h>
#include<stdlib.h>

#include<Memory/memory.h>
#include<CPU/instruction_set.h>
#include<CPU/CPU.h>
#include<ISA_encoding_info.h>


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
    printf("set.\n");
}
void _out(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;

    int read = CPU_read_operand_bytes(cpu, mem, ins);

    if (read == MEM_READ_FAILURE) {
        CPU_fail(cpu, "Invalid read operation @ 0x%08X. CPU shutting down.\n", cpu->registers->PC);
    }
    else {
        uint32_t val = CPU_decode_dest(cpu, mem, ins);
        printf("%u", val);
    }

}
void _add(CPU* cpu, memory* mem) { }
void _sub(CPU* cpu, memory* mem) { }
void _mul(CPU* cpu, memory* mem) { }
void _div(CPU* cpu, memory* mem) { }
void _pow(CPU* cpu, memory* mem) { }
void _sqrt(CPU* cpu, memory* mem) { }
void _rnd(CPU* cpu, memory* mem) { }
void _cti(CPU* cpu, memory* mem) { }
void _inc(CPU* cpu, memory* mem) { }
void _dec(CPU* cpu, memory* mem) { }
void _la(CPU* cpu, memory* mem) { }
void _len(CPU* cpu, memory* mem) { }
void _str(CPU* cpu, memory* mem) { }
void _outs(CPU* cpu, memory* mem) { }
void _inp(CPU* cpu, memory* mem) { }
void _inps(CPU* cpu, memory* mem) { }
void _push(CPU* cpu, memory* mem) { }
void _pop(CPU* cpu, memory* mem) { }
void _call(CPU* cpu, memory* mem) { }
void _cale(CPU* cpu, memory* mem) { }
void _calne(CPU* cpu, memory* mem) { }
void _cal(CPU* cpu, memory* mem) { }
void _calle(CPU* cpu, memory* mem) { }
void _calg(CPU* cpu, memory* mem) { }
void _calge(CPU* cpu, memory* mem) { }
void _calb(CPU* cpu, memory* mem) { }
void _jmp(CPU* cpu, memory* mem) { }
void _je(CPU* cpu, memory* mem) { }
void _jne(CPU* cpu, memory* mem) { }
void _jl(CPU* cpu, memory* mem) { }
void _jle(CPU* cpu, memory* mem) { }
void _jg(CPU* cpu, memory* mem) { }
void _jge(CPU* cpu, memory* mem) { }
void _jb(CPU* cpu, memory* mem) { }
void _and_(CPU* cpu, memory* mem) { }
void _or_(CPU* cpu, memory* mem) { }
void _xor(CPU* cpu, memory* mem) { }
void _not_(CPU* cpu, memory* mem) { }
void _nand(CPU* cpu, memory* mem) { }
void _nor(CPU* cpu, memory* mem) { }
void _cmp(CPU* cpu, memory* mem) { }
void _loop(CPU* cpu, memory* mem) { }
void _ret(CPU* cpu, memory* mem) { }
void _end(CPU* cpu, memory* mem) { }
void _int_(CPU* cpu, memory* mem) { }

void _outc(CPU* cpu, memory* mem) {
    ins_encoding encoding = {0};
    ins_encoding* ins = &encoding;

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
void _jc(CPU* cpu, memory* mem) { }
void _jo(CPU* cpu, memory* mem) { }
void _jnc(CPU* cpu, memory* mem) { }
void _jno(CPU* cpu, memory* mem) { }
void _calc(CPU* cpu, memory* mem) { }
void _calo(CPU* cpu, memory* mem) { }
void _calnc(CPU* cpu, memory* mem) { }
void _calno(CPU* cpu, memory* mem) { }
void _clc(CPU* cpu, memory* mem) { }
void _clo(CPU* cpu, memory* mem) { }
void _clz(CPU* cpu, memory* mem) { }
void _cln(CPU* cpu, memory* mem) { }
void _cli(CPU* cpu, memory* mem) { }
void _slc(CPU* cpu, memory* mem) { }
void _slo(CPU* cpu, memory* mem) { }
void _slz(CPU* cpu, memory* mem) { }
void _sln(CPU* cpu, memory* mem) { }
void _sli(CPU* cpu, memory* mem) { }