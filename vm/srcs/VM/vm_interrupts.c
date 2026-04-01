#include<VM/vm_interrupts.h>
#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<VM/vm_input.h>

#include<stdint.h>

int vm_hardware_interrupt(CPU* cpu, memory* mem) {

}

int vm_software_interrupt(CPU* cpu, memory* mem) {
    
    switch(cpu->registers->ALH){ // interrupt code register: AH

        case VM_SINTR_READ_CHR: {
            int c = vm_getchr();
            cpu->registers->DXL = (uint16_t) c; // result stored in DX register
            break;
        }
        case VM_SINTR_READ_NUM: {
            uint8_t mode = cpu->registers->ALL; // read mode: AL
            uint32_t n = vm_getnum();
            // result is stored in D register
            if (mode == READ_MODE_BYTE) cpu->registers->DLL = (uint8_t) n;
            else if (mode == READ_MODE_WORD) cpu->registers->DXL = (uint16_t) n;
            else if (mode == READ_MODE_DWORD) cpu->registers->D = n;
            else cpu->registers->DLL = (uint8_t) n;
            break;
        }
        case VM_SINTR_READ_TO_BUFF: {
            // destination buffer address: SI
            // write amount: EDX
            // write mode: AL
            uint32_t buffaddr = cpu->registers->SI;
            uint8_t mode = cpu->registers->ALL;
            uint32_t amount = cpu->registers->D;
            int inc = 1;
            
            if (mode == READ_MODE_BYTE) inc = 1;
            else if (mode == READ_MODE_WORD) inc = 2;
            else if (mode == READ_MODE_DWORD) inc = 4;
            amount *= inc;

            uint32_t index = 0;
            uint32_t chr;
            while ( (chr = vm_getchr()) != VM_KEY_ENTER && index < amount) {
                if (mode == READ_MODE_BYTE) mem_write_byte(mem, buffaddr + index, (uint8_t) chr);
                else if (mode == READ_MODE_WORD) mem_write_word(mem, buffaddr + index,(uint16_t) chr);
                else if (mode == READ_MODE_DWORD) mem_write_dword(mem, buffaddr + index, chr);
                else mem_write_byte(mem, buffaddr + index, (uint8_t) chr);
                printf("%c", chr);
                if(chr == VM_KEY_DEL) index -= inc;
                else index += inc;
            }
            cpu->registers->D = index / inc; // write the amount written back to EDX
            break;
        }
        default:
        return VM_UNKNOWN_INTERRUPT;
        break;
    }
}