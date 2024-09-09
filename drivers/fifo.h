#ifndef _FIFO_H
#define _FIFO_H

typedef struct {
  uint16_t l, r, m; //, c;
  uint8_t *buf;
} fifo_t;

int fifo_Get(fifo_t *f);
void fifo_Put(fifo_t *f, uint8_t ch);
void fifo_Init(fifo_t *f, uint8_t *buf, uint16_t len);
uint16_t fifo_Count(fifo_t *f);
uint16_t fifo_Space(fifo_t *f);
uint8_t fifo_Empty(fifo_t *f);

#endif
