#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<Memory/memory.h>
#include<utils/ds.h>
#include<utils/helpers.h>
#include<VM/vm_events.h>

memory mem_init(int sizeB) {
  memory m = {0};
  if (sizeB > BIT32_MAX_SIZE) {
    m.size = -1;
    m.mem = NULL;
    return m;
  }
  m.size = sizeB;
  m.mem = (uint8_t*) calloc(sizeB, sizeof(uint8_t));
  return m;
}

void mem_reset(memory* mem) {
  free(mem->mem);
  mem->mem = calloc(mem->size, sizeof(uint8_t));
}

int is_valid_addr(memory* mem, uint32_t addr) {
  if (addr < 0 || addr >= mem->size)
    return 0;
  else
    return 1;
}

int mem_size(const memory *m) { return m->size; }

int mem_write_byte(memory *m, uint32_t addr, uint8_t val) {
  if (!is_valid_addr(m, addr))
    return MEM_WRITE_FAILURE;
  else
    m->mem[addr] = val;
  return val;
}

int mem_write_word(memory *m, uint32_t addr, uint16_t val) {
  if (!is_valid_addr(m, addr) || !is_valid_addr(m, addr + 1))
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
unsigned int mem_write_dword(memory *m, uint32_t addr, uint32_t val) {
  for (int i = 0; i < 4; i++)
    if (!is_valid_addr(m, addr + i))
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

int mem_write_bytes(memory *m, uint32_t addr, int count, uint8_t *vals) {

  for (int i = 0; i < count; i++) {
    if (!is_valid_addr(m, addr + i))
      return MEM_WRITE_FAILURE;
    m->mem[addr + i] = vals[i];
  }
  return 0;
}

int mem_read_byte(memory *m, uint32_t addr, uint8_t* result) {
  if (is_valid_addr(m, addr)){
	  *result = m->mem[addr];
	  return 0;
  }
  else return MEM_READ_FAILURE;
}
int mem_read_word(memory *m, uint32_t addr, uint16_t* result) {
  if (is_valid_addr(m, addr) && is_valid_addr(m, addr + 1)) {
    uint8_t high = m->mem[addr];
    uint8_t low = m->mem[addr + 1];
    *result = (high << 8) | low;
    return 0;
  }
  else return MEM_READ_FAILURE;
}
int mem_read_dword(memory *m, uint32_t addr, uint32_t* result) {
  for (int i = 0; i < 4; i++)
    if (!is_valid_addr(m, addr + i))
      return MEM_READ_FAILURE;

  uint16_t high = (m->mem[addr + 0] << 8) | m->mem[addr + 1];
  uint16_t low = (m->mem[addr + 2] << 8) | m->mem[addr + 3];

  *result = (high << 16) | low;
  return 0;
}
int mem_read_bytes(memory *m, uint32_t addr, int count, uint8_t* result) {
  uint8_t *vals = malloc(count * sizeof(uint8_t));
  for (int i = 0; i < count; i++) {
    if (!is_valid_addr(m, addr + i)){
	    free(vals);
      	    return MEM_READ_FAILURE;
    }
    vals[i] = m->mem[addr + i];
  }
  result = vals;
  return 0;
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

  if (count == -1 || count == NULL)
    count = m->size;
  if (start == -1 || start == NULL)
    start = 0;
  if (chunk_size == -1 || chunk_size == NULL)
    chunk_size = DEFAULT_CHUNK_SIZE;

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
  }

  strbuilder_fill_chr(&mem_str, ' ', addr_size);
  strbuilder_append(&mem_str, strbuilder_getstr(&offset));
  strbuilder_append(&mem_str, "\n");
  strbuilder_destroy(&offset);

  for (int i = start; i < start + count; i++) {

    if (i % chunk_size == 0) {
      strbuilder_appendf(&mem_str, "%08X:  ", i);
    }
    strbuilder_appendf(&mem_str, "0x%02X  ", m->mem[i]);

    strbuilder_append_chr(&charset, (is_printable(m->mem[i]) == 0 ? (char)m->mem[i] : '.'));

    if ((i + 1) % chunk_size == 0) {
      strbuilder_append_chr(&charset, '\0');
      strbuilder_append(&mem_str, "\t|");
      strbuilder_append(&mem_str, strbuilder_getstr(&charset));
      strbuilder_append(&mem_str, "|\n");
      strbuilder_reset(&charset);
    }
  }
  strbuilder_destroy(&charset);
  return strbuilder_getstr(&mem_str);
}

void mem_destroy(memory* m) {
  free(m->mem);
  m->size = -1;
}
