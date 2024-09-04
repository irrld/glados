INCLUDE_DIRS = ../drivers/video/include ../drivers/keyboard/include ../shell/include ./include/

CFLAGS :=-Wall -Wextra -Wpedantic -m64 -O0 -std=c17 -finline-functions -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function -Wno-unused-parameter -g -mno-red-zone -mstackrealign -mpreferred-stack-boundary=4 -mgeneral-regs-only
BUILD_DIR := ../build

SRC_DIR := src

all : kernel.elf

kernel.elf: $(BUILD_DIR)/kernel.c.o $(BUILD_DIR)/string.c.o $(BUILD_DIR)/kernel.asm.o $(BUILD_DIR)/time.c.o $(BUILD_DIR)/thread.c.o $(BUILD_DIR)/page.c.o $(BUILD_DIR)/mutex.c.o $(BUILD_DIR)/kmalloc.c.o
	ld -i -o elf64 $^ -o $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/glados/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/glados/%.asm
	@mkdir -p $(dir $@)
	nasm -gdwarf -f elf64 $< -o $(BUILD_DIR)/$@
