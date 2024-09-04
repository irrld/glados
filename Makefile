BUILD_DIR := build
KERNEL_EXT := $(BUILD_DIR)/video.elf $(BUILD_DIR)/keyboard.o
KERNEL := $(BUILD_DIR)/kernel.elf
BOOTLOADER := $(BUILD_DIR)/bootloader.elf

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
	ld --nmagic -T linker.ld -o elf64 $(BOOTLOADER) $(KERNEL) $(KERNEL_EXT) -o $(BUILD_DIR)/linked.elf

#linked.bin: linked.elf
#	objcopy -O binary $(BUILD_DIR)/linked.elf $(BUILD_DIR)/linked.bin

build-image: all linked.elf
# Create a disk image
	qemu-img create -f raw build/disk.img 1G

# Create a GPT partition table
	sudo parted build/disk.img mklabel gpt

# Create a primary EFI System partition of type EFI System (use FAT32)
	sudo parted build/disk.img mkpart EFI fat32 1MiB 512MiB
	sudo parted build/disk.img mkpart primary ext2 512MiB 100%

# Set up a loop device
	sudo losetup -fP build/disk.img
	sleep 1

# Format the partition 1 as FAT32
	sudo mkfs.fat -F32 /dev/loop0p1
# Format the partition 2 as EXT2
	sudo mkfs.ext2 /dev/loop0p2

# Mount the EFI partition and install grub
	sudo mount /dev/loop0p1 /mnt
	sudo grub-install --target=x86_64-efi --efi-directory=/mnt --boot-directory=/mnt/boot --removable
	sudo cp grub/grub.cfg /mnt/boot/grub/grub.cfg
	sudo umount /mnt

# Mount the Ext2 partition
	sudo mount /dev/loop0p2 /mnt
# Copy the kernel and other files
	sudo mkdir -p /mnt/boot/grub
	sudo date | tee build/info.txt
	sudo cp build/linked.elf /mnt/boot/linked.elf
	sudo umount /mnt

# Detach the file
	sudo losetup -d /dev/loop0
