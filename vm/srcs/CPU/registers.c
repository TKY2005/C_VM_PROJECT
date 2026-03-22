#include<string.h>
#include<stdint.h>
#include<ctype.h>
#include<stdlib.h>

#include<headers/CPU/registers.h>
#include<utils/ds.h>

void writeGPRb(union registerfile* regfile, int regcode, uint8_t val) {
	regfile->b[regcode] = val;
}
void writeGPRx(union registerfile* regfile, int regcode, uint16_t val) {
	regfile->w[regcode] = val;
}
void writeSPR(union registerfile* regfile, int regcode, uint32_t val) {
	regfile->dw[regcode] = val;
}

uint8_t readGPRb(union registerfile* regfile, int regcode) {
	return regfile->b[regcode];
}
uint16_t readGPRx(union registerfile* regfile, int regcode) {
	return regfile->w[regcode];
}
uint32_t readSPR(union registerfile* regfile, int regcode) {
	return regfile->dw[regcode];
}

int get_reg_code(const char* reg) {
	
	char regname[10] = {0};
	for(int i = 0; reg[i] && i < sizeof(regname); i++) regname[i] = (char) tolower(reg[i]);

	if (strcmp("pc", regname) == 0) return REG_PC_IDX * 4;
	else if (strcmp("di", regname) == 0) return REG_DI_IDX * 4;
	else if (strcmp("si", regname) == 0) return REG_SI_IDX * 4;
	else if (strcmp("sp", regname) == 0) return REG_SP_IDX * 4;
	else if (strcmp("bp", regname) == 0) return REG_BP_IDX * 4;
	else if (strcmp("cs", regname) == 0) return REG_CS_IDX * 4;
	else if (strcmp("ss", regname) == 0) return REG_SS_IDX * 4;
	else if (strcmp("ds", regname) == 0) return REG_DS_IDX * 4;

	else {
		char alias = regname[0];
		char mode = regname[1];

		if (alias >= 'a' && alias <= 'f'){

			if (mode == 'l') return (alias - 'a') + (alias - 'a');
			else if (mode == 'h') return (alias - 'a') + (alias - 'a') + 1;
			else if (mode == 'x') return (alias - 'a') * 2;
			else return -1;
		}
		else return -1;
	}
}

char* display_registers(union registerfile* regfile) {
	
	strbuilder regs = strbuilder_init();

	char reg_char = 'A';
	int row_count = 3;
	for(int i = 0; i < 6; i++) {
		if ( i % row_count == 0) strbuilder_append(&regs, "\n");
		strbuilder_appendf(&regs, "%cX: 0x%04X  ", reg_char, regfile->w[i]);
		reg_char++;
	}
	strbuilder_append(&regs, "\n");
	strbuilder_appendf(&regs, "PC: 0x%08X\n", regfile->dw[REG_PC_IDX]);
	strbuilder_appendf(&regs, "SI: 0x%08X\n", regfile->dw[REG_SI_IDX]);
	strbuilder_appendf(&regs, "DI: 0x%08X\n", regfile->dw[REG_DI_IDX]);
	strbuilder_appendf(&regs, "SP: 0x%08X\n", regfile->dw[REG_SP_IDX]);
	strbuilder_appendf(&regs, "BP: 0x%08X\n", regfile->dw[REG_BP_IDX]);
	strbuilder_appendf(&regs, "CS: 0x%08X\n", regfile->dw[REG_CS_IDX]);
	strbuilder_appendf(&regs, "SS: 0x%08X\n", regfile->dw[REG_SS_IDX]);
	strbuilder_appendf(&regs, "DS: 0x%08X", regfile->dw[REG_DS_IDX]);
	return strbuilder_getstr(&regs);
}
