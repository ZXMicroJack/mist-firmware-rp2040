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

#if defined(USB) && !defined(PIODEBUG)
uint8_t usbdebug = 1;
static char str[256];
uint16_t usb_cdc_write(const char *pData, uint16_t length);
int usbprintf(const char *fmt, ...) {
  int i;

  if (!usbdebug) return;

  va_list argp;
  va_start(argp, fmt);
  int l = vsprintf(str, fmt, argp);

  /* if bytes written to buffer */
  if (l >= 0) {
#if 1
    int c = 0;
    for (int i=0; i<l; i++) {
      if (str[i] == '\n') c++;
    }

    /* if \n found */
    if (c && (l+c) < sizeof str) {
      i = l;
      l = l + c;
      while (i) {
        str[i+c] = str[i];
        if (str[i] == '\n') { c--; str[i+c] = '\r'; }
        i--;
      }
    }
#endif
    usb_cdc_write(str, l);
  }
  return l;
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
#ifdef DEBUG_OFFSET
    pio_add_program_at_offset(dbg_pio, &uart_tx_program, DEBUG_OFFSET);
    uint offset = DEBUG_OFFSET;
#else
    uint offset = pio_add_program(dbg_pio, &uart_tx_program);
#endif
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
