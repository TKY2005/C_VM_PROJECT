#ifndef HELPERS_H
#define HELPERS_H

#define DS_INIT_SIZE 50
#include<stdint.h>

int vptoint(void* d);
uint32_t vptouint(void* d);
float vptofloat(void* d);
char vptochar(void* d);
char* vptostr(void* d);

int is_printable(char c);
int strtolower(char* str, int size);
char** split_string(const char* str, char delimiter);

#endif