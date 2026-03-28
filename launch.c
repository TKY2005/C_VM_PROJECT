#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<VM/vm.h>
#include<CPU/registers.h>

#include<stdlib.h>
#include<stdio.h>

int main(int argc, char** argv) {

    if (argc < 2) {
        vm_shell();
    }
    else {
        vm_init(1024);
        
        vm_runf(vm_cpu, &vm_memory, argv[1]);

        char* regs = display_registers(vm_cpu->registers);
        char* mem = mem_display(&vm_memory, 0, 0xff, -1);

        printf("%s\n\n", regs);
        printf("%s\n\n", mem);

        free(regs);
        free(mem);
        
        vm_shutdown();
    }
    return 0;
}
