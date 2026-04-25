#ifndef VM_OUTPUT_H
#define VM_OUTPUT_H

#include<stdarg.h>

void vm_printf(const char* txt, ...);

void vm_console_print(const char* txt);
void vm_display_print(const char* txt);

#endif