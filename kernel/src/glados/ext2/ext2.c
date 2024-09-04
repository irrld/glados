#include "glados/ext2/superblock.h"
#include "glados/kernel.h"

// Constants for example purposes
#define ATA_SECTOR_SIZE 512
#define ATA_PRIMARY_IO_BASE 0x1F0
#define ATA_DRIVE_HEAD_REG (ATA_PRIMARY_IO_BASE + 6)
#define ATA_STATUS_REG (ATA_PRIMARY_IO_BASE + 7)
#define ATA_DATA_REG (ATA_PRIMARY_IO_BASE)
#define ATA_COMMAND_REG (ATA_PRIMARY_IO_BASE + 7)
#define ATA_CMD_READ_SECTORS 0x20
#define SUPERBLOCK_SIZE sizeof(ext2_superblock_t);

// Wait until the disk is ready to accept commands
void wait_for_disk() {
  while (port_byte_in(ATA_STATUS_REG) & 0x80) { // BUSY bit
                                        // wait
  }
}

void read_block(int block_number, void* buf) {
  int sector_count = SUPERBLOCK_SIZE / ATA_SECTOR_SIZE; // SUPERBLOCK_SIZE should be defined elsewhere
  int start_sector = (SUPERBLOCK_OFFSET / ATA_SECTOR_SIZE) + block_number * (SUPERBLOCK_SIZE / ATA_SECTOR_SIZE);

  wait_for_disk();

  // Select the drive and the starting sector
  port_byte_out(ATA_DRIVE_HEAD_REG, 0xE0 | ((start_sector >> 24) & 0x0F)); // Select master drive
  port_byte_out(ATA_PRIMARY_IO_BASE + 2, sector_count); // Sector count
  port_byte_out(ATA_PRIMARY_IO_BASE + 3, (uint8_t) start_sector); // Start sector (LBA low byte)
  port_byte_out(ATA_PRIMARY_IO_BASE + 4, (uint8_t)(start_sector >> 8)); // LBA mid byte
  port_byte_out(ATA_PRIMARY_IO_BASE + 5, (uint8_t)(start_sector >> 16)); // LBA high byte

  // Send the read command
  port_byte_out(ATA_COMMAND_REG, ATA_CMD_READ_SECTORS);

  wait_for_disk(); // Wait for the disk to be ready

  // Read the data
  for (int i = 0; i < sector_count * (ATA_SECTOR_SIZE / 2); ++i) {
    ((uint16_t*)buf)[i] = port_word_in(ATA_DATA_REG);
  }
}