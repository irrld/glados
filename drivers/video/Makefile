INCLUDE_DIRS = ./include/ ../../kernel/include/

include ../../common.mk

BUILD_DIR := ../../build

all: $(BUILD_DIR)/video.o $(BUILD_DIR)/fbterm.o
	ld -r -o elf64 $^ -o $(BUILD_DIR)/video.elf

$(BUILD_DIR)/%.o : src/%.c
	mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $@
