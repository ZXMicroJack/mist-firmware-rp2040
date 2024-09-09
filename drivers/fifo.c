#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fifo.h"

void fifo_Init(fifo_t *f, uint8_t *buf, uint16_t len) {
  f->l = f->r = 0;
  f->m = len - 1;
  f->buf = buf;
}

int fifo_Get(fifo_t *f) {
  int r = -1;
  if (f->l != f->r) {
    r = f->buf[f->l++];
    f->l &= f->m;
    // f->c --;
  }
  return r;
}

void fifo_Put(fifo_t *f, uint8_t ch) {
  int r = -1;
  uint16_t rn = (f->r + 1) & f->m;
  if (rn != f->l) {
    f->buf[f->r] = ch;
    f->r = rn;
    // f->c ++;
  }
}

uint16_t fifo_Count(fifo_t *f) {
  if (f->r >= f->l)
    return f->r - f->l;
  else
    return f->r + f->m + 1 - f->l;
  // return f->c;
}

uint16_t fifo_Space(fifo_t *f) {
  return f->m - fifo_Count(f);
}

uint8_t fifo_Empty(fifo_t *f) {
  return f->l == f->r;
}