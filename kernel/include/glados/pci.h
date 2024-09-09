//
// Created by irrl on 9/6/24.
//

#ifndef GLADOS_PCI_H
#define GLADOS_PCI_H

#include "glados/stddef.h"

#define PCI_CMD_MEMORY_SPACE  (1 << 1)
#define PCI_CMD_BUS_MASTER    (1 << 2)
#define PCI_CMD_INTERRUPT     (1 << 10)

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

enum pci_baseclass {
  PCI_CLASS_UNCLASSIFIED = 0x0,
  PCI_CLASS_MASS_STORAGE_CONTROLLER = 0x1,
  PCI_CLASS_NETWORK_CONTROLLER = 0x2,
  PCI_CLASS_DISPLAY_CONTROLLER = 0x3,
  PCI_CLASS_MULTIMEDIA_CONTROLLER = 0x4,
  PCI_CLASS_MEMORY_CONTROLLER = 0x5,
  PCI_CLASS_BRIDGE = 0x6,
  PCI_CLASS_SIMPLE_COM_CONTROLLER = 0x7,
  PCI_CLASS_BASE_SYS_PERIPHERAL = 0x8,
  PCI_CLASS_INPUT_DEVICE_CONTROLLER = 0x9,
  PCI_CLASS_DOCKING_STATION = 0xA,
  PCI_CLASS_PROCESSOR = 0xB,
  PCI_CLASS_SERIAL_BUS_CONTROLLER = 0xC,
  PCI_CLASS_WIRELESS_CONTROLLER = 0xD,
  PCI_CLASS_INTELLIGENT_CONTROLLER = 0xE,
  PCI_CLASS_SATELLITE_COM_CONTROLLER = 0xF,
  PCI_CLASS_ENCRYPTION_CONTROLLER = 0x10,
  PCI_CLASS_SIGNAL_PROCESSING_CONTROLLER = 0x11,
  PCI_CLASS_PROCESSING_ACCELERATOR = 0x12,
  PCI_CLASS_NON_ESSENTIAL_INSTRUMENTATION = 0x13,
  PCI_CLASS_CO_PROCESSOR = 0x40,
  PCI_CLASS_UNASSIGNED_CLASS = 0xFF,
};

typedef uint8_t pci_baseclass_t;
typedef uint8_t pci_subclass_t;

typedef struct pci_device_common {
  uint16_t vendor_id;
  uint16_t device_id;
  uint16_t command;
  uint16_t status;
  uint8_t revision_id;
  uint8_t prog_id;
  pci_subclass_t subclass;
  pci_baseclass_t baseclass;
  uint8_t cache_line_size;
  uint8_t latency_timer;
  uint8_t header_type;
  uint8_t bist;
} __attribute__((packed)) pci_device_common_t;

// Generic device
typedef struct pci_device_0x00_specific {
  uint32_t bar[6];
  uint32_t cardbus_cis_ptr;
  uint16_t subsystem_vendor_id;
  uint16_t subsystem_id;
  uint32_t expansion_rom_base_addr;
  uint8_t capabilities_ptr;
  uint8_t reserved_0[7];
  uint8_t interrupt_line;
  uint8_t interrupt_pin;
  uint8_t min_grant;
  uint8_t max_latency;
} __attribute__((packed)) pci_device_0x00_specific_t;

// PCI-to-PCI bridge
typedef struct pci_device_0x01_specific {
  uint32_t bar[2];
  uint8_t primary_bus_num;
  uint8_t secondary_bus_number;
  uint8_t subordinate_bus_number;
  uint8_t secondary_latency_timer;
  uint8_t io_base;
  uint8_t io_limit;
  uint16_t secondary_status;
  uint16_t memory_base;
  uint16_t memory_limit;
  uint16_t prefetchable_memory_base;
  uint16_t prefetchable_memory_limit;
  uint32_t prefetchable_base_upper;
  uint32_t prefetchable_limit_upper;
  uint16_t io_base_upper;
  uint16_t io_limit_upper;
  uint8_t capability_ptr;
  uint8_t reserved_0[3];
  uint32_t expansion_rom_base_address;
  uint8_t interrupt_line;
  uint8_t interrupt_pin;
  uint16_t bridge_control;
} __attribute__((packed)) pci_device_0x01_specific_t;

// Todo PCI-to-CardBus bridge struct

typedef struct pci_device {
  uint8_t bus_id;
  uint8_t device_id;
  uint8_t function_id;
  pci_device_common_t* common;
  void* specific;
} pci_device_t;

void init_pci();
pci_device_t* find_pci_device_by_id(uint8_t bus_id, uint8_t device_id, uint8_t function_id);

void iterate_pci_devices(void(*fn)(pci_device_t*));

void pci_request_specific(pci_device_t* device);

uint64_t arch_msi_address(uint64_t* data, size_t vector, uint32_t processor, uint8_t edgetrigger, uint8_t deassert);
void pci_config_write(uint8_t bus_id, uint8_t device_id, uint8_t function_id, uint8_t offset, uint32_t value);
uint32_t pci_config_read(uint8_t bus_id, uint8_t device_id, uint8_t function_id, uint8_t offset);
void write_pci_config(uint32_t address, uint32_t value);
uint32_t read_pci_config(uint32_t address);

#endif  //GLADOS_PCI_H
