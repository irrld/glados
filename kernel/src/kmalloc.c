//
// Created by root on 8/27/24.
//

#include "kmalloc.h"
#include "page.h"

/*
typedef struct heap_block_bm {
  struct heap_block_bm* next;
  uint32_t size;
  uint32_t used;
  uint32_t bsize;
  uint32_t lfb;
} heap_block_bm_t;

typedef struct heap_bm {
  heap_block_bm_t* fblock;
} heap_bm_t;

heap_bm_t kheap;*/

uintptr_t heap_start_;
size_t used_;
uintptr_t heap_end_;

void extend_heap() {
  map_page(heap_end_, alloc_page(), PAGE_RW);
  heap_end_ = heap_end_ + PAGE_SIZE;
}

void kmalloc_init() {
  kprintf("Initializing malloc\n", heap_start_);
  heap_start_ = 0x10000000;
  heap_end_ = heap_start_;
  extend_heap();
  kprintf("Heap will start from: 0x%llx\n", heap_start_);
}

void* kmalloc(size_t size) {
  uintptr_t heap_start = heap_start_ + used_;
  while ((heap_end_ - heap_start) < size) {
    extend_heap();
  }
  used_ += size;
  return (void*) heap_start;
}

void kfree(void* ptr) {
}




