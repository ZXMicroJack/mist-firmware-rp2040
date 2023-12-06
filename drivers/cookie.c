#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cookie.h"
#include "debug.h"

#define COOKIE_SIZE   16
#define COOKIE_LOC    (0x20000000 + (256*1024) - COOKIE_SIZE)

const uint8_t cookie[] = "MiCrOjAcKCoOkIe";

void cookie_Set() {
  memcpy((uint8_t *)COOKIE_LOC, cookie, COOKIE_SIZE);
}

void cookie_Reset() {
  memset((uint8_t *)COOKIE_LOC, 0x00, COOKIE_SIZE);
}

uint8_t cookie_IsPresent() {
  return !memcmp((uint8_t *)COOKIE_LOC, cookie, COOKIE_SIZE);
}
