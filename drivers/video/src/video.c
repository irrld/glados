#include "video.h"

char* video_memory_ = (char*) 0xb8000;
int console_cursor_ = 0;

void ClearConsole() {
  // Create a pointer to a char, and point it to the first text cell of
  // video memory (i.e. the top-left of the screen)
  // At the address pointed to by video_memory, store the character 'X' // (i.e. display 'X' in the top-left of the screen).
  for (int i = 0; i < 80*25; i += 2) {
    video_memory_[i] = 0x0;
    video_memory_[i + 1] = 0x0f;
  }
  console_cursor_ = 0;
//  video_memory[0] = 'X';
}

void SetCursorPos(int row, int col) {
  console_cursor_ = row * 80 + col;
}

void UpdatePosition() {
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t) (pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void Print(const char* str) {
  char* video_memory = (char*) 0xb8000;
  while (1) {
    if (str[console_cursor_] == 0) break;
    video_memory[console_cursor_ * 2] = str[console_cursor_];
    console_cursor_++;
    UpdatePosition();
  }
}

void PrintLn(const char* str) {
  Print(str);
  Print("\n");
}