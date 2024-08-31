INCLUDE_DIRS = ../drivers/video/include ../drivers/keyboard/include ../shell/include ./include/

# We need to use -O1 or higher because -O0 some issues with stack management that I haven't been able to figure out
CFLAGS := -Wall -Wextra -pedantic -m64 -O1 -std=c99 -finline-functions -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function -Wno-unused-parameter -g
# -std=c99 -ffreestanding -m64 -mno-red-zone -mgeneral-regs-only
BUILD_DIR := ../build

SRC_DIR := src

all : kernel.elf

kernel.elf: $(BUILD_DIR)/kernel.c.o $(BUILD_DIR)/string.c.o $(BUILD_DIR)/kernel.asm.o $(BUILD_DIR)/time.c.o $(BUILD_DIR)/thread.c.o $(BUILD_DIR)/thread.asm.o $(BUILD_DIR)/page.c.o $(BUILD_DIR)/malloc.c.o
	ld -i -o elf64 $^ -o $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	nasm -gdwarf -f elf64 $< -o $(BUILD_DIR)/$@
