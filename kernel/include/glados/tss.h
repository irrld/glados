//
// Created by root on 9/5/24.
//

#ifndef GLADOS_TSS_H
#define GLADOS_TSS_H

#include "glados/stddef.h"

typedef struct tss_entry {
  uint32_t reserved1;
  uint64_t rsp0;  // Stack pointer for ring 0
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved2;
  uint64_t ist1;  // Stack pointers for IST
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved3;
  uint16_t reserved4;
  uint16_t iopb_offset;
} __attribute__((packed)) tss_entry_t;


void init_tss();

tss_entry_t* get_tss();

#endif  //GLADOS_TSS_H
