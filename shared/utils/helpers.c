#include "./helpers.h"

int is_printable(char c) {
    return (c >= 0x20 && c <= 0x7E) ? 0 : 1;
}