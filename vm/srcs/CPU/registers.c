#include<string.h>
#include<stdint.h>
#include<ctype.h>
#include<stdlib.h>

#include<CPU/registers.h>
#include<utils/ds.h>

void reg_write_b(union registerfile* regfile, int regcode, uint8_t val) {
	regfile->b[regcode] = val % (0xff + 1);
}
void reg_write_w(union registerfile* regfile, int regcode, uint16_t val) {
	regfile->w[regcode] = val % (0xffff + 1);
}
void reg_write_dw(union registerfile* regfile, int regcode, uint32_t val) {
	regfile->dw[regcode] = (uint32_t) val;
}

uint8_t reg_read_b(union registerfile* regfile, int regcode) {
	return regfile->b[regcode];
}
uint16_t reg_read_w(union registerfile* regfile, int regcode) {
	return regfile->w[regcode];
}
uint32_t reg_read_dw(union registerfile* regfile, int regcode) {
	return regfile->dw[regcode];
}

void reg_set_flags(union registerfile* regfile, uint8_t flags) {
	regfile->flags |= flags;
}
void reg_clear_flags(union registerfile* regfile, uint8_t flags) {
	regfile->flags &= ~flags;
}
int reg_check_flag(union registerfile* regfile, uint8_t flag) {
	return regfile->flags & flag;
}

int reg_get_index(const char* reg) {
	
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
		if (strlen(regname) == 3) {
			char alias = regname[1];
			if (alias >= 'a' && alias <= 'f') return (alias - 'a') * 4;
			else return -1;
		}
		else {
			char alias = regname[0];
			char mode = regname[1];

			if (alias >= 'a' && alias <= 'f'){

				if (mode == 'l') return (alias - 'a') * 4;
				else if (mode == 'h') return ((alias - 'a') * 4) + 1;
				else if (mode == 'x') return (alias - 'a') * 4;
				else return -1;
			}
		}
	}
	return -1;
}

char* display_registers(union registerfile* regfile) {
	
	strbuilder regs = strbuilder_init();

	int row_count = 3;
	int reg_count = sizeof(reglist) / sizeof(reglist[0]);
	for(int i = 0; i < reg_count; i++) {
		if ( i % row_count == 0) strbuilder_append(&regs, "\n");
		
		if (i >= REG_PC_IDX){
			strbuilder_appendf(&regs, "%s: 0x%08X  ", reglist[i], regfile->dw[i]);
		}
		else strbuilder_appendf(&regs, "E%cX: 0x%08X  ", toupper(reglist[i][0]) ,regfile->dw[i]);
	}
	strbuilder_appendf(&regs, "FLAG REGISTER: Z = %d N = %d C = %d O = %d I = %d T = %d", 
	((regfile->flags & FLG_Z) != 0) ? 1 : 0,
	((regfile->flags & FLG_N) != 0) ? 1 : 0,
	((regfile->flags & FLG_C) != 0) ? 1 : 0,
	((regfile->flags & FLG_O) != 0) ? 1 : 0,
	((regfile->flags & FLG_I) != 0) ? 1 : 0,
	((regfile->flags & FLG_T) != 0) ? 1 : 0);
	return strbuilder_getstr(&regs);
}
