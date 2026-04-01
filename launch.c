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
        
        vm_shutdown();
    }
    return 0;
}
