#include <string.h>
#include <time.h>

#include "pico/stdlib.h"

#include "midi.h"

#define USE_IRQ

#define BAUDRATE 31250 // midi baudrate

#ifdef USE_IRQ

#if PICO_NO_FLASH
#define CBUFF_SIZE  128
#define CBUFF_SIZE_MASK   0x7f
#else
#define CBUFF_SIZE  2048
#define CBUFF_SIZE_MASK   0x7ff
#endif

typedef struct {
  uint16_t l;
  uint16_t r;
  uint8_t buff[CBUFF_SIZE];
} cbuff_t;

cbuff_t ur;

#define cbf_inc(a) (((a) + 1) & CBUFF_SIZE_MASK)


static void midi_irq() {
  if (uart_is_readable(uart1) && (cbf_inc(ur.r) != ur.l)) {
    uint8_t ch = uart_getc(uart1);
    if (cbf_inc(ur.r) != ur.l) {
      ur.buff[ur.r] = ch;
      ur.r = cbf_inc(ur.r);
    }
  }
}
#endif

void midi_setbaud(uint32_t baud) {
  uart_set_baudrate (uart1, baud ? baud : BAUDRATE);
}

void midi_init(void) {
  gpio_set_function(4, GPIO_FUNC_UART);
  gpio_set_function(5, GPIO_FUNC_UART);

  uart_init (uart1, BAUDRATE);
#ifdef USE_IRQ
  ur.l = ur.r = 0;
  uart_set_fifo_enabled(uart1, false);
#else
  uart_set_fifo_enabled(uart1, true);
#endif
  
  uart_set_hw_flow(uart1, false, false);

#ifdef USE_IRQ
  irq_set_exclusive_handler(UART1_IRQ, midi_irq);
  irq_set_enabled(UART1_IRQ, true);
  uart_set_irq_enables(uart1, true, false);
#endif
}

int midi_getc() {
#ifdef USE_IRQ
  if (ur.l != ur.r) {
    int ch = ur.buff[ur.l];
    ur.l = cbf_inc(ur.l);
    return ch;
  }
  return -1;
#else
  uart_getc(uart1);
#endif
}

int midi_get(uint8_t *d, int len) {
#ifndef USE_IRQ
  uart_read_blocking(uart1, d, len);
  return len;
#else
  int read = 0;
  
  while (ur.l != ur.r && len--) {
    *d++ = ur.buff[ur.l];
    ur.l = cbf_inc(ur.l);
    read++;
  }
  return read;
#endif
}

int midi_isdata() {
#ifndef USE_IRQ
  return uart_is_readable(uart1);
#else
  return ur.l > ur.r ? (ur.r + CBUFF_SIZE - ur.l) : (ur.r - ur.l);
#endif
}
