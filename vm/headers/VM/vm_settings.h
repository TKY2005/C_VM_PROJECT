#ifndef VM_SETTINGS_H
#define VM_SETTINGS_H

#define CONFIG_PATH "./config.vconf"

#define SETTINGS_MEM_FIELD "memsize"
#define SETTINGS_SPEED_FIELD "cycles"
#define SETTINGS_BIOS_FIELD "bios"
#define SETTINGS_DISPLAY_FIELD "display"

#define SETTINGS_DEFAULT_SETTINGS "memsize=10MB\ncycles=1MHZ\nbios=./bios.tky\ndisplay=640x480@30"

#include<stdint.h>
#include<stdio.h>
#include<utils/hashmap/hmap.h>

struct vm_config {
    uint16_t displayW;
    uint16_t displayH;
    uint16_t displayR;
    uint32_t mem_size;
    uint64_t cycle_count;
    char bios_path[1024];
};

extern struct vm_config vm_conf;

FILE* create_config_file();

hmap* vm_get_settings(const char* filepath);
void vm_set_settings(hmap* source, struct vm_config* target);
uint32_t vm_parse_conf_field(const char* fieldval);
void vm_set_display_settings(const char* fieldval, struct vm_config* target);
#endif