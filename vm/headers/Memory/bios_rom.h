#ifndef BIOS_ROM_H
#define BIOS_ROM_H

#include<stdint.h>
#include<stdbool.h>
#include<stddef.h>

#define BIOS_START 0xF9C00000
#define BIOS_ENTRY 0xF9C00000
#define BIOS_END 0xF9C0FFFF
#define BIOS_SIZE 0xffff

typedef struct BIOS_ROM {
    size_t size;
    uint8_t* mem;
} BIOS_ROM;

BIOS_ROM* bios_rom_init(int sizeB);
void bios_rom_reset(BIOS_ROM* BIOS_ROM);

int bios_rom_size(const BIOS_ROM* m);

uint32_t addr2bios(uint32_t addr);

uint8_t* load_BIOS_img(const char* imgpath);

int bios_rom_write_byte(BIOS_ROM* m, uint32_t addr, uint8_t val);
int bios_rom_write_word(BIOS_ROM* m, uint32_t addr, uint16_t val);
int bios_rom_write_dword(BIOS_ROM* m,uint32_t addr, uint32_t val);
int bios_rom_write_bytes(BIOS_ROM* m, uint32_t addr, size_t count, uint8_t* vals);

int bios_rom_read_byte(BIOS_ROM* m, uint32_t addr, uint8_t* result);
int bios_rom_read_word(BIOS_ROM* m, uint32_t addr, uint16_t* result);
int bios_rom_read_dword(BIOS_ROM* m, uint32_t addr, uint32_t* result);
int bios_rom_read_bytes(BIOS_ROM* m, uint32_t addr, size_t count, uint8_t* result);

bool bios_is_valid_addr(BIOS_ROM* m, uint32_t addr);
char* bios_rom_display(BIOS_ROM* BIOS_ROM, uint32_t start, size_t count, int chunk_size);

void bios_rom_destroy(BIOS_ROM* m);

#endif