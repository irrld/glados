//
// Created by irrl on 8/27/24.
//

#ifndef GLADOS_MALLOC_H
#define GLADOS_MALLOC_H

#include "glados/stddef.h"

void kmalloc_init();

void* kmalloc(size_t size);

void kfree(void* ptr);

#endif  //GLADOS_MALLOC_H
