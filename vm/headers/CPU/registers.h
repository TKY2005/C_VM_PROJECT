#ifndef REGISTERS_H
#define REGISTERS_H

#include<stdint.h>
#include<stdio.h>

#define REG_PC_IDX 3
#define REG_SI_IDX 4
#define REG_DI_IDX 5
#define REG_SP_IDX 6
#define REG_BP_IDX 7
#define REG_CS_IDX 8
#define REG_SS_IDX 9
#define REG_DS_IDX 10


static char* reglist[] = {"a", "b", "c", "d", "e", "f", "pc", "si", "di", "sp", "bp", "cs", "ss", "ds"};

union registerfile {
	
	uint8_t b[44]; // total byte size = 12 + 32 = 44
	uint16_t w[22]; // total word size = 44 / 2 = 22
	uint32_t dw[11]; // total dword size = 22 / 2 = 11

	struct {
		// 6 * 2 = 12 bytes
		uint16_t A, B, C, D, E, F; // GPRs

		// SPRs
		// 8 * 4 = 32 bytes
		uint16_t PCL, PCH;
		uint16_t SIL, SIH;
		uint16_t DIL, DIH;
		uint16_t SPL, SPH;
		uint16_t BPL, BPH;
		// Segment registers
		uint16_t CSL, CSH;
		uint16_t SSL, SSH;
		uint16_t DSL, DSH;
	};
	
	struct {
		uint16_t _pad[6]; // skip the GPR addresses
		uint32_t PC, IP, SP, BP, CS, SS, DS;
	};
};

// These functions take the index of the corresponding word size, not the byte index
void writeGPRb(union registerfile* regfile, int regcode, uint8_t val);
void writeGPRx(union registerfile* regfile, int regcode, uint16_t val);
void writeSPR(union registerfile* regfile, int regcode, uint32_t val);

uint8_t readGPRb(union registerfile* regfile, int regcode);
uint16_t readGPRx(union registerfile* regfile, int regcode);
uint32_t readSPR(union registerfile* regfile, int regcode);

int get_reg_code(const char* reg); // returns the byte index of the given register, -1 if not found.

char* display_registers(union registerfile* regfile);

#endif
