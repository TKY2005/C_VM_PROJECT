#include<VM/vm_input.h>
#include<IO/keyboard_driver.h>

int vm_getchr() {
    int c = keyboard_getchr();
    return keyboard_translate_key(c);
}