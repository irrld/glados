//
// Created by root on 9/5/24.
//

#ifndef GLADOS_PIT_H
#define GLADOS_PIT_H

#include "glados/stddef.h"

uint32_t get_timer_frequency();

void init_pit(uint32_t frequency);

#endif  //GLADOS_PIT_H
