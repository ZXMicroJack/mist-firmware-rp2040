#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cookie.h"
#include "debug.h"

#define COOKIE_SIZE   16
#define COOKIE_LOC    (0x20000000 + (256*1024) - COOKIE_SIZE)
#define COOKIE_LOC2    (0x20000000 + (256*1024) - 2*COOKIE_SIZE)

const uint8_t cookie[] = "MiCrOjAcKCoOkIe";
const uint8_t cookie2[] = "mIcRoJaCkcOoKiE";

void cookie_Set() {
  memcpy((uint8_t *)COOKIE_LOC, cookie, COOKIE_SIZE);
}

void cookie_Set2(uint8_t data) {
  if (data == 1) {
    memcpy((uint8_t *)COOKIE_LOC2, cookie, COOKIE_SIZE);
  } else if (data == 2) {
    memcpy((uint8_t *)COOKIE_LOC2, cookie2, COOKIE_SIZE);
  } else {
    memset((uint8_t *)COOKIE_LOC2, 0x00, COOKIE_SIZE);
  }
}

void cookie_Reset() {
  memset((uint8_t *)COOKIE_LOC, 0x00, COOKIE_SIZE);
}

uint8_t cookie_IsPresent() {
  return !memcmp((uint8_t *)COOKIE_LOC, cookie, COOKIE_SIZE);
}

uint8_t cookie_IsPresent2() {
  return !memcmp((uint8_t *)COOKIE_LOC2, cookie, COOKIE_SIZE) ? 1 :
         !memcmp((uint8_t *)COOKIE_LOC2, cookie2, COOKIE_SIZE) ? 2 : 0;
}

//TODO move to central position
#ifdef USE_COOKIE3
#define RP2U_MIST_COOKIE         0x10101000

uint8_t cookie_IsPresent3() {
#ifdef SWITCHABLE_MIST
  return !memcmp((uint8_t *)RP2U_MIST_COOKIE, cookie, COOKIE_SIZE);
#else
  return 1;
#endif
}
#endif
