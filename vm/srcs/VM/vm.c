#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>

#include<VM/vm.h>
#include<CPU/CPU.h>
#include<CPU/registers.h>
#include<Memory/memory.h>
#include<Memory/RAM.h>
#include<Memory/bios_rom.h>
#include<Memory/framebuffer.h>
#include<CPU/instruction_set.h>
#include<ISA_encoding_info.h>
#include<VM/vm_input.h>
#include<VM/vm_interrupts.h>
#include<VM/vm_settings.h>

#include<utils/helpers.h>
#include<utils/hashmap/hmap.h>

CPU* vm_cpu;
memory vm_memory;
bool ui_mode;

void vm_init(int memsize) {
    if (vm_memory.ram != NULL && vm_memory.bios_rom != NULL && vm_memory.fb != NULL) mem_destroy(&vm_memory);
    if (vm_cpu) CPU_reset_state(vm_cpu);
    else {
        union registerfile* regfile = calloc(1, sizeof(union registerfile));    
        instruction* ins_set = setup_instruction_set();
        vm_cpu = mkCPU(regfile, ins_set);
    }
    vm_memory = mem_init(memsize);
    vm_memory.bios_rom->mem = load_BIOS_img(vm_conf.bios_path);
    vm_cpu->clock_delay_ms = vm_calculate_delay_ms(vm_conf.cycle_count);
}

uint64_t vm_calculate_delay_ms(uint64_t cycles) {
    return 1000 / cycles;
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

int vm_boot_sequence(CPU* cpu, memory* mem) {
    cpu->registers->PC = BIOS_ENTRY;
    cpu->state->CPU_RUNNING = 1;
    reg_set_flags(cpu->registers, FLG_I);
    CPU_run(cpu, mem);
}

int vm_runf(CPU* cpu, memory* mem, const char* filepath) {

    int read = vm_load_code_file(mem->ram, filepath, &cpu->registers->PC);

    if (read == NO_VALID_TARGET) {
        printf("File has been opened but couldn't be loaded into memory.\n");
        return VM_COULDNT_EXECUTE;
    }
    else if (read == FILE_NOT_OPEN) {
        printf("Couldn't open the requested file '%s'\n", filepath);
        return VM_COULDNT_EXECUTE;
    }

    cpu->state->CPU_RUNNING = 1;
    reg_set_flags(cpu->registers, FLG_I);
    CPU_run(cpu, mem);

    return VM_SUCCESS;
}

int vm_load_code_file(RAM* target, const char* filepath, uint32_t* entry_dest) {
    
    FILE* src = fopen(filepath, "rb");
    if (src == NULL) return FILE_NOT_OPEN;
    if (target == NULL){
        fclose(src);
        return NO_VALID_TARGET;
    }

    fseek(src, 0, SEEK_END);
    long size = ftell(src);
    rewind(src);

    if (size > ram_size(target)){
        fclose(src);
        return NOT_ENOUGH_MEMORY;
    }

    uint8_t* metadata = malloc(METADATA_SIZE * sizeof(uint8_t));
    int metalen = fread(metadata, sizeof(uint8_t), METADATA_SIZE, src);
    
    // the entrypoint is stored in little endian format in the binary file metadata.
    // the VM understands only big endian format.
    // this reads the bytes of the entrypoint and reconstructs them in big endian format
    uint32_t entry_hh = metadata[metalen - 1];
    uint32_t entry_lh = metadata[metalen - 2];
    uint32_t entry_hl = metadata[metalen - 3];
    uint32_t entry_ll = metadata[metalen - 4];

    if(entry_dest != NULL) *entry_dest = (entry_ll) | (entry_hl << 8) | (entry_lh << 16) | (entry_hh << 24);
    free(metadata);

    long read = fread(target->mem, sizeof(uint8_t), size, src);

    if (read != size - METADATA_SIZE){
        fclose(src);
        return FILE_INCOMPLETE;
    }

    fclose(src);
    return VM_SUCCESS;
}

int vm_load_binary_file(RAM* target, const char* filepath, uint32_t load_addr) {
    FILE* src = fopen(filepath, "rb");
    if (src == NULL) return FILE_NOT_OPEN;
    if (target == NULL){
        fclose(src);
        return NO_VALID_TARGET;
    }

    fseek(src, 0, SEEK_END);
    long size = ftell(src);
    rewind(src);

    if (size > ram_size(target) - load_addr){
        fclose(src);
        return NOT_ENOUGH_MEMORY;
    }

    long read = fread(target->mem + load_addr, sizeof(uint8_t), size, src);

    if (read < size){
        fclose(src);
        return FILE_INCOMPLETE;
    }

    fclose(src);
    return VM_SUCCESS;
}

int vm_runp(CPU* cpu, memory* mem, uint32_t entry) {
    
    cpu->registers->PC = entry;
    cpu->state->CPU_RUNNING = 1;
    reg_set_flags(cpu->registers, FLG_I);

    CPU_run(cpu, mem);
    return VM_SUCCESS;
}

int vm_interrupt(CPU* cpu, memory* mem, uint8_t icode) {
    
    if (!reg_check_flag(cpu->registers, FLG_I)) {
        return INTERRUPT_FLAG_DISABLED;
    }
    else {
        if (icode == VM_HARDWARE_INTERRUPT) return vm_hardware_interrupt(cpu, mem);
        else if (icode == VM_SOFTWARE_INTERRUPT) return vm_software_interrupt(cpu, mem);
        else if (icode == VM_TRAP_INTERRUPT){
            reg_set_flags(vm_cpu->registers, FLG_T);
            vm_debug_shell();
        }
        else return VM_UNKNOWN_INTERRUPT;
    }
}