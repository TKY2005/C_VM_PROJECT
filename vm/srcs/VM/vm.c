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

int vm_runf(CPU* cpu, memory* mem, const char* filepath) {

    int read = vm_load_binary_file(mem, filepath, &cpu->registers->PC);

    if (read == NO_VALID_TARGET) {
        printf("File has been opened but couldn't be loaded into memory.\n");
        return VM_COULDNT_EXECUTE;
    }
    else if (read == FILE_NOT_OPEN) {
        printf("Couldn't open the requested file '%s'\n", filepath);
        return VM_COULDNT_EXECUTE;
    }

    cpu->state->CPU_RUNNING = 1;
    CPU_run(cpu, mem);

    return VM_SUCCESS;
}

int vm_load_binary_file(memory* target, const char* filepath, uint32_t* entry_dest) {
    
    FILE* src = fopen(filepath, "rb");
    if (src == NULL) return FILE_NOT_OPEN;
    if (target == NULL) return NO_VALID_TARGET;

    fseek(src, 0, SEEK_END);
    long size = ftell(src);
    rewind(src);

    if (size > mem_size(target)) return NOT_ENOUGH_MEMORY;

    uint8_t* metadata = malloc(METADATA_SIZE * sizeof(uint8_t));
    int metalen = fread(metadata, sizeof(uint8_t), METADATA_SIZE, src);
    
    // the entrypoint is stored in little endian format in the binary file metadata.
    // the VM understands only big endian format.
    // this reads the bytes of the entrypoint and reconstructs them in big endian format
    uint32_t entry_hh = metadata[metalen - 1];
    uint32_t entry_lh = metadata[metalen - 2];
    uint32_t entry_hl = metadata[metalen - 3];
    uint32_t entry_ll = metadata[metalen - 4];

    *entry_dest = (entry_ll) | (entry_hl << 8) | (entry_lh << 16) | (entry_hh << 24);

    long read = fread(target->mem, sizeof(uint8_t), size, src);

    if (read != size - METADATA_SIZE) return FILE_INCOMPLETE;

    return VM_SUCCESS;
}

int vm_runp(CPU* cpu, memory* mem, uint32_t entry) {
    
    cpu->registers->PC = entry;
    cpu->state->CPU_RUNNING = 1;

    CPU_run(cpu, mem);
}