#ifndef VM_H
#define VM_H

#include "../headers/CPU/CPU.h"
#include "../headers/Memory/memory.h"

memory* mem;
CPU* cpu;

void vm_init();
void vm_load_binary_file(memory* target, FILE* src);
int vm_handle_interrupt(CPU* cpu);
int vm_run_prog(CPU* cpu, memory* mem);

#endif
