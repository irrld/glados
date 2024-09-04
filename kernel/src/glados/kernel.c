
#include "glados/kernel.h"
#include "keyboard.h"
#include "video.h"
#include "fbterm.h"
#include "glados/kmalloc.h"
#include "glados/mutex.h"
#include "glados/page.h"
#include "glados/string.h"
#include "glados/thread.h"
#include "glados/time.h"
#include "glados/idt.h"
#include "glados/pit.h"
#include "glados/pic.h"
#include "glados/multiboot2.h"

cpu_state_t isr_cpu_state_; // Saved cpu state for ISR to load back

// todo handle nmi's
// todo apic
void enable_nmi() {
  port_byte_out(CMOS_ADDRESS, port_byte_in(CMOS_ADDRESS) & 0x7F);
  port_byte_in(CMOS_DATA);
}

void disable_nmi() {
  port_byte_out(CMOS_ADDRESS, port_byte_in(CMOS_ADDRESS) | 0x80);
  port_byte_in(CMOS_DATA);
}

void kernel_panic(const char* str) {
  kprintf("\n---[ Kernel Panic: %s ]---\n", str);
  while (true) {
    halt();
  }
}

cpu_state_t* get_saved_cpu_state() {
  return &isr_cpu_state_;
}

irh_t interrupt_handlers[IDT_ENTRIES];

void irh_common(uint16_t irq) {
  if (interrupt_handlers[irq]) {
    interrupt_handlers[irq](irq);
  }
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
}

void register_irh(uint16_t irq, irh_t fn) {
  interrupt_handlers[irq] = fn;
}

static mutex_t mutex_;
void thread_a() {
  while (true) {
    /*mutex_lock(&mutex_);
    kprintf("A\n");
    mutex_unlock(&mutex_);*/
  }
}

void thread_b() {
  while (true) {
    /*mutex_lock(&mutex_);
    kprintf("B\n");
    mutex_unlock(&mutex_);*/
  }
}

multiboot_parsed_data_t multiboot_info;

void graphics_init() {
  multiboot_tag_framebuffer_t* framebuffer = multiboot_info.framebuffer;
  if (!framebuffer) {
    return;
  }
  uint64_t framebuffer_addr = framebuffer->common.framebuffer_addr;
  uint32_t width = framebuffer->common.framebuffer_width;
  uint32_t height = framebuffer->common.framebuffer_height;
  uint32_t pitch = framebuffer->common.framebuffer_pitch;
  uint8_t  bpp = framebuffer->common.framebuffer_bpp;

  identity_map(framebuffer_addr, pitch * height);
  driver_init_video(framebuffer_addr, width, height, pitch, bpp);
  init_fbterm(width, height);
}

extern uint32_t multiboot_info_addr_;

void kmain(uintptr_t image_end) {
  multiboot_info = parse_multiboot_info(multiboot_info_addr_);
  init_gdt();
  paging_init((image_end + 0xFFF) & ~0xFFF); // Aligns the image end address to 0x1000
  kmalloc_init();
  graphics_init();
  kprintf("Total available memory: 0x%x\n", multiboot_info.available_memory);

  init_idt();
  init_time();
  init_pit(100); // 10ms
  init_pic();
  init_tss();

  driver_init_keyboard();

  kprintf("Creating threads\n");
  create_thread(thread_a);
  create_thread(thread_b);

  kprintf("Enabling interrupts!\n");
  enable_interrupts();
  // After this, we halt and threads should get activated by the timer
}