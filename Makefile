CC = gcc
CCCOMP = g++
INCLUDE_EMU = ./vm/headers
INCLUDE_ASM = ./assembler/headers
INCLUDE_PLATFORM = ./vm/platform_code/headers
INCLUDE_PLATFORM_SRC = ./vm/platform_code/srcs
INCLUDE_SHARED = ./shared
VM_FILES = $(wildcard ./vm/srcs/*/*.c)
PLATFORM_FILES = $(wildcard ./vm/platform_code/srcs/*.c)
SHARED_FILES = $(wildcard ./shared/*.c ./shared/utils/*.c ./shared/utils/*/*.c)
LIBS = -lm
SRCS = $(wildcard ./*.c) -std=c11 $(VM_FILES) $(PLATFORM_FILES) $(SHARED_FILES) -I $(INCLUDE_EMU) -I $(INCLUDE_PLATFORM) -I $(INCLUDE_PLATFORM_SRC) -I $(INCLUDE_SHARED) $(LIBS)
ASM_FILES = $(wildcard ./assembler/*.cpp ./assembler/srcs/*.cpp ./assembler/srcs/*/*.cpp) -std=c++11 $(SHARED_FILES) -I $(INCLUDE_ASM) -I $(INCLUDE_SHARED)

dbg_flags = -g -fno-omit-frame-pointer
make_output = mkdir bin

ifeq ($(OS),Windows_NT)
    # --- Raylib Windows Configuration ---
    RAYLIB_DIR = ./vm/raylib5_win
    RAYLIB_FLAGS = -I $(RAYLIB_DIR)/include -L $(RAYLIB_DIR)/lib -lraylib -lgdi32 -lwinmm
    SRCS += -DPLT_WIN $(RAYLIB_FLAGS)
    # ------------------------------------

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
    logcheck = .\logs
    output_target_os = .\bin\windows
else
    # --- Raylib Linux Configuration ---
    RAYLIB_DIR = ./vm/raylib5_linux
    RAYLIB_ORIGIN = ../../../vm/raylib5_linux/lib
    RAYLIB_FLAGS = -I $(RAYLIB_DIR)/include -L $(RAYLIB_DIR)/lib -lraylib -lm -lpthread -ldl -lrt -lX11 -Wl,-rpath,'$$ORIGIN/$(RAYLIB_ORIGIN)'
    SRCS += -DPLT_LINUX $(RAYLIB_FLAGS)
    # ----------------------------------

    dbg_flags += -fsanitize=address
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
    logcheck = ./logs
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
ifeq ("$(wildcard $(logcheck))","")
	mkdir $(logcheck)
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
ifeq ("$(wildcard $(logcheck))","")
	mkdir $(logcheck)
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
ifeq ("$(wildcard $(logcheck))","")
	mkdir $(logcheck)
endif

build_comp_dbg:
ifeq ("$(wildcard $(logcheck))","")
	mkdir $(logcheck)
endif
	$(CCCOMP) $(dbg_flags) $(ASM_FILES) -o $(target_asm_dbg)

build_emu_dbg:
ifeq ("$(wildcard $(logcheck))","")
	mkdir $(logcheck)
endif
	$(CC) $(dbg_flags) $(SRCS) -o $(target_dbg)

build_dbg:
ifeq ("$(wildcard $(logcheck))","")
	mkdir $(logcheck)
endif
	$(CC) $(dbg_flags) $(SRCS) -o $(target_dbg)
	$(CCCOMP) $(dbg_flags) $(ASM_FILES) -o $(target_asm_dbg)
