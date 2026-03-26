#ifndef ISA_ENCODING_INFO_H
#define ISA_ENCODING_INFO_H

#include<stdint.h>

#define OPERTYPE_DEFAULT 0b11111100
#define REGSELECT_DEFAULT 0b11111111
#define DISP_DEFAULT 0b00111111

#define INSTRUCTION_LIST(X) \
    X(EXT,   0xFF, _ext) \
    X(NOP,   0x01, _nop) \
    X(SET,   0x02, _set) \
    X(OUT,   0x03, _out) \
    X(ADD,   0x04, _add) \
    X(SUB,   0x05, _sub) \
    X(MUL,   0x06, _mul) \
    X(DIV,   0x07, _div) \
    X(POW,   0x08, _pow) \
    X(SQRT,  0x09, _sqrt) \
    X(RND,   0x0A, _rnd) \
    X(CTI,   0x0B, _cti) \
    X(INC,   0x0C, _inc) \
    X(DEC,   0x0D, _dec) \
    X(LA,    0x0E, _la) \
    X(LEN,   0x0F, _len) \
    X(STR,   0x10, _str) \
    X(OUTS,  0x11, _outs) \
    X(INP,   0x12, _inp) \
    X(INPS,  0x13, _inps) \
    X(PUSH,  0x14, _push) \
    X(POP,   0x15, _pop) \
    X(CALL,  0x16, _call) \
    X(CALE,  0x17, _cale) \
    X(CALNE, 0x18, _calne) \
    X(CAL,   0x19, _cal) \
    X(CALLE, 0x1A, _calle) \
    X(CALG,  0x1B, _calg) \
    X(CALGE, 0x1C, _calge) \
    X(CALB,  0x1D, _calb) \
    X(JMP,   0x1E, _jmp) \
    X(JE,    0x1F, _je) \
    X(JNE,   0x20, _jne) \
    X(JL,    0x21, _jl) \
    X(JLE,   0x22, _jle) \
    X(JG,    0x23, _jg) \
    X(JGE,   0x24, _jge) \
    X(JB,    0x25, _jb) \
    X(AND,   0x26, _and_) \
    X(OR,    0x27, _or_) \
    X(XOR,   0x28, _xor) \
    X(NOT,   0x29, _not_) \
    X(NAND,  0x2A, _nand) \
    X(NOR,   0x2B, _nor) \
    X(CMP,   0x2C, _cmp) \
    X(LOOP,  0x2D, _loop) \
    X(RET,   0x2E, _ret) \
    X(END,   0x2F, _end) \
    X(INT,   0x30, _int_) \
    X(OUTC,  0x31, _outc) \
    X(SHL,   0x32, _shl) \
    X(SHR,   0x33, _shr) \
    X(OUTSW, 0x34, _outsw) \
    X(LENW,  0x35, _lenw) \
    X(JC,    0x36, _jc) \
    X(JO,    0x37, _jo) \
    X(JNC,   0x38, _jnc) \
    X(JNO,   0x39, _jno) \
    X(CALC,  0x3A, _calc) \
    X(CALO,  0x3B, _calo) \
    X(CALNC, 0x3C, _calnc) \
    X(CALNO, 0x3D, _calno) \
    X(CLC,   0x3E, _clc) \
    X(CLO,   0x3F, _clo) \
    X(CLZ,   0x40, _clz) \
    X(CLN,   0x41, _cln) \
    X(CLI,   0x42, _cli) \
    X(SLC,   0x43, _slc) \
    X(SLO,   0x44, _slo) \
    X(SLZ,   0x45, _slz) \
    X(SLN,   0x46, _sln) \
    X(SLI,   0x47, _sli)


#define X(mnemonic, opcode, funcname) INS_##mnemonic = opcode,

typedef enum {
    INSTRUCTION_LIST(X)
    DUMMY // just to avoid the problems the trailing comma might make
} INS_OPCODES;

#undef X

typedef struct {
    const char* signature;
    const char* date;
    uint32_t ver;
    uint8_t arch;
    uint32_t entrypoint;
} file_metadata;

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

    int32_t disp_val;
    uint32_t imm_val;
    // used by the CPU. not the assembler.
    uint32_t sym;
    int single_oper : 1;
    int symflg : 1;
    int resolve_sym : 1;
    int flg_mod : 1;
    
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