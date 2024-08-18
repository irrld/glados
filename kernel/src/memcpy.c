#include "memcpy.h"

void* memcpy(void* dst, const void* src, size_t len) {
  size_t pos = 0;

  while (pos < len) {
    ((char *)dst)[pos] = ((const char *)src)[pos];
    pos++;
  }
  return dst;
}