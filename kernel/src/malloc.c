//
// Created by root on 8/27/24.
//

#include "malloc.h"

#define HEAP_SIZE 0x20000

uint8_t heap[HEAP_SIZE];
uint64_t heap_top = 0;

void* malloc(uint64_t size) {
  if (heap_top + size >= HEAP_SIZE) {
    // Out of memory
    return NULL;
  }
  void* ptr = &heap[heap_top];
  heap_top += size;
  return ptr;
}

void free(void* ptr) {
  // fuck
}