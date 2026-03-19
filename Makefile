CC = gcc
CCCOMP = g++
SRCS = $(wildcard ./*.c ./vm/srcs/*/*.c)
ASM_FILES = $(wildcard ./assembler/*.cpp ./assembler/srcs/*.cpp ./assembler/srcs/*/*.cpp)
SHARED = $(wildcard ./shared/*/*.c)

dbg_flags = -g -fno-omit-frame-pointer -fsanitize=address

ifeq ($(OS),Windows_NT)
	exec_path_r = .\bin\windows\release
	exec_path_d = .\bin\windows\debug
	target = $(exec_path_r)\emu.exe
	target_asm = $(exec_path_r)\tkyc.exe
	target_dbg = $(exec_path_d)\emu_dbg.exe
	target_asm_dbg = $(exec_path_d)\tkyc_dbg.exe
	shortcut = cmd /c mklink .\emu.exe $(target)
	shortcut_asm = cmd /c mklink .\tkyc.exe $(target_asm)
	shortcut_check = .\emu.exe
	shortcut_check_asm = .\tkyc.exe
else
	exec_path_r = ./bin/linux/release
	exec_path_d = ./bin/linux/debug
	target = $(exec_path_r)/emu
	target_asm = $(exec_path_r)/tkyc
	target_dbg = $(exec_path_d)/emu_dbg
	target_asm_dbg = $(exec_path_d)/tkyc_dbg
	shortcut = ln -s $(target) ./emu
	shortcut_asm = ln -s $(target_asm) ./tkyc
	shortcut_check = ./emu
	shortcut_check_asm = ./tkyc
endif


build_all:
	$(CC) $(SRCS) $(SHARED) -o $(target)
	$(CCCOMP) $(ASM_FILES) $(SHARED) -o $(target_asm)
ifeq ("$(wildcard $(shortcut_check))","")
	$(shortcut)
endif
ifeq ("$(wildcard $(shortcut_check_asm))","")
	$(shortcut_asm)
endif

build_emu:
	$(CC) $(SRCS) $(SHARED) -o $(target)
ifeq ("$(wildcard $(shortcut_check))","")
	$(shortcut)
endif

build_comp:
	$(CCCOMP) $(ASM_FILES) $(SHARED) -o $(target_asm)
ifeq ("$(wildcard $(shortcut_check_asm))","")
	$(shortcut_asm)
endif

build_comp_dbg:
	$(CCCOMP) $(dbg_flags) $(ASM_FILES) $(SHARED) -o $(target_asm_dbg)

build_emu_dbg:
	$(CC) $(dbg_flags) $(SRCS) $(SHARED) -o $(target_dbg)

build_dbg:
	$(CC) $(dbg_flags) $(SRCS) $(SHARED) -o $(target_dbg)
	$(CCCOMP) $(dbg_flags) $(ASM_FILES) $(SHARED) -o $(target_asm_dbg)
