all: clean
	+$(MAKE) -C bootloader
	+$(MAKE) -C drivers
	+$(MAKE) -C kernel

clean:
	+$(MAKE) clean -C bootloader
	+$(MAKE) clean -C drivers
	+$(MAKE) clean -C kernel

# Run bochs to simulate booting of our code.
run: os-image
	qemu-system-x86_64 -drive format=raw,file=build/image.img

debug: os-image
	qemu-system-x86_64 -s -S -drive format=raw,file=build/image.img

only-debug:
	qemu-system-x86_64 -s -S -drive format=raw,file=build/image.img

# This is the actual disk image that the computer loads,
# which is the combination of our compiled bootloader and kernel os-image: boot_sect.bin kernel.bin
os-image: all
	mkdir -p build
	qemu-img create build/image.img 1G
	dd if=/dev/zero of=build/image.img bs=1M count=128
	dd if=kernel/build/kernel.bin of=build/image.img bs=512 seek=0 count=25 conv=notrunc
	#dd if=bootloader/build/bootloader.bin of=build/image.img bs=512 seek=0 count=1 conv=notrunc
	#dd if=kernel/build/kernel.bin of=build/image.img bs=512 seek=1 count=16 conv=notrunc

