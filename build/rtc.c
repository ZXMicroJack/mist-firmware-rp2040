#include <stdio.h>
#include <string.h>

#include "spi.h"
#include "hardware.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"

// #define DEBUG
#include "drivers/debug.h"

#include "rtc.h"

//RTC function
static uint8_t RTCSPI(uint8_t ctrl, uint8_t rtc[7]);

static uint8_t unbcd(uint8_t n) {
  return (10 * (n >> 4)) + (n & 0xf);
}

static uint8_t bcd(uint8_t n) {
  return ((n / 10) << 4) | (n % 10);
}


#ifdef DEBUG
char *day_text[] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

char *month_text[] = {
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec"
};

void debugDate(uint8_t *data) {
  uint8_t seconds = unbcd(data[0] & 0x7f);
  uint8_t minutes = unbcd(data[1] & 0x7f);
  uint8_t hour = unbcd(data[2] & 0x3f);
  uint8_t day = unbcd(data[3] & 0x3f);
  uint8_t weekday = data[4] & 0x7;
  uint8_t month = data[5] & 0x1f;
  uint8_t year = unbcd(data[6]);

  debug(("%s, %d %s %d %02d:%02d:%02d\n", day_text[weekday], day, month_text[month-1], 2000+year, hour, minutes, seconds));
}
#else
#define debugDate(d)
#endif

#define NRST    1
#define RTCGET  2
#define RTCSET  4

// rtc = ss mm hh DD WD MM YY, where WD MM is decimal, the rest are BCD
static uint8_t RTCSPI(uint8_t ctrl, uint8_t rtc[7]) {
  uint8_t data[10];

  debug(("RTCSPI: ctrl:%02X rtc:", ctrl));
  debugDate(rtc);

  EnableIO();

  data[0] = 0xfe; // RTC operations - unique to ZX3/N/N+/ZX1+
  data[1] = ctrl; // 1 = active low reset, 2 = rtc read, 3 = rtc write

  SPI(0xfe);
  ctrl = SPI(ctrl);

  for (int i=0; i<7; i++) {
    rtc[i] = SPI(rtc[i]);
  }
  SPI(0x00); // last byte
  DisableIO();

  debug(("RTCSPI: ret:%02X rtc:", ctrl));
  debugDate(rtc);

  return ctrl;
}

static uint8_t sync_hw_get_pending = 0;
static uint8_t sync_hw_set_pending = 0;

void rtc_Init() {
  debug(("rtc_Init: startup rp2040 RTC\n"));
  // Start the RTC
  rtc_init();

  // Start on Friday 5th of June 2020 15:45:00
  datetime_t t = {
    .year= 2020,
    .month = 06,
    .day= 05,
    .dotw= 5, // 0 is Sunday, so 5 is Friday
    .hour= 15,
    .min= 45,
    .sec= 00
  };
  rtc_set_datetime(&t);
  sync_hw_get_pending = 1;
}

uint8_t rtc_SetInternal() {
  uint8_t d[7], od[7];
  datetime_t t;

  memset(d, 0, sizeof d);
  uint8_t ctrl = RTCSPI(NRST, d);
  if (ctrl != 0xfe) {
    // RTC functionality not implemented
    debug(("rtc_SetInternal: returns %02X - not RTC not implemented\n", ctrl));
    return 1;
  }
  sleep_us(100);

  rtc_get_datetime(&t);
  d[0] = bcd(t.sec);
  d[1] = bcd(t.min);
  d[2] = bcd(t.hour);
  d[3] = bcd(t.day);
  d[4] = t.dotw + 1;
  d[5] = t.month;
  d[6] = bcd(t.year - 2000);
  memcpy(od, d, sizeof od);
  RTCSPI(NRST, d);
  sleep_us(100);
  memcpy(d, od, sizeof od);
  RTCSPI(NRST|RTCSET, d);

  sleep_ms(10);

  memcpy(d, od, sizeof od);
  RTCSPI(0, d);
  debug(("rtc_SetInternal: All good, return RTC control to reset state\n", ctrl));
  return 0;
}

uint8_t rtc_GetInternal() {
  uint8_t d[7];
  datetime_t t;

  memset(d, 0, sizeof d);
  uint8_t ctrl = RTCSPI(NRST, d);
  if (ctrl != 0xfe) {
    // RTC functionality not implemented
    debug(("rtc_GetInternal: returns %02X - not RTC not implemented\n", ctrl));
    return 1;
  }
  sleep_us(100);
  RTCSPI(NRST|RTCGET, d);
  sleep_ms(10);
  RTCSPI(NRST, d);

  t.sec = unbcd(d[0] & 0x7f);
  t.min = unbcd(d[1] & 0x7f);
  t.hour = unbcd(d[2] & 0x3f);
  t.day = unbcd(d[3] & 0x3f);
  t.dotw = (d[4] & 0x7) - 1;
  t.month = d[5] & 0x1f;
  t.year = unbcd(d[6]) + 2000;
  RTCSPI(0, d);
  bool ret = rtc_set_datetime(&t);
  debug(("SetRTC: %x %x %x %x %d %d %x %s\n", d[0], d[1], d[2], d[3], d[4], d[5], d[6], ret ? "OK" : "FAILED"));
  debug(("rtc_GetInternal: All good, return RTC control to reset state\n", ctrl));
  return 0;
}


void rtc_AttemptSync() {
  if (sync_hw_set_pending) {
    debug(("rtc_AttemptSync: Attempting to set HW RTC...\n"));
    if (!rtc_SetInternal()) sync_hw_set_pending = 0;
    // sync_hw_set_pending = 0;
  }

  if (sync_hw_get_pending) {
    debug(("rtc_AttemptSync: Attempting to get HW RTC...\n"));
    if (!rtc_GetInternal()) sync_hw_get_pending = 0;
    // sync_hw_get_pending = 0;
  }
}

// MiST layer set/get rtc functions
char GetRTC(unsigned char *d) {
  datetime_t t;
  // implemented as d[0-7] -
  //   [y-100] [m] [d] [H] [M] [S] [Day1-7]

  if (rtc_get_datetime(&t)) {
    d[0] = t.year - 1900;
    d[1] = t.month;
    d[2] = t.day;
    d[3] = t.hour;
    d[4] = t.min;
    d[5] = t.sec;
    d[6] = t.dotw + 1;
  } else {
    memset(d, 0, 7);
  }
  debug(("GetRTC: %d %d %d %d %d %d %d\n", d[0], d[1], d[2], d[3], d[4], d[5], d[6]));
  return 1;
}

char SetRTC(unsigned char *d) {
  debug(("SetRTC: %d %d %d %d %d %d %d\n", d[0], d[1], d[2], d[3], d[4], d[5], d[6]));
  datetime_t t = {
    .year = d[0] + 1900,
    .month = d[1],
    .day= d[2],
    .dotw= d[6] - 1, // 0 is Sunday, so 5 is Friday
    .hour= d[3],
    .min= d[4],
    .sec= d[5]
  };
  rtc_set_datetime(&t);
  sync_hw_set_pending = 1;
  rtc_AttemptSync();
  return 1;
}
