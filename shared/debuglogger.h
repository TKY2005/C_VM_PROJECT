#include<stdio.h>
#include<stdarg.h>

void printd(char* log, ...) {
    
    char buff[4096] = {0};
    va_list args;
    va_start(args, log);

    vsnprintf(buff, sizeof(buff), log, args);
    
    va_end(args);

    printf("%s", buff);
}