#include "glados/string.h"

void* memcpy(void* dst, const void* src, size_t len) {
  size_t pos = 0;

  while (pos < len) {
    ((char *)dst)[pos] = ((const char *)src)[pos];
    pos++;
  }
  return dst;
}

void* memset(void* dst, char value, size_t len) {
  size_t pos = 0;

  while (pos < len) {
    ((char *)dst)[pos] = value;
    pos++;
  }
  return dst;
}

#define BUFFER_SIZE 32

// Converts an integer to a string
void itoa(int num, char* str, int radix) {
  static char digits[] = "0123456789abcdef";
  char buffer[BUFFER_SIZE];
  int i = 0;
  int is_negative = 0;

  // Handle 0 explicitly, otherwise empty string is printed
  if (num == 0) {
    str[i++] = '0';
    str[i] = '\0';
    return;
  }

  // Handle negative numbers only for base 10
  if (num < 0 && radix == 10) {
    is_negative = 1;
    num = -num;
  }

  // Process individual digits
  while (num != 0) {
    buffer[i++] = digits[(num % radix)];
    num /= radix;
  }

  // Append negative sign for base 10
  if (is_negative) {
    buffer[i++] = '-';
  }

  // Null-terminate string
  buffer[i] = '\0';

  // Reverse the string
  int start = 0;
  int end = i - 1;
  while (start <= end) {
    str[start] = buffer[end];
    str[end] = buffer[start];
    start++;
    end--;
  }
  str[i] = '\0';
}

bool is_upper(char c) {
  return c >= 'A' && c <= 'Z';
}

bool is_number(char c) {
  return c >= '0' && c <= '9';
}

char atoi(char c) {
  if (!is_number(c)) {
    return 0;
  }
  return '0' - c;
}