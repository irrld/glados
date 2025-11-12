 #include "glados/ahci.h"
#include "glados/port.h"
#include "glados/stddef.h"
#include "glados/pci.h"
#include "glados/page.h"
#include "glados/stdio.h"
#include "glados/string.h"

int cmd_slots_;

// Global AHCI device storage
#define MAX_AHCI_DEVICES 8
typedef struct {
  hba_port_t* port;
  int port_num;
  uint64_t sector_count;
  bool is_active;
} ahci_device_t;

static ahci_device_t ahci_devices[MAX_AHCI_DEVICES];
static int ahci_device_count = 0;

// Dynamically allocated AHCI memory base
static uintptr_t ahci_mem_base = 0;
#define AHCI_MEM_PAGES 128  // ~512KB for AHCI structures

// Check device type
static int check_type(hba_port_t* port) {
  uint32_t ssts = port->ssts;

  uint8_t ipm = (ssts >> 8) & 0x0F;
  uint8_t det = ssts & 0x0F;

  if (det != HBA_PORT_DET_PRESENT) {  // Check drive status
    return AHCI_DEV_NULL;
  }
  if (ipm != HBA_PORT_IPM_ACTIVE) {
    return AHCI_DEV_NULL;
  }
  switch (port->sig) {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
  }
}

static void probe_port(hba_mem_t* abar) {
  // Search disk in implemented ports
  uint32_t pi = abar->port_implemented;
  int i = 0;
  while (i < 32) {
    if (pi & 1) {
      int dt = check_type(&abar->ports[i]);
      if (dt == AHCI_DEV_SATA) {
        //trace_ahci("SATA drive found at port %d\n", i);
      } else if (dt == AHCI_DEV_SATAPI) {
        //trace_ahci("SATAPI drive found at port %d\n", i);
      } else if (dt == AHCI_DEV_SEMB) {
        //trace_ahci("SEMB drive found at port %d\n", i);
      } else if (dt == AHCI_DEV_PM) {
        //trace_ahci("PM drive found at port %d\n", i);
      } else {
        //trace_ahci("No drive found at port %d\n", i);
      }
    }
    pi >>= 1;
    i ++;
  }
}

// Find a free command list slot
static int find_cmdslot(hba_port_t* port) {
  // If not set in SACT and CI, the slot is free
  uint32_t slots = (port->sact | port->ci);
  for (int i = 0; i < cmd_slots_; i++) {
    if ((slots & 1) == 0) {
      return i;
    }
    slots >>= 1;
  }
  //trace_ahci("Cannot find free command list entry\n");
  return -1;
}

