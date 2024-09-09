//
// Created by root on 9/5/24.
//

#ifndef GLADOS_ATA_H
#define GLADOS_ATA_H

#include "stddef.h"

#define ATA_DEV_BUSY    0x80
#define ATA_DEV_DRQ     0x08

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define AHCI_BASE       0x400000     // 4M
#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000
#define HBA_PxIS_TFES   (1 << 30)       /* TFES - Task File Error Status */

#define SATA_SIG_ATA    0x00000101     // SATA drive
#define SATA_SIG_ATAPI  0xEB140101     // SATAPI drive
#define SATA_SIG_SEMB   0xC33C0101     // Enclosure management bridge
#define SATA_SIG_PM     0x96690101     // Port multiplier

#define AHCI_DEV_NULL   0
#define AHCI_DEV_SATA   1
#define AHCI_DEV_SEMB   2
#define AHCI_DEV_PM     3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE   1
#define HBA_PORT_DET_PRESENT  3

enum fis_type {
  FIS_TYPE_REG_H2D    = 0x27,     // Register FIS - host to device
  FIS_TYPE_REG_D2H    = 0x34,     // Register FIS - device to host
  FIS_TYPE_DMA_ACT    = 0x39,     // DMA activate FIS - device to host
  FIS_TYPE_DMA_SETUP  = 0x41,     // DMA setup FIS - bidirectional
  FIS_TYPE_DATA       = 0x46,     // Data FIS - bidirectional
  FIS_TYPE_BIST       = 0x58,     // BIST activate FIS - bidirectional
  FIS_TYPE_PIO_SETUP  = 0x5F,     // PIO setup FIS - device to host
  FIS_TYPE_DEV_BITS   = 0xA1,     // Set device bits FIS - device to host
};
typedef uint8_t fis_type_t;

typedef volatile struct hba_port {
  uint32_t clb;       // 0x00, command list base address, 1K-byte aligned
  uint32_t clbu;      // 0x04, command list base address upper 32 bits
  uint32_t fb;        // 0x08, FIS base address, 256-byte aligned
  uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
  uint32_t is;        // 0x10, interrupt status
  uint32_t ie;        // 0x14, interrupt enable
  uint32_t cmd;       // 0x18, command and status
  uint32_t rsv0;      // 0x1C, Reserved
  uint32_t tfd;       // 0x20, task file data
  uint32_t sig;       // 0x24, signature
  uint32_t ssts;      // 0x28, SATA status (SCR0:SStatus)
  uint32_t sctl;      // 0x2C, SATA control (SCR2:SControl)
  uint32_t serr;      // 0x30, SATA error (SCR1:SError)
  uint32_t sact;      // 0x34, SATA active (SCR3:SActive)
  uint32_t ci;        // 0x38, command issue
  uint32_t sntf;      // 0x3C, SATA notification (SCR4:SNotification)
  uint32_t fbs;       // 0x40, FIS-based switch control
  uint32_t rsv1[11];  // 0x44 ~ 0x6F, Reserved
  uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
} hba_port_t;

typedef volatile struct hba_mem {
  // 0x00 - 0x2B, Generic Host Control
  uint32_t cap;              // 0x00, Host capability
  uint32_t ghc;              // 0x04, Global host control
  uint32_t interrupt_status; // 0x08, Interrupt status
  uint32_t port_implemented; // 0x0C, Port implemented
  uint32_t version;          // 0x10, Version
  uint32_t ccc_ctl;          // 0x14, Command completion coalescing control
  uint32_t ccc_pts;          // 0x18, Command completion coalescing ports
  uint32_t em_loc;           // 0x1C, Enclosure management location
  uint32_t em_ctl;           // 0x20, Enclosure management control
  uint32_t cap_2;            // 0x24, Host capabilities extended
  uint32_t bios_handoff;     // 0x28, BIOS/OS handoff control and status

  // 0x2C - 0x9F, Reserved
  uint8_t reserved_0[0xA0 - 0x2C];

  // 0xA0 - 0xFF, Vendor specific registers
  uint8_t vendor[0x100 - 0xA0];

  // 0x100 - 0x10FF, Port control registers
  hba_port_t ports[1];     // 1 ~ 32
} hba_mem_t;

