#include<VM/vm_output.h>
#include<VM/vm_settings.h>
#include<VM/vm.h>

#include<stdio.h>
#include<stdarg.h>

void vm_printf(const char* txt, ...) {

    char buff[4096];
    va_list args;
    va_start(args, txt);

    vsnprintf(buff, sizeof(buff), txt, args);

    va_end(args);

    if (ui_mode) vm_display_print(buff);
    else vm_console_print(buff);
}

void vm_display_print(const char* txt) {

}
void vm_console_print(const char* txt) {
    printf(txt);
}