#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#ifdef PIODEBUG
#include "hardware/pio.h"
#endif

#define DEBUG
#include "debug.h"
#include "pins.h"

#ifdef DEBUG
void hexdump(uint8_t *buf, int len) {
  for (int i=0; i<len; i++) {
    debug(("%02X ", buf[i]));
    if ((i & 0xf) == 0xf) {
      debug(("\n"));
    }
  }
  debug(("\n"));
}
#endif

#ifdef PIODEBUG
#include "uart_tx.pio.h"

static PIO dbg_pio = DEBUG_PIO;
static uint dbg_sm = DEBUG_SM;

void debuginit() {
  static uint8_t started = 0;
  const uint SERIAL_BAUD = 115200;
  if (!started) {
    uint offset = pio_add_program(dbg_pio, &uart_tx_program);
    uart_tx_program_init(dbg_pio, dbg_sm, offset, GPIO_DEBUG_TX_PIN, SERIAL_BAUD);
    started = 1;
  }
}

static char str[256];
int dbgprintf(const char *fmt, ...) {
  int i;
  va_list argp;
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);

  debuginit();

  i=0;
  while (str[i]) {
    if (str[i] == '\n') uart_tx_program_putc(dbg_pio, dbg_sm, '\r');
    uart_tx_program_putc(dbg_pio, dbg_sm, str[i]);
    i++;
  }
  return i;
}
#endif
