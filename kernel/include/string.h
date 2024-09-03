//
// Created by irrl on 8/12/23.
//

#ifndef GLADOS_STRING_H
#define GLADOS_STRING_H

#include "stddef.h"

#define OCTAL 0x08
#define DECIMAL 0x0A
#define HEX 0x10

void* memcpy(void* dst, const void* src, size_t len) __attribute__((optimize(1)));
void* memset(void* dst, char value, size_t len)  __attribute__((optimize(1)));

// Converts an integer to a string
void itoa(int num, char* buf, int radix);

bool is_upper(char c);
bool is_number(char c);

// Converts ascii to integer, returns 0 if not a number
char atoi(char c);

#endif  //GLADOS_STRING_H

