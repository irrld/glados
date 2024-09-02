//
// Created by root on 8/28/24.
//

#include "page.h"
#include "kernel.h"
#include "stddef.h"
#include "string.h"
#include "video.h"

typedef struct pgd_entry {
  uint8_t present : 1;
  uint8_t rw : 1; // Read/Write Select
  uint8_t us : 1; // User/Supervisor select
  uint8_t pwt : 1; // Write through
  uint8_t pcd : 1; // Cache disable
  uint8_t a : 1; // Accessed
  uint8_t u1 : 1; // Ignored
  uint8_t u2 : 1; // Reserved
  uint8_t u3 : 4; // Ignored
  uint64_t addr : 39; // PUD Physical Address
  uint8_t u4 : 1; // Reserved
  uint16_t u5 : 11; // Available
  uint8_t nx : 1; // Execute disable
} pgd_entry_t;

typedef struct pud_entry {
  uint8_t present : 1;
  uint8_t rw : 1; // Read/Write Select
  uint8_t us : 1; // User/Supervisor select
  uint8_t pwt : 1; // Write through
  uint8_t pcd : 1; // Cache disable
  uint8_t a : 1; // Accessed
  uint8_t u1 : 1; // Ignored
  uint8_t page_size : 1; // Reserved
  uint8_t u2 : 4; // Ignored
  uint64_t addr : 39; // PMD Physical Address
  uint8_t u3 : 1; // Reserved
  uint16_t u4 : 11; // Available
  uint8_t nx : 1; // Execute disable
} pud_entry_t;

typedef struct pmd_entry {
  uint8_t present : 1;
  uint8_t rw : 1; // Read/Write Select
  uint8_t us : 1; // User/Supervisor select
  uint8_t pwt : 1; // Write through
  uint8_t pcd : 1; // Cache disable
  uint8_t a : 1; // Accessed
  uint8_t u1 : 1; // Ignored
  uint8_t page_size : 1; // Reserved
  uint8_t u2 : 4; // Ignored
  uint64_t addr : 39; // PMD Physical Address
  uint8_t u3 : 1; // Reserved
  uint16_t u4 : 11; // Available
  uint8_t nx : 1; // Execute disable
} pmd_entry_t;

typedef struct pt_entry {
  uint8_t present : 1;
  uint8_t rw : 1; // Read/Write Select
  uint8_t us : 1; // User/Supervisor select
  uint8_t pwt : 1; // Write through
  uint8_t pcd : 1; // Cache disable
  uint8_t a : 1; // Accessed
  uint8_t dirty : 1; // Dirty
  uint8_t pat : 1; // Page Attribute Table
  uint8_t global : 1; // Ignored
  uint8_t u1 : 3; // Ignored
  uint64_t addr : 39; // PMD Physical Address
  uint8_t u3 : 1; // Reserved
  uint16_t u4 : 7; // Available
  uint16_t mpk : 4; // Memory Protection Key
  uint8_t nx : 1; // Execute disable
} pt_entry_t;


#define PHYS_MEM_SIZE  0x200000
#define EXTRA_SPACE 0x16000 // 16 pages

uintptr_t first_free_page_;
uintptr_t next_free_page_;
uintptr_t last_free_page_;
uint64_t* pml4;
bool extend_lock_ = false;

uintptr_t alloc_page() {
  uintptr_t available_space = last_free_page_ - next_free_page_;
  // Make sure we always have enough pages.
  if (!extend_lock_ && available_space < EXTRA_SPACE) {
    extend_lock_ = true;
    identity_map(last_free_page_, EXTRA_SPACE);
    last_free_page_ += EXTRA_SPACE;
    extend_lock_ = false;
  }
  uintptr_t allocated_page = next_free_page_;
  next_free_page_ += PAGE_SIZE;
  if (allocated_page > last_free_page_) {
    kernel_panic("Cannot allocate more pages!");
  }
  return allocated_page;
}

void flush_tlb_entry(uintptr_t address) {
  __asm__ __volatile__("invlpg (%0)" : : "r"(address) : "memory");
}

static inline uintptr_t read_cr3() {
  uintptr_t cr3_value;
  // Inline assembly to read CR3
  __asm__ __volatile__("mov %%cr3, %0" : "=r" (cr3_value));
  return cr3_value;
}

