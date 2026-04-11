#ifndef STRBUILDER_H
#define STRBUILDER_H

#include<stdarg.h>

typedef struct {
	int size;
	int capacity;
	char* str;
}strbuilder;

strbuilder strbuilder_init();

void strbuilder_append(strbuilder* sb, char* s);
void strbuilder_appendf(strbuilder* sb, char* s, ...);
void strbuilder_append_chr(strbuilder* sb, char c);

void strbuilder_fill(strbuilder* sb, char* f, int count);
void strbuilder_fill_chr(strbuilder* sb, char f, int count);

void strbuilder_reset(strbuilder* sb);

char* strbuilder_getstr(strbuilder* sb);
char* strbuilder_substr(strbuilder* sb, int start, int end);
int strbuilder_size(strbuilder* sb);
int strbuilder_capacity(strbuilder* sb);

void strbuilder_destroy(strbuilder* sb);

#endif