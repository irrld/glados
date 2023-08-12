#pragma once
#include <stdint.h>
#include <stdbool.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

#define FOREGROUND_BLACK 0x00
#define FOREGROUND_BLUE 0x01
#define FOREGROUND_GREEN 0x02
#define FOREGROUND_CYAN 0x03
#define FOREGROUND_RED 0x04
#define FOREGROUND_MAGENTA 0x05
#define FOREGROUND_BROWN 0x06
#define FOREGROUND_LIGHT_GRAY 0x07
#define FOREGROUND_DARK_GRAY 0x08
#define FOREGROUND_LIGHT_BLUE 0x09
#define FOREGROUND_LIGHT_GREEN 0x0a
#define FOREGROUND_LIGHT_CYAN 0x0b
#define FOREGROUND_LIGHT_RED 0x0c
#define FOREGROUND_LIGHT_MAGENTA 0x0d
#define FOREGROUND_YELLOW 0x0e
#define FOREGROUND_WHITE 0x0f

#define BACKGROUND_BLACK 0x00
#define BACKGROUND_BLUE 0x10
#define BACKGROUND_GREEN 0x20
#define BACKGROUND_CYAN 0x30
#define BACKGROUND_RED 0x40
#define BACKGROUND_MAGENTA 0x50
#define BACKGROUND_BROWN 0x60
#define BACKGROUND_LIGHT_GRAY 0x70
#define BACKGROUND_DARK_GRAY 0x80
#define BACKGROUND_LIGHT_BLUE 0x90
#define BACKGROUND_LIGHT_GREEN 0xa0
#define BACKGROUND_LIGHT_CYAN 0xb0
#define BACKGROUND_LIGHT_RED 0xc0
#define BACKGROUND_LIGHT_MAGENTA 0xd0
#define BACKGROUND_YELLOW 0xe0
#define BACKGROUND_WHITE 0xf0

#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

struct CursorPos {
  uint8_t x;
  uint8_t y;
};

void DisableCursor();
void EnableCursor();
void ClearConsole();
void SetCursorPos(uint8_t row, uint8_t col);
struct CursorPos GetCursorPos();
void SetColor(uint8_t color);
void Print(const char* str);
void PrintLn(const char* str);
