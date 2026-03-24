CC = gcc
CCCOMP = g++
INCLUDE_EMU = ./vm/headers
INCLUDE_ASM = ./assembler/headers
INCLUDE_SHARED = ./shared
SHARED_FILES = $(wildcard ./shared/*/*.c)
SRCS = $(wildcard ./*.c ./vm/srcs/*/*.c) $(SHARED_FILES) -I $(INCLUDE_EMU) -I $(INCLUDE_SHARED)
ASM_FILES = $(wildcard ./assembler/*.cpp ./assembler/srcs/*.cpp ./assembler/srcs/*/*.cpp) $(SHARED_FILES) -I $(INCLUDE_ASM) -I $(INCLUDE_SHARED)

dbg_flags = -g -fno-omit-frame-pointer -fsanitize=address
make_output = mkdir bin

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

	outputcheck = .\bin
	output_target_os = .\bin\windows
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
	outputcheck = ./bin
	output_target_os = ./bin/linux
	release = /release
endif


build_all:
ifeq ("$(wildcard $(outputcheck))","")
	$(make_output)
	mkdir $(output_target_os)
	mkdir $(exec_path_r)
	mkdir $(exec_path_d)
endif
	$(CC) $(SRCS) -o $(target)
	$(CCCOMP) $(ASM_FILES) -o $(target_asm)
ifeq ("$(wildcard $(shortcut_check))","")
	$(shortcut)
endif
ifeq ("$(wildcard $(shortcut_check_asm))","")
	$(shortcut_asm)
endif

build_emu:
ifeq ("$(wildcard $(outputcheck))","")
	$(make_output)
	mkdir $(output_target_os)
	mkdir $(exec_path_r)
	mkdir $(exec_path_d)
endif
	$(CC) $(SRCS) -o $(target)
ifeq ("$(wildcard $(shortcut_check))","")
	$(shortcut)
endif

build_comp:
ifeq ("$(wildcard $(outputcheck))","")
	$(make_output)
	mkdir $(output_target_os)
	mkdir $(exec_path_r)
	mkdir $(exec_path_d)
endif
	$(CCCOMP) $(ASM_FILES) -o $(target_asm)
ifeq ("$(wildcard $(shortcut_check_asm))","")
	$(shortcut_asm)
endif

build_comp_dbg:
	$(CCCOMP) $(dbg_flags) $(ASM_FILES) -o $(target_asm_dbg)

build_emu_dbg:
	$(CC) $(dbg_flags) $(SRCS) -o $(target_dbg)

build_dbg:
	$(CC) $(dbg_flags) $(SRCS) -o $(target_dbg)
	$(CCCOMP) $(dbg_flags) $(ASM_FILES) -o $(target_asm_dbg)
