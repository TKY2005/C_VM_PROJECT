#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<stddef.h>

#include<Memory/memory.h>
#include<Memory/bios_rom.h>
#include<Memory/framebuffer.h>
#include<Memory/RAM.h>
#include<CPU/CPU.h>
#include<utils/strbuilder/strbuilder.h>
#include<utils/helpers.h>
#include<VM/vm_events.h>
#include<VM/vm.h>

memory mem_init(int sizeB) {
  memory m = {0};

  if (sizeB > 0xffffffff) {
    return m;
  }
  m.ram = ram_init(sizeB);
  m.bios_rom = bios_rom_init(BIOS_SIZE);
  m.fb = framebuffer_init(FRAMEBUFFER_SIZE);
  return m;
}

void mem_reset(memory* mem) {
  uint32_t size = mem->ram->size;
  ram_destroy(mem->ram);
  bios_rom_destroy(mem->bios_rom);
  framebuffer_destroy(mem->fb);

  mem->ram = ram_init(size);
  mem->bios_rom = bios_rom_init(BIOS_SIZE);
  mem->fb = framebuffer_init(FRAMEBUFFER_SIZE);
}

bool is_addr_ROM(uint32_t addr) {
  return (addr >= BIOS_START && addr <= BIOS_END);
}
bool is_addr_fb(uint32_t addr) {
  return (addr >= FRAMEBUFFER_START && addr <= FRAMEBUFFER_END);
}

int mem_dispatch_read(memory* m, uint32_t addr, size_t count, uint8_t* result) {

  if (is_addr_ROM(addr)) return bios_rom_read_bytes(m->bios_rom, addr, count, result);
  else if (is_addr_fb(addr)) return framebuffer_read_bytes(m->fb, addr, count, result);
  else return ram_read_bytes(m->ram, addr, count, result);
}

int mem_dispatch_write(memory* m, uint32_t addr, size_t count, uint8_t* val) {
  // BIOS IS RDONLY
  if (is_addr_fb(addr)) return framebuffer_write_bytes(m->fb, addr, count, val);
  else return ram_write_bytes(m->ram, addr, count, val);
}

int mem_size(const memory *m) { return m->ram->size; }

int mem_write_byte(memory *m, uint32_t addr, uint8_t val) {
  return mem_dispatch_write(m, addr, 1, &val);
}

int mem_write_word(memory *m, uint32_t addr, uint16_t val) {
  uint8_t x[2] = {(val >> 8) & 0xff, val & 0xff};
  return mem_dispatch_write(m, addr, 2, x);
}
int mem_write_dword(memory *m, uint32_t addr, uint32_t val) {
  uint8_t x[4] = { 
      (val >> 24) & 0xff,
      (val >> 16) & 0xff,
      (val >> 8) & 0xff,
      val & 0xff
   };
   return mem_dispatch_write(m, addr, 4, x);
}

int mem_write_bytes(memory *m, uint32_t addr, int count, uint8_t *vals) {

  return mem_dispatch_write(m, addr, count, vals);
}

int mem_read_byte(memory *m, uint32_t addr, uint8_t* result) {
  uint8_t* r = calloc(1, sizeof(uint8_t));
  int s = mem_dispatch_read(m, addr, 1, r);
  *result = r[0];
  if(r) free(r);
  return s;
}
int mem_read_word(memory *m, uint32_t addr, uint16_t* result) {
  uint8_t* r = calloc(2, sizeof(uint8_t));
  int s = mem_dispatch_read(m, addr, 2, r);
  uint8_t high = r[0];
  uint8_t low = r[1];
  *result = (high << 8) | low;
  free(r);
  return s;
}
int mem_read_dword(memory *m, uint32_t addr, uint32_t* result) {
  
  uint8_t* r = calloc(4, sizeof(uint8_t));
  int s = mem_dispatch_read(m, addr, 4, r);

  uint16_t high = (r[0] << 8) | r[1];
  uint16_t low = (r[2] << 8) | r[3];

  *result = (high << 16) | low;
  free(r);
  return s;
}
int mem_read_bytes(memory *m, uint32_t addr, int count, uint8_t* result) {
  return mem_dispatch_read(m, addr, count, result);
}

