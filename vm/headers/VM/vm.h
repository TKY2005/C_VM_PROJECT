#ifndef VM_H
#define VM_H

#include "../headers/CPU/CPU.h"
#include "../headers/Memory/memory.h"

memory* mem;
CPU* cpu;

vm_init();
vm_load_mem_image(FILE* f);
vm_handle_interrupt(CPU* cpu);
vm_run_prog(CPU* cpu, memory* mem);

#endif
