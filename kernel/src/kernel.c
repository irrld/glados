#include <video.h>
#include <stdint.h>

void KernelMain() {
  InitializeVideo();
//  SetColor(FOREGROUND_LIGHT_BLUE | BACKGROUND_BLACK);
  PrintLn("Hello World!");
  PrintLn("Hello World!");
  Print("Hello: ");
//  DisableCursor();
  EnableCursor();
}