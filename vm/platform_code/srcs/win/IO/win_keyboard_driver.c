#include<IO/keyboard_driver.h>
#include<vm/vm.h>

#include<conio.h>

#define KEY_ENTR 0x0d
#define KEY_DEL 0x08

int keyboard_getchr() {
    int c = _getch();
    return c;
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