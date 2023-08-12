#include <video.h>

void KernelMain() {
  ClearConsole();
  SetColor(FOREGROUND_LIGHT_BLUE | BACKGROUND_BLACK);
  PrintLn("Hello World!");
  PrintLn("BRUH!");
  DisableCursor();
  EnableCursor();
}