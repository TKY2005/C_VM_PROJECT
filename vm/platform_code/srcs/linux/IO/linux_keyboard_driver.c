#include<IO/keyboard_driver.h>
#include<vm/vm.h>
#include<termios.h>
#include<unistd.h>
#include<stdio.h>

#define KEY_ENTR 0x0A
#define KEY_DEL 0x7F

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
    switch(key) {
        case KEY_ENTR: 
        return VM_KEY_ENTR;
        break;
        case KEY_DEL:
        return VM_KEY_DEL;
        break;
        default:
        return key;
        break;
    }
}