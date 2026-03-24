#include<stdlib.h>
#include<stdint.h>

#include<VM/vm.h>
#include<CPU/CPU.h>
#include<CPU/registers.h>
#include<Memory/memory.h>
#include<CPU/instruction_set.h>
#include<ISA_encoding_info.h>

CPU* vm_cpu;
memory vm_memory;

void vm_init(int memsize) {
    union registerfile* regfile = calloc(1, sizeof(union registerfile));
    
    instruction* ins_set = setup_instruction_set();
    vm_cpu = mkCPU(regfile, ins_set);
    vm_memory = mem_init(memsize);
}

void vm_init_nomem() {
    union registerfile* regfile = calloc(1, sizeof(union registerfile));
    
    instruction* ins_set = setup_instruction_set();
    vm_cpu = mkCPU(regfile, ins_set);
}

void vm_shutdown() {
    
    CPU_destroy(vm_cpu);
    mem_destroy(&vm_memory);
}

int vm_load_binary_file(memory* target, const char* filepath) {
    
    FILE* src = fopen(filepath, "rb");
    if (src == NULL) return FILE_NOT_OPEN;
    if (target == NULL) return NO_VALID_TARGET;

    fseek(src, 0, SEEK_END);
    long size = ftell(src);

    if (size > mem_size(target)) return NOT_ENOUGH_MEMORY;

    rewind(src);

    long read = fread(target->mem, sizeof(uint8_t), size, src);

    if (read != size) return FILE_INCOMPLETE;

    return SUCCESS;
}

int vm_runp(CPU* cpu, memory* mem, uint32_t entry) {
    
    cpu->registers->PC = entry;
    cpu->state->CPU_RUNNING = 1;

    CPU_run(cpu, mem);
}