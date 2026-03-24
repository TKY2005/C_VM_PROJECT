#ifndef MEMORY_H
#define MEMORY_H

#include<stdint.h>

#include<vm/vm_events.h>

#define DEFAULT_CHUNK_SIZE 16
#define BIT32_MAX_SIZE 0xffffffff
#define BIT16_MAX_SIZE 0xffff
#define BIT8_MAX_SIZE 0xff

#define MEM_READ_FAILURE -1
#define MEM_WRITE_FAILURE -2

typedef struct memory {
	int size;
	uint8_t* mem;
} memory;

memory mem_init(int sizeB);
int mem_size(const memory* m);
int mem_write_byte(memory* m, uint32_t addr, uint8_t val);
int mem_write_word(memory* m, uint32_t addr, uint16_t val);
unsigned int mem_write_dword(memory* m,uint32_t addr, uint32_t val);
int mem_write_bytes(memory* m, uint32_t addr, int count, uint8_t* vals);

int mem_read_byte(memory* m, uint32_t addr, uint8_t* result);
int mem_read_word(memory* m, uint32_t addr, uint16_t* result);
int mem_read_dword(memory* m, uint32_t addr, uint32_t* result);
int mem_read_bytes(memory* m, uint32_t addr, int count, uint8_t* result);

int mem_read_byte_e(memory* m, uint32_t addr, uint8_t* result, vm_func_event e, void** args);
int mem_read_word_e(memory* m, uint32_t addr, uint16_t* result, vm_func_event e, void** args);
int mem_read_dword_e(memory* m, uint32_t addr, uint32_t* result, vm_func_event e, void** args);
int mem_read_bytes_e(memory* m, uint32_t addr, int count, uint8_t* result, vm_func_event e, void** args);

int is_valid_addr(uint32_t addr);
char* mem_display(memory* m, uint32_t start, int count, int chunk_size);

void mem_destroy(memory* m);

#endif
