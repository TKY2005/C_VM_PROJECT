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
	instruction* instruction_set;
} CPU;


CPU* mkCPU();
int CPU_runm(CPU* cpu, memory* mem);
int CPU_runf(CPU* cpu, memory* mem, FILE* f);

#endif
