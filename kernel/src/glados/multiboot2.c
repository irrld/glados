//
// Created by root on 9/5/24.
//

#include "glados/multiboot2.h"

multiboot_parsed_data_t parse_multiboot_info(uint32_t addr) {
  multiboot_parsed_data_t parsed;
  multiboot_tag_t* tag = (multiboot_tag_t*) (addr + 8);  // Skip total size and reserved fields
  while (tag->type != MULTIBOOT_TAG_TYPE_END) {
    if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
      parsed.framebuffer = (multiboot_tag_framebuffer_t*) tag;
    } else if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO) {
      multiboot_tag_mmap_t* mem_map_tag =
          (multiboot_tag_mmap_t*)tag;
      multiboot_mmap_entry_t* entry =
          (multiboot_mmap_entry_t*)(mem_map_tag + 1);

      uint32_t num_entries =
          (mem_map_tag->size - sizeof(multiboot_tag_mmap_t)) /
          mem_map_tag->entry_size;

      for (uint32_t i = 0; i < num_entries; i++) {
        // Print or process each memory map entry
        uint64_t base = entry->addr;
        uint64_t length = entry->len;
        uint32_t type = entry->type;

        // Memory types:
        // 1 = Available memory
        // 2 = Reserved memory
        // 3 = ACPI Reclaimable
        // 4 = NVS (Non-volatile storage)
        // 5 = Bad RAM

        if (type == 1) {
          parsed.available_memory += length;
          // save all ranges maybe?
        }

        // Move to the next memory entry
        entry = (multiboot_mmap_entry_t*)((uint8_t*)entry +
                                          mem_map_tag->entry_size);
      }
    } else if (tag->type == MULTIBOOT_TAG_TYPE_BOOTDEV) {
      multiboot_tag_bootdev_t* boot_device_tag = (multiboot_tag_bootdev_t*) tag;

      uint32_t biosdev = boot_device_tag->biosdev;  // The BIOS device number (0x80 is first hard disk)
      uint32_t part = boot_device_tag->part;  // The partition number
      uint32_t slice = boot_device_tag->slice;  // The sub-partition number

      // Print or use the drive information
      // Example: You could store the boot device for later use
    }
    // Move to the next tag
    tag = (multiboot_tag_t*) ((uint8_t*) tag + ((tag->size + 7) & ~7));  // Align to 8 bytes
  }
  return parsed;
}
