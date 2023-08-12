INCLUDE_DIRS = ../drivers/video/include include

all : clean kernel.bin
	/usr/local/opt/binutils/bin/objcopy --target=elf64-x86-64 --extract-symbol build/kernel.elf build/kernel.elf.sym
#	i386-elf-objcopy --extract-symbol build/kernel.elf build/kernel.elf.sym
#	i386-elf-objcopy --only-keep-debug build/kernel.elf build/kernel.elf.sym
	/usr/local/opt/binutils/bin/objcopy --target=elf64-x86-64 -O binary --strip-debug build/kernel.elf build/kernel.bin
#	i386-elf-objcopy -O binary build/kernel.elf build/kernel.bin

kernel_entry.o : asm/kernel_entry.asm
	mkdir -p build
	nasm $< -f elf64 -o build/kernel_entry.o

kernel.bin: kernel_entry.o kernel.o low_level.o video.o
	/usr/local/opt/llvm/bin/ld.lld -g -o build/kernel.elf -e KernelMain -Ttext 0x8000 $(addprefix build/, $^)

video.o : ../drivers/build/video.o
	mkdir -p build
	cp ../drivers/build/video.o build/video.o

%.o : src/%.c
	mkdir -p build
	clang -O0 --target=x86_64-unknown-linux-gnu -g -fno-pie -ffreestanding -c $< $(addprefix -I, ${INCLUDE_DIRS}) -o build/$@

clean:
	rm -rf build