void map_page(uintptr_t virtual_addr, uintptr_t physical_addr, uint64_t flags) {
  if ((virtual_addr % 0x1000) != 0 || (physical_addr % 0x1000) != 0) {
    kernel_panic("Addresses should be aligned with the page size!");
  }
  uint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
  uint64_t pdpt_index = (virtual_addr >> 30) & 0x1FF;
  uint64_t pdt_index = (virtual_addr >> 21) & 0x1FF;
  uint64_t pt_index = (virtual_addr >> 12) & 0x1FF;

  uint64_t* pdpt;
  uint64_t* pdt;
  uint64_t* pt;

  // Get or create PDPT
  if (!(pml4[pml4_index] & PAGE_PRESENT)) {
    pdpt = (uint64_t*)alloc_page();
    memset(pdpt, 0, 8);
    pml4[pml4_index] = ((uintptr_t)pdpt) | flags | PAGE_PRESENT;
  } else {
    pdpt = (uint64_t*)(pml4[pml4_index] & PAGE_MASK);
  }

  // Get or create PDT
  if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
    pdt = (uint64_t*)alloc_page();
    memset(pdt, 0, 8);
    pdpt[pdpt_index] = ((uintptr_t)pdt) | flags | PAGE_PRESENT;
  } else {
    pdt = (uint64_t*)(pdpt[pdpt_index] & PAGE_MASK);
  }

  // Get or create PT
  if (!(pdt[pdt_index] & PAGE_PRESENT)) {
    pt = (uint64_t*)alloc_page();
    memset(pt, 0, 8);
    pdt[pdt_index] = ((uintptr_t)pt) | flags | PAGE_PRESENT;
  } else {
    pt = (uint64_t*)(pdt[pdt_index] & PAGE_MASK);
  }

  if (pt[pt_index] & PAGE_PRESENT) {
    uint64_t previous_address = pt[pt_index] & PAGE_MASK;
    if (previous_address != physical_addr) {
      kernel_panic("Tried to map page that already exists!");
    }
  }

  // Set the physical address in the page table
  pt[pt_index] = (physical_addr & PAGE_MASK) | flags | PAGE_PRESENT;
  flush_tlb_entry(virtual_addr);
  uintptr_t located = get_physical_address(virtual_addr);
  if (located != physical_addr) {
    kernel_panic("Mapped page did not match the requested address");
  }
}

uintptr_t get_physical_address(uintptr_t virtual_addr) {
  uint64_t* pdpt;
  uint64_t* pdt;
  uint64_t* pt;

  uint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
  uint64_t pdpt_index = (virtual_addr >> 30) & 0x1FF;
  uint64_t pdt_index = (virtual_addr >> 21) & 0x1FF;
  uint64_t pt_index = (virtual_addr >> 12) & 0x1FF;

  // Check PML4
  if (!(pml4[pml4_index] & PAGE_PRESENT)) {
    return 0; // Page not present
  }
  pdpt = (uint64_t*)(pml4[pml4_index] & PAGE_MASK);

  // Check PDPT
  if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
    return 0; // Page not present
  }
  pdt = (uint64_t*)(pdpt[pdpt_index] & PAGE_MASK);

  // Check PDT
  if (!(pdt[pdt_index] & PAGE_PRESENT)) {
    return 0; // Page not present
  }
  pt = (uint64_t*)(pdt[pdt_index] & PAGE_MASK);

  // Check PT
  if (!(pt[pt_index] & PAGE_PRESENT)) {
    return 0; // Page not present
  }

  // Calculate physical address
  uintptr_t page_offset = virtual_addr & ~PAGE_MASK;
  uintptr_t physical_addr = (pt[pt_index] & PAGE_MASK) | page_offset;

  return physical_addr;
}

void identity_map(uintptr_t start_addr, uint64_t size) {
  uint64_t end_addr = start_addr + size;
  for (uint64_t addr = start_addr; addr < end_addr; addr += PAGE_SIZE) {
    map_page(addr, addr, PAGE_RW);
  }
}

void paging_init(uintptr_t start_addr) {
  kprintf("Initializing paging\n");
  kprintf("Paging will start from 0x%x\n", start_addr);
  pml4 = (uint64_t*) read_cr3();
  first_free_page_ = start_addr;
  next_free_page_ = start_addr;
  last_free_page_ = start_addr + PHYS_MEM_SIZE;
  // Before 0x200000 was identity mapped by the bootloader itself.
  identity_map(0x200000, PHYS_MEM_SIZE);
}

