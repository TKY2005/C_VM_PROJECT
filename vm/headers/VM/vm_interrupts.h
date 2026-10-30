#ifndef VM_INTERRUPTS_H
#define VM_INTERRUPTS_H

#include<stdio.h>

#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<utils/tky_queue.h>

#define IC_COMMAND_PORT 0x20
#define IC_DATA_PORT 0x21

#define IC_COMMAND_NONE 0
#define IC_COMMAND_DISABLE 1
#define IC_COMMAND_ENABLE 2
#define IC_COMMAND_DROPALL 3
#define IC_COMMAND_DISCARD_VEC 4
#define IC_COMMAND_REJECT_VEC 5
#define IC_COMMAND_ALLOW_VEC 6
#define IC_COMMAND_REJECTALL 7
#define IC_COMMAND_ALLOWALL 8

#define IC_ENABLED 1
#define IC_DISABLED 0

typedef struct interrupt_controller {
    uint8_t data;
    uint8_t enabled;
    tky_queue* interrupt_queue;
    uint8_t rejection_list[256];
} interrupt_controller;

interrupt_controller ic_init();

int ic_interrupt_enqueue(interrupt_controller* ic, uint8_t icode);
int ic_interrupt_acknowledge(interrupt_controller* ic);
int ic_interrupt_line_status(interrupt_controller* ic);

int ic_run_command(interrupt_controller* ic, uint8_t command);

int ic_cmd_disable(interrupt_controller* ic);
int ic_cmd_enable(interrupt_controller* ic);
int ic_cmd_dropall(interrupt_controller* ic);
int ic_cmd_discard_vector(interrupt_controller* ic);
int ic_cmd_reject_vector(interrupt_controller* ic);
int ic_cmd_allow_vector(interrupt_controller* ic);
int ic_cmd_rejectall(interrupt_controller* ic);
int ic_cmd_allowall(interrupt_controller* ic);

int ic_read_data(interrupt_controller* ic, uint8_t* result);

int ic_write_data(interrupt_controller* ic, uint8_t data);

int ic_isrejected(interrupt_controller* ic, uint8_t port);

void ic_reset(interrupt_controller* ic);

void ic_free(interrupt_controller* ic);

#endif