static bool read(hba_port_t* port, uint64_t start, uint16_t count, uint16_t* buf) {
  port->is = (uint32_t) - 1; // Clear pending interrupt bits
  int spin = 0; // Spin lock timeout counter
  int slot = find_cmdslot(port);
  if (slot == -1) {
    return false;
  }

  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)(uintptr_t)((port->clbu << 32) | port->clb);
  cmdheader += slot;
  cmdheader->command_fis_length = sizeof(fis_reg_h2d) / sizeof(uint32_t); // Command FIS size
  cmdheader->write = 0; // Read from device
  cmdheader->prdt_len = (uint16_t)((count-1)>>4) + 1; // PRDT entries count

  hba_cmd_table_t* cmdtbl = (hba_cmd_table_t*)(uintptr_t)(cmdheader->ctd_base_addr);
  memset(cmdtbl, 0, sizeof(hba_cmd_table_t) +
                        (cmdheader->prdt_len - 1) * sizeof(hba_prdt_entry_t));

  // 8K bytes (16 sectors) per PRDT
  int i;
  uintptr_t buf_virt = (uintptr_t)buf;
  for (i = 0; i < cmdheader->prdt_len - 1; i++) {
    uintptr_t phys_addr = get_physical_address(buf_virt);
    if (phys_addr == 0) {
      kprintf("ERROR: Failed to translate buffer address to physical\n");
      return false;
    }
    cmdtbl->prdt_entry[i].dba = phys_addr;
    cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
    cmdtbl->prdt_entry[i].interrupt = true;
    buf_virt += 8*1024; // 8K bytes
    count -= 16; // 16 sectors
  }
  // Last entry
  uintptr_t phys_addr = get_physical_address(buf_virt);
  if (phys_addr == 0) {
    kprintf("ERROR: Failed to translate buffer address to physical\n");
    return false;
  }
  cmdtbl->prdt_entry[i].dba = phys_addr;
  cmdtbl->prdt_entry[i].dbc = (count << 9) - 1; // 512 bytes per sector
  cmdtbl->prdt_entry[i].interrupt = true;

  // Setup command
  fis_reg_h2d* cmdfis = (fis_reg_h2d*) (&cmdtbl->command_fis);

  cmdfis->fis_type = FIS_TYPE_REG_H2D;
  cmdfis->command_select = true;
  cmdfis->command = ATA_CMD_READ_DMA_EXT;

  cmdfis->lba0 = (uint8_t) start;
  cmdfis->lba1 = (uint8_t) (start >> 8);
  cmdfis->lba2 = (uint8_t) (start >> 16);
  cmdfis->lba3 = (uint8_t) (start >> 24);
  cmdfis->lba4 = (uint8_t) (start >> 32);
  cmdfis->lba5 = (uint8_t) (start >> 40);
  cmdfis->device = 1 << 6; // LBA mode

  cmdfis->count = count;

  // The below loop waits until the port is no longer busy before issuing a new command
  while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
    spin++;
  }
  if (spin == 1000000) {
    //trace_ahci("Port is hung\n");
    return false;
  }

  port->ci = 1 << slot;     // Issue command

  // Wait for completion
  while (true) {
    // In some longer duration reads, it may be helpful to spin on the DPS bit
    // in the PxIS port field as well (1 << 5)
    if ((port->ci & (1 << slot)) == 0) {
      break;
    }
    if (port->is & HBA_PxIS_TFES) { // Task file error
      //trace_ahci("Read disk error\n");
      return false;
    }
  }

  // Check again
  if (port->is & HBA_PxIS_TFES) {
    //trace_ahci("Read disk error\n");
    return false;
  }

  return true;
}

static bool write(hba_port_t* port, uint64_t start, uint16_t count, uint16_t* buf) {
  port->is = (uint32_t) - 1; // Clear pending interrupt bits
  int spin = 0;
  int slot = find_cmdslot(port);
  if (slot == -1) {
    return false;
  }

  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)(uintptr_t)((port->clbu << 32) | port->clb);
  cmdheader += slot;
  cmdheader->command_fis_length = sizeof(fis_reg_h2d) / sizeof(uint32_t);
  cmdheader->write = 1; // Write to device
  cmdheader->prdt_len = (uint16_t)((count-1)>>4) + 1;

  hba_cmd_table_t* cmdtbl = (hba_cmd_table_t*)(uintptr_t)(cmdheader->ctd_base_addr);
  memset(cmdtbl, 0, sizeof(hba_cmd_table_t) +
                        (cmdheader->prdt_len - 1) * sizeof(hba_prdt_entry_t));

  // 8K bytes (16 sectors) per PRDT
  int i;
  uintptr_t buf_virt = (uintptr_t)buf;
  for (i = 0; i < cmdheader->prdt_len - 1; i++) {
    uintptr_t phys_addr = get_physical_address(buf_virt);
    if (phys_addr == 0) {
      kprintf("ERROR: Failed to translate buffer address to physical\n");
      return false;
    }
    cmdtbl->prdt_entry[i].dba = phys_addr;
    cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1;
    cmdtbl->prdt_entry[i].interrupt = true;
    buf_virt += 8*1024;
    count -= 16;
  }
  // Last entry
  uintptr_t phys_addr = get_physical_address(buf_virt);
  if (phys_addr == 0) {
    kprintf("ERROR: Failed to translate buffer address to physical\n");
    return false;
  }
  cmdtbl->prdt_entry[i].dba = phys_addr;
  cmdtbl->prdt_entry[i].dbc = (count << 9) - 1;
  cmdtbl->prdt_entry[i].interrupt = true;

  // Setup command
  fis_reg_h2d* cmdfis = (fis_reg_h2d*) (&cmdtbl->command_fis);

  cmdfis->fis_type = FIS_TYPE_REG_H2D;
  cmdfis->command_select = true;
  cmdfis->command = ATA_CMD_WRITE_DMA_EXT;

  cmdfis->lba0 = (uint8_t) start;
  cmdfis->lba1 = (uint8_t) (start >> 8);
  cmdfis->lba2 = (uint8_t) (start >> 16);
  cmdfis->lba3 = (uint8_t) (start >> 24);
  cmdfis->lba4 = (uint8_t) (start >> 32);
  cmdfis->lba5 = (uint8_t) (start >> 40);
  cmdfis->device = 1 << 6; // LBA mode

  cmdfis->count = count;

  // Wait until port is no longer busy
  while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
    spin++;
  }
  if (spin == 1000000) {
    return false;
  }

  port->ci = 1 << slot;

  // Wait for completion
  while (true) {
    if ((port->ci & (1 << slot)) == 0) {
      break;
    }
    if (port->is & HBA_PxIS_TFES) {
      return false;
    }
  }

  if (port->is & HBA_PxIS_TFES) {
    return false;
  }

  return true;
}

