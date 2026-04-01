#include<VM/vm_input.h>
#include<IO/keyboard_driver.h>

#include<stdint.h>
#include<stdio.h>

int vm_getchr() {
    int c = keyboard_getchr();
    return keyboard_translate_key(c);
}

uint32_t vm_getnum() {
    int x;
    scanf("%u", &x);
    return x;
}