typedef struct hba_cmd_header {
  // DW0
  uint8_t  command_fis_length : 5;          // Command FIS length in DWORDS, 2 ~ 16
  uint8_t  atapi : 1;          // ATAPI
  uint8_t  write : 1;          // Write, 1: H2D, 0: D2H
  uint8_t  prefetchable : 1;          // Prefetchable

  uint8_t  reset : 1;          // Reset
  uint8_t  bist : 1;          // BIST
  uint8_t  clear_busy_on_ok : 1;          // Clear busy upon R_OK
  uint8_t  reserved_0 : 1;          // Reserved
  uint8_t  pm_port : 4;          // Port multiplier port

  uint16_t prdt_len; // Physical region descriptor table length in entries

  // DW1
  volatile uint32_t prd_transferred_bytes;          // Physical region descriptor byte count transferred

  // DW2, 3
  uint64_t ctd_base_addr;          // Command table descriptor base address

  // DW4 - 7
  uint32_t reserved_1[4];     // Reserved
} hba_cmd_header_t;

typedef struct fis_dma_setup {
  fis_type_t fis_type;     // FIS_TYPE_DMA_SETUP
  uint8_t port_multiplier : 4;     // Port multiplier
  uint8_t reserved_0 : 1;          // Reserved
  uint8_t transfer_dir : 1;          // Data transfer direction, 1 - device to host
  uint8_t interrupt : 1;          // Interrupt bit
  uint8_t auto_activate : 1;            // Auto-activate. Specifies if DMA Activate FIS is needed
  uint16_t reserved_1;       // Reserved
  uint64_t dma_buffer_id;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
                         // SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
  uint32_t reserved_2;
  uint32_t dma_buf_offset; // Byte offset into buffer. First 2 bits must be 0
  uint32_t transfer_count; // Number of bytes to transfer. Bit 0 must be 0
  uint32_t reserved_3;

} fis_dma_setup_t;

typedef struct fis_pio_setup {
  // DWORD 0
  fis_type_t fis_type;          // FIS_TYPE_PIO_SETUP
  uint8_t port_multiplier : 4;  // Port multiplier
  uint8_t reserved_0 : 1;       // Reserved
  uint8_t transfer_dir : 1;     // Data transfer direction, 1 - device to host
  uint8_t interrupt : 1;        // Interrupt bit
  uint8_t reserved_1 : 1;
  uint8_t status;          // Status register
  uint8_t error;           // Error register
  uint8_t lba0;            // LBA low register, 7:0
  uint8_t lba1;            // LBA mid register, 15:8
  uint8_t lba2;            // LBA high register, 23:16
  uint8_t device;          // Device register
  uint8_t lba3;          // LBA register, 31:24
  uint8_t lba4;          // LBA register, 39:32
  uint8_t lba5;          // LBA register, 47:40
  uint8_t reserved_2;    // Reserved
  uint16_t count;        // Count register, 7:0
  uint8_t reserved_3;    // Reserved
  uint8_t e_status;      // New value of status register
  uint16_t transfer_count; // Transfer count
  uint8_t reserved_4[2];   // Reserved
} fis_pio_setup_t;

typedef struct fis_dev_bits {
  uint8_t fis_type; // FIS Type
  uint8_t pm_port : 4; // Port multiplier port
  uint8_t reserved_0 : 2;
  uint8_t interrupt : 1; // Interrupt bit
  uint8_t notification : 1; // Notification
  uint8_t status; // Status register
  uint8_t error; // Error register
  uint32_t protocol_specific; // Protocol specific
} fis_dev_bits_t;

