#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "./helpers.h"

int is_printable(char c) {
    return (c >= 0x20 && c <= 0x7E) ? 0 : 1;
}

int strtolower(char* str, int size) {

    for(int i = 0; i < size; i++) str[i] = tolower(str[i]);
    return 0;
}

char** split_string(const char* str, char delimiter) {
    if (!str) return NULL;

    // First pass: count tokens
    int count = 1;  
    for (const char *p = str; *p; p++) {
        if (*p == delimiter) count++;
    }

    // Allocate array of char* (plus NULL terminator)
    char **result = (char**) malloc((count + 1) * sizeof(char *));
    if (!result) return NULL;

    int index = 0;
    const char *start = str;
    const char *p = str;

    while (*p) {
        if (*p == delimiter) {
            size_t len = p - start;
            result[index] = (char*) malloc(len + 1);
            memcpy(result[index], start, len);
            result[index][len] = '\0';
            index++;
            start = p + 1;
        }
        p++;
    }

    // Last token
    size_t len = p - start;
    result[index] = (char*) malloc(len + 1);
    memcpy(result[index], start, len);
    result[index][len] = '\0';
    index++;

    result[index] = NULL; // Null-terminate the array
    return result;
}