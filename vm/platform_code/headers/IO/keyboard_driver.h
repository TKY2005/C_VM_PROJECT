#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include<stdint.h>

int keyboard_getchr();
int keyboard_translate_key(int key);

#endif