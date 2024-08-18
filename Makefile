BUILD_DIR := build
KERNEL_EXT := $(BUILD_DIR)/video.o
BOOTLOADER := $(BUILD_DIR)/bootloader.elf
KERNEL := $(BUILD_DIR)/kernel.elf

all: clean
	+$(MAKE) -C bootloader
	+$(MAKE) -C drivers
	+$(MAKE) -C kernel

clean:
	rm -rf build

# Run qemu to simulate booting of our code.
run: build-image
	qemu-system-x86_64 -drive format=raw,file=build/image.img

debug: build-image
	qemu-system-x86_64 -s -S -drive format=raw,file=build/image.img

linked.o:
	ld -T linker.ld -o elf64 $(BOOTLOADER) $(KERNEL) $(KERNEL_EXT) -o $(BUILD_DIR)/linked.o
	objcopy --target=elf64-x86-64 --extract-symbol $(BUILD_DIR)/linked.o $(BUILD_DIR)/linked.o.sym

linked.bin: linked.o
	objcopy -O binary $(BUILD_DIR)/linked.o $(BUILD_DIR)/linked.bin

build-image: all linked.bin
	qemu-img create build/system.img 1G
	dd if=/dev/zero of=build/system.img bs=1M count=128
	dd if=build/linked.bin of=build/system.img bs=512 seek=0 count=25 conv=notrunc

