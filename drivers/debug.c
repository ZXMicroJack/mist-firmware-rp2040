#include <stdio.h>

#define DEBUG
#include "debug.h"

#ifdef DEBUG
void hexdump(uint8_t *buf, int len) {
  for (int i=0; i<len; i++) {
    printf("%02X ", buf[i]);
    if ((i & 0xf) == 0xf) {
      printf("\n");
    }
  }
  printf("\n");
}
#endif

