//
// Created by root on 9/5/24.
//

#include "glados/pic.h"
#include "glados/kernel.h"

void init_pic() {
  kprintf("Initializing PIC\n");

  // Mask all interrupts initially
  port_byte_out(PIC1_DATA, 0xFF);  // Mask all IRQs on PIC1
  port_byte_out(PIC2_DATA, 0xFF);  // Mask all IRQs on PIC2

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

  irq_clear_mask(0); // IRQ0 (System Timer)
  irq_clear_mask(1); // IRQ1 (Keyboard)
  irq_clear_mask(8); // IRQ8 (RTC)

  // Send End of Interrupt to both PICs to clear any pending interrupts
  port_byte_out(PIC1_COMMAND, PIC_EOI);
  port_byte_out(PIC2_COMMAND, PIC_EOI);
}