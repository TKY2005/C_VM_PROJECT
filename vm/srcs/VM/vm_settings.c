#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<utils/tky_hashmap.h>
#include<utils/helpers.h>
#include<VM/vm_settings.h>

struct vm_config vm_conf = {0};

FILE* create_config_file() {
    FILE* f = fopen(CONFIG_PATH, "w+");
    fputs(SETTINGS_DEFAULT_SETTINGS, f);
    rewind(f);
    return f;
}

tky_hashmap* vm_get_settings(const char* filepath) {
    FILE* f = fopen(filepath, "r+");
    if (!f) {
        printf("No config file was found. creating a new one with the default settings.\n");
        f = create_config_file();
    }

    char buff[255];
    tky_hashmap* h = hashmap_init(hf_fnv1a);

    char** split = NULL;
    while ( (fgets(buff, sizeof(buff), f)) ) {
        split = split_string(buff, '=');
        split[1][strcspn(split[1], "\r\n")] = '\0';

        int fieldlen = strlen(split[0]);
        int valuelen = strlen(split[1]);

        char* field = calloc(fieldlen + 1, sizeof(char));
        char* value = calloc(valuelen + 1, sizeof(char));

        memcpy(field, split[0], fieldlen);
        memcpy(value, split[1], valuelen);
        hashmap_put(h, field, fieldlen, (void*) value);

        free(field);
        if(split) split_string_free(split);
    }
    fclose(f);
    return h;
}

void vm_set_settings(tky_hashmap* source, struct vm_config* target) {

    char* mem_field = (char*) hashmap_get(source, SETTINGS_MEM_FIELD, strlen(SETTINGS_MEM_FIELD));
    char* cycle_field = (char*) hashmap_get(source, SETTINGS_SPEED_FIELD, strlen(SETTINGS_SPEED_FIELD));
    char* bios_field = (char*) hashmap_get(source, SETTINGS_BIOS_FIELD, strlen(SETTINGS_BIOS_FIELD));
    char* display_field = (char*) hashmap_get(source, SETTINGS_DISPLAY_FIELD, strlen(SETTINGS_DISPLAY_FIELD));
    char* entry_field = (char*) hashmap_get(source, SETTINGS_ENTRY_FIELD, strlen(SETTINGS_ENTRY_FIELD));
    char* timer_field = (char*) hashmap_get(source, SETTINGS_TIMER_FIELD, strlen(SETTINGS_TIMER_FIELD));
    char* speedup_field = (char*) hashmap_get(source, SETTINGS_SPEEDUP_FIELD, strlen(SETTINGS_SPEEDUP_FIELD));

    target->mem_size = vm_parse_conf_field(mem_field);
    target->cycle_count = vm_parse_conf_field(cycle_field);
    target->timer_interval = vm_parse_conf_field(timer_field);
    sscanf(speedup_field, "%f", &target->timer_speedup_factor);

    int path_size = strlen(bios_field);
    strncpy(target->bios_path, bios_field, path_size);

    vm_set_display_settings(display_field, target);

    target->entrypoint = strtoul(entry_field, NULL, 0);

    free(mem_field);
    free(cycle_field);
    free(bios_field);
    free(display_field);
    free(entry_field);
    free(timer_field);
    free(speedup_field);
}

uint32_t vm_parse_conf_field(const char* fieldval) {
    uint32_t val = 0;
    char mode[5] = {0};

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

void vm_set_display_settings(const char* fieldval, struct vm_config* target) {
    uint16_t w, h, r;
    sscanf(fieldval, "%hux%hu@%hu", &w, &h, &r);
    target->displayW = w;
    target->displayH = h;
    target->displayR = r;
}


void vm_free_settings(tky_hashmap* conf) {
    hashmap_free(conf);
}