#ifndef VM_INPUT_H
#define VM_INPUT_H

#define VM_KEY_ENTER 0x10
#define VM_KEY_DEL 0x11
#define VM_KEY_ARROW_LEFT 0x12
#define VM_KEY_ARROW_RIGHT 0x13

#define KEYMAP_SIZE 4

#define READ_MODE_BYTE 1
#define READ_MODE_WORD 2
#define READ_MODE_DWORD 3

#include<stdint.h>

extern int vm_keymap[KEYMAP_SIZE];

int vm_getchr();
uint32_t vm_getnum();
int vmchr_to_platform(int vmchar);

#endif