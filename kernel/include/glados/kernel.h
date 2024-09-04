//
// Created by irrl on 8/12/23.
//

#ifndef GLADOS_KERNEL_H
#define GLADOS_KERNEL_H

#include "stddef.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20		/* End-of-interrupt command code */

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define PIPELINE_MAX 256

#define INTERRUPT_GENERAL_PROTECTION_FAULT 13
#define INTERRUPT_PAGE_FAULT 14
#define INTERRUPT_TIMER 32
#define INTERRUPT_KEYBOARD 33
#define INTERRUPT_RTC 40

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define TSS_SELECTOR 0x18

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
void send_eoi(uint8_t irq);

void kernel_panic(const char* str) __attribute__((noreturn));

cpu_state_t* get_saved_cpu_state();

void irh_0();
void irh_13();
void irh_14();
void irh_32();
void irh_33();
void irh_34();
void irh_40();

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
unsigned short port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);

#endif  //GLADOS_KERNEL_H
