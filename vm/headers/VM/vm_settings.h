#ifndef VM_SETTINGS_H
#define VM_SETTINGS_H

#define SETTINGS_MEM_FIELD "memsize"
#define SETTINGS_SPEED_FIELD "cycles"
#define SETTINGS_BIOS_FIELD "bios"

#include<stdint.h>
#include<utils/hashmap/hmap.h>

struct vm_config {
    uint64_t cycle_count;
    uint32_t mem_size;
    char bios_path[1024];
};

extern struct vm_config vm_conf;


hmap* vm_get_settings(const char* filepath);
void vm_set_settings(hmap* source, struct vm_config* target);
uint32_t vm_parse_conf_field(const char* fieldval);

#endif