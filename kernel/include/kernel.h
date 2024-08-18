//
// Created by irrl on 8/12/23.
//

#ifndef GLADOS_KERNEL_H
#define GLADOS_KERNEL_H

#include "stdint.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

extern void disable_interrupts();

extern void enable_interrupts();

extern void halt();

typedef void (*isr_t)(void);

void register_interrupt_handler(uint8_t irq, isr_t handler);

unsigned char port_byte_in(unsigned short port);

void port_byte_out(unsigned short port, unsigned char data);

unsigned short port_word_in(unsigned short port);

void port_word_out(unsigned short port, unsigned short data);

#endif  //GLADOS_KERNEL_H
