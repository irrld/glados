INCLUDE_DIRS = ./include/
BUILD_DIR := ../build


all: bootloader.asm.o multiboot_header.asm.o
	ld -i -o elf64 $(addprefix $(BUILD_DIR)/, $^) -o $(BUILD_DIR)/bootloader.elf

%.asm.o: src/%.asm
	@mkdir -p $(BUILD_DIR)
	nasm -gdwarf -f elf64 $< -o $(BUILD_DIR)/$@
