#include<headers/CPU/CPU.h>
#include<headers/Memory/memory.h>
#include<headers/VM/vm.h>

#include<stdlib.h>
#include<stdio.h>

int main() {

    vm_init(1024);

    vm_load_binary_file(&vm_memory, "./prog.tky");

    char* display = mem_display(&vm_memory, -1, -1, -1);

    printf("%s\n", display);

    free(display);

    vm_shutdown();
    return 0;
}