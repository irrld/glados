//
// Created by root on 9/4/24.
//

#include "glados/gdt.h"
#include "glados/string.h"
#include "glados/tss.h"

typedef struct gdt_entry {
  uint16_t limit_low;
  uint16_t base1;
  uint8_t  base2;
  uint8_t  access;
  uint8_t  limit_high : 4;
  uint8_t  flags : 4;
  uint8_t  base3;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_ptr {
  uint16_t limit; // size
  uint64_t base; // offset
} __attribute__((packed)) gdt_ptr_t;

extern void read_gdt_ptr(gdt_ptr_t* ptr);
extern void load_gdt();

gdt_entry_t gdt_[5];
gdt_ptr_t gdtp_;

void init_gdt() {
  // Read the old one
  gdt_ptr_t old_ptr;
  read_gdt_ptr(&old_ptr);
  // Setup the descriptor for the new one
  gdtp_.limit = sizeof(gdt_) - 1;
  gdtp_.base = (uint64_t)gdt_;

  // Copy old values to the new values
  memset(gdt_, 0, sizeof(gdt_));
  memcpy((uintptr_t*) gdtp_.base, (uintptr_t*) old_ptr.base, old_ptr.limit + 1);

  uint64_t tss_base = (uint64_t)get_tss();
  uint64_t tss_limit = sizeof(tss_entry_t) - 1;

  // Fill the rest
  // TSS descriptor
  gdt_[3].base1 = (uint16_t)(tss_base & 0xFFFF);
  gdt_[3].base2 = (uint8_t)((tss_base >> 16) & 0xFF);
  gdt_[3].base3 = (uint8_t)((tss_base >> 24) & 0xFF);
  //gdt_[3].base4 = (uint16_t)((uint64_t)&tss_ >> 32);
  gdt_[3].limit_low = (uint16_t)(tss_limit & 0xFFFF);
  gdt_[3].limit_high = (uint8_t)((tss_limit >> 16) & 0x0F);
  gdt_[3].flags = 0x0;//0b0000;
  gdt_[3].access = 0x89;//0b10001001;

  // Upper 4 bytes of the 5th TSS descriptor
  uint32_t base4 = (uint32_t)((tss_base >> 32) & 0xFFFFFFFF);
  memcpy(&gdt_[4], &base4, sizeof(uint32_t));

  // Update the GDT to the new location
  load_gdt();
}
