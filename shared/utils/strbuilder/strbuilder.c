#include<stdarg.h>
#include "./strbuilder.h"
#include "../helpers.h"

#include<stdio.h>
#include<stdlib.h>

strbuilder strbuilder_init() {
	strbuilder s;
	s.size = 0;
	s.capacity = DS_INIT_SIZE;
	s.str = (char*) malloc(DS_INIT_SIZE * sizeof(char));
	s.str[0] = '\0';
	return s;
}

static void strbuilder_resize(strbuilder* sb) {
	sb->capacity *= 2;
	sb->str = (char*) realloc(sb->str, sb->capacity);
}

void strbuilder_append(strbuilder* sb, char* s) {

	for(int i = 0; s[i]; i++, sb->size++) {
		if (sb->size + 1 >= sb->capacity)
			strbuilder_resize(sb);

		sb->str[sb->size] = s[i];
	}
	sb->str[sb->size] = '\0';
}
void strbuilder_appendf(strbuilder* sb, char* s, ...) {
	
	char buff[4096] = {0};
	va_list args;
	va_start(args, s);

	vsnprintf(buff, sizeof(buff), s, args);

	va_end(args);

	for(int i = 0; buff[i]; i++, sb->size++) {
		if (sb->size + 1 >= sb->capacity)
			strbuilder_resize(sb);

		sb->str[sb->size] = buff[i];
	}
	sb->str[sb->size] = '\0';
}
void strbuilder_append_chr(strbuilder* sb, char c) {
	
	if (sb->size + 1 >= sb->capacity)
		strbuilder_resize(sb);
	sb->str[sb->size] = c;
	sb->str[++sb->size] = '\0';
}
void strbuilder_fill(strbuilder* sb, char* f, int count) {

	for(int i = 0; i < count; i++) {
		for(int j = 0; f[j]; j++, sb->size++) {
			if (sb->size + 1 >= sb->capacity)
				strbuilder_resize(sb);
			sb->str[sb->size] = f[j];
		}
	}
	sb->str[sb->size] = '\0';
}
void strbuilder_fill_chr(strbuilder* sb, char f, int count) {
	for(int i = 0; i < count; i++, sb->size++) {
		if (sb->size + 1 >= sb->capacity)
			strbuilder_resize(sb);
		sb->str[sb->size] = f;
	}
	sb->str[sb->size] = '\0';
}

void strbuilder_reset(strbuilder* sb) {
	sb->size = 0;
	sb->str[sb->size] = '\0';
}

char* strbuilder_getstr(strbuilder* sb) {
	if (sb == NULL) return NULL;
	return sb->str;
}

char* strbuilder_substr(strbuilder* sb, int start, int end) {
	if (sb == NULL) return NULL;
	if (start < 0 || end >= sb->size) return NULL;
	int size = end - start;
	char* str = (char*) malloc(size * sizeof(char));

	for(int i = start; i < end; i++) str[i] = sb->str[i];
	return str;
}

int strbuilder_size(strbuilder* sb) {
	return sb->size;
}
int strbuilder_capacity(strbuilder* sb) {
	return sb->capacity;
}

void strbuilder_destroy(strbuilder* sb) {
	free(sb->str);
	sb->capacity = 0;
	sb->size = 0;
}