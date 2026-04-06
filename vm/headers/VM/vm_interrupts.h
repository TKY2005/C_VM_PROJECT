#ifndef VM_INTERRUPTS_H
#define VM_INTERRUPTS_H

#include<CPU/CPU.h>
#include<Memory/memory.h>

#define VM_UNKNOWN_INTERRUPT -1

#define VM_SOFTWARE_INTERRUPT 0x01
#define VM_HARDWARE_INTERRUPT 0x02
#define VM_TRAP_INTERRUPT 0x03

#define VM_HINTR_KEY_HIT 0x01

#define VM_SINTR_WRITE_FD 0x01
#define VM_SINTR_READ_TO_BUFF 0x02
#define VM_SINTR_READ_CHR 0x03
#define VM_SINTR_READ_NUM 0x04
#define VM_SINTR_TRAP 0x05

int vm_hardware_interrupt(CPU* cpu, memory* mem);
int vm_software_interrupt(CPU* cpu, memory* mem);

#endif