typedef struct fis_reg_d2h {
  uint8_t fis_type; // FIS Type
  uint8_t pm_port : 4; // Port multiplier port
  uint8_t reserved_0 : 3;
  uint8_t interrupt : 1; // Interrupt bit
  uint8_t status; // Status register
  uint8_t error; // Error register
  // LBA low register, LBA mid register, LBA high register
  uint8_t lba0;
  uint8_t lba1;
  uint8_t lba2;

  // Device register
  uint8_t device;

  // LBA register, 32 bits
  uint8_t lba3;
  uint8_t lba4;
  uint8_t lba5;

  // Reserved
  uint8_t reserved1;

  // Count register
  uint16_t count;

  // Reserved
  uint8_t reserved_2[6];
} fis_reg_d2h_t;

typedef struct fis_reg_h2d {
  uint8_t fis_type; // FIS_TYPE_REG_H2D
  uint8_t pm_port : 4; // Port multiplier
  uint8_t reserved_0 : 3; // Reserved
  uint8_t command_select : 1; // 1: Command, 0: Control
  uint8_t command;	// Command register
  uint8_t feature_low;	// Feature register, 7:0
  uint8_t lba0; // LBA low register, 7:0
  uint8_t lba1; // LBA mid register, 15:8
  uint8_t lba2; // LBA high register, 23:16
  uint8_t device; // Device register
  uint8_t lba3; // LBA register, 31:24
  uint8_t lba4; // LBA register, 39:32
  uint8_t lba5; // LBA register, 47:40
  uint8_t feature_high; // Feature register, 15:8
  uint16_t count; // Count register, 7:0
  uint8_t icc; // Isochronous command completion
  uint8_t control; // Control register
  uint8_t reserved_1[4]; // Reserved
} fis_reg_h2d;

typedef struct identify_device_data {
  uint16_t general_config;
  uint16_t reserved_1[9];
  char serial_number[20];   // Words 10-19
  uint16_t reserved_2[3];
  char firmware_revision[8]; // Words 23-26
  char model_number[40];     // Words 27-46
  uint16_t reserved_3[2];
  uint16_t capabilities;
  uint16_t reserved_4[2];
  uint16_t valid_ext_data;
  uint16_t reserved_5[5];
  uint16_t size_of_rw_multiple;
  uint16_t reserved_6[1];
  uint32_t lba28_sector_count; // Words 60-61 (combining as a 32-bit value for convenience)
  uint16_t reserved_7[38];
  uint16_t lba48_support;      // Word 83
  uint16_t reserved_8[23];
  uint64_t lba48_sector_count; // Words 100-103 (combining as a 64-bit value for convenience)
  uint16_t reserved_9[128];
} __attribute__((packed)) identify_device_data_t;

typedef volatile struct hba_fis {
  fis_dma_setup_t dma_setup_fis; // DMA Setup FIS
  uint8_t pad_0[4];
  fis_pio_setup_t pio_setup_fis; // PIO Setup FIS
  uint8_t pad_1[12];
  fis_reg_d2h_t register_fis; // Register – Device to Host FIS
  uint8_t pad_2[4];
  fis_dev_bits_t sdb_fis; // Set Device Bit FIS
  uint8_t u_fis[64];
  uint8_t reserved_0[0x100-0xA0];
} hba_fis_t;

typedef struct hba_prdt_entry {
  uint64_t dba; // Data base address
  uint32_t reserved_0; // Reserved
  uint32_t dbc : 22; // Byte count, 4M max
  uint32_t reserved_1 : 9; // Reserved
  uint32_t interrupt : 1;  // Interrupt on completion
} hba_prdt_entry_t;

typedef struct hba_cmd_table {
  uint8_t command_fis[64]; // Command FIS
  uint8_t atapi_command[16]; // ATAPI command, 12 or 16 bytes
  uint8_t reserved_0[48]; // Reserved
  hba_prdt_entry_t prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
} hba_cmd_table_t;

void init_ahci();

#endif  //GLADOS_ATA_H
