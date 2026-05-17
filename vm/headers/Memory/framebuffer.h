#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include<stdint.h>

#define FRAMEBUFFER_START 0xF9D10000
#define FRAMEBUFFER_END 0xFA20FFFF
#define FRAMEBUFFER_SIZE 0x500000

typedef struct framebuffer {
    size_t size;
    uint8_t* mem;
} framebuffer;

framebuffer* framebuffer_init(size_t sizeB);
void framebuffer_reset(framebuffer* framebuffer);

int framebuffer_size(const framebuffer* m);

uint32_t addr2framebuffer(uint32_t addr);

int framebuffer_write_byte(framebuffer* m, uint32_t addr, uint8_t val);
int framebuffer_write_word(framebuffer* m, uint32_t addr, uint16_t val);
int framebuffer_write_dword(framebuffer* m,uint32_t addr, uint32_t val);
int framebuffer_write_bytes(framebuffer* m, uint32_t addr, int count, uint8_t* vals);

int framebuffer_read_byte(framebuffer* m, uint32_t addr, uint8_t* result);
int framebuffer_read_word(framebuffer* m, uint32_t addr, uint16_t* result);
int framebuffer_read_dword(framebuffer* m, uint32_t addr, uint32_t* result);
int framebuffer_read_bytes(framebuffer* m, uint32_t addr, int count, uint8_t* result);

int framebuffer_is_valid_addr(framebuffer* m, uint32_t addr);
char* framebuffer_display(framebuffer* framebuffer, uint32_t start, int count, int chunk_size);

void framebuffer_destroy(framebuffer* m);

#endif