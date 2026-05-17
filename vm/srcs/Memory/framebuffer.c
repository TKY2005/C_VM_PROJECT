#include<stdint.h>
#include<stdbool.h>
#include<stdlib.h>
#include<stddef.h>

#include<Memory/framebuffer.h>
#include<Memory/memory.h>

framebuffer* framebuffer_init(size_t sizeB) {
    framebuffer* f = malloc(sizeof(framebuffer));
    f->size = sizeB;
    f->mem = calloc(sizeB, sizeof(uint8_t));
    return f;
}

int framebuffer_write_byte(framebuffer* m, uint32_t addr, uint8_t val) {}
int framebuffer_write_word(framebuffer* m, uint32_t addr, uint16_t val) {}
int framebuffer_write_dword(framebuffer* m,uint32_t addr, uint32_t val) {}
int framebuffer_write_bytes(framebuffer* m, uint32_t addr, int count, uint8_t* vals) {}

int framebuffer_read_byte(framebuffer* m, uint32_t addr, uint8_t* result) {}
int framebuffer_read_word(framebuffer* m, uint32_t addr, uint16_t* result) {}
int framebuffer_read_dword(framebuffer* m, uint32_t addr, uint32_t* result) {}
int framebuffer_read_bytes(framebuffer* m, uint32_t addr, int count, uint8_t* result) {}

void framebuffer_destroy(framebuffer* f) {
    if (f || f->mem) {
        free(f->mem);
        free(f);
    }
}