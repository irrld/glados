//
// Created by irrl on 8/18/24.
//

#ifndef GLADOS_KEYBOARD_H
#define GLADOS_KEYBOARD_H

#include "stddef.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEYBOARD_KEY_LEFT_SHIFT 0x2A
#define KEYBOARD_KEY_RIGHT_SHIFT 0x36
#define KEYBOARD_KEY_CONTROL 0x1D
#define KEYBOARD_KEY_ALT 0x38
#define KEYBOARD_KEY_BACKSPACE 0x0E

typedef void (*keyboard_type_t)(uint8_t scancode, char key, bool is_press);

void driver_init_keyboard(keyboard_type_t handler);

bool is_shift();
bool is_alt();
bool is_ctrl();

#endif  //GLADOS_KEYBOARD_H
