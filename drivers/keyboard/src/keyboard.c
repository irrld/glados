//
// Created by irrl on 8/18/24.
//

#include "keyboard.h"
#include "../../../kernel/include/glados/kernel.h"
#include "../../../kernel/include/glados/string.h"

const char keymap_[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 0x00 - 0x09 */
    '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', /* 0x0A - 0x13 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, /* 0x14 - 0x1D */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 0x1E - 0x27 */
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 0x28 - 0x31 */
    'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, /* 0x32 - 0x3B */
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', /* 0x3C - 0x45 */
    '4', '5', '6', '+', '1', '2', '3', '0', '.', /* 0x46 - 0x4E */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x4F - 0x59 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x5A - 0x64 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* 0x65 - 0x6F */
};

const char shift_keymap_[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 0x00 - 0x09 */
    '9', '0', '-', '=', '\b', '\t', 'Q', 'W', 'E', 'R', /* 0x0A - 0x13 */
    'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0, /* 0x14 - 0x1D */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', /* 0x1E - 0x27 */
    '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', /* 0x28 - 0x31 */
    'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, /* 0x32 - 0x3B */
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', /* 0x3C - 0x45 */
    '4', '5', '6', '+', '1', '2', '3', '0', '.', /* 0x46 - 0x4E */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x4F - 0x59 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x5A - 0x64 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /* 0x65 - 0x6F */
};


keyboard_type_t callback_;
bool shift_pressed_ = false;
bool ctrl_pressed_ = false;
bool alt_pressed_ = false;

void keyboard_isr() {
  uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);
  /*bool is_press = !(scancode & 0b10000000);
  scancode &= ~0b10000000;
  if (scancode == KEYBOARD_KEY_LEFT_SHIFT || scancode == KEYBOARD_KEY_RIGHT_SHIFT) {
    shift_pressed_ = is_press;
    return;
  }
  if (scancode == KEYBOARD_KEY_CONTROL) {
    ctrl_pressed_ = is_press;
    return;
  }
  if (scancode == KEYBOARD_KEY_ALT) {
    alt_pressed_ = is_press;
    return;
  }

  char key;
  if (shift_pressed_) {
    key = shift_keymap_[scancode];
  } else {
    key = keymap_[scancode];
  }
  callback_(scancode, key, is_press);*/
}

void driver_init_keyboard(keyboard_type_t handler) {
  callback_ = handler;
}

bool is_shift() {
  return shift_pressed_;
}

bool is_alt() {
  return alt_pressed_;
}
bool is_ctrl() {
  return ctrl_pressed_;
}