int mem_read_byte_e(memory* m, uint32_t addr, uint8_t* result, vm_func_event e, void** args) {
  int read = mem_read_byte(m, addr, result);
  if (!e) return read;
  e(args);
  return read;
}
int mem_read_word_e(memory* m, uint32_t addr, uint16_t* result, vm_func_event e, void** args) {
  int read = mem_read_word(m, addr, result);
  if (!e) return read;
  e(args);
  e(args);
  return read;
}
int mem_read_dword_e(memory* m, uint32_t addr, uint32_t* result, vm_func_event e, void** args) {
  int read = mem_read_dword(m, addr, result);
  if (!e) return read;
  e(args);
  e(args);
  e(args);
  e(args);
  return read;
}
int mem_read_bytes_e(memory* m, uint32_t addr, int count, uint8_t* result, vm_func_event e, void** args) {
  int read = mem_read_bytes(m, addr, count, result);
  if (!e) return read;
  for(int i = 0; i < count; i++) e(args);
  return read;
}

char* mem_display(memory* m, uint32_t start, int count, int chunk_size) {
  if (!m || !m->ram) return NULL;
  if (start >= m->ram->size && !is_addr_ROM(start) && !is_addr_fb(start)) return NULL;
  if (start == -1) {
    start = 0;
  }
  if (count == -1){
    if (is_addr_ROM(start)) count = m->bios_rom->size;
    else if (is_addr_fb(start)) count = m->fb->size;
    else count = m->ram->size;
  }
  if (chunk_size == -1)
    chunk_size = DEFAULT_CHUNK_SIZE;

  int chunk_half = chunk_size / 2;
  // calculate the required padding to align the offset to the values
  char addr_dummy[25];
  snprintf(addr_dummy, sizeof(addr_dummy), "%08X:  ", 0x00);
  int addr_size = strlen(addr_dummy);

  char val_dummy[25];
  snprintf(val_dummy, sizeof(val_dummy), "%02X  ", 0x00);
  int val_size = strlen(val_dummy);

  strbuilder charset = strbuilder_init();
  strbuilder mem_str = strbuilder_init();

  strbuilder offset = strbuilder_init();
  for (int i = 0; i < chunk_size; i++) {
  
    strbuilder_appendf(&offset, "%02X", i);
    strbuilder_fill_chr(&offset, ' ', val_size);
    if ((i + 1) % chunk_half == 0) {
      strbuilder_append(&offset, "  ");
    }
  }

  strbuilder_fill_chr(&mem_str, ' ', addr_size);
  strbuilder_append(&mem_str, strbuilder_getstr(&offset));
  strbuilder_append(&mem_str, "\n");
  strbuilder_destroy(&offset);
  uint8_t c;
  for (int i = start; i < start + count; i++) {
    
    if (i % chunk_size == 0 || i - start == 0) {
      strbuilder_appendf(&mem_str, "%08X:  ", i);
    }
    mem_read_byte(m, i, &c);
    strbuilder_appendf(&mem_str, "0x%02X  ", c);
    strbuilder_append_chr(&charset, (is_printable(c) == 0 ? (char) c : '.'));

    if ((i + 1) % chunk_half == 0 && (i + 1) % chunk_size != 0) {
      strbuilder_append(&mem_str, "  ");
      strbuilder_append(&charset, "  ");
    }

    if ((i + 1) % chunk_size == 0) {
      strbuilder_append_chr(&charset, '\0');
      strbuilder_append(&mem_str, "\t|");
      strbuilder_append(&mem_str, strbuilder_getstr(&charset));
      strbuilder_append(&mem_str, "|\n");
      strbuilder_reset(&charset);
    }
  }
  if (strbuilder_size(&charset) != 0){
    strbuilder_append(&mem_str, "\t|");
    strbuilder_append(&mem_str, strbuilder_getstr(&charset));
    strbuilder_append(&mem_str, "|\n");
  }
  strbuilder_destroy(&charset);
  return strbuilder_getstr(&mem_str);
}

void mem_destroy(memory* m) {
  if (m) {
    if (m->ram) ram_destroy(m->ram);
    if (m->bios_rom) bios_rom_destroy(m->bios_rom);
    if (m->fb) framebuffer_destroy(m->fb);
  }
}
