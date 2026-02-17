-include .config.mk

SHELL 		:= /bin/bash

TARGET 		?= qemu

LIB_SRCS 	:= $(shell find ./$(PROJECT) -type f -name "*.h" -printf " -I %h " | sort -u)

ifeq ($(TARGET), qemu)
	FLAGS 	:= $(LIB_SRCS) -mcpu=arm926ej-s -mfloat-abi=soft
	CC			:= arm-none-eabi-gcc
endif
ifeq ($(TARGET), beagle)
	FLAGS 	:= $(LIB_SRCS) -mcpu=cortex-a8 -Wall -Werror -O2 -ffreestanding -mfpu=neon -mfloat-abi=hard
	CC			:= arm-none-eabi-gcc
endif
ifeq ($(TARGET), linux)
	FLAGS		:=
	CC			:= gcc
endif

BIN 			:= $(PROJECT)/bin
SRC_DIRS 	:= $(PROJECT)/{lib,os,source}

C_SRCS 		:= $(shell find $(SRC_DIRS) -type f -name '*.c')
S_SRCS 		:= $(shell find $(PROJECT)/os -type f -name '*.s')
LINK_SRC 	:= $(wildcard ${PROJECT}/*.ld)

VPATH 		:= $(sort $(dir -u $(C_SRCS)))

START 		:= $(patsubst %.s, $(BIN)/%.o, $(notdir $(S_SRCS)))
OBJS 			:= $(patsubst %.c, $(BIN)/%.o, $(notdir $(C_SRCS)))

.PHONY: default clean init run debug help

default: $(BIN)/main.elf

$(BIN)/main.elf: save-project $(BIN) $(START) $(OBJS)
ifeq ($(CC), gcc)
	@echo "Linking object files..."
	gcc $(OBJS) -o $(BIN)/main
	@echo "Program main built correctly."
else
	@echo "Linking object files..."
	arm-none-eabi-gcc $(FLAGS) -nostdlib -T $(LINK_SRC) $(START) $(OBJS) -lgcc -o $(BIN)/main.elf
	@echo "Converting ELF to binary..."
	arm-none-eabi-objcopy -O binary $(BIN)/main.elf $(BIN)/main.bin
	@echo "Files main.elf and main.bin built correctly."
endif

$(BIN):
	mkdir -p $(BIN)

$(BIN)/%.o: %.c
	@echo "Compiling file $<"
	$(CC) $(FLAGS) -c -g $< -o $@

$(BIN)/%.o: %.s
	@echo "Assembling startup file..."
	$(CC) $(FLOAT_FLAG) -c -g $< -o $@

save-project:
	@echo "PROJECT = $(PROJECT)" > .config.mk

clean: save-project
	@echo "Cleaning up..."
	@rm -rf $(BIN)

init: save-project
	@mkdir -p $(SRC_DIRS)

run: $(BIN)/main.elf
ifeq ($(TARGET), qemu)
	qemu-system-arm -M versatilepb -nographic -kernel $(BIN)/main.elf
else ifeq ($(TARGET), linux)
	$(BIN)/main
endif

debug: $(BIN)/main.elf
ifeq ($(TARGET), qemu)
	qemu-system-arm -M versatilepb -nographic -s -S -kernel $(BIN)/main.elf
endif

help:
	@echo "Compiling and running of C program for the OS course."
	@echo "Available commands:"
	@echo " - default: compile the project to generate the .elf and .bin file for execution"
	@echo " - init: sets up the directory for starting up a project"
	@echo " - clean: cleans the compiled files"
	@echo " - run: runs the program on QEMU"
	@echo " - debug: enters debug mode on GDB"
	@echo "NOTE: the default target is QEMU, to change it use 'TARGET=beagle'"