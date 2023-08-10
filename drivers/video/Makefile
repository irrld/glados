INCLUDE_DIRS = include

all: clean video.o

video.o : src/video.c
	mkdir -p build
	i386-elf-gcc -ffreestanding -c $< -Iinclude -o build/video.o

clean:
	rm -rf build
