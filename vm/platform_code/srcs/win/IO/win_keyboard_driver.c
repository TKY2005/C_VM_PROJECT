#include<IO/keyboard_driver.h>
#include<VM/vm_input.h>

#include<conio.h>

#define KEY_ENTR 0x0d
#define KEY_DEL 0x08
#define KEY_ARROW_LEFT 0x25
#define KEY_ARROW_RIGHT 0x27

int keyboard_getchr() {
    int c = _getch();
    return c;
}

int keyboard_translate_key(int key) {
    switch(key) {
        case KEY_ENTR: 
        return VM_KEY_ENTER;
        break;
        case KEY_DEL:
        return VM_KEY_DEL;
        break;
        case KEY_ARROW_LEFT:
        return VM_KEY_ARROW_LEFT;
        break;
        case KEY_ARROW_RIGHT:
        return VM_KEY_ARROW_RIGHT;
        break;
        default:
        return key;
        break;
    }
}