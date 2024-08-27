//
// Created by root on 8/19/24.
//

#ifndef GLADOS_TIME_H
#define GLADOS_TIME_H

#include "stdint.h"
#include "stdbool.h"

uint8_t read_rtc_register(int reg);
uint8_t bcd_to_binary(uint8_t bcd);

typedef struct rtc_time {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} rtc_time_t;

rtc_time_t get_bios_time();

// Time in unix time
uint64_t get_unix_time();

#endif  //GLADOS_TIME_H
