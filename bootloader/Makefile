all: clean bootloader.bin

bootloader.bin : src/bootloader.asm
	mkdir -p build
	nasm $< -f bin -I 'src/' -o build/bootloader.bin

clean:
	rm -rf build
