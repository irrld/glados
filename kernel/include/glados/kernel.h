//
// Created by irrl on 8/12/23.
//

#ifndef GLADOS_KERNEL_H
#define GLADOS_KERNEL_H

#include "stddef.h"
#include "kdef.h"

typedef struct cpu_state {
  uint64_t rax, rbx, rcx, rdx;
  uint64_t rbp, rsp, rsi, rdi;
  uint64_t r8, r9, r10, r11;
  uint64_t r12, r13, r14, r15;

  uint64_t rip;
  uint64_t rflags;

  uint64_t cs;
  uint64_t ss;
} cpu_state_t;

extern void disable_interrupts();
extern void enable_interrupts();
extern void halt();
void enable_nmi();
void disable_nmi();

void kernel_panic(const char* str) __attribute__((noreturn));

cpu_state_t* get_saved_cpu_state();

void irh_common(uint16_t irq);
void irh_0();
void irh_13();
void irh_14();
void irh_32();
void irh_33();
void irh_40();

typedef void(*irh_t)(uint16_t);

void register_irh(uint16_t irq, irh_t fn);

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
unsigned short port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);

#endif  //GLADOS_KERNEL_H
