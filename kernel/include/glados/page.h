//
// Created by irrl on 8/28/24.
//

#ifndef GLADOS_PAGE_H
#define GLADOS_PAGE_H

#include "glados/stddef.h"

#define PAGE_PRESENT    0x01
#define PAGE_RW         0x02
#define PAGE_USER       0x04
#define PAGE_WRITE      0x80
#define PAGE_SIZE       0x1000
#define PAGE_MASK       0xFFFFFFFFFFFFF000

uintptr_t alloc_page();

void map_page(uintptr_t virtual_addr, uintptr_t physical_addr, uint64_t flags);

uintptr_t get_physical_address(uintptr_t virtual_addr);

void identity_map(uintptr_t start_addr, uint64_t size);

void paging_init(uintptr_t start_addr);

#endif  //GLADOS_PAGE_H
