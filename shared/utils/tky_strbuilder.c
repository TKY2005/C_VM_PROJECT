#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>

#include "tky_strbuilder.h"

#include<stddef.h>

typedef struct tky_strbuilder {
    size_t  length;
    size_t  capacity;
    char*   str;

} tky_strbuilder;

static void strbuilder_resize(tky_strbuilder* s) {
    s->capacity *= 2;
    s->str = (char*) realloc(s->str, s->capacity);
}

tky_strbuilder* strbuilder_init() {
    tky_strbuilder* s = (tky_strbuilder*) malloc(sizeof(tky_strbuilder));
    s->length = 0;
    s->capacity = SB_INIT_SIZE;
    s->str = (char*) calloc(s->capacity, sizeof(char));
    return s;
}

void strbuilder_append(tky_strbuilder* s, const char* t) {
    for (int i = 0; t[i]; i++, s->length++) {
        if (s->length + 1 >= s->capacity) strbuilder_resize(s);
        s->str[s->length] = t[i];
    }
    s->str[s->length] = '\0';
}

void strbuilder_appendf(tky_strbuilder* s, const char* t, ...) {
    va_list args;
    va_start(args, t);

    char buff[4096] = {0};
    vsnprintf(buff, sizeof(buff), t, args);

    for(int i = 0; buff[i]; i++, s->length++) {
        if (s->length + 1 >= s->capacity) strbuilder_resize(s);
        s->str[s->length] = buff[i];
    }
    s->str[s->length] = '\0';
}

void strbuilder_appendc(tky_strbuilder* s, const char c) {
    if (s->length + 1 >= s->capacity) strbuilder_resize(s);
    s->str[s->length] = c;
    s->str[++s->length] = '\0';
}


void strbuilder_fill(tky_strbuilder* s, const char* t, int fill_count) {

	for(int i = 0; i < fill_count; i++) {
		strbuilder_append(s, t);
	}
}

char* strbuilder_getstr(tky_strbuilder* s) {
    return s->str;
}

void strbuilder_reset(tky_strbuilder* s) {
    s->length = 0;
    s->str[s->length] = '\0';
}

size_t strbuilder_size(tky_strbuilder* s) {
    return s->length;
}

size_t strbuilder_capacity(tky_strbuilder* s) {
    return s->capacity;
}

void strbuilder_free(tky_strbuilder* s) {
    free(s->str);
    free(s);
}
