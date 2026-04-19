#include<stdlib.h>
#include<stdint.h>
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

void vm_shell() {
    printf("======== TKY VIRTUAL MACHINE V%s ========\n", VM_VER);
    printf("The shell is still under construction. please run with the binary file path as argument for now.\n");
    while (1) {
        printf(">> ");
        char command[2048] = {0};
        fgets(command, sizeof(command), stdin);
        strtolower(command, strlen(command));
        command[strlen(command) - 1] = '\0';

        if (strcmp(command, "exit") == 0){
            printf("Goodbye.\n");
            vm_shutdown();
            break;
        }
        vm_run_shell_command(command);
    }
    return;
}

void vm_debug_shell() {
    uint8_t o; mem_read_byte(&vm_memory, vm_cpu->registers->PC + 1, &o);
    printf("PC = 0x%08X -> 0x%02X\n", vm_cpu->registers->PC + 1, o);
    while (reg_check_flag(vm_cpu->registers, FLG_T) != 0) {
        printf(">> ");
        char buff[2048] = {0};
        fgets(buff, sizeof(buff), stdin);
        buff[strlen(buff) - 1] = '\0';
        vm_run_shell_command(buff);
        printf("\n");
    }
}

void vm_run_shell_command(char* command) {

    char** parts = split_string(command, ' ');

    if (strcmp(parts[0], "vm_init") == 0) {
        hmap* conf = vm_get_settings(CONFIG_PATH);
        vm_set_settings(conf, &vm_conf);
        free(conf);
        uint32_t size;
        char* msize = &parts[1][0];
        if (!msize) size = vm_conf.mem_size;
        else size = strtol(parts[1], NULL, 0);
        vm_init(size);
        printf("Initialized the VM with %d bytes of available virtual memory.\n", ram_size(vm_memory.ram));
        printf("CPU speed set to %lu cycles/second with a delay of %lums\n", vm_conf.cycle_count, vm_cpu->clock_delay_ms);
    }
    else if (strcmp(parts[0], "boot") == 0) {
        vm_boot_sequence(vm_cpu, &vm_memory);
    }
    else if (strcmp(parts[0], "loadfile") == 0) {

        if (vm_memory.ram == NULL) printf("No virtual memory to load the file to. initialize with vm_init first.\n");
        else {
            int load = vm_load_code_file(vm_memory.ram, parts[1], &vm_cpu->registers->PC);
            switch(load) {
                case NOT_ENOUGH_MEMORY:
                printf("There isn't enough memory to load the file to. please allocate more memory.\n");
                break;
                case FILE_INCOMPLETE:
                printf("Warning: The file has not been completley loaded into memory.\n");
                break;
                case FILE_NOT_OPEN:
                printf("Couldn't open the requested file '%s'\n", parts[1]);
                break;
                case VM_SUCCESS:
                printf("Successfully loaded file into memory.\n");
                break;
            }
        }
    }
    else if (strcmp(parts[0], "loadbin") == 0) {
        if (vm_memory.ram == NULL) printf("No virtual memory to load the file to. initialize with vm_init first.\n");
        else {
            char* al = &parts[2][0];
            if (!al) {
                printf("for flat binary files, you must specify the address where the file will be loaded.\n");
            }
            else {
                uint32_t load_addr = (uint32_t) strtoul(parts[2], NULL, 0);
                int load = vm_load_binary_file(vm_memory.ram, parts[1], load_addr);
                switch(load) {
                    case NOT_ENOUGH_MEMORY:
                    printf("There isn't enough memory to load the file to. please allocate more memory.\n");
                    break;
                    case FILE_INCOMPLETE:
                    printf("Warning: The file has not been completley loaded into memory.\n");
                    break;
                    case FILE_NOT_OPEN:
                    printf("Couldn't open the requested file '%s'\n", parts[1]);
                    break;
                    case VM_SUCCESS:
                    printf("Successfully loaded file into memory.\n");
                    break;
                }
            }
        }
    }
    else if (strcmp(parts[0], "run") == 0) {
        vm_runp(vm_cpu, &vm_memory, vm_cpu->registers->PC);
    }
    else if (strcmp(parts[0], "memview") == 0) {
        uint32_t start = (uint32_t) strtoul(parts[1], NULL, 0);
        int count = strtol(parts[2], NULL, 0);
        int chunk = strtol(parts[3], NULL, 0);
        char* s = mem_display(&vm_memory, start, count, chunk);
        if (s == NULL) printf("Error displaying memory. Either the start address is out of bounds or memory is not initialized.\n");
        else {
            printf("%s\n", s);
            free(s);
        }
    }
    else if (strcmp(parts[0], "regview") == 0) {
        char* s = display_registers(vm_cpu->registers);
        printf("%s\n", s);
        free(s);
    }

    else if (strcmp(parts[0], "memwrite") == 0) {
        uint32_t addr = strtoul(parts[1], NULL, 0);
        uint32_t val = strtoul(parts[2], NULL, 0);
        char* wmode = &parts[3][0];
        if (!wmode) printf("You need to specifiy the write mode.\n");
        else{
            char mode = *wmode;
            uint32_t wr = 0;
            if (mode == 'b') wr = mem_write_byte(&vm_memory, addr, val);
            else if (mode == 'w') wr = mem_write_word(&vm_memory, addr, val);
            else if (mode == 'd') wr = mem_write_dword(&vm_memory, addr, val);
            else printf("Unknown write mode '%c'\n", mode);

            if (wr != val) printf("Unable to write value: %u to address: 0x%08X\n", val, addr);
        }
    }
    else if (strcmp(parts[0], "regwrite") == 0) {
        int index = reg_get_index(parts[1]);
        if (index == -1) printf("Unkown register alias: '%s'\n", parts[1]);
        else{
            uint32_t val = strtol(parts[2], NULL, 0);
            char* wmode = &parts[3][0];
            if (!wmode) printf("You need to specify the write mode.\n");
            else {
                char mode = *wmode;
                if (mode == 'b') reg_write_b(vm_cpu->registers, index, val);
                else if (mode == 'w') reg_write_w(vm_cpu->registers, index / 2, val);
                else if (mode == 'd') reg_write_dw(vm_cpu->registers, index / 4, val);
            }
        }
    }
    else if (strcmp(parts[0], "c") == 0) reg_clear_flags(vm_cpu->registers, FLG_T);
    else printf("Unkown command.\n");
}