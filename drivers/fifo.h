#ifndef _FIFO_H
#define _FIFO_H

#define FIFO_SIZE   64

typedef struct {
  uint8_t buf[FIFO_SIZE];
  uint8_t l, r;
} fifo_t;

int fifo_Get(fifo_t *f);
void fifo_Put(fifo_t *f, uint8_t ch);
void fifo_Init(fifo_t *f);

#endif
