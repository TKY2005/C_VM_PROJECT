#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<VM/vm.h>
#include<CPU/registers.h>

#include<stdlib.h>
#include<stdio.h>

int main() {

    vm_init(1024);

    vm_load_binary_file(&vm_memory, "./test.tky");

    vm_cpu->registers->A = 0x00004050;
    vm_cpu->registers->DI = 0x30;
    // f0 11 ff 90
    mem_write_dword(&vm_memory, 0x50, 0xf011ff90);
    vm_runp(vm_cpu, &vm_memory, 0x0);
    
    vm_shutdown();
    return 0;
}