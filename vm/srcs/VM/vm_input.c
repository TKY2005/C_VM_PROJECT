#include<VM/vm_input.h>
#include<IO/keyboard_driver.h>

#include<stdint.h>
#include<stdio.h>

int vm_keymap[KEYMAP_SIZE] = {VM_KEY_ENTER, VM_KEY_DEL, VM_KEY_ARROW_LEFT, VM_KEY_ARROW_RIGHT};

int vm_getchr() {
    int c = keyboard_getchr();
    return keyboard_translate_key(c);
}

uint32_t vm_getnum() {
    int x;
    scanf("%u", &x);
    return x;
}

int vmchr_to_platform(int vmchar)
{
    return keyboard_get_platform_code(vmchar);
}
