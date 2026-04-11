#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>

#include<VM/BIOS.h>
#include<VM/vm.h>
#include<CPU/CPU.h>

uint8_t* load_BIOS_img(const char* path) {

    FILE* f = fopen(path, "rb");
    if (f == NULL) return NULL;

    uint8_t* bios_content = (uint8_t*) calloc(BIOS_SIZE, sizeof(uint8_t));

    fread(bios_content, sizeof(uint8_t), BIOS_SIZE, f);
    fclose(f);
    
    return bios_content;
}

uint32_t mem_addr_2_bios(uint32_t addr) {
    return (addr) % BIOS_ENTRY_ADDR;
}

int BIOS_read_byte(uint32_t addr, uint8_t *result)
{
    addr = mem_addr_2_bios(addr);

    *result = bios_memory[addr];
    return 0;
}

int BIOS_read_word(uint32_t addr, uint16_t* result) {
    addr = mem_addr_2_bios(addr);
    uint8_t high = bios_memory[addr];
    uint8_t low = bios_memory[addr + 1];
    *result = (high << 8) | low;
    return 0;
}

int BIOS_read_dword(uint32_t addr, uint32_t* result) {
    addr = mem_addr_2_bios(addr);
    uint16_t high = (bios_memory[addr + 0] << 8) | bios_memory[addr + 1];
    uint16_t low = (bios_memory[addr + 2] << 8) | bios_memory[addr + 3];
    *result = (high << 16) | low;
    return 0;
}