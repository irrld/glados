//
// Created by irrl on 8/12/23.
//

#ifndef GLADOS_VIDEO_H
#define GLADOS_VIDEO_H

#include "glados/stddef.h"

void driver_init_video(uint64_t framebuffer_addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp);
void clear_screen(uint32_t color);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);

#endif  //GLADOS_VIDEO_H