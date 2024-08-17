//#include <video.h>
//#include <stdint.h>

#define VGA_COLUMNS_NUM 80
#define VGA_ROWS_NUM 25

#define ARRAY_SIZE(arr) ((int)sizeof(arr) / (int)sizeof((arr)[0]))

void _start_kernel() {
  volatile char *vga_buf = (char *)0xb8000;
  const char msg[] = "Hello from C";
  int i;

  for (i = 0; i < VGA_COLUMNS_NUM * VGA_ROWS_NUM * 2; i++)
    vga_buf[i] = '\0';

  for (i = 0; i < ARRAY_SIZE(msg) - 1; i++) {
    vga_buf[i * 2] = msg[i];
    vga_buf[i * 2 + 1] = 0x07; /* White on black */
  }

//  InitializeVideo();
//  SetColor(FOREGROUND_LIGHT_BLUE | BACKGROUND_BLACK);
//  PrintLn("Hello World!");
//  PrintLn("Hello World!");
//  Print("Hello: ");
//  DisableCursor();
//  EnableCursor();
}