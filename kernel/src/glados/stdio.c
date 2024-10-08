//
// Created by root on 9/4/24.
//

#include "glados/stdio.h"
#include "fbterm.h"


void kputc(const char c) {
  fbterm_putc(c);
}

typedef enum printf_length_field {
  PRINTF_LEN_NONE,
  PRINTF_LEN_CHAR,
  PRINTF_LEN_SHORT,
  PRINTF_LEN_LONG,
  PRINTF_LEN_LONG_LONG,
  PRINTF_LEN_LONG_DOUBLE,
  PRINTF_LEN_SIZE_T,
  PRINTF_LEN_INTMAX_T,
  PRINTF_LEN_PTRDIFF_T
} printf_length_field_t;

int print_number(uint64_t num, int radix, bool is_capital) {
  static char digits[] = "0123456789abcdef0123456789ABCDEF";
  int capitals = is_capital ? 16 : 0; // offset
  char buffer[32];
  int i = 0;

  // Handle 0 explicitly, otherwise empty string is printed
  if (num == 0) {
    kputc('0');
    return 1;
  }

  // Process individual digits
  while (num != 0) {
    buffer[i++] = digits[(num % radix) + capitals];
    num /= radix;
  }

  // Print the string in reverse
  for (int j = i - 1; j >= 0; j--) {
    kputc(buffer[j]);
  }
  return i;
}

int _va_print(printf_length_field_t length, va_list* ptr, bool is_signed, int radix, bool is_capital) {
  int64_t n;
  uint64_t u;
  switch (length) {
    case PRINTF_LEN_NONE: {
      n = va_arg(*ptr, int);
      break;
    }
    case PRINTF_LEN_CHAR: {
      n = va_arg(*ptr, char);
      break;
    }
    case PRINTF_LEN_SHORT: {
      n = va_arg(*ptr, short);
      break;
    }
    case PRINTF_LEN_LONG: {
      n = va_arg(*ptr, long);
      break;
    }
    case PRINTF_LEN_LONG_LONG: {
      n = va_arg(*ptr, long long);
      break;
    }
    case PRINTF_LEN_SIZE_T: {
      n = va_arg(*ptr, size_t);
      break;
    }
    case PRINTF_LEN_INTMAX_T: {
      n = va_arg(*ptr, uintmax_t);
      break;
    }
    case PRINTF_LEN_PTRDIFF_T: {
      n = va_arg(*ptr, ptrdiff_t);
      break;
    }
    default:
      return 0;
  }
  u = (uint64_t) n;
  if (n < 0 && is_signed) {
    kputc('-');
    u = -n;
  }
  return print_number(u, radix, is_capital);
}

// A simple printf implementation, missing ton of features
int kprintf(const char* str, ...) {
  va_list ptr;
  va_start(ptr, str);

  int written = 0;
  const char* c = str;
  bool fmt = false;
  printf_length_field_t length = PRINTF_LEN_NONE;
  while (true) {
    if (*c == '\0') {
      break;
    }
    char ch = *c;
    if (ch == '%') {
      if (fmt) {
        kputc('%');
        written++;
        fmt = false;
      } else {
        fmt = true;
      }
    } else if (fmt) {
      bool uppercase = is_upper(ch);
      // Length field
      if (ch == 'h') {
        if (length == PRINTF_LEN_CHAR) { // hh
          length = PRINTF_LEN_SHORT;
        } else {
          length = PRINTF_LEN_CHAR;
        }
        goto next_char;
      } else if (ch == 'l') {
        if (length == PRINTF_LEN_LONG) { // ll
          length = PRINTF_LEN_LONG_LONG;
        } else {
          length = PRINTF_LEN_LONG;
        }
        goto next_char;
      } else if (ch == 'L') {
        length = PRINTF_LEN_LONG_DOUBLE;
        goto next_char;
      }  else if (ch == 'z') {
        length = PRINTF_LEN_SIZE_T;
        goto next_char;
      } else if (ch == 'j') {
        length = PRINTF_LEN_INTMAX_T;
        goto next_char;
      } else if (ch == 't') {
        length = PRINTF_LEN_PTRDIFF_T;
        goto next_char;
      }
      // Type field
      if (ch == 'd' || ch == 'i') {
        // print int as signed integer
        written += _va_print(length, &ptr, true, 10, uppercase);
        length = 0;
      } else if (ch == 'u') {
        // print decimal unsigned int
        written += _va_print(length, &ptr, false, 10, uppercase);
        length = 0;
      } else if (ch == 'f' || ch == 'F') {
        // print double in normal fixed point notation
      } else if (ch == 'e' || ch == 'E') {
        // print double in standard form (d.ddde+dd)
      } else if (ch == 'g' || ch == 'G') {
        // double in either normal or exponential, whichever is more appropriate
      } else if (ch == 'x' || ch == 'X') {
        // uint as hex number
        written += _va_print(length, &ptr, false, 16, uppercase);
        length = 0;
      } else if (ch == 'o') {
        // uint in octal
        written += _va_print(length, &ptr, false, 8, uppercase);
        length = 0;
      } else if (ch == 's') {
        // string (null terminated)
        const char* s = va_arg(ptr, const char*);
        while (*s != '\0') {
          kputc(*s);
          s++;
          written++;
        }
        length = 0;
      } else if (ch == 'c') {
        // char
        char c = va_arg(ptr, char);
        kputc(c);
      } else if (ch == 'p') {
        // pointer
        uint64_t n = (uint64_t) va_arg(ptr, uintptr_t*);
        written += kprintf("%s", "0x");
        written += print_number(n, 8, false);
      } else if (ch == 'a' || ch == 'A') {
        // double in hex notation
      } else if (ch == 'n') {
        // number of written characters
        written += kprintf("%i", written);
      }
      fmt = false;
    } else {
      kputc(ch);
      written++;
    }
  next_char:
    c++;
  }
end_print:
  // Ending argument list traversal
  va_end(ptr);

  return written;
}