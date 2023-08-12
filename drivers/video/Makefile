INCLUDE_DIRS = include ../../kernel/include

all: clean video.o

video.o : src/video.c
	mkdir -p build
	clang -O0 -m32 -target elf32-i386 -g -fno-pie -ffreestanding -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/video.o

clean:
	rm -rf build
