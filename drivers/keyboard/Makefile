INCLUDE_DIRS = ./include/ ../../kernel/include/

include ../../common.mk

BUILD_DIR := ../../build

all: $(BUILD_DIR)/keyboard.o

$(BUILD_DIR)/%.o : src/%.c
	mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o $@
