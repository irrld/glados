BUILD_DIR := build
KERNEL_EXT := $(BUILD_DIR)/video.o $(BUILD_DIR)/keyboard.o
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

linked.elf:
	ld -T linker.ld -o elf64 $(BOOTLOADER) $(KERNEL) $(KERNEL_EXT) -o $(BUILD_DIR)/linked.elf

linked.bin: linked.elf
	objcopy -O binary $(BUILD_DIR)/linked.elf $(BUILD_DIR)/linked.bin

build-image: all linked.bin
	qemu-img create -f raw build/system.img 1G
	dd if=/dev/zero of=build/system.img bs=1M count=512
	dd if=build/linked.bin of=build/system.img bs=512 seek=0 conv=notrunc
	echo ",,83" | sudo sfdisk build/system.img --append
	sudo losetup -fP build/system.img
	sudo mke2fs -t ext2 /dev/loop0p1
	sudo mount /dev/loop0p1 /mnt
	cp linked.s /mnt/linked.s
	sudo umount /mnt
	sudo losetup -d /dev/loop0

