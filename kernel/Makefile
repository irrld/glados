INCLUDE_DIRS = ../drivers/video/include ./include/

CFLAGS := -std=c99 -ffreestanding -m64 -mno-red-zone -fno-builtin -nostdinc -Wall -Wextra -g -O0
BUILD_DIR := ../build

SRC_DIR := src

all : kernel.elf

kernel.elf: $(BUILD_DIR)/kernel.o $(BUILD_DIR)/low_level.o $(BUILD_DIR)/memcpy.o
	ld -i -o elf64 $^ -o $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $(BUILD_DIR)/$@

