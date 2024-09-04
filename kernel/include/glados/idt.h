//
// Created by root on 9/4/24.
//

#ifndef GLADOS_IDT_H
#define GLADOS_IDT_H

#include "glados/stddef.h"

void init_idt();
void send_eoi(uint8_t irq);

#endif  //GLADOS_IDT_H
