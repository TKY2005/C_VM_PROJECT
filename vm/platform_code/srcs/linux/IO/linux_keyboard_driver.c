#include<IO/keyboard_driver.h>
#include<VM/vm_input.h>
#include<termios.h>
#include<unistd.h>
#include<stdio.h>

#define KEY_ENTR 0x0A
#define KEY_DEL 0x7F
#define KEY_L_ARR 0x1B5B44
#define KEY_R_ARR 0xff53

int platform_keymap[] = {KEY_ENTR, KEY_DEL, KEY_L_ARR, KEY_R_ARR};

int keyboard_getchr() {

    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Get current settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable buffering and echoing
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings

    int ch = getchar(); // Returns immediately on keypress

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
    return ch;
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