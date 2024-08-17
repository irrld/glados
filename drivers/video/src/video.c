#include "video.h"
#include "low_level.h"

struct VideoData {
  char character_;
  char color_;
};

volatile int cursor_pos_;
volatile bool cursor_enabled_;
volatile uint8_t color_;

void InitializeVideo() {
  cursor_enabled_ = true;
  cursor_pos_ = 0;
  color_ = FOREGROUND_WHITE | BACKGROUND_BLACK;
  ClearConsole();
  DisableCursor();
}

struct VideoData* GetVideoDataFromIndex(int index) {
  void* ptr = (void*) (VIDEO_ADDRESS + (index * sizeof(struct VideoData)));
  return (struct VideoData*) ptr;
}

struct VideoData* GetVideoData(uint8_t col, uint8_t row) {
  int index = (row * MAX_COLS) + col;
  return GetVideoDataFromIndex(index);
}

// todo move this somewhere else
void MemoryCopy(void* source, void* dest, int bytes) {
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
    MemoryCopy(GetVideoData(0, row),
               GetVideoData(0, row - 1),
               MAX_COLS * 2
    );
  }
  /* Blank the last line by setting all bytes to 0 */
  for (int i = 0; i < MAX_COLS; i++) {
    struct VideoData* data = GetVideoData(i, MAX_ROWS - 1);
    data->character_ = 0x0;
    data->color_ = FOREGROUND_WHITE | BACKGROUND_BLACK;
  }

  cursor_pos_ -= MAX_COLS;
}

void UpdateCursorPosition() {
  if (!cursor_enabled_) {
    return;
  }
  port_byte_out(REG_SCREEN_CTRL, 0x0F);
  port_byte_out(REG_SCREEN_DATA, (uint8_t) (cursor_pos_ + MAX_COLS & 0xFF));
  port_byte_out(REG_SCREEN_CTRL, 0x0E);
  port_byte_out(REG_SCREEN_DATA, (uint8_t) ((cursor_pos_ + MAX_COLS >> 8) & 0xFF));
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
  cursor_enabled_ = true;
  port_byte_out(REG_SCREEN_CTRL, 0x0A);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xC0) | 0);
  port_byte_out(REG_SCREEN_CTRL, 0x0B);
  port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xE0) | 1);
  UpdateCursorPosition();
}

void ClearConsole() {
  for (int i = 0; i < MAX_COLS * MAX_ROWS; i++) {
    struct VideoData* data = GetVideoDataFromIndex(i);
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
    struct VideoData* data = GetVideoDataFromIndex(cursor_pos_);
    data->character_ = str[i];
    data->color_ = color_;
    i++;
    cursor_pos_++;
  }
  UpdateCursorPosition();
  HandleScroll();
}

void PrintLn(const char* str) {
  Print(str);
  Print("\n");
}