#ifndef REGISTERS_H
#define REGISTERS_H

#include<stdint.h>
#include<stdio.h>

#define REG_PC_IDX 6
#define REG_SI_IDX 7
#define REG_DI_IDX 8
#define REG_SP_IDX 9
#define REG_BP_IDX 10
#define REG_CS_IDX 11
#define REG_SS_IDX 12
#define REG_DS_IDX 13

#define FLG_Z (1 << 0)
#define FLG_N (1 << 1)
#define FLG_C (1 << 2)
#define FLG_O (1 << 3)
#define FLG_I (1 << 4)
#define FLG_T (1 << 5)


static char* reglist[] = {"a", "b", "c", "d", "e", "f", "PC", "SI", "DI", "SP", "BP", "CS", "SS", "DS"};

union registerfile {
	
	uint8_t b[56]; // total byte size = 24 + 32 = 56
	uint16_t w[28]; // total word size = 56 / 2 = 28
	uint32_t dw[14]; // total dword size = 28 / 2 = 14

	struct {
		// GPRs
		uint8_t ALL, ALH, AUL, AUH;
		uint8_t BLL, BLH, BUL, BUH;
		uint8_t CLL, CLH, CUL, CUH;
		uint8_t DLL, DLH, DUL, DUH;
		uint8_t ELL, ELH, EUL, EUH;
		uint8_t FLL, FLH, FUL, FUH;
		// Special purpose registers
		uint8_t PCLL, PCLH, PCUL, PCUH;
		uint8_t SILL, SILH, SIUL, SIUH;
		uint8_t DILL, DILH, DIUL, DIUH;
		uint8_t SPLL, SPLH, SPUL, SPUH;
		uint8_t BPLL, BPLH, BPUL, BPUH;
		// Segment registers
		uint8_t CSLL, CSLH, CSUL, CSUH;
		uint8_t SSLL, SSLH, SSUL, SSUH;
		uint8_t DSLL, DSLH, DSUL, DSUH;
	};

	struct {
		// 6 * 4 = 24 bytes
		uint16_t AXL, AXH;
		uint16_t BXL, BXH;
		uint16_t CXL, CXH;
		uint16_t DXL, DXH;
		uint16_t EXL, EXH;
		uint16_t FXL, FXH;
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
		uint32_t A, B, C, D, E, F;
		uint32_t PC, SI, DI, SP, BP, CS, SS, DS;
		uint8_t flags;
	};	
};

// These functions take the index of the corresponding word index, not the byte index
void reg_write_b(union registerfile* regfile, int regcode, uint8_t val);
void reg_write_w(union registerfile* regfile, int regcode, uint16_t val);
void reg_write_dw(union registerfile* regfile, int regcode, uint32_t val);

uint8_t reg_read_b(union registerfile* regfile, int regcode);
uint16_t reg_read_w(union registerfile* regfile, int regcode);
uint32_t reg_read_dw(union registerfile* regfile, int regcode);

void reg_set_flags(union registerfile* regfile, uint8_t flags);
void reg_clear_flags(union registerfile* regfile, uint8_t flags);
int reg_check_flag(union registerfile* regfile, uint8_t flag);

int reg_get_index(const char* reg); // returns the byte index of the given register, -1 if not found.

char* display_registers(union registerfile* regfile);

#endif
