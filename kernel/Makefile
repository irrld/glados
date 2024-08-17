INCLUDE_DIRS = ../drivers/video/include include

CFLAGS := -std=c99 -ffreestanding -m64 -mno-red-zone -fno-builtin -nostdinc -Wall -Wextra
LDFLAGS := -T linker.ld

all : clean kernel.bin
	objcopy --target=elf64-x86-64 --extract-symbol build/kernel.elf build/kernel.elf.sym
	objcopy --target=elf64-x86-64 -O binary --strip-debug build/kernel.elf build/kernel.bin

kernel.bin: kernel.o bootloader.o
	ld $(LDFLAGS) $(addprefix build/, $^) -o build/kernel.elf

video.o : ../drivers/build/video.o
	mkdir -p build
	cp ../drivers/build/video.o build/video.o

bootloader.o : ../bootloader/build/bootloader.o
	mkdir -p build
	cp ../bootloader/build/bootloader.o build/bootloader.o

%.o : src/%.c
	mkdir -p build
	gc $(CFLAGS) -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/$@

clean:
	rm -rf build

