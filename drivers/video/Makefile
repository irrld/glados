INCLUDE_DIRS = ./include/ ../../kernel/include/

CFLAGS := -std=c99 -ffreestanding -m64 -mno-red-zone -fno-builtin -nostdinc -Wall -Wextra -g -O0

BUILD_DIR := ../../build

all: $(BUILD_DIR)/video.o

$(BUILD_DIR)/%.o : src/%.c
	mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $@
