#include "video.h"
#include "low_level.h"

struct VideoData {
  char character_;
  char color_;
};
struct VideoData* video_memory_ = VIDEO_ADDRESS;

int cursor_pos_ = 0;
bool cursor_enabled_ = true;
uint8_t color_ = FOREGROUND_WHITE | BACKGROUND_BLACK;

// todo move this somewhere else
void MemoryCopy(char* source, char* dest, int bytes) {
  for (int i = 0; i < bytes; i++) {
    char* dest_ptr = dest + i;
    char* source_ptr = source + i;
    *dest_ptr = *source_ptr;
  }
}

int GetScreenOffset(uint8_t col, uint8_t row) {
  return (row * MAX_COLS + col) * 2;
}

void HandleScroll() {
  /* Advance the text cursor, scrolling the video buffer if necessary. */
  // If the cursor is within the screen, return it unmodified.
  if (cursor_pos_ < MAX_ROWS * MAX_COLS) {
    return;
  }
  /* Shuffle the rows back one. */
  for (int row = 1; row < MAX_ROWS; row++) {
    MemoryCopy(VIDEO_ADDRESS + GetScreenOffset(0, row),
               VIDEO_ADDRESS + GetScreenOffset(0, row - 1),
               MAX_COLS * 2
    );
  }
  /* Blank the last line by setting all bytes to 0 */
  int offset = GetScreenOffset(0, MAX_ROWS - 1);
  char* last_line = VIDEO_ADDRESS + offset;
  for (int i = 0; i < MAX_COLS * 2; i++) {
    last_line[i] = 0;
  }

  cursor_pos_ -= MAX_COLS;
}

void UpdateCursorPosition() {
  if (!cursor_enabled_) {
    return;
  }
  port_byte_out(REG_SCREEN_CTRL, 0x0A);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xC0) | cursor_pos_);
  port_byte_out(REG_SCREEN_CTRL, 0x0B);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xE0) | cursor_pos_);
}

void DisableCursor() {
  if (!cursor_enabled_) {
    return;
  }
  port_byte_out(REG_SCREEN_CTRL, 0x0A);
  port_byte_out(REG_SCREEN_DATA, 0x20);
  cursor_enabled_ = false;
}

void EnableCursor() {
  if (cursor_enabled_) {
    return;
  }
  port_byte_out(REG_SCREEN_CTRL, 0x0A);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xC0) | cursor_pos_);
  port_byte_out(REG_SCREEN_CTRL, 0x0B);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xE0) | cursor_pos_);
}

void ClearConsole() {
  for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
    struct VideoData* data = &video_memory_[i];
    data->character_ = 0x0;
    data->color_ = FOREGROUND_WHITE | BACKGROUND_BLACK;
  }
  cursor_pos_ = 0;
  UpdateCursorPosition();
}

void SetCursorPos(uint8_t row, uint8_t col) {
  cursor_pos_ = row * MAX_COLS + col;
  UpdateCursorPosition();
}

struct CursorPos GetCursorPos() {
  struct CursorPos pos;
  port_byte_out(REG_SCREEN_CTRL, 0x0F);
  pos.x = port_byte_in(REG_SCREEN_DATA);
  port_byte_out(REG_SCREEN_CTRL, 0x0E);
  pos.y = port_byte_in(REG_SCREEN_DATA);
  return pos;
}

void SetColor(uint8_t color) {
  color_ = color;
}

void Print(const char* str) {
  int i = 0;
  while (true) {
    if (str[i] == 0) {
      break;
    }
    if (str[i] == '\n') {
      i++;
      cursor_pos_ = (cursor_pos_ / MAX_COLS + 1) * MAX_COLS;
      HandleScroll();
      continue;
    }
    struct VideoData* data = &video_memory_[cursor_pos_];
    data->character_ = str[i];
    data->color_ = color_;
    i++;
    cursor_pos_++;
    UpdateCursorPosition();
  }
  HandleScroll();
}

void PrintLn(const char* str) {
  Print(str);
  Print("\n");
}