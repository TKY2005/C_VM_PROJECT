#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include<stdint.h>

#include "../headers/Memory/memory.h"
#include "./CPU.h"

void (*instruction)(CPU*, memory*);

void set(CPU* cpu, memory* mem);
void add(CPU* cpu, memory* mem);
void sub(CPU* cpu, memory* mem);

#endif