#ifndef VM_H
#define VM_H

#include<CPU/CPU.h>
#include<Memory/memory.h>

#include<stdint.h>

// ERRORS: < 0
#define FILE_NOT_OPEN -1
#define NOT_ENOUGH_MEMORY -2
#define NO_VALID_TARGET -3

// WARNINGS: > 0
#define FILE_INCOMPLETE 1

#define SUCCESS 0

extern CPU* vm_cpu;
extern memory vm_memory;

void vm_init(int memsize);
void vm_init_nomem();
int vm_load_binary_file(memory* target, const char* filepath);
int vm_handle_interrupt(CPU* cpu);
int vm_runp(CPU* cpu, memory* mem, uint32_t entry);
int vm_runf(CPU* cpu, memory* mem, const char* filepath);

void vm_shutdown();

#endif
