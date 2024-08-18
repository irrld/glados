//
// Created by irrl on 8/12/23.
//

#ifndef TANTALUM_STRING_H
#define TANTALUM_STRING_H

#include "stdint.h"

void* memcpy(void* dst, const void* src, size_t len);
void* memset(void* dst, char value, size_t len);

// Converts an integer to a string
void itoa(int num, char *str, int base);

#endif  //TANTALUM_STRING_H

