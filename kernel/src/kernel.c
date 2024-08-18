
#include "kernel.h"
#include "string.h"
#include "video.h"

extern void idt_load();
extern void isr_33();

// IDT entry structure
struct idt_entry {
  uint16_t offset_1;
  uint8_t  rpl : 2;
  uint8_t  ti : 1;
  uint16_t  segment_index : 13;
  uint8_t  ist : 3;
  uint8_t  reserved1 : 5;
  uint8_t  gate_type : 4;
  uint8_t  zero : 1;
  uint8_t  dpl : 2;
  uint8_t  p : 1;
  uint16_t offset_2;
  uint32_t offset_3;
  uint32_t reserved2;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256

struct idt_entry idt[IDT_ENTRIES];  // Create an IDT with 256 entries
struct idt_ptr idtp;
isr_t interrupt_handlers[IDT_ENTRIES];

void idt_set_entry(int num, uint64_t base, uint16_t segment) {
  idt[num].offset_1 = base & 0xFFFF;
  idt[num].offset_2 = (base >> 16) & 0xFFFF;
  idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;

  idt[num].segment_index = segment;
  idt[num].rpl = 0;
  idt[num].ti = 0;
  idt[num].ist = 0;
  idt[num].dpl = 0;
  idt[num].p = 1;
  idt[num].gate_type = 0xE; // Interrupt Gate

  idt[num].reserved1 = 0;
  idt[num].reserved2 = 0;
  idt[num].zero = 0;
}

void send_eoi(uint8_t irq) {
  if (irq >= 8) {
    port_byte_out(PIC2_COMMAND, 0x20);  // Tell PIC2 that IRQ has been handled
  }
  port_byte_out(PIC1_COMMAND, 0x20);      // Tell PIC1 that IRQ has been handled
}

void isr_common_stub(uint8_t irq) {
  if (interrupt_handlers[irq]) {
    interrupt_handlers[irq]();
    send_eoi(irq);
  }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
  interrupt_handlers[n] = handler;
}

void idt_init() {
  memset(&interrupt_handlers, NULL, IDT_ENTRIES);

  idtp.limit = sizeof(idt) - 1;
  idtp.base = (uint64_t)idt;
  memset(&idt, 0, idtp.limit + 1);

  idt_set_entry(33, (uint64_t)isr_33, 1);

  idt_load();
}

unsigned char port_byte_in(unsigned short port) {
  // A handy C wrapper function that reads a byte from the specified port
  // "=a" (result) means: put AL register in variable RESULT when finished
  // "d" (port) means: load EDX with port
  unsigned char result;
  __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
  return result;
}

void port_byte_out(unsigned short port, unsigned char data) {
  // "a" (data) means: load EAX with data
  // "d" (port) means: load EDX with port
  __asm__("out %%al, %%dx" : :"a" (data), "d" (port));
}

unsigned short port_word_in(unsigned short port) {
  unsigned short result;
  __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
  return result;
}

void port_word_out(unsigned short port, unsigned short data) {
  __asm__("out %%ax, %%dx" : :"a" (data), "d" (port));
}

void _start_kernel() {
  driver_init_video();
  set_color(FOREGROUND_LIGHT_BLUE | BACKGROUND_BLACK);
  println("Hello from C!");
  disable_cursor();

  idt_init();

  // Initialize PIC1
  port_byte_out(PIC1_COMMAND, 0x11);   // Initialization command
  port_byte_out(PIC1_DATA, 0x20);      // Vector offset for PIC1
  port_byte_out(PIC1_DATA, 0x04);      // Configure cascading
  port_byte_out(PIC1_DATA, 0x01);      // 8086/88 mode

  // Initialize PIC2
  port_byte_out(PIC2_COMMAND, 0x11);   // Initialization command
  port_byte_out(PIC2_DATA, 0x28);      // Vector offset for PIC2
  port_byte_out(PIC2_DATA, 0x02);      // Cascade PIC2 to PIC1
  port_byte_out(PIC2_DATA, 0x01);      // 8086/88 mode

  // Enable interrupts
  port_byte_out(PIC1_DATA, 0xFD);
  port_byte_out(PIC2_DATA, 0xFF);

  driver_init_keyboard();

  enable_interrupts();

  for(;;) {
    halt();
  }
}