#ifndef VM_INTERRUPTS_H
#define VM_INTERRUPTS_H

#include<stdio.h>

#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<utils/tky_queue.h>


typedef struct interrupt_controller {
    tky_queue* interrupt_queue;
} interrupt_controller;

interrupt_controller ic_init();

int ic_interrupt_enqueue(interrupt_controller* ic, uint8_t icode);
int ic_interrupt_acknowledge(interrupt_controller* ic);
int ic_interrupt_line_status(interrupt_controller* ic);

void ic_reset(interrupt_controller* ic);

void ic_free(interrupt_controller* ic);

#endif