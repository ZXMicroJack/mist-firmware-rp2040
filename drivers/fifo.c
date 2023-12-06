#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo.h"

void fifo_Init(fifo_t *f) {
  f->l = f->r = 0;
}

int fifo_Get(fifo_t *f) {
  int r = -1;
  if (f->l != f->r) {
    r = f->buf[f->l++];
    f->l &= FIFO_SIZE - 1;
  }
  return r;
}

void fifo_Put(fifo_t *f, uint8_t ch) {
  int r = -1;
  uint8_t rn = (f->r + 1) & (FIFO_SIZE - 1);
  if (rn != f->l) {
    f->buf[f->r] = ch;
    f->r = rn;
  }
}

