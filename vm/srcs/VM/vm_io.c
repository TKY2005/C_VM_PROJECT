#include<stdint.h>

#include<VM/vm_io.h>
#include<VM/vm.h>
#include<VM/vm_interrupts.h>
#include<VM/vm_display.h>

int io_read(uint8_t port, uint32_t* result) {

    switch(port) {
        case IC_DATA_PORT:
            return ic_read_data(&vm_ic, (uint8_t*) result);
        
        case DISP_WIDTH_PORT:
            return disp_getw(display, (uint16_t*) result);

        case DISP_HEIGHT_PORT:
            return disp_geth(display, (uint16_t*) result);
        default:
            return -1;
    }
}

int io_write(uint8_t port, uint32_t data) {
    switch(port) {
        case IC_COMMAND_PORT:
            return ic_run_command(&vm_ic, (uint8_t) data);
        case IC_DATA_PORT:
            return ic_write_data(&vm_ic, (uint8_t) data);
        
        default:
            return -1;
    }
}