// Start command engine
static void start_cmd(hba_port_t* port) {
  // Wait until CR (bit15) is cleared
  while (port->cmd & HBA_PxCMD_CR);

  // Set FRE (bit4) and ST (bit0)
  port->cmd |= HBA_PxCMD_FRE;
  port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
static void stop_cmd(hba_port_t* port) {
  // Clear ST (bit0)
  port->cmd &= ~HBA_PxCMD_ST;

  // Clear FRE (bit4)
  port->cmd &= ~HBA_PxCMD_FRE;

  // Wait until FR (bit14), CR (bit15) are cleared
  while (true) {
    if (port->cmd & HBA_PxCMD_FR) {
      continue;
    }
    if (port->cmd & HBA_PxCMD_CR) {
      continue;
    }
    break;
  }
}

void port_rebase(hba_port_t* port, int portno) {
  stop_cmd(port); // Stop command engine

  if (ahci_mem_base == 0) {
    kprintf("ERROR: AHCI memory not initialized!\n");
    return;
  }

  // Command list offset: 1K*portno
  // Command list entry size = 32
  // Command list entry maxim count = 32
  // Command list maxim size = 32*32 = 1K per port
  uintptr_t clb_virt = ahci_mem_base + (portno << 10);
  port->clb = (uint32_t)clb_virt;
  port->clbu = (uint32_t)(clb_virt >> 32);
  memset((void*)clb_virt, 0, 1024);

  // FIS offset: 32K+256*portno
  // FIS entry size = 256 bytes per port
  uintptr_t fb_virt = ahci_mem_base + (32 << 10) + (portno<<8);
  port->fb = (uint32_t)fb_virt;
  port->fbu = (uint32_t)(fb_virt >> 32);
  memset((void*)fb_virt, 0, 256);

  // Command table offset: 40K + 8K*portno
  // Command table size = 256*32 = 8K per port
  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)clb_virt;
  for (int i = 0; i < 32; i++) {
    cmdheader[i].prdt_len = 8; // 8 prdt entries per command table
                               // 256 bytes per command table, 64+16+48+16*8
    // Command table offset: 40K + 8K*portno + cmdheader_index*256
    uintptr_t ctd_virt = ahci_mem_base + (40<<10) + (portno<<13) + (i<<8);
    cmdheader[i].ctd_base_addr = ctd_virt;
    memset((void*)ctd_virt, 0, 256);
  }

  start_cmd(port);	// Start command engine
}

