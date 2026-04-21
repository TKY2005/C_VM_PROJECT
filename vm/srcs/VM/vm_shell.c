#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<utils/helpers.h>

#include<VM/vm_shell.h>
#include<VM/vm.h>
#include<VM/vm_settings.h>

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