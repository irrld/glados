INCLUDE_DIRS = ../drivers/video/include ./include/

CFLAGS := -Wall -Wextra -pedantic -m64 -O0 -std=c99 -finline-functions -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function -Wno-unused-parameter -g
# -std=c99 -ffreestanding -m64 -mno-red-zone -mgeneral-regs-only
BUILD_DIR := ../build

SRC_DIR := src

all : kernel.elf

kernel.elf: $(BUILD_DIR)/kernel.o $(BUILD_DIR)/string.o $(BUILD_DIR)/native.o
	ld -i -o elf64 $^ -o $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $(BUILD_DIR)/$@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	nasm -gdwarf -f elf64 $< -o $(BUILD_DIR)/$@
