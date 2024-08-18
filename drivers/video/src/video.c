#include "video.h"
#include "kernel.h"
#include "string.h"

struct video_data {
  char character;
  char color;
};

volatile int cursor_pos_;
volatile bool cursor_enabled_;
volatile uint8_t color_;

void driver_init_video() {
  cursor_enabled_ = true;
  cursor_pos_ = 0;
  color_ = FOREGROUND_WHITE | BACKGROUND_BLACK;
  clear_console();
  disable_cursor();
}

struct video_data* get_video_data_from_index(int index) {
  void* ptr = (void*) (VIDEO_ADDRESS + (index * sizeof(struct video_data)));
  return (struct video_data*) ptr;
}

struct video_data* get_video_data(uint8_t col, uint8_t row) {
  int index = (row * MAX_COLS) + col;
  return get_video_data_from_index(index);
}

int get_screen_offset(uint8_t col, uint8_t row) {
  return (row * MAX_COLS + col) * 2;
}

void handle_scroll() {
  /* Advance the text cursor, scrolling the video buffer if necessary. */
  // If the cursor is within the screen, return it unmodified.
  if (cursor_pos_ < MAX_ROWS * MAX_COLS) {
    return;
  }
  /* Shuffle the rows back one. */
  for (int row = 1; row < MAX_ROWS; row++) {
    memcpy(get_video_data(0, row - 1),
           get_video_data(0, row),
           MAX_COLS * 2);
  }
  /* Blank the last line by setting all bytes to 0 */
  for (int i = 0; i < MAX_COLS; i++) {
    struct video_data* data = get_video_data(i, MAX_ROWS - 1);
    data->character = 0x0;
    data->color = FOREGROUND_WHITE | BACKGROUND_BLACK;
  }

  cursor_pos_ -= MAX_COLS;
}

void update_cursor_pos() {
  if (!cursor_enabled_) {
    return;
  }
  port_byte_out(REG_SCREEN_CTRL, 0x0F);
  port_byte_out(REG_SCREEN_DATA, (uint8_t) (cursor_pos_ + MAX_COLS & 0xFF));
  port_byte_out(REG_SCREEN_CTRL, 0x0E);
  port_byte_out(REG_SCREEN_DATA, (uint8_t) ((cursor_pos_ + MAX_COLS >> 8) & 0xFF));
}

void disable_cursor() {
  if (!cursor_enabled_) {
    return;
  }
  port_byte_out(REG_SCREEN_CTRL, 0x0A);
  port_byte_out(REG_SCREEN_DATA, 0x20);
  cursor_enabled_ = false;
}

void enable_cursor() {
  if (cursor_enabled_) {
    return;
  }
  cursor_enabled_ = true;
  port_byte_out(REG_SCREEN_CTRL, 0x0A);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xC0) | 0);
  port_byte_out(REG_SCREEN_CTRL, 0x0B);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xE0) | 1);
  update_cursor_pos();
}

void clear_console() {
  for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
    struct video_data* data = get_video_data_from_index(i);
    data->character = 0x0;
    data->color = FOREGROUND_WHITE | BACKGROUND_BLACK;
  }
  cursor_pos_ = 0;
  update_cursor_pos();
}

void set_cursor_pos(uint8_t row, uint8_t col) {
  cursor_pos_ = row * MAX_COLS + col;
  update_cursor_pos();
}

struct cursor_pos get_cursor_pos() {
  struct cursor_pos pos;
  port_byte_out(REG_SCREEN_CTRL, 0x0F);
  pos.x = port_byte_in(REG_SCREEN_DATA);
  port_byte_out(REG_SCREEN_CTRL, 0x0E);
  pos.y = port_byte_in(REG_SCREEN_DATA);
  return pos;
}

void set_color(uint8_t color) {
  color_ = color;
}

void print(const char* str) {
  int i = 0;
  while (true) {
    if (str[i] == 0) {
      break;
    }
    if (str[i] == '\n') {
      i++;
      cursor_pos_ = (cursor_pos_ / MAX_COLS + 1) * MAX_COLS;
      handle_scroll();
      continue;
    }
    struct video_data* data = get_video_data_from_index(cursor_pos_);
    data->character = str[i];
    data->color = color_;
    i++;
    cursor_pos_++;
  }
  update_cursor_pos();
  handle_scroll();
}

void println(const char* str) {
  print(str);
  print("\n");
}