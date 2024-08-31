
#include "kernel.h"
#include "string.h"
#include "video.h"
#include "keyboard.h"
#include "thread.h"
#include "shell.h"
#include "page.h"
#include "malloc.h"

extern void idt_load();

extern void isr_13();
extern void isr_14();
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

void kernel_panic(const char* str) {
  driver_init_video();
  set_color(FOREGROUND_RED | BACKGROUND_BLACK);
  println("KERNEL PANIC");
  println(str);
  disable_cursor();
  while (true) {
    halt();
  }
}

cpu_state_t* get_saved_cpu_state() {
  return &isr_cpu_state_;
}

void irh_0() {
  // todo kill the current process instead of kernel panic-ing
  kernel_panic("Divide-by-zero");
}

void irh_13() {
  kernel_panic("General protection fault occurred");
}

void irh_14() {
  // Read error code and faulting address
  uint64_t error_code;
  uint64_t faulting_address;

  // Get the error code from the CPU
  __asm__ volatile("mov %%cr2, %0" : "=r"(faulting_address));
  __asm__ volatile("pushf; pop %0" : "=r"(error_code));

  //printf("Page fault occurred! Address: %p, Error Code: %lx\n", faulting_address, error_code);
  kernel_panic("Page fault occurred");
}

void irh_32() {
  handle_threads();
  //println("TIMER!");
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

  idt_set_entry(INTERRUPT_GENERAL_PROTECTION_FAULT, (uint64_t)isr_13, 1);
  idt_set_entry(INTERRUPT_PAGE_FAULT, (uint64_t)isr_14, 1);
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
  while (true) {
    disable_interrupts();
    println("TANRI");
    enable_interrupts();
  }
}

void thread_b() {
  while (true) {
    disable_interrupts();
    println("ULUDUR");
    enable_interrupts();
  }
}

typedef struct mmap_entry {
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
  uint32_t acpi_attr;
} __attribute__((packed)) mmap_entry_t;

extern mmap_entry_t mmap_entries_[];
extern uint32_t mmap_entry_count_;
uint64_t available_memory_;

void parse_memory_map() {
  available_memory_ = 0;
  mmap_entry_t* entry = (mmap_entry_t*) mmap_entries_;

  while (entry->type != 0) { // Assuming 0 is an invalid type and end of entries
    if (entry->type == 1) {  // Type 1 indicates usable memory
      uint64_t start_addr = entry->base_addr;
      uint64_t size = entry->length;
      available_memory_ += size;
    }

    entry++;  // Move to the next entry
  }

  if (available_memory_ <= 0) {
    kernel_panic("No available memory");
  } else {
    print("Total available memory: 0x");
    char str[32];
    itoa(available_memory_, str, 16);
    println(str);
  }
}

void start_kernel(uintptr_t image_end) {
  driver_init_video();
  set_color(FOREGROUND_LIGHT_GRAY | BACKGROUND_BLACK);

  println("Initializing IDT");
  idt_init();
  println("Initializing PIC");
  pic_init();
  println("Initializing PIT");
  pit_init(100); // 10ms


  parse_memory_map();
  println("Initializing paging");
  paging_init((image_end + 0xFFF) & ~0xFFF); // Aligns the image end address to 0x1000
  println("Initializing malloc");
  malloc_init();
  println("Creating threads");

  create_thread(thread_a);
  create_thread(thread_b);

  println("Enabling interrupts");
  enable_interrupts();
  // After this, we halt and threads should get activated by the timer
}