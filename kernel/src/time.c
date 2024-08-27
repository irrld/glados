//
// Created by root on 8/19/24.
//

#include "time.h"
#include "kernel.h"

uint8_t read_rtc_register(int reg) {
  port_byte_out(CMOS_ADDRESS, reg);
  return port_byte_in(CMOS_DATA);
}

uint8_t bcd_to_binary(uint8_t bcd) {
  return ((bcd / 16) * 10) + (bcd & 0xF);
}

struct rtc_time get_bios_time() {
  struct rtc_time time;
  time.second = bcd_to_binary(read_rtc_register(0x00));
  time.minute = bcd_to_binary(read_rtc_register(0x02));
  time.hour = bcd_to_binary(read_rtc_register(0x04));
  time.day = bcd_to_binary(read_rtc_register(0x07));
  time.month = bcd_to_binary(read_rtc_register(0x08));
  time.year = bcd_to_binary(read_rtc_register(0x09)) + 2000;  // Assuming 21st century
  return time;
}

void time_init() {
  //port_byte_out(CMOS_ADDRESS, 0x8A);	// select Status Register A, and disable NMI (by setting the 0x80 bit)
  //port_byte_out(CMOS_DATA, 0x20);	// write to CMOS/RTC RAM
}

uint64_t get_now() {
  return 0LL;
}