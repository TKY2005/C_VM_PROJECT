#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<utils/hashmap/hmap.h>
#include<utils/helpers.h>
#include<VM/vm_settings.h>

struct vm_config vm_conf = {0};

hmap* vm_get_settings(const char* filepath) {
    FILE* f = fopen(filepath, "r");

    if (!f) {
        return NULL;
    }

    char buff[255];
    hmap* h = hmap_create(fnv1_1a_hash);

    while ( (fgets(buff, sizeof(buff), f)) ) {
        char** split = split_string(buff, '=');
        split[1][strcspn(split[1], "\r\n")] = '\0';
        hmap_put(h, split[0], strlen(split[0]), (void*) split[1]);
    }
    fclose(f);
    return h;
}

void vm_set_settings(hmap* source, struct vm_config* target) {
    target->mem_size = vm_parse_conf_field( (char*) hmap_get(source, SETTINGS_MEM_FIELD, strlen(SETTINGS_MEM_FIELD)) );
    target->cycle_count = vm_parse_conf_field( (char*) hmap_get(source, SETTINGS_SPEED_FIELD, strlen(SETTINGS_SPEED_FIELD)));
    if (target->bios_path) {
        free(target->bios_path);
    }
    target->bios_path = (char*) malloc(sizeof(char) * 1024);
    target->bios_path = (char*) hmap_get( source, SETTINGS_BIOS_FIELD, strlen(SETTINGS_BIOS_FIELD) );
}

uint32_t vm_parse_conf_field(const char* fieldval) {
    int len = strlen(fieldval);
    uint32_t val = 0;
    char mode[5];

    sscanf(fieldval, "%u%4s", &val, mode);
    strtolower(mode, sizeof(mode));

    if      (strcmp(mode, "b") == 0) return val;
    else if (strcmp(mode, "kb") == 0) return val * 1024;
    else if (strcmp(mode, "mb") == 0) return val * 1024 * 1024;
    else if (strcmp(mode, "gb") == 0) return val * 1024 * 1024 * 1024;
    else if (strcmp(mode, "hz") == 0) return val;
    else if (strcmp(mode, "mhz") == 0) return val * 1000000u;
    else if (strcmp(mode, "ghz") == 0) return val * 1000000000u;
    else return val;
    return val;
}