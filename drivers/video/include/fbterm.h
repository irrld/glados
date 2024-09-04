//
// Created by root on 9/4/24.
//

#ifndef GLADOS_FBTERM_H
#define GLADOS_FBTERM_H

#include "glados/stddef.h"

void init_fbterm(uint32_t screen_width, uint32_t screen_height);

void fbterm_handle_scroll();
void fbterm_disable_cursor();
void fbterm_enable_cursor();
void fbterm_clear_console();
void fbterm_set_color(uint8_t color);
void fbterm_draw_char(uint32_t x, uint32_t y, uint32_t color, char c);

#endif  //GLADOS_FBTERM_H
