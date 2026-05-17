#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<stdio.h>
#include<stddef.h>

#include<Memory/bios_rom.h>
#include<Memory/memory.h>

BIOS_ROM* bios_rom_init(int sizeB) {
    BIOS_ROM* b = malloc(sizeof(BIOS_ROM));
    b->size = sizeB;
    b->mem = calloc(b->size, sizeof(uint8_t));
    return b;
}
void bios_rom_reset(BIOS_ROM* BIOS_ROM) {

}

int bios_rom_size(const BIOS_ROM* m) {
    return m->size;
}

uint32_t addr2bios(uint32_t addr) {
    return addr % BIOS_START;
}

uint8_t* load_BIOS_img(const char* imgpath) {
    FILE* f = fopen(imgpath, "rb");
    if (f == NULL) return NULL;

    uint8_t* bios_content = (uint8_t*) calloc(BIOS_SIZE, sizeof(uint8_t));

    fread(bios_content, sizeof(uint8_t), BIOS_SIZE, f);
    fclose(f);
    
    return bios_content;
}

// BIOS is supposed to be a read-only section of memory and it's not mapped in the mem_dispatch_write function
// but i'm just gonna leave these in case we ever need them.
int bios_rom_write_byte(BIOS_ROM* m, uint32_t addr, uint8_t val) {
    addr = addr2bios(addr);
    if (!bios_is_valid_addr(m, addr)) return MEM_WRITE_FAILURE;
    else {
        m->mem[addr] = val;
        return MEM_WRITE_SUCCESS;
    }
}
int bios_rom_write_word(BIOS_ROM* m, uint32_t addr, uint16_t val) {
    addr = addr2bios(addr);

    if (!bios_is_valid_addr(m, addr) || !bios_is_valid_addr(m, addr + 1))
    return MEM_WRITE_FAILURE;
  else {
    uint8_t low = 0, high = 0;
    low = val & 0xff;
    high = (val >> 8) & 0xff;
    m->mem[addr] = high;
    m->mem[addr + 1] = low;
    return val;
  }
}
int bios_rom_write_dword(BIOS_ROM* m,uint32_t addr, uint32_t val) {
    addr = addr2bios(addr);
    for (int i = 0; i < 4; i++)
    if (!bios_is_valid_addr(m, addr + i))
        return MEM_WRITE_FAILURE;

  uint8_t high, lhigh, hlow, low;

  low = val & 0xff;
  hlow = (val >> 8) & 0xff;
  lhigh = (val >> 16) & 0xff;
  high = (val >> 24) & 0xff;

  m->mem[addr + 0] = high;
  m->mem[addr + 1] = lhigh;
  m->mem[addr + 2] = hlow;
  m->mem[addr + 3] = low;
  return val;
}
int bios_rom_write_bytes(BIOS_ROM* m, uint32_t addr, size_t count, uint8_t* vals) {
    addr = addr2bios(addr);
    for (int i = 0; i < count; i++) {
    if (!bios_is_valid_addr(m, addr + i))
      return MEM_WRITE_FAILURE;
    m->mem[addr + i] = vals[i];
  }
  return 0;
}

int bios_rom_read_byte(BIOS_ROM* m, uint32_t addr, uint8_t* result) {
    addr = addr2bios(addr);
    if (bios_is_valid_addr(m, addr)){
	  *result = m->mem[addr];
	  return 0;
    }
  else return MEM_READ_FAILURE;
}
int bios_rom_read_word(BIOS_ROM* m, uint32_t addr, uint16_t* result) {
    addr = addr2bios(addr);
    if (bios_is_valid_addr(m, addr) && bios_is_valid_addr(m, addr + 1)) {
    uint8_t high = m->mem[addr];
    uint8_t low = m->mem[addr + 1];
    *result = (high << 8) | low;
    return 0;
  }
  else return MEM_READ_FAILURE;
}
int bios_rom_read_dword(BIOS_ROM* m, uint32_t addr, uint32_t* result) {
    addr = addr2bios(addr);
    for (int i = 0; i < 4; i++)
    if (!bios_is_valid_addr(m, addr + i))
      return MEM_READ_FAILURE;

  uint16_t high = (m->mem[addr + 0] << 8) | m->mem[addr + 1];
  uint16_t low = (m->mem[addr + 2] << 8) | m->mem[addr + 3];

  *result = (high << 16) | low;
  return 0;
}
int bios_rom_read_bytes(BIOS_ROM* m, uint32_t addr, size_t count, uint8_t* result) {
    addr = addr2bios(addr);
    
    for (int i = 0; i < count; i++) {
        if (!bios_is_valid_addr(m, addr + i)){
            return MEM_READ_FAILURE;
        }
        result[i] = m->mem[addr + i];
    }
    return 0;
}

bool bios_is_valid_addr(BIOS_ROM* m, uint32_t addr) {
    return ( addr >= 0 && addr <= BIOS_SIZE );
}

void bios_rom_destroy(BIOS_ROM* m) {
    if (m || m->mem) {
        free(m->mem);
        free(m);
    }
}