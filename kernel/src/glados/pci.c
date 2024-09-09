//
// Created by irrl on 9/7/24.
//

#include "glados/pci.h"
#include "glados/port.h"
#include "glados/stdio.h"

// maybe use binary tree instead?
typedef struct entry_node {
  pci_device_t device;
  struct entry_node* next;
} entry_node_t;

static uint32_t device_counter_;
static entry_node_t* devices_head_;

static uint16_t get_vendor_id(uint8_t bus, uint8_t device, uint8_t function) {
  return pci_config_read(bus, device, function, 0x0) & 0xFFFF;
}

static pci_subclass_t get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function) {
  return pci_config_read(bus, device, function, 0x0A) & 0xFF;
}

static pci_device_common_t* get_pci_device_common(uint8_t bus, uint8_t device, uint8_t function) {
  pci_device_common_t* common = kmalloc(sizeof(pci_device_common_t));
  for (uint8_t i = 0; i < 4; i++) {
    uint32_t reg = pci_config_read(bus, device, function, i * 0x4);
    ((uint32_t*)common)[i] = reg;
  }
  return common;
}

static void handle_device(uint8_t bus_id, uint8_t device_id, uint8_t function_id, pci_device_common_t* common) {
  entry_node_t* entry = kmalloc(sizeof(entry_node_t));
  if (devices_head_ != NULL) {
    entry->next = devices_head_;
  }
  entry->device.bus_id = bus_id;
  entry->device.device_id = device_id;
  entry->device.function_id = function_id;
  entry->device.common = common;
  entry->device.specific = NULL;
  devices_head_ = entry;
}

static void check_function(uint8_t bus, uint8_t device, uint8_t function) {
  uint16_t vendor_id = get_vendor_id(bus, device, function);
  if (vendor_id == 0xFFFF) {
    return; // Device doesn't exist
  }

  pci_device_common_t* common = get_pci_device_common(bus, device, function);
  handle_device(bus, device, function, common);
}

static void enumerate_pci() {
  for (uint16_t bus = 0; bus < 256; bus++) {
    for (uint8_t device = 0; device < 32; device++) {
      for (uint8_t function = 0; function < 8; function++) {
        check_function(bus, device, function);
      }
    }
  }
}

uint64_t arch_msi_address(uint64_t* data, size_t vector, uint32_t processor, uint8_t edgetrigger, uint8_t deassert) {
  *data = (vector & 0xFF) | (edgetrigger == 1 ? 0 : (1 << 15)) | (deassert == 1 ? 0 : (1 << 14));
  return (0xFEE00000 | (processor << 12));
}

void pci_config_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
  /* Create configuration address as per PCI Configuration Mechanism #1 */
  uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                (function << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
  write_pci_config(address, value);
}

uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
  /* Create configuration address as per PCI Configuration Mechanism #1 */
  uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                       (function << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
  return read_pci_config(address);
}

void write_pci_config(uint32_t address, uint32_t value) {
  port_dword_out(PCI_CONFIG_ADDRESS, address);
  port_dword_out(PCI_CONFIG_DATA, value);
}

uint32_t read_pci_config(uint32_t address) {
  port_dword_out(PCI_CONFIG_ADDRESS, address);
  return port_dword_in(PCI_CONFIG_DATA);
}

void init_pci() {
  enumerate_pci();
}

static entry_node_t* find_device_entry_by_id(entry_node_t* base, uint8_t bus_id, uint8_t device_id, uint8_t function_id) {
  if (!base) {
    return NULL;
  }
  if (base->device.bus_id == bus_id && base->device.device_id == device_id && base->device.function_id) {
    return base;
  }
  return find_device_entry_by_id(base->next, bus_id, device_id, function_id);
}

pci_device_t* find_pci_device_by_id(uint8_t bus_id, uint8_t device_id, uint8_t function_id) {
  entry_node_t* entry = find_device_entry_by_id(devices_head_, bus_id, device_id, function_id);
  if (entry) {
    return &entry->device;
  }
  return NULL;
}

static void iterate_pci_device_entries(entry_node_t* base, void(*fn)(pci_device_t*)) {
  if (!base) {
    return;
  }
  fn(&base->device);
  iterate_pci_device_entries(base->next, fn);
}

void iterate_pci_devices(void(*fn)(pci_device_t*)) {
  iterate_pci_device_entries(devices_head_, fn);
}

void pci_request_specific(pci_device_t* device) {
  uint32_t size = 0x100 - sizeof(pci_device_t);
  void* specific = kmalloc(size);
  for (uint8_t i = 0; i < size / 0x04; i++) {
    uint32_t reg = pci_config_read(device->bus_id, device->device_id, device->function_id, 0x10 + (i * 0x4));
    ((uint32_t*)specific)[i] = reg;
  }
  device->specific = specific;
}