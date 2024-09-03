INCLUDE_DIRS = ./include/ ../../kernel/include/

CFLAGS :=-Wall -Wextra -Wpedantic -m64 -O0 -std=c17 -finline-functions -fno-stack-protector -nostdinc -ffreestanding -Wno-unused-function -Wno-unused-parameter -g -mno-red-zone -mstackrealign -mpreferred-stack-boundary=4 -mgeneral-regs-only

BUILD_DIR := ../../build

all: $(BUILD_DIR)/video.o

$(BUILD_DIR)/%.o : src/%.c
	mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $@
