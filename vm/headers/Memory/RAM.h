#ifndef RAM_H
#define RAM_H

#include<stdint.h>
#include<stddef.h>

typedef struct RAM {
	int size;
	uint8_t* mem;
} RAM;

RAM* ram_init(size_t sizeB);
void ram_reset(RAM* ram);

size_t ram_size(const RAM* r);
int ram_write_byte(RAM* r, uint32_t addr, uint8_t val);
int ram_write_word(RAM* r, uint32_t addr, uint16_t val);
int ram_write_dword(RAM* r,uint32_t addr, uint32_t val);
int ram_write_bytes(RAM* r, uint32_t addr, size_t count, uint8_t* vals);

int ram_read_byte(RAM* r, uint32_t addr, uint8_t* result);
int ram_read_word(RAM* r, uint32_t addr, uint16_t* result);
int ram_read_dword(RAM* r, uint32_t addr, uint32_t* result);
int ram_read_bytes(RAM* r, uint32_t addr, size_t count, uint8_t* result);

int is_valid_addr(RAM* r, uint32_t addr);
char* ram_display(RAM* ram, uint32_t start, size_t count, int chunk_size);

void ram_destroy(RAM* r);

#endif