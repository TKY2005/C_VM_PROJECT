#include<headers/CPU/CPU.h>
#include<headers/Memory/memory.h>
#include<headers/CPU/registers.h>
#include<headers/CPU/instruction_set.h>
#include<ISA_encoding_info.h>
#include<debuglogger.h>

#include<stdarg.h>
#include<stdlib.h>
#include<stdint.h>

CPU* mkCPU(union registerfile* regfile, instruction* ins_set) {
    CPU* cpu = malloc(sizeof(CPU));

    cpu->arch = 32;
    cpu->registers = calloc(1, sizeof(union registerfile));
    cpu->state = calloc(1, sizeof(CPU_STATE));

    cpu->instruction_set = ins_set;
    return cpu;
}

void CPU_destroy(CPU* cpu) {
    free(cpu->instruction_set);
    free(cpu->registers);
    free(cpu->state);
    free(cpu);
}

int CPU_run(CPU* cpu, memory* mem) {
    
    while (cpu->state->CPU_RUNNING) {
        uint8_t opcode;
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
        printd("the CPU requested instruction opcode: 0x%02X(%d) but it isn't defined.", opcode, opcode);
        cpu->state->CPU_RUNNING = 0;
    }
    else {
        ins(cpu, mem);
        cpu->registers->PC++;
    }
}

uint32_t CPU_step(CPU* cpu) {
    return cpu->registers->PC++;
}

int CPU_read_opertype(CPU* cpu, memory* mem, ins_encoding* ins) {

    int read = mem_read_byte(mem, CPU_step(cpu), &ins->operand_types);
    return read;
}
int CPU_read_regselect(CPU* cpu, memory* mem, ins_encoding* ins) {

    int read = mem_read_byte(mem, CPU_step(cpu), &ins->register_select);
    return read;
}

uint32_t CPU_decode_dest(CPU* cpu, memory* mem, ins_encoding* ins) {

    if (ins->opertype.dest_type >= REG8 && ins->opertype.dest_type <= REG32) {
        uint32_t regval;
        
    }
}

uint32_t CPU_decode_src(CPU* cpu, memory* mem, ins_encoding* ins) {

}

void CPU_fail(CPU* cpu, const char* msg, ...) {
    char buff[4096];

    va_list args;
    va_start(args, msg);

    vsnprintf(buff, sizeof(buff), msg, args);

    va_end(args);

    printf("%s\n", buff);

    cpu->state->CPU_RUNNING = 0;
}