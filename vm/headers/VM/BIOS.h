#ifndef BIOS_H
#define BIOS_H

#include<stdint.h>

#define BIOS_SIZE 0x8000
#define BIOS_ENTRY_ADDR 0xFFFF7FFFu

uint8_t* load_BIOS_img(const char* imgpath);

uint32_t mem_addr_2_bios(uint32_t addr);

int BIOS_read_byte(uint32_t addr, uint8_t* result);
int BIOS_read_word(uint32_t addr, uint16_t* result);
int BIOS_read_dword(uint32_t addr, uint32_t* result);

#endif