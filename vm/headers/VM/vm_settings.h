#ifndef VM_SETTINGS_H
#define VM_SETTINGS_H

#define CONFIG_PATH "./config.vconf"

#define SETTINGS_MEM_FIELD "memsize"
#define SETTINGS_SPEED_FIELD "cycles"
#define SETTINGS_BIOS_FIELD "bios"
#define SETTINGS_ENTRY_FIELD "entry"
#define SETTINGS_DISPLAY_FIELD "display"
#define SETTINGS_TIMER_FIELD "timerms"
#define SETTINGS_SPEEDUP_FIELD "timerspeedup"

#define SETTINGS_DEFAULT_SETTINGS \
"memsize=10MB\n \
cycles=1MHZ\n \
bios=./bios.tky\n \
entry=0xF9C00000\n \
display=640x480@30\n \
timerms=100\n \
timerspeedup=1"

#include<stdint.h>
#include<stdio.h>
#include<utils/tky_hashmap.h>

struct vm_config {
    uint16_t displayW;
    uint16_t displayH;
    uint16_t displayR;
    uint32_t mem_size;
    float timer_speedup_factor;
    uint32_t entrypoint;
    uint64_t cycle_count;
    uint64_t timer_interval;
    char bios_path[1024];
};

extern struct vm_config vm_conf;

FILE* create_config_file();

tky_hashmap* vm_get_settings(const char* filepath);
void vm_set_settings(tky_hashmap* source, struct vm_config* target);
uint32_t vm_parse_conf_field(const char* fieldval);
void vm_set_display_settings(const char* fieldval, struct vm_config* target);

void vm_free_settings(tky_hashmap* conf);
#endif