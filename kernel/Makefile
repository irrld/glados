INCLUDE_DIRS = ../drivers/video/include include

all : clean kernel.bin
	/opt/homebrew/opt/binutils/bin/objcopy --target=elf32-i386 --extract-symbol build/kernel.elf build/kernel.elf.sym
#	i386-elf-objcopy --extract-symbol build/kernel.elf build/kernel.elf.sym
#	i386-elf-objcopy --only-keep-debug build/kernel.elf build/kernel.elf.sym
	/opt/homebrew/opt/binutils/bin/objcopy --target=elf32-i386 -O binary --strip-debug build/kernel.elf build/kernel.bin
#	i386-elf-objcopy -O binary build/kernel.elf build/kernel.bin

kernel.o : src/kernel.c
	mkdir -p build
	clang -O0 -m32 -target elf32-i386 -g -fno-pie -ffreestanding -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/kernel.o

kernel_entry.o : asm/kernel_entry.asm
	mkdir -p build
	nasm $< -f elf -o build/kernel_entry.o

kernel.bin: kernel_entry.o kernel.o low_level.o video.o
	i386-elf-ld -g -o build/kernel.elf -e KernelMain -Ttext 0x1000 $(addprefix build/, $^)

video.o : ../drivers/build/video.o
	mkdir -p build
	cp ../drivers/build/video.o build/video.o

%.o : src/%.c
	clang -O0 -m32 -target elf32-i386 -g -fno-pie -ffreestanding -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/$@

clean:
	rm -rf build

