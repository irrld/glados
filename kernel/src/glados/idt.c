//
// Created by irrl on 9/4/24.
//

#include "glados/idt.h"
#include "glados/kernel.h"
#include "glados/string.h"
#include "glados/kdef.h"

// IDT entry structure
typedef struct idt_entry {
  uint16_t offset_1;
  uint16_t segment_selector;
  uint8_t  ist;
  uint8_t  flags;
  uint16_t offset_2;
  uint32_t offset_3;
  uint32_t zero;
} __attribute__((packed)) idt_entry_t;

// IDT pointer structure
typedef struct idt_ptr {
  uint16_t limit; // size
  uint64_t base; // offset
} __attribute__((packed)) idt_ptr_t;

extern void load_idt();

extern void isr_13();
extern void isr_14();
extern void isr_32();
extern void isr_33();
extern void isr_40();

idt_entry_t idt_[IDT_ENTRIES];  // Create an IDT with 256 entries
idt_ptr_t idtp_;

void idt_set_entry(uint8_t irq, uint64_t base, uint16_t segment_selector, uint8_t ist) {
  idt_[irq].offset_1 = base & 0xFFFF;
  idt_[irq].offset_2 = (base >> 16) & 0xFFFF;
  idt_[irq].offset_3 = (base >> 32) & 0xFFFFFFFF;

  idt_[irq].segment_selector = segment_selector;
  idt_[irq].ist = ist;
  idt_[irq].flags = 0x8E;
  idt_[irq].zero = 0;
}

void init_idt() {
  kprintf("Initializing IDT\n");
  idtp_.limit = sizeof(idt_) - 1;
  idtp_.base = (uint64_t)idt_;
  memset(&idt_, 0, idtp_.limit + 1);

  idt_set_entry(INTERRUPT_GENERAL_PROTECTION_FAULT, (uint64_t)isr_13, KERNEL_CODE_SELECTOR, 1);
  idt_set_entry(INTERRUPT_PAGE_FAULT, (uint64_t)isr_14, KERNEL_CODE_SELECTOR, 1);
  idt_set_entry(INTERRUPT_TIMER, (uint64_t)isr_32, KERNEL_CODE_SELECTOR, 1);
  idt_set_entry(INTERRUPT_KEYBOARD, (uint64_t)isr_33, KERNEL_CODE_SELECTOR, 1);
  idt_set_entry(INTERRUPT_RTC, (uint64_t)isr_40, KERNEL_CODE_SELECTOR, 1);

  load_idt();
}

void irq_set_mask(uint8_t irq_line) {
  uint16_t port;
  uint8_t value;

  if(irq_line >= 8) {
    port = PIC2_DATA;
    irq_line -= 8;
  } else {
    port = PIC1_DATA;
  }
  value = port_byte_in(port) | (1 << irq_line);
  port_byte_out(port, value);
}

void irq_clear_mask(uint8_t irq_line) {
  uint16_t port;
  uint8_t value;

  if(irq_line >= 8) {
    port = PIC2_DATA;
    irq_line -= 8;
  } else {
    port = PIC1_DATA;
  }
  value = port_byte_in(port) & ~(1 << irq_line);
  port_byte_out(port, value);
}

void send_eoi(uint8_t irq) {
  if (irq >= 8) {
    port_byte_out(PIC2_COMMAND, PIC_EOI);  // Tell PIC2 that IRQ has been handled
  }
  port_byte_out(PIC1_COMMAND, PIC_EOI);      // Tell PIC1 that IRQ has been handled
}