static bool send_identify_device(hba_port_t* port, uint16_t* buf) {
  int slot = find_cmdslot(port);
  if (slot == -1) return false;

  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)(uintptr_t)((port->clbu << 32) | port->clb);
  cmdheader += slot;
  cmdheader->command_fis_length = sizeof(fis_reg_h2d) / sizeof(uint32_t);
  cmdheader->write = 0;  // This command does not write to the device
  cmdheader->prdt_len = 1;  // Only one PRDT entry is needed

  hba_cmd_table_t* cmdtbl = (hba_cmd_table_t*)(uintptr_t)(cmdheader->ctd_base_addr);
  memset(cmdtbl, 0, sizeof(hba_cmd_table_t));

  // Only one PRDT entry is needed to receive the IDENTIFY DEVICE data
  uintptr_t phys_addr = get_physical_address((uintptr_t)buf);
  if (phys_addr == 0) {
    kprintf("ERROR: Failed to translate identify buffer to physical address\n");
    return false;
  }
  cmdtbl->prdt_entry[0].dba = phys_addr;
  cmdtbl->prdt_entry[0].dbc = 511;  // 512 bytes (minus 1 as per spec), one sector
  cmdtbl->prdt_entry[0].interrupt = 1;

  fis_reg_h2d* cmdfis = (fis_reg_h2d*)(&cmdtbl->command_fis);
  cmdfis->fis_type = FIS_TYPE_REG_H2D;
  cmdfis->command = ATA_CMD_IDENTIFY;
  cmdfis->device = 0;  // LBA mode (not used here)

  // Issue the command
  port->ci = 1 << slot;

  // Wait for completion
  while (port->ci & (1 << slot)) {
    if (port->is & HBA_PxIS_TFES) {  // Check for errors
      return false;
    }
  }
  return true;
}

static void initiate_sata_drive(hba_mem_t* abar, hba_port_t* port, int port_num) {
  uint16_t* buffer = (uint16_t*) kmalloc(512);
  if (buffer == NULL) {
    kprintf("Failed to allocate memory for IDENTIFY DEVICE command\n");
    return;
  }

  int cmd_slot = find_cmdslot(port);
  if (cmd_slot == -1) {
    kfree(buffer);
    kprintf("No command slots available\n");
    return;
  }

  // Perform IDENTIFY DEVICE command
  if (!send_identify_device(port, buffer)) {
    kprintf("Failed to identify device on port %d\n", port_num);
    kfree(buffer);
    return;
  }

  // Parse IDENTIFY data
  identify_device_data_t* identify = (identify_device_data_t*) buffer;
  uint64_t sector_count = 0;

  // Check for LBA48 support
  if (identify->lba48_support & (1 << 10)) {
    sector_count = identify->lba48_sector_count;
    kprintf("LBA48 supported, sector count: %llu\n", sector_count);
  } else {
    sector_count = identify->lba28_sector_count;
    kprintf("LBA28, sector count: %u\n", (uint32_t)sector_count);
  }

  // Store device info
  if (ahci_device_count < MAX_AHCI_DEVICES) {
    ahci_devices[ahci_device_count].port = port;
    ahci_devices[ahci_device_count].port_num = port_num;
    ahci_devices[ahci_device_count].sector_count = sector_count;
    ahci_devices[ahci_device_count].is_active = true;

    kprintf("Registered AHCI device %d: port %d, %llu sectors (%llu MB)\n",
            ahci_device_count, port_num, sector_count, (sector_count * 512) / (1024 * 1024));
    ahci_device_count++;
  } else {
    kprintf("Maximum AHCI devices reached\n");
  }

  kfree(buffer);
}

static void check_ports(hba_mem_t* abar) {
  uint32_t pi = abar->port_implemented;
  for (int i = 0; i < 32; i++) {
    if (!(pi & (1 << i))) {
      continue;  // Check if the port is implemented
    }
    hba_port_t* port = &abar->ports[i];
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;  // Interface Power Management
    uint8_t det = ssts & 0x0F;         // Device Detection

    if (det == 3 && ipm == 1) {  // Device is present and interface is active
      kprintf("SATA drive detected at port %d\n", i);

      // Rebase the port to set up command structures
      port_rebase(port, i);

      // Start the port command engine
      start_cmd(port);

      // Initialize the drive and register it
      initiate_sata_drive(abar, port, i);
    }
  }
}

static void initiate_ahci_device(hba_mem_t* abar) {
  cmd_slots_ = ((abar->cap & 0x1F00) >> 8) + 1;
  kprintf("Command Slots: %d\n", cmd_slots_);
  check_ports(abar);
}

