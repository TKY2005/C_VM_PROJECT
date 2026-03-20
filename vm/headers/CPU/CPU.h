#ifndef CPU_H
#define CPU_H

#include<stdint.h>

#include "./memory.h"
#include "./instruction_set.h"
#include "./registers.h"

#define CPU_RUNNING 0b00000001

typedef struct {
    uint8_t state;
    uint8_t arch;
	union regfile* registers;
} CPU;


CPU* mkCPU(union regfile* register_file);
int CPU_runm(CPU* cpu, memory* mem);
int CPU_runf(CPU* cpu, memory* mem, FILE* f);
void CPU_exec_ins(CPU* cpu, memory* mem, void (*instruction)(CPU*, memory*));

#endif
