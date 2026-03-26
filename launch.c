#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<VM/vm.h>
#include<CPU/registers.h>

#include<stdlib.h>
#include<stdio.h>

int main(int argc, char** argv) {
    
    vm_init(1024);

    vm_cpu->registers->A = 0xfcbb4050;
    vm_cpu->registers->DI = 0x30;
    vm_cpu->registers->SP = 0x7f;

    reg_set_flags(vm_cpu->registers, FLG_I);
    
    // f0 11 ff 90
    mem_write_dword(&vm_memory, 0x50, 0xf011ff90);
    vm_runf(vm_cpu, &vm_memory, "./test.tky");

    char* regs = display_registers(vm_cpu->registers);
    char* mem = mem_display(&vm_memory, 0, 0xff, -1);

    printf("%s\n\n", regs);
    printf("%s\n\n", mem);

    free(regs);
    free(mem);
    
    vm_shutdown();
    return 0;
}