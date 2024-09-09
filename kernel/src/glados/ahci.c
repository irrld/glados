 #include "glados/ahci.h"
#include "glados/port.h"
#include "glados/stddef.h"
#include "glados/pci.h"
#include "glados/page.h"

int cmd_slots_;

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

  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*) port->clb;
  cmdheader += slot;
  cmdheader->command_fis_length = sizeof(fis_reg_h2d) / sizeof(uint32_t); // Command FIS size
  cmdheader->write = 0; // Read from device
  cmdheader->prdt_len = (uint16_t)((count-1)>>4) + 1; // PRDT entries count

  hba_cmd_table_t* cmdtbl = (hba_cmd_table_t*)(cmdheader->ctd_base_addr);
  memset(cmdtbl, 0, sizeof(hba_cmd_table_t) +
                        (cmdheader->prdt_len - 1) * sizeof(hba_prdt_entry_t));

  // 8K bytes (16 sectors) per PRDT
  int i;
  for (i = 0; i < cmdheader->prdt_len - 1; i++) {
    cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
    cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
    cmdtbl->prdt_entry[i].interrupt = true;
    buf += 4*1024; // 4K words
    count -= 16; // 16 sectors
  }
  // Last entry
  cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
  cmdtbl->prdt_entry[i].dbc = (count << 9) - 1; // 512 bytes per sector
  cmdtbl->prdt_entry[i].interrupt = true;

  // Setup command
  fis_reg_h2d* cmdfis = (fis_reg_h2d*) (&cmdtbl->command_fis);

  cmdfis->fis_type = FIS_TYPE_REG_H2D;
  cmdfis->command_select = true;
  cmdfis->command = ATA_CMD_READ_DMA_EX;

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

  // Command list offset: 1K*portno
  // Command list entry size = 32
  // Command list entry maxim count = 32
  // Command list maxim size = 32*32 = 1K per port
  port->clb = AHCI_BASE + (portno << 10);
  port->clbu = 0;
  memset((void*)(port->clb), 0, 1024);

  // FIS offset: 32K+256*portno
  // FIS entry size = 256 bytes per port
  port->fb = AHCI_BASE + (32 << 10) + (portno<<8);
  port->fbu = 0;
  memset((void*)(port->fb), 0, 256);

  // Command table offset: 40K + 8K*portno
  // Command table size = 256*32 = 8K per port
  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*) (port->clb);
  for (int i = 0; i < 32; i++) {
    cmdheader[i].prdt_len = 8; // 8 prdt entries per command table
                               // 256 bytes per command table, 64+16+48+16*8
    // Command table offset: 40K + 8K*portno + cmdheader_index*256
    cmdheader[i].ctd_base_addr = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
    memset((void*)cmdheader[i].ctd_base_addr, 0, 256);
  }

  start_cmd(port);	// Start command engine
}

static bool send_identify_device(hba_port_t* port, uint16_t* buf) {
  int slot = find_cmdslot(port);
  if (slot == -1) return false;

  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*) port->clb;
  cmdheader += slot;
  cmdheader->command_fis_length = sizeof(fis_reg_h2d) / sizeof(uint32_t);
  cmdheader->write = 0;  // This command does not write to the device
  cmdheader->prdt_len = 1;  // Only one PRDT entry is needed

  hba_cmd_table_t* cmdtbl = (hba_cmd_table_t*)(cmdheader->ctd_base_addr);
  memset(cmdtbl, 0, sizeof(hba_cmd_table_t));

  // Only one PRDT entry is needed to receive the IDENTIFY DEVICE data
  cmdtbl->prdt_entry[0].dba = (uint32_t) buf;
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

static void initiate_sata_drive(hba_mem_t* abar, hba_port_t* port) {

  // todo move all this to its own function
  uint16_t* buffer = (uint16_t*) malloc(512);  // Allocate buffer for the IDENTIFY DEVICE data
  if (buffer == NULL) {
    kprintf("Failed to allocate memory for IDENTIFY DEVICE command\n");
    return;
  }

  int cmd_slot = find_cmdslot(port);
  if (cmd_slot == -1) {
    free(buffer);
    kprintf("No command slots available\n");
    return;
  }

  start_cmd(port);  // Start command engine

  // Perform IDENTIFY DEVICE command
  if (!send_identify_device(port, buffer)) {
    kprintf("Failed to identify device\n");
    stop_cmd(port);  // Stop command engine
    free(buffer);
    return;
  }

  // todo

  stop_cmd(port);  // Stop command engine
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
      initiate_sata_drive(abar, port);
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
  iterate_pci_devices(check_device);
}

