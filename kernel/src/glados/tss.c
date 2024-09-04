//
// Created by root on 9/5/24.
//

#include "glados/tss.h"
#include "glados/kernel.h"

tss_entry_t tss_;

extern void load_tss(uint16_t selector);

void init_tss() {
  kprintf("Initializing TSS\n");
  memset(&tss_, 0, sizeof(tss_));
  tss_.ist1 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  tss_.rsp0 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  tss_.rsp1 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  tss_.rsp2 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  load_tss(TSS_SELECTOR);
}

tss_entry_t* get_tss() {
  return &tss_;
}