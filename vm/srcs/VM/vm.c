#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#include<VM/vm.h>
#include<CPU/CPU.h>
#include<CPU/registers.h>
#include<Memory/memory.h>
#include<CPU/instruction_set.h>
#include<ISA_encoding_info.h>
#include<VM/vm_input.h>

#include<utils/helpers.h>

CPU* vm_cpu;
memory vm_memory;

void vm_init(int memsize) {
    if (&vm_memory.mem != NULL) mem_destroy(&vm_memory);
    if (vm_cpu) CPU_reset_state(vm_cpu);
    else {
        union registerfile* regfile = calloc(1, sizeof(union registerfile));    
        instruction* ins_set = setup_instruction_set();
        vm_cpu = mkCPU(regfile, ins_set);
    }
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
    reg_set_flags(cpu->registers, FLG_I);
    CPU_run(cpu, mem);

    return VM_SUCCESS;
}

int vm_load_binary_file(memory* target, const char* filepath, uint32_t* entry_dest) {
    
    FILE* src = fopen(filepath, "rb");
    if (src == NULL) return FILE_NOT_OPEN;
    if (target == NULL){
        fclose(src);
        return NO_VALID_TARGET;
    }

    fseek(src, 0, SEEK_END);
    long size = ftell(src);
    rewind(src);

    if (size > mem_size(target)){
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

        switch(icode) {
            case VM_INTR_WRITE:
            break;
            case VM_INTR_READ_TO_BUFF: {
                    uint32_t write_addr = cpu->registers->SI; // buffer address
                    uint32_t write_amount = cpu->registers->D; // buffer size
                    uint8_t mode = cpu->registers->ALL; // write mode.
                    int chr;
                    uint32_t offset = 0;
                    while ( (chr = vm_getchr()) != VM_KEY_ENTER && offset < write_amount) {
                        
                        if (chr == VM_KEY_DEL) {
                            if (offset <= 0) continue;
                            mem_write_byte(mem, write_addr + offset, 0x0);
                            printf("%c", chr);
                            offset--;
                        }
                        else {
                            mem_write_byte(mem, write_addr + offset, chr);
                            printf("%c", chr);
                            offset++;
                        }
                    }
                break;
            }
            case VM_INTR_READ_CHR: {
                int chr = vm_getchr();
                cpu->registers->D = chr; // the character will be placed in the EDX register
                break;
            }
            case VM_INTR_READ_NUM: {
                int mode = cpu->registers->ALL; // read mode: AL
                uint32_t num; scanf("%u", &num);
                if (mode == 1) cpu->registers->DLL = (uint8_t) num;
                else if (mode == 2) cpu->registers->DXL = (uint16_t) num;
                else if (mode == 3) cpu->registers->D = num;
                else cpu->registers->DXL = (uint16_t) num; 
            }
        }
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

void vm_run_shell_command(char* command) {

    char** parts = split_string(command, ' ');

    if (strcmp(parts[0], "vm_init") == 0) {
        uint32_t size = strtol(parts[1], NULL, 0);
        vm_init(size);
        printf("Initialized the VM with %d bytes of available virtual memory.\n", vm_memory.size);
    }
    else if (strcmp(parts[0], "loadfile") == 0) {

        if (vm_memory.mem == NULL) printf("No virtual memory to load the file to. initialize with vm_init first.\n");
        else {
            int load = vm_load_binary_file(&vm_memory, parts[1], &vm_cpu->registers->PC);
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
    else if (strcmp(parts[0], "run") == 0) {
        vm_runp(vm_cpu, &vm_memory, vm_cpu->registers->PC);
    }
    else if (strcmp(parts[0], "memview") == 0) {
        uint32_t start = strtol(parts[1], NULL, 0);
        int count = strtol(parts[2], NULL, 0);
        int chunk = strtol(parts[3], NULL, 0);
        char* s = mem_display(&vm_memory, start, count, chunk);
        printf("%s\n", s);
        free(s);
    }
    else if (strcmp(parts[0], "regview") == 0) {
        char* s = display_registers(vm_cpu->registers);
        printf("%s\n", s);
        free(s);
    }

    else if (strcmp(parts[0], "memwrite") == 0) {
        uint32_t addr = strtol(parts[1], NULL, 0);
        uint32_t val = strtol(parts[2], NULL, 0);
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
    else printf("Unkown command.\n");
}