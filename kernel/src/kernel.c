
#include "kernel.h"
#include "keyboard.h"
#include "kmalloc.h"
#include "mutex.h"
#include "page.h"
#include "shell.h"
#include "string.h"
#include "thread.h"
#include "video.h"

extern void load_gdt();
extern void load_idt();
extern void load_tss(uint16_t selector);

extern void isr_13();
extern void isr_14();
extern void isr_32();
extern void isr_33();
extern void isr_40();

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

typedef struct tss_entry {
  uint32_t reserved1;
  uint64_t rsp0;  // Stack pointer for ring 0
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved2;
  uint64_t ist1;  // Stack pointers for IST
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved3;
  uint16_t reserved4;
  uint16_t iopb_offset;
} __attribute__((packed)) tss_entry_t;

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

gdt_entry_t gdt_[5];
gdt_ptr_t gdtp_;
#define IDT_ENTRIES 256
idt_entry_t idt_[IDT_ENTRIES];  // Create an IDT with 256 entries
idt_ptr_t idtp_;
tss_entry_t tss_;
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

void idt_set_entry(uint8_t irq, uint64_t base, uint16_t segment_selector, uint8_t ist) {
  idt_[irq].offset_1 = base & 0xFFFF;
  idt_[irq].offset_2 = (base >> 16) & 0xFFFF;
  idt_[irq].offset_3 = (base >> 32) & 0xFFFFFFFF;

  idt_[irq].segment_selector = segment_selector;
  idt_[irq].ist = ist;
  idt_[irq].flags = 0x8E;
  idt_[irq].zero = 0;
}

void send_eoi(uint8_t irq) {
  if (irq >= 8) {
    port_byte_out(PIC2_COMMAND, PIC_EOI);  // Tell PIC2 that IRQ has been handled
  }
  port_byte_out(PIC1_COMMAND, PIC_EOI);      // Tell PIC1 that IRQ has been handled
}

void kernel_panic(const char* str) {
  mutex_deinit();
  set_color(FOREGROUND_WHITE | BACKGROUND_BLACK);
  kprintf("\n---[ Kernel Panic: %s ]---\n", str);
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

  //kprintf("Page fault occurred! Address: %p, Error Code: %lx\n", faulting_address, error_code);
  kernel_panic("Page fault occurred");
}

void irh_32() {
  handle_threads();
  //println("TIMER!");
}

void irh_33() {
  uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);
  kprintf("Key pressed: %u", scancode);
}

void irh_40() {
  kprintf("RTC!\n");
}

void idt_init() {
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

void tss_init() {
  kprintf("Initializing TSS\n");
  memset(&tss_, 0, sizeof(tss_));
  tss_.ist1 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  tss_.rsp0 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  tss_.rsp1 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  tss_.rsp2 = kmalloc(0x1000) + 0xFF0; // 4KiB stack
  load_tss(TSS_SELECTOR);
}

void gdt_init() {
  kprintf("Initializing GDT\n");
  // Read the old one
  gdt_ptr_t old_ptr;
  read_gdt_ptr(&old_ptr);
  // Setup the descriptor for the new one
  gdtp_.limit = sizeof(gdt_) - 1;
  gdtp_.base = (uint64_t)gdt_;

  // Copy old values to the new values
  memset(gdt_, 0, sizeof(gdt_));
  memcpy((uintptr_t*) gdtp_.base, (uintptr_t*) old_ptr.base, old_ptr.limit + 1);

  uint64_t tss_base = (uint64_t)&tss_;
  uint64_t tss_limit = sizeof(tss_) - 1;

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

void pic_init() {
  kprintf("Initializing PIC\n");
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

  irq_clear_mask(0); // IRQ0 (System Timer)
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
  kprintf("Initializing PIT. Interrupt frequency: %luhz (%lums)\n", frequency, 1000 / frequency);
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
      //uint64_t start_addr = entry->base_addr;
      uint64_t size = entry->length;
      available_memory_ += size;
    }

    entry++;  // Move to the next entry
  }

  if (available_memory_ <= 0) {
    kernel_panic("No available memory");
  } else {
    kprintf("Total available memory: 0x%llx\n", available_memory_);
  }
}
static mutex_t mutex_;

void thread_a() {
  while (true) {
    mutex_lock(&mutex_);
    kprintf("A\n");
    mutex_unlock(&mutex_);
  }
}

void thread_b() {
  while (true) {
    mutex_lock(&mutex_);
    kprintf("B\n");
    mutex_unlock(&mutex_);
  }
}

void start_kernel(uintptr_t image_end) {
  driver_init_video();
  set_color(FOREGROUND_LIGHT_GRAY | BACKGROUND_BLACK);

  idt_init();
  pic_init();
  pit_init(100); // 10ms

  parse_memory_map();
  paging_init((image_end + 0xFFF) & ~0xFFF); // Aligns the image end address to 0x1000
  kmalloc_init();

  gdt_init();
  tss_init();

  kprintf("Creating threads\n");
  create_thread(thread_a);
  create_thread(thread_b);

  kprintf("Enabling interrupts!\n");
  mutex_init();
  // We need to immediately enable interrupts after this
  // Using mutexes (such as via calling kprintf) will enable interrupts
  enable_interrupts();
  // After this, we halt and threads should get activated by the timer
}