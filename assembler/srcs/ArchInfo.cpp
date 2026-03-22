#include<string>
#include<vector>
#include<map>

#include "../headers/ArchInfo.hpp"
#include "../../shared/dtypes.h"
#include "../../shared/ISA_encoding_info.h"

using std::string, std::map, std::vector;


string ArchInfo::operators = "+-*/";
int ArchInfo::dwrstart = 18;

vector<string> ArchInfo::reglist = {
    "al", "ah", "ax", "eax", 
    "bl", "bh", "bx", "ebx", 
    "cl", "ch", "cx", "ecx", 
    "dl", "dh", "dx", "edx", 
    "el", "eh", "ex", "eex", 
    "fl", "fh", "fx", "efx",
    "pc", "sp", "bp",
    "di", "si",
    "cs", "ds", "ss"
};

vector<string> ArchInfo::dirList = {
	"org", "section", "equ", "define",
	"byte", "word",
	"defb", "defw",
	"resb", "resw"
};

map<string, uint8_t> ArchInfo::insmap = {
    {"ext",  (ubyte)INS_EXT},
    {"nop",  (ubyte)INS_NOP},
    {"set",  (ubyte)INS_SET},
    {"mov",  (ubyte)INS_SET},
    {"out",  (ubyte)INS_OUT},
    {"add",  (ubyte)INS_ADD},
    {"sub",  (ubyte)INS_SUB},
    {"mul",  (ubyte)INS_MUL},
    {"div",  (ubyte)INS_DIV},
    {"pow",  (ubyte)INS_POW},
    {"sqrt", (ubyte)INS_SQRT},
    {"rnd",  (ubyte)INS_RND},
    {"cti",  (ubyte)INS_CTI},
    {"inc",  (ubyte)INS_INC},
    {"dec",  (ubyte)INS_DEC},
    {"la",   (ubyte)INS_LA},
    {"len",  (ubyte)INS_LEN},
    {"str",  (ubyte)INS_STR},
    {"outs", (ubyte)INS_OUTS},
    {"inp",  (ubyte)INS_INP},
    {"inps", (ubyte)INS_INPS},
    {"push", (ubyte)INS_PUSH},
    {"pop",  (ubyte)INS_POP},
    {"call", (ubyte)INS_CALL},
    {"cale", (ubyte)INS_CALE},
    {"calne",(ubyte)INS_CALNE},
    {"cal",  (ubyte)INS_CAL},
    {"calle",(ubyte)INS_CALLE},
    {"calg", (ubyte)INS_CALG},
    {"calge",(ubyte)INS_CALGE},
    {"calb", (ubyte)INS_CALB},
    {"jmp",  (ubyte)INS_JMP},
    {"je",   (ubyte)INS_JE},
    {"jne",  (ubyte)INS_JNE},
    {"jl",   (ubyte)INS_JL},
    {"jle",  (ubyte)INS_JLE},
    {"jg",   (ubyte)INS_JG},
    {"jge",  (ubyte)INS_JGE},
    {"jb",   (ubyte)INS_JB},
    {"and",  (ubyte)INS_AND},
    {"or",   (ubyte)INS_OR},
    {"xor",  (ubyte)INS_XOR},
    {"not",  (ubyte)INS_NOT},
    {"nand", (ubyte)INS_NAND},
    {"nor",  (ubyte)INS_NOR},
    {"cmp",  (ubyte)INS_CMP},
    {"loop", (ubyte)INS_LOOP},
    {"ret",  (ubyte)INS_RET},
    {"end",  (ubyte)INS_END},
    {"int",  (ubyte)INS_INT},
    {"outc", (ubyte)INS_OUTC},
    {"shl",  (ubyte)INS_SHL},
    {"shr",  (ubyte)INS_SHR},
    {"outsw",(ubyte)INS_OUTSW},
    {"lenw", (ubyte)INS_LENW},
    {"jc",   (ubyte)INS_JC},
    {"jo",   (ubyte)INS_JO},
    {"jnc",  (ubyte)INS_JNC},
    {"jno",  (ubyte)INS_JNO},
    {"calc", (ubyte)INS_CALC},
    {"calo", (ubyte)INS_CALO},
    {"calnc",(ubyte)INS_CALNC},
    {"calno",(ubyte)INS_CALNO},
    {"clc",  (ubyte)INS_CLC},
    {"clo",  (ubyte)INS_CLO},
    {"clz",  (ubyte)INS_CLZ},
    {"cln",  (ubyte)INS_CLN},
    {"cli",  (ubyte)INS_CLI},
    {"slc",  (ubyte)INS_SLC},
    {"slo",  (ubyte)INS_SLO},
    {"slz",  (ubyte)INS_SLZ},
    {"sln",  (ubyte)INS_SLN},
    {"sli",  (ubyte)INS_SLI}
};

map<string, uint8_t> ArchInfo::reg_select_map = {
    {"al", (ubyte) REG_EAX},
    {"ah", (ubyte) REG_EAX},
    {"ax", (ubyte) REG_EAX},
    {"eax", (ubyte) REG_EAX},
    {"bl", (ubyte) REG_EBX},
    {"bh", (ubyte) REG_EBX},
    {"bx", (ubyte) REG_EBX},
    {"ebx", (ubyte) REG_EBX},
    {"cl", (ubyte) REG_ECX},
    {"ch", (ubyte) REG_ECX},
    {"cx", (ubyte) REG_ECX},
    {"ecx", (ubyte) REG_ECX},
    {"dl", (ubyte) REG_EDX},
    {"dh", (ubyte) REG_EDX},
    {"dx", (ubyte) REG_EDX},
    {"edx", (ubyte) REG_EDX},
    {"el", (ubyte) REG_EEX},
    {"eh", (ubyte) REG_EEX},
    {"ex", (ubyte) REG_EEX},
    {"eex", (ubyte) REG_EEX},
    {"fl", (ubyte) REG_EFX},
    {"fh", (ubyte) REG_EFX},
    {"fx", (ubyte) REG_EFX},
    {"efx", (ubyte) REG_EFX},
    {"sp", (ubyte) REG_SP},
    {"bp", (ubyte) REG_BP},
    {"di", (ubyte) REG_DI},
    {"si", (ubyte) REG_SI},
    {"ss", (ubyte) REG_SS},
    {"ds", (ubyte) REG_DS},
    {"ss", (ubyte) REG_SS}
};

map<string, uint8_t> ArchInfo::index_select_map = {
    {"al", (ubyte) EAX},
    {"ah", (ubyte) EAX},
    {"ax", (ubyte) EAX},
    {"eax", (ubyte) EAX},
    {"bl", (ubyte) EBX},
    {"bh", (ubyte) EBX},
    {"bx", (ubyte) EBX},
    {"ebx", (ubyte) EBX},
    {"cl", (ubyte) ECX},
    {"ch", (ubyte) ECX},
    {"cx", (ubyte) ECX},
    {"ecx", (ubyte) ECX},
    {"dl", (ubyte) EDX},
    {"dh", (ubyte) EDX},
    {"dx", (ubyte) EDX},
    {"edx", (ubyte) EDX},
    {"el", (ubyte) EEX},
    {"eh", (ubyte) EEX},
    {"ex", (ubyte) EEX},
    {"eex", (ubyte) EEX},
    {"fl", (ubyte) EFX},
    {"fh", (ubyte) EFX},
    {"fx", (ubyte) EFX},
    {"efx", (ubyte) EFX},
};

map<string, uint8_t> ArchInfo::base_select_map = {
    {"sp", (ubyte) SP},
    {"bp", (ubyte) BP},
    {"si", (ubyte) SI},
    {"di", (ubyte) DI},
    {"cs", (ubyte) CS},
    {"ss", (ubyte) SS},
    {"ds", (ubyte) DS},
};