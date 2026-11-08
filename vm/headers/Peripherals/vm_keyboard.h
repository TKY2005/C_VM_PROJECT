#ifndef VM_KEYBOARD_H
#define VM_KEYBOARD_H

#include<utils/tky_queue.h>

#include<stdint.h>

typedef struct kbrd_controller {
    tky_queue* key_queue;
    uint8_t data;
} kbrd_controller;



#endif