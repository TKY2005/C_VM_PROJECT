#include<stdio.h>
#include<stdlib.h>

#include "./vm/headers/CPU/registers.h"
#include "./vm/headers/Memory/memory.h"

int main() {
	union registerfile regfile = {0};

	printf("bl index: %d\n", get_reg_code("bl"));
	printf("cx index: %d\n", get_reg_code("cx"));
	printf("dh index: %d\n", get_reg_code("dh"));
	printf("ss index: %d\n", get_reg_code("ss"));
	printf("bp index: %d\n\n", get_reg_code("bp"));

	writeGPRb(&regfile, 0, 0xFA);
	writeGPRx(&regfile, 2, 0x03C0);

	writeSPR(&regfile, REG_PC_IDX, 0xFFFC8730);
	writeSPR(&regfile, REG_SS_IDX, 0x77C0F098);

	writeGPRx(&regfile, get_reg_code("dx") / 2, 0xEEBC);
	writeSPR(&regfile, get_reg_code("bp") / 4, 0x0007FFFC);

	char* regs = display_registers(&regfile);
	printf("%s\n", regs);
	memory m = mem_init(0x50);
	
	mem_write_byte(&m, 0x00, 0xBC);
	mem_write_byte(&m, 0x07, 'A');
	mem_write_word(&m, 0x05, 0xFA90);
	mem_write_dword(&m, 0x40, 0x7FFFC);
	uint8_t code[] = {0x02, 0x07, 0x0b, 0x03, 0x00, 0x00, 0x00, 0x05, 0x1e, 0x06, 0x00, 0x07, 0xfc, 0x11};
	char msg[] = "Hello world!";
	char msg2[] = "Youssef Mohamed Ahmed";
	mem_write_bytes(&m, 0x20, sizeof(code), code);
	mem_write_bytes(&m, 0x30, sizeof(msg), msg);
	mem_write_bytes(&m, 0x10, sizeof(msg2), msg2);
	
	char* disp = mem_display(&m, -1, -1, -1);
	printf("\n%s\n", disp);

	free(disp);
	free(regs);

	mem_destroy(&m);
}
