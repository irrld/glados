//
// Created by irrl on 8/18/24.
//

#include "keyboard.h"
#include "kernel.h"


void keyboard_isr() {
  uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);
  char buffer[12];
  itoa(scancode, buffer, 10);
  print("Key pressed: ");
  print(buffer);
  print("\n");
}

void driver_init_keyboard() {
  register_interrupt_handler(33, keyboard_isr);
}