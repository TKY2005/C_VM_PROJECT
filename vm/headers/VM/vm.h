#ifndef VM_H
#define VM_H

#include<CPU/CPU.h>
#include<Memory/memory.h>
#include<utils/hashmap/hmap.h>

#include<stdint.h>

#define VM_VER "1.3.0b"

#define CONFIG_PATH "./config.vconf"

// ERRORS: < 0
#define FILE_NOT_OPEN -1
#define NOT_ENOUGH_MEMORY -2
#define NO_VALID_TARGET -3
#define VM_COULDNT_EXECUTE -4
#define INTERRUPT_FLAG_DISABLED -5

// WARNINGS: > 0
#define FILE_INCOMPLETE 1

#define VM_SUCCESS 0

#define METADATA_SIZE 33

struct vm_config {
    char* bios_path;
    uint64_t cycle_count;
    uint32_t mem_size;
};

extern struct vm_config vm_conf;
extern CPU* vm_cpu;
extern memory vm_memory;
extern uint8_t* bios_memory;

hmap* vm_get_settings(const char* filepath);
void vm_set_settings(hmap* source, struct vm_config* target);

void vm_init(int memsize);
void vm_init_nomem();
int vm_load_code_file(memory* target, const char* filepath, uint32_t* entry_dest);
int vm_load_binary_file(memory* target, const char* filepath, uint32_t load_addr);
int vm_interrupt(CPU* cpu, memory* mem, uint8_t icode);
int vm_runp(CPU* cpu, memory* mem, uint32_t entry);
int vm_runf(CPU* cpu, memory* mem, const char* filepath);
int vm_boot_sequence(CPU* cpu, memory* mem);

void vm_shutdown();

void vm_shell();
void vm_run_shell_command(char* command);
void vm_debug_shell();

#endif
