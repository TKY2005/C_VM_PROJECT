#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<VM/vm.h>
#include<VM/vm_settings.h>
#include<VM/vm_shell.h>
#include<CPU/registers.h>
#include<utils/tky_hashmap.h>
#include<debuglogger.h>

#include<stdlib.h>
#include<string.h>

int main(int argc, char** argv) {
    char* positional_args[2];
    int positional_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            dolog = 1;
        }
        else if (positional_count < 2) {
            positional_args[positional_count++] = argv[i];
        }
    }

    log_init();
    if (positional_count == 0) {
        printd("Starting from shell.\n");
        vm_shell();
    }
    else {
        printd("Starting from file '%s'\n", positional_args[0]);
        vm_settings = vm_get_settings(CONFIG_PATH);
        vm_set_settings(vm_settings, &vm_conf);
        vm_init(vm_conf.mem_size);
        
        vm_load_binary_file(vm_memory.ram, positional_args[0],
                            positional_count > 1 ? strtoul(positional_args[1], NULL, 0) : 0);
        vm_boot_sequence(vm_cpu, &vm_memory);
        vm_shutdown();
    }
    log_close();
    return 0;
}
