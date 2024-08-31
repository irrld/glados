//
// Created by root on 8/27/24.
//

#ifndef GLADOS_MALLOC_H
#define GLADOS_MALLOC_H

#include "stdint.h"

void malloc_init();

void* malloc(size_t size);

void free(void* ptr);

#endif  //GLADOS_MALLOC_H