static void check_device(pci_device_t* device) {
  if (device->common->baseclass == PCI_CLASS_MASS_STORAGE_CONTROLLER &&
      device->common->subclass == 0x06) {
    kprintf("AHCI Device found: Bus %d, Device %d, Func %d, Vendor ID: 0x%X, Device ID: 0x%X\n",
            device->bus_id, device->device_id, device->function_id, device->common->vendor_id, device->common->device_id);
    pci_request_specific(device);
    pci_device_0x00_specific_t* specific = device->specific;
    hba_mem_t* abar = (hba_mem_t*) specific->bar[5];
    // Writing 0xFFFFFFFF results in writing a mask of the size.
    pci_config_write(device->bus_id, device->device_id, device->function_id, 0x24, 0xFFFFFFFF);
    uint32_t size_mask = pci_config_read(device->bus_id, device->device_id, device->function_id, 0x24);
    pci_config_write(device->bus_id, device->device_id, device->function_id, 0x24, specific->bar[5]);
    // We revert the bits and add 1 to it
    uint32_t size = (~size_mask) + 1;
    // Divide and round the page size to higher.t
    uint16_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    identity_map(abar, num_pages);
    initiate_ahci_device(abar);
  }
}

void init_ahci() {
  // Initialize device array
  for (int i = 0; i < MAX_AHCI_DEVICES; i++) {
    ahci_devices[i].is_active = false;
  }
  ahci_device_count = 0;

  // Allocate and identity-map memory for AHCI command structures
  // This ensures virtual address == physical address for DMA
  kprintf("Allocating AHCI memory (%d pages)...\n", AHCI_MEM_PAGES);

  // Allocate first physical page
  ahci_mem_base = alloc_page();
  if (ahci_mem_base == 0) {
    kprintf("ERROR: Failed to allocate AHCI memory base\n");
    return;
  }

  // Identity map first page
  identity_map(ahci_mem_base, 1);

  // Allocate and identity map remaining contiguous pages
  for (int i = 1; i < AHCI_MEM_PAGES; i++) {
    uintptr_t phys_page = alloc_page();
    if (phys_page == 0) {
      kprintf("ERROR: Failed to allocate AHCI page %d\n", i);
      return;
    }
    // Verify pages are contiguous (alloc_page should return contiguous pages)
    uintptr_t expected_addr = ahci_mem_base + (i * PAGE_SIZE);
    if (phys_page != expected_addr) {
      kprintf("WARNING: AHCI pages not contiguous (expected 0x%llx, got 0x%llx)\n",
              expected_addr, phys_page);
    }
    identity_map(phys_page, 1);
  }

  // Clear the allocated memory
  memset((void*)ahci_mem_base, 0, AHCI_MEM_PAGES * PAGE_SIZE);
  kprintf("AHCI memory base: 0x%llx\n", ahci_mem_base);

  // Scan PCI bus for AHCI controllers
  iterate_pci_devices(check_device);

  if (ahci_device_count == 0) {
    kprintf("No AHCI devices found\n");
  } else {
    kprintf("Found %d AHCI device(s)\n", ahci_device_count);
  }
}

// Public API functions

int ahci_get_device_count() {
  return ahci_device_count;
}

bool ahci_read_sectors(int device_id, uint64_t start_sector, uint16_t sector_count, void* buffer) {
  if (device_id < 0 || device_id >= ahci_device_count) {
    kprintf("Invalid AHCI device ID: %d\n", device_id);
    return false;
  }

  if (!ahci_devices[device_id].is_active) {
    kprintf("AHCI device %d is not active\n", device_id);
    return false;
  }

  if (start_sector + sector_count > ahci_devices[device_id].sector_count) {
    kprintf("Read beyond device capacity\n");
    return false;
  }

  return read(ahci_devices[device_id].port, start_sector, sector_count, (uint16_t*)buffer);
}

bool ahci_write_sectors(int device_id, uint64_t start_sector, uint16_t sector_count, void* buffer) {
  if (device_id < 0 || device_id >= ahci_device_count) {
    kprintf("Invalid AHCI device ID: %d\n", device_id);
    return false;
  }

  if (!ahci_devices[device_id].is_active) {
    kprintf("AHCI device %d is not active\n", device_id);
    return false;
  }

  if (start_sector + sector_count > ahci_devices[device_id].sector_count) {
    kprintf("Write beyond device capacity\n");
    return false;
  }

  return write(ahci_devices[device_id].port, start_sector, sector_count, (uint16_t*)buffer);
}

uint64_t ahci_get_sector_count(int device_id) {
  if (device_id < 0 || device_id >= ahci_device_count) {
    return 0;
  }

  if (!ahci_devices[device_id].is_active) {
    return 0;
  }

  return ahci_devices[device_id].sector_count;
}

