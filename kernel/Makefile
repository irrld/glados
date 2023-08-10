INCLUDE_DIRS = ../drivers/video/include include

all : clean kernel.bin

kernel.o : src/kernel.c
	mkdir -p build
	i386-elf-gcc -ffreestanding -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/kernel.o

kernel_entry.o : asm/kernel_entry.asm
	mkdir -p build
	nasm $< -f elf -o build/kernel_entry.o

kernel.bin: kernel_entry.o kernel.o video.o
	i386-elf-ld -o build/kernel.bin -e kernel_main -Ttext 0x1000 $(addprefix build/, $^) --oformat binary

video.o : ../drivers/build/video.o
	mkdir -p build
	cp ../drivers/build/video.o build/video.o

clean:
	rm -rf build

