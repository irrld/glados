//
// Created by root on 9/5/24.
//

#include "glados/pit.h"

uint32_t timer_frequency_;

uint32_t get_timer_frequency() {
  return  timer_frequency_;
}

void init_pit(uint32_t frequency) {
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