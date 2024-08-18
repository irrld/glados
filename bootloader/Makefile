INCLUDE_DIRS = ./include/
BUILD_DIR := ../build


all: boot_sector.o stage2.o
	ld -i -o elf64 $(addprefix $(BUILD_DIR)/, $^) -o $(BUILD_DIR)/bootloader.elf

%.o: src/%.asm
	@mkdir -p $(BUILD_DIR)
	nasm -f elf64 $< -o $(BUILD_DIR)/$@
