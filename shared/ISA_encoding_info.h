#ifndef ISA_ENCODING_INFO_H
#define ISA_ENCODING_INFO_H

#include<stdint.h>

#define OPERTYPE_DEFAULT 0b11111100
#define REGSELECT_DEFAULT 0b11111111
#define DISP_DEFAULT 0b00111111

typedef enum {
    INS_EXT  = 0xFF,
    INS_NOP  = 0x01,
    INS_SET  = 0x02,
    INS_OUT  = 0x03,
    INS_ADD  = 0x04,
    INS_SUB  = 0x05,
    INS_MUL  = 0x06,
    INS_DIV  = 0x07,
    INS_POW  = 0x08,
    INS_SQRT = 0x09,
    INS_RND  = 0x0A,
    INS_CTI  = 0x0B,
    INS_INC  = 0x0C,
    INS_DEC  = 0x0D,
    INS_LA   = 0x0E,
    INS_LEN  = 0x0F,
    INS_STR  = 0x10,
    INS_OUTS = 0x11,
    INS_INP  = 0x12,
    INS_INPS = 0x13,
    INS_PUSH = 0x14,
    INS_POP  = 0x15,
    INS_CALL = 0x16,
    INS_CALE = 0x17,
    INS_CALNE= 0x18,
    INS_CAL  = 0x19,
    INS_CALLE= 0x1A,
    INS_CALG = 0x1B,
    INS_CALGE= 0x1C,
    INS_CALB = 0x1D,
    INS_JMP  = 0x1E,
    INS_JE   = 0x1F,
    INS_JNE  = 0x20,
    INS_JL   = 0x21,
    INS_JLE  = 0x22,
    INS_JG   = 0x23,
    INS_JGE  = 0x24,
    INS_JB   = 0x25,
    INS_AND  = 0x26,
    INS_OR   = 0x27,
    INS_XOR  = 0x28,
    INS_NOT  = 0x29,
    INS_NAND = 0x2A,
    INS_NOR  = 0x2B,
    INS_CMP  = 0x2C,
    INS_LOOP = 0x2D,
    INS_RET  = 0x2E,
    INS_END  = 0x2F,
    INS_INT  = 0x30,
    INS_OUTC = 0x31,
    INS_SHL  = 0x32,
    INS_SHR  = 0x33,
    INS_OUTSW= 0x34,
    INS_LENW = 0x35,
    INS_JC   = 0x36,
    INS_JO   = 0x37,
    INS_JNC  = 0x38,
    INS_JNO  = 0x39,
    INS_CALC = 0x3A,
    INS_CALO = 0x3B,
    INS_CALNC= 0x3C,
    INS_CALNO= 0x3D,
    INS_CLC  = 0x3E,
    INS_CLO  = 0x3F,
    INS_CLZ  = 0x40,
    INS_CLN  = 0x41,
    INS_CLI  = 0x42,
    INS_SLC  = 0x43,
    INS_SLO  = 0x44,
    INS_SLZ  = 0x45,
    INS_SLN  = 0x46,
    INS_SLI  = 0x47
} INS_OPCODES;


// LOWER HALF BITS ARE DECLARED FIRST. AND THEN WALK OUR WAY THROUGH HIGHER HALF BITS //
typedef struct {
    uint8_t mem_mode : 2;
    uint8_t dest_type : 3;
    uint8_t src_type : 3;
} OPER_TYPE;

typedef struct {
    uint8_t dest : 4;
    uint8_t src : 4;
} REG_SELECT;

typedef struct {
    uint8_t base : 3;
    uint8_t index : 3;
    uint8_t disp_enable : 1;
    uint8_t reg_enable : 1;
} DISP_INFO;

typedef struct {
    uint8_t opcode;

    union {
        OPER_TYPE opertype;
        uint8_t operand_types;
    };
    union {
        REG_SELECT regselect;
        uint8_t register_select;
    };
    union {
        DISP_INFO dispinfo;
        uint8_t displacement_info;
    };

    uint32_t disp_val;
    uint32_t imm_val;
    
} ins_encoding;

typedef enum {
    REG8 = 0b000,
    REG16,
    REG32,
    MEM8,
    MEM16,
    MEM32,
    NUMBER,
    UNKNOWN = 0b111
} operand_types;

typedef enum {
    NOMEM = 0b00,
    X1SCALE,
    X2SCALE,
    X4SCALE
} mem_modes;

typedef enum {
    REG_EAX = 0b0000,
    REG_EBX,
    REG_ECX,
    REG_EDX,
    REG_EEX,
    REG_EFX,
    REG_SI,
    REG_DI,
    REG_BP,
    REG_SP,
    REG_SS,
    REG_DS,
    REG_CS,
    REG_UNSELECTED = 0b1111
} reg_select_list;

typedef enum {
    NOREG = 0,
    REG = 1
} reg_enable;
typedef enum {
    NODISP = 0,
    DISP = 1
} disp_enable;

typedef enum {
    EAX = 0b000,
    EBX,
    ECX,
    EDX,
    EEX,
    EFX,
    INDEX_UNSELECTED = 0b111
} index_registers;

typedef enum {
    SP = 0b000,
    BP,
    SI,
    DI,
    CS,
    SS,
    DS,
    BASE_UNSELECTED = 0b111
} base_registers;

#endif