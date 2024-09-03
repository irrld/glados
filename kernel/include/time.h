//
// Created by root on 8/19/24.
//

#ifndef GLADOS_TIME_H
#define GLADOS_TIME_H

#include "stddef.h"

typedef struct rtc_time {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} rtc_time_t;

rtc_time_t get_bios_time();
uint64_t get_unix_time();

void time_init();
void handle_rtc();

#endif  //GLADOS_TIME_H
