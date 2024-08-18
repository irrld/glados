#include <video.h>

void _start_kernel() {
  driver_init_video();
  set_color(FOREGROUND_LIGHT_BLUE | BACKGROUND_BLACK);
  println("Hello from C!");
  disable_cursor();
}