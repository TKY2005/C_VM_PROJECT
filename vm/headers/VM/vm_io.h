#ifndef VM_IO_H
#define VM_IO_H

#include<stdint.h>

int io_read(uint8_t port, uint32_t* result);
int io_write(uint8_t port, uint32_t data);

#endif