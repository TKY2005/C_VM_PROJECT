#include<IO/keyboard_driver.h>
#include<VM/vm_input.h>

#include<conio.h>

#define KEY_ENTR 0x0d
#define KEY_DEL 0x08
#define KEY_ARROW_LEFT 0x25
#define KEY_ARROW_RIGHT 0x27

int platform_keymap[] = {KEY_ENTR, KEY_DEL, KEY_ARROW_LEFT, KEY_ARROW_RIGHT};

int keyboard_getchr() {
    int c = _getch();
    return c;
}

int keyboard_translate_key(int key) {
    for(int i = 0; i < KEYMAP_SIZE; i++) {
        if (key == platform_keymap[i]) return vm_keymap[i];
    }
    return key;
}

int keyboard_get_platform_code(int vmcode) {
    for(int i = 0; i < KEYMAP_SIZE; i++) {
        if (vmcode == vm_keymap[i]) return platform_keymap[i];
    }
    return vmcode;
}