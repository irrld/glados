INCLUDE_DIRS = ../drivers/video/include ../drivers/keyboard/include ../shell/include ./include/

CFLAGS :=-Wall -Wextra -Wpedantic -m64 -O0 -std=c17 -finline-functions -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function -Wno-unused-parameter -g -mno-red-zone -mstackrealign -mpreferred-stack-boundary=4 -mgeneral-regs-only
BUILD_DIR := ../build
SRC_DIR := src

SRC_FILES := $(shell find $(SRC_DIR) -type f \( -name "*.c" -o -name "*.asm" \))
OBJ_FILES := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%.o, $(SRC_FILES))

all : kernel.elf

kernel.elf: $(OBJ_FILES)
	ld -i -o elf64 $(OBJ_FILES) -o $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	nasm -gdwarf -f elf64 $< -o $(BUILD_DIR)/$@
