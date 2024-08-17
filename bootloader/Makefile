INCLUDE_DIRS = include

all: clean bootloader.o

bootloader.o : src/bootloader.asm src/stage2.asm
	mkdir -p build
	nasm $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/bootloader.o

clean:
	rm -rf build
