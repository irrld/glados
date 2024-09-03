//
// Created by irrl on 8/19/24.
//

#include "time.h"
#include "kernel.h"

uint8_t read_rtc_register(unsigned char reg) {
  port_byte_out(CMOS_ADDRESS, reg);
  return port_byte_in(CMOS_DATA);
}

bool rtc_is_updating() {
  port_byte_out(CMOS_ADDRESS, 0x0A); // Select Status Register A
  return (port_byte_in(CMOS_DATA) & 0x80) != 0; // Check UIP bit
}

uint8_t read_rtc_register_safe(unsigned char reg) {
  // Wait until the RTC is not updating
  while (rtc_is_updating());
  return read_rtc_register(reg);
}

bool is_binary_;
bool is_binary() {
  return read_rtc_register_safe(0x0B) & 0x04;
}

uint8_t bcd_to_binary(uint8_t bcd) {
  if (is_binary_) {
    return bcd;
  }
  return (bcd >> 4) * 10 + (bcd & 0x0F);
}

rtc_time_t get_bios_time() {
  rtc_time_t time;

  time.second = bcd_to_binary(read_rtc_register_safe(0x00));
  time.minute = bcd_to_binary(read_rtc_register_safe(0x02));
  time.hour = bcd_to_binary(read_rtc_register_safe(0x04));
  time.day = bcd_to_binary(read_rtc_register_safe(0x07));
  time.month = bcd_to_binary(read_rtc_register_safe(0x08));
  time.year = bcd_to_binary(read_rtc_register_safe(0x09));
  uint8_t century = read_rtc_register_safe(0x32); // This might not be available on all systems
  if (century) {
    time.year += bcd_to_binary(century) * 100;
  } else {
    // Default to 2000s if century register is not available
    time.year += 2000;
  }
  return time;
}

// Helper function to check if a given year is a leap year
int is_leap_year(int year) {
  if (year % 4 == 0) {
    if (year % 100 == 0) {
      return year % 400 == 0;
    }
    return 1;
  }
  return 0;
}

// Total days for each month in a non-leap year
int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// Converts the RTC time structure to a Unix timestamp
uint64_t rtc_time_to_unix_timestamp(rtc_time_t rtc) {
  uint64_t timestamp = 0;
  int year, month;

  // Count the number of seconds in the years since 1970 up to the year before the current year
  for (year = 1970; year < rtc.year; year++) {
    timestamp += is_leap_year(year) ? 366 : 365;
  }

  // Add days from months in the current year
  for (month = 0; month < rtc.month - 1; month++) {
    timestamp += days_in_month[month];
    // Add one day if it's a leap year and the month is February
    if (month == 1 && is_leap_year(rtc.year)) {
      timestamp++;
    }
  }

  // Add the days passed in the current month
  timestamp += (rtc.day - 1);

  // Convert total days to seconds
  timestamp *= 86400;

  // Add hours, minutes, and seconds of the current day
  timestamp += rtc.hour * 3600 + rtc.minute * 60 + rtc.second;

  return timestamp;
}

uint64_t unix_time = 0;

void set_rtc_rate(int rate) {
  uint8_t prev = read_rtc_register(0x0A);
  port_byte_out(CMOS_DATA, (prev & 0xF0) | rate); // Set new rate
}

void time_init() {
  kprintf("Initializing time\n");
  disable_nmi();

  char prev = read_rtc_register_safe(0x0B);
  port_byte_out(CMOS_DATA, prev | 0x40);

  set_rtc_rate(0x06);

  is_binary_ = is_binary();
  rtc_time_t current_time = get_bios_time();
  unix_time = rtc_time_to_unix_timestamp(current_time);

  enable_nmi();
  kprintf("Time: %llu\n", unix_time);
}

void synchronize_time() {
  rtc_time_t current_time = get_bios_time();
  uint64_t new_unix_time = rtc_time_to_unix_timestamp(current_time);
  if (new_unix_time != unix_time) {
    //kprintf("Time corrected: %llu -> %llu\n", unix_time, new_unix_time);
    unix_time = new_unix_time;
  }
}

uint16_t divisor_ = 0;

// todo maybe use more accurate timers instead of RTC
void handle_rtc() {
  read_rtc_register(0x0C);
  divisor_++;
  // This number is just trial and error, I should try it on real hardware!
  // It's highly possible that QEMU isn't able to keep up the timer.
  if (divisor_ >= 745) {
    divisor_ = 0;
    unix_time++;
    //kprintf("Time: %llu\n", unix_time);
    if ((unix_time % 2) == 0) {
      synchronize_time();
    }
  }
}

uint64_t get_unix_time() {
  return unix_time;
}