#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include<stdint.h>

#include "../headers/Memory/memory.h"

typedef struct {
	uint8_t opcode;
	char* mnemonic;
	void (*impl)(CPU*, memory*, int*, int*);
} instruction;

uint8_t ins_getopcode(instruction* ins_set, char* mnemonic);
char* ins_getmnemonic(instruction* ins_set, uint8_t opcode);
void* ins_getimpl(instruction* ins_set, uint8_t opcode);


#endif

// The instruction model //
// A regular struct is also fine.
/*
#include <stdio.h>
#include<stdlib.h>
#include<stdint.h>

typedef struct {
    uint8_t opcode;
    char* mnemonic;
    void(*impl)();
} instruction;

void add() {
    printf("add..\n");
}
void sub(){
    printf("sub..\n");
}
void jmp(){
    printf("jmp..\n");
}

int main() {
    instruction* ins_add = malloc(sizeof(instruction));
    ins_add->opcode = 0x00;
    ins_add->mnemonic = "add";
    ins_add->impl = add;
    
    instruction* ins_sub = malloc(sizeof(instruction));
    ins_sub->opcode = 0x01;
    ins_sub->mnemonic = "sub";
    ins_sub->impl = sub;
    
    ins_add->impl();
    ins_sub->impl();
}
*/
