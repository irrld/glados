#include "video.h"
#include "glados/kernel.h"
#include "glados/mutex.h"
#include "glados/string.h"

volatile uint64_t framebuffer_addr_;
volatile uint64_t framebuffer_size_;
volatile uint32_t width_;
volatile uint32_t height_;
volatile uint32_t pitch_;
volatile uint32_t bpp_;
volatile bool init_ = false;

void driver_init_video(uint64_t framebuffer_addr, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
  framebuffer_addr_ = framebuffer_addr;
  framebuffer_size_ = height * pitch;
  width_ = width;
  height_ = height;
  pitch_ = pitch;
  bpp_ = bpp;
  memset(framebuffer_addr, 0, pitch * height);
  init_ = true;
}

void clear_screen(uint32_t color) {
  if (!init_) {
    return;
  }
  memset(framebuffer_addr_, color, framebuffer_size_);
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
  if (!init_) {
    return;
  }
  uint32_t* pixel = (uint32_t*)(framebuffer_addr_ + y * pitch_ + x * (bpp_ / 8));
  *pixel = color;
}