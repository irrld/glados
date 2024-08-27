
#include "kernel.h"
#include "string.h"
#include "video.h"
#include "keyboard.h"
#include "thread.h"
#include "shell.h"

extern void idt_load();
extern void isr_32();
extern void isr_33();
extern void isr_40();

// IDT entry structure
struct idt_entry {
  uint16_t offset_1;
  uint8_t  rpl : 2;
  uint8_t  ti : 1;
  uint16_t segment_index : 13;
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

struct idt_entry idt_[IDT_ENTRIES];  // Create an IDT with 256 entries
struct idt_ptr idtp_;
uint32_t timer_frequency_;

cpu_state_t isr_cpu_state_; // Saved cpu state for ISR to load back

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

void idt_set_entry(uint8_t irq, uint64_t base, uint16_t segment) {
  idt_[irq].offset_1 = base & 0xFFFF;
  idt_[irq].offset_2 = (base >> 16) & 0xFFFF;
  idt_[irq].offset_3 = (base >> 32) & 0xFFFFFFFF;

  idt_[irq].segment_index = segment;
  idt_[irq].rpl = 0;
  idt_[irq].ti = 0;
  idt_[irq].ist = 0;
  idt_[irq].dpl = 0;
  idt_[irq].p = 1;
  idt_[irq].gate_type = 0xE; // Interrupt Gate

  idt_[irq].reserved1 = 0;
  idt_[irq].reserved2 = 0;
  idt_[irq].zero = 0;
}

void send_eoi(uint8_t irq) {
  if (irq >= 8) {
    port_byte_out(PIC2_COMMAND, PIC_EOI);  // Tell PIC2 that IRQ has been handled
  }
  port_byte_out(PIC1_COMMAND, PIC_EOI);      // Tell PIC1 that IRQ has been handled
}

cpu_state_t* get_saved_cpu_state() {
  return &isr_cpu_state_;
}

void irh_32() {
  handle_threads();
}

void irh_33() {
  println("KEYBOARD!");
}

void irh_40() {
  println("RTC!");
}

void idt_init() {
  idtp_.limit = sizeof(idt_) - 1;
  idtp_.base = (uint64_t)idt_;
  memset(&idt_, 0, idtp_.limit + 1);

  idt_set_entry(INTERRUPT_TIMER, (uint64_t)isr_32, 1);
  idt_set_entry(INTERRUPT_KEYBOARD, (uint64_t)isr_33, 1);
  idt_set_entry(INTERRUPT_RTC, (uint64_t)isr_40, 1);

  idt_load();
}

void pic_init() {
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

  port_byte_out(PIC1_DATA, 0xFF);
  port_byte_out(PIC2_DATA, 0xFF);

  irq_clear_mask(0); // IRQ0 (System)
  irq_clear_mask(1); // IRQ1 (Keyboard)
  irq_clear_mask(8); // IRQ8 (RTC)
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

void pit_init(uint32_t frequency) {
  timer_frequency_ = frequency;
  // The PIT input clock frequency
  uint32_t pit_input_freq = 1193182;

  // Calculate the count value
  uint16_t divisor = (uint16_t)(pit_input_freq / frequency);

  // Send the command byte to PIT
  port_byte_out(0x43, 0x36);

  // Send the divisor (low byte first, then high byte)
  port_byte_out(0x40, (uint8_t)(divisor & 0xFF));        // Low byte
  port_byte_out(0x40, (uint8_t)((divisor >> 8) & 0xFF)); // High byte
}

void thread_a() {
  println("A");
}

void thread_b() {
  println("B");
}

void _start_kernel() {
  driver_init_video();
  set_color(FOREGROUND_LIGHT_BLUE | BACKGROUND_BLACK);
  println("TANTALUM");
  set_color(FOREGROUND_WHITE | BACKGROUND_BLACK);
  enable_cursor();

  idt_init();
  pic_init();
  pit_init(100); // 10ms

  create_thread(thread_a);
  create_thread(thread_b);

  enable_interrupts();

  shell_main();
}