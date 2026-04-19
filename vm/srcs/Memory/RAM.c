#include<stdint.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stddef.h>

#include<Memory/RAM.h>
#include<Memory/memory.h>

bool ram_is_valid_addr(RAM* r, uint32_t addr) {
    return (addr >= 0 && addr <= r->size);
}

RAM* ram_init(size_t sizeB) {
    RAM* r = (RAM*) malloc(sizeof(RAM));
    r->size = sizeB;
    r->mem = (uint8_t*) calloc(r->size, sizeof(uint8_t));

    return r;
}

size_t ram_size(const RAM* r) {
    return r->size;
}

int ram_write_byte(RAM* r, uint32_t addr, uint8_t val) {

    if (!ram_is_valid_addr(r, addr)) return MEM_WRITE_FAILURE;
    else {
        r->mem[addr] = val;
        return MEM_WRITE_SUCCESS;
    }
}

int ram_write_word(RAM* r, uint32_t addr, uint16_t val) {
    if (!ram_is_valid_addr(r, addr) || !ram_is_valid_addr(r, addr + 1))
    return MEM_WRITE_FAILURE;
  else {
    uint8_t low = 0, high = 0;
    low = val & 0xff;
    high = (val >> 8) & 0xff;
    r->mem[addr] = high;
    r->mem[addr + 1] = low;
    return val;
  }
}

int ram_write_dword(RAM* r, uint32_t addr, uint32_t val) {
    for (int i = 0; i < 4; i++)
    if (!ram_is_valid_addr(r, addr + i))
        return MEM_WRITE_FAILURE;

  uint8_t high, lhigh, hlow, low;

  low = val & 0xff;
  hlow = (val >> 8) & 0xff;
  lhigh = (val >> 16) & 0xff;
  high = (val >> 24) & 0xff;

  r->mem[addr + 0] = high;
  r->mem[addr + 1] = lhigh;
  r->mem[addr + 2] = hlow;
  r->mem[addr + 3] = low;
  return val;
}

int ram_write_bytes(RAM* r, uint32_t addr, size_t count, uint8_t* vals) {

  for (int i = 0; i < count; i++) {
    if (!ram_is_valid_addr(r, addr + i))
      return MEM_WRITE_FAILURE;
    r->mem[addr + i] = vals[i];
  }
  return 0;
}

int ram_read_byte(RAM* r, uint32_t addr, uint8_t* result) {
    if (ram_is_valid_addr(r, addr)){
	  *result = r->mem[addr];
	  return 0;
    }
  else return MEM_READ_FAILURE;
}

int ram_read_word(RAM* r, uint32_t addr, uint16_t* result) {
    if (ram_is_valid_addr(r, addr) && ram_is_valid_addr(r, addr + 1)) {
    uint8_t high = r->mem[addr];
    uint8_t low = r->mem[addr + 1];
    *result = (high << 8) | low;
    return 0;
  }
  else return MEM_READ_FAILURE;
}

int ram_read_dword(RAM* r, uint32_t addr, uint32_t* result) {
    for (int i = 0; i < 4; i++)
    if (!ram_is_valid_addr(r, addr + i))
      return MEM_READ_FAILURE;

  uint16_t high = (r->mem[addr + 0] << 8) | r->mem[addr + 1];
  uint16_t low = (r->mem[addr + 2] << 8) | r->mem[addr + 3];

  *result = (high << 16) | low;
  return 0;
}

int ram_read_bytes(RAM* r, uint32_t addr, size_t count, uint8_t* result) {

  for (int i = 0; i < count; i++) {
    if (!ram_is_valid_addr(r, addr + i)){
      	return MEM_READ_FAILURE;
    }
    result[i] = r->mem[addr + i];
  }
  return 0;
}

void ram_destroy(RAM* r) {
    if (r->mem) free(r->mem);
    free(r);
}