#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pins.h"
#include "ps2.h"
#include "gpioirq.h"
#include "jamma.h"
//#define DEBUG
#include "debug.h"

#include "jammadb9.pio.h"
#include "jamma.pio.h"

#ifdef JAMMA_JAMMA
#include "jammaj.pio.h"
#endif

uint16_t reload_data = 0x0000;
uint8_t joydata[2];

static PIO jamma_pio = JAMMA_PIO;
static unsigned jamma_sm = JAMMA_SM;
#ifdef JAMMA_JAMMA
static unsigned jamma2_sm = JAMMA2_SM;
#endif
static uint jamma_offset;
#ifdef JAMMA_JAMMA
static uint jamma_offset2;
#endif

static uint8_t detect_jamma_bitshift;
static uint8_t jamma_bitshift;

static uint32_t db9_data;
static uint8_t db9_Changed;
static uint32_t db9_debounce;
static uint32_t db9_debounce_data;

#ifdef JAMMA_JAMMA
static uint32_t jamma_data;
static uint32_t jamma_debounce;
static uint32_t jamma_debounce_data;
static uint8_t jamma_Changed;
#endif

#define DEBOUNCE_COUNT    120

static void init_jamma() {
  db9_data = 0;
  db9_Changed = 0;
  db9_debounce = 0;
  db9_debounce_data = 0;

#ifdef JAMMA_JAMMA
  jamma_data = 0;
  jamma_debounce = 0;
  jamma_debounce_data = 0;
  jamma_Changed = 0;
#endif
}

static void do_debounce(uint32_t _data, uint32_t *data, uint32_t *debounce_data, uint32_t *debounce, uint8_t *changed) {
  if (*debounce_data != _data) {
    *debounce = DEBOUNCE_COUNT;
    *debounce_data = _data;
  }

  if (*debounce) {
    (*debounce)--;
    if (!*debounce) {
      *changed = _data != *data;
      *data = _data;
    }
  }
}

static void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == GPIO_RP2U_XLOAD) {
    while (!pio_sm_is_tx_fifo_full(jamma_pio, jamma_sm))
      pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data);
    
#ifdef JAMMA_JAMMA
    while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma2_sm)) {
      uint32_t _data = pio_sm_get_blocking(jamma_pio, jamma2_sm);
      do_debounce(_data, &jamma_data, &jamma_debounce_data, &jamma_debounce, &jamma_Changed);
      if (detect_jamma_bitshift) {
        detect_jamma_bitshift --;
        if (!detect_jamma_bitshift) {
          jamma_bitshift = 0;
          while (!(_data & 1)) {
            jamma_bitshift ++;
            _data >>= 1;
          }
        }
      }
    }
#endif
  }
}

#define MAX_JOYSTATES 16

uint32_t joystates[MAX_JOYSTATES];
uint32_t pio_ints = 0;
uint32_t nrstates = 0;

void debug_joystates() {
  printf("pio_ints = %d; nrstates = %d\n", pio_ints, nrstates);
#if 0
  for (int i=0; i<MAX_JOYSTATES; i++) {
    printf("%d = %08X\n", i, joystates[i]);
    joystates[i] = 0;
  }
#endif
  int i;
  uint32_t _data;
  while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma_sm)) {
    _data = pio_sm_get_blocking(jamma_pio, jamma_sm);
    joystates[i & (MAX_JOYSTATES-1)] = _data;
    i ++;
  }
  for (int i=0; i<MAX_JOYSTATES; i++) {
    printf("%d = %08X\n", i, joystates[i]);
    joystates[i] = 0;
  }

}

static void pio_callback() {
  int i = 0;
  uint32_t _data;

  while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma_sm)) {
    _data = pio_sm_get_blocking(jamma_pio, jamma_sm);
    joystates[i & (MAX_JOYSTATES-1)] = _data;
    i ++;
    
    // do_debounce(_data, &db9_data, &db9_debounce_data, &db9_debounce, &db9_Changed);
    // joydata[0] = ~(db9_data >> (jamma_bitshift + 8));
    // joydata[1] = ~(db9_data >> jamma_bitshift);
  }
  nrstates = i;
  pio_ints ++;
  pio_interrupt_clear (jamma_pio, 0);

#ifdef JAMMA_JAMMA
  if (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma2_sm)) {
    _data = pio_sm_get_blocking(jamma_pio, jamma2_sm);
    do_debounce(_data, &jamma_data, &jamma_debounce_data, &jamma_debounce, &jamma_Changed);
  }
#endif
}

static uint8_t inited = 0;

void jamma_InitDB9() {
  /* don't need to detect shifting */
  init_jamma();
  detect_jamma_bitshift = 0;
  jamma_bitshift = 8;

#ifndef NO_JAMMA
  debug(("jamma_Init: DB9 mode\n"));
  if (inited) return;
  gpio_init(GPIO_RP2U_XLOAD);
  gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_OUT);

  gpio_init(GPIO_RP2U_XSCK);
  gpio_set_dir(GPIO_RP2U_XSCK, GPIO_OUT);

  gpio_init(GPIO_RP2U_XDATA);
  gpio_set_dir(GPIO_RP2U_XDATA, GPIO_IN);

#ifdef JAMMA_JAMMA
  gpio_init(GPIO_RP2U_XDATAJAMMA);
  gpio_set_dir(GPIO_RP2U_XDATAJAMMA, GPIO_IN);
#endif

#ifdef JAMMA_OFFSET
  pio_add_program_at_offset(jamma_pio, &jammadb9_program, JAMMA_OFFSET);
  jamma_offset = JAMMA_OFFSET;
#else
  jamma_offset = pio_add_program(jamma_pio, &jammadb9_program);
#endif
  jammadb9_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XLOAD, GPIO_RP2U_XDATA);
#ifdef JAMMA_JAMMA
  jammadb9_program_init(jamma_pio, jamma2_sm, jamma_offset, GPIO_RP2U_XLOAD, GPIO_RP2U_XDATAJAMMA);
#endif
  pio_sm_clear_fifos(jamma_pio, jamma_sm);
#ifdef JAMMA_JAMMA
  pio_sm_clear_fifos(jamma_pio, jamma2_sm);
#endif
  irq_set_exclusive_handler (JAMMA_PIO_IRQ, pio_callback);
  pio_set_irq0_source_enabled(jamma_pio, pis_interrupt0, true);
#ifdef JAMMA_JAMMA
  pio_set_irq0_source_enabled(jamma_pio, jamma2_sm, true);
#endif
  irq_set_enabled (JAMMA_PIO_IRQ, true);
  inited = 2;
#endif
}

void jamma_InitUSB() {
  init_jamma();
  detect_jamma_bitshift = 20;
  jamma_bitshift = 0;

#ifndef NO_JAMMA
  debug(("jamma_Init: USB mode\n"));
  if (inited) return;
  gpio_init(GPIO_RP2U_XLOAD);
  gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_IN);

  gpio_init(GPIO_RP2U_XSCK);
  gpio_set_dir(GPIO_RP2U_XSCK, GPIO_IN);

  gpio_init(GPIO_RP2U_XDATA);
  gpio_set_dir(GPIO_RP2U_XDATA, GPIO_OUT);

#ifdef JAMMA_JAMMA
  gpio_init(GPIO_RP2U_XDATAJAMMA);
  gpio_set_dir(GPIO_RP2U_XDATAJAMMA, GPIO_IN);
#endif
#ifdef JAMMA_OFFSET
  pio_add_program_at_offset(jamma_pio, &jamma_program, JAMMA_OFFSET);
  jamma_offset = JAMMA_OFFSET;
#ifdef JAMMA_JAMMA
  pio_add_program_at_offset(jamma_pio, &jammaj_program, JAMMA_OFFSET + JAMMAU_INSTR);
  jamma_offset2 = JAMMA_OFFSET + JAMMAU_INSTR;
#endif
#else
  jamma_offset = pio_add_program(jamma_pio, &jamma_program);
#endif
  jamma_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XDATA);
  pio_sm_clear_fifos(jamma_pio, jamma_sm);
#ifdef JAMMA_JAMMA
  jammaj_program_init(jamma_pio, jamma2_sm, jamma_offset2, GPIO_RP2U_XDATAJAMMA);
  pio_sm_clear_fifos(jamma_pio, jamma2_sm);
#endif

  gpioirq_SetCallback(IRQ_JAMMA, gpio_callback);
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, true);

  // pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data);d
  pio_interrupt_clear (jamma_pio, 0);
  inited = 1;
#endif
}

void jamma_Kill() {
#ifndef NO_JAMMA
  if (!inited) return;
  // disable interrupts
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, false);
  pio_set_irq0_source_enabled(jamma_pio, jamma_sm, false);
#ifdef JAMMA_JAMMA
  pio_set_irq0_source_enabled(jamma_pio, jamma2_sm, false);
#endif
  gpioirq_SetCallback(IRQ_JAMMA, NULL);

  // shutdown sm
  pio_sm_set_enabled(jamma_pio, jamma_sm, false);
#ifdef JAMMA_JAMMA
  pio_sm_set_enabled(jamma_pio, jamma2_sm, false);
#endif
  if (inited == 2) {
    pio_remove_program(jamma_pio, &jammadb9_program, jamma_offset);
  } else {
    pio_remove_program(jamma_pio, &jamma_program, jamma_offset);
#ifdef JAMMA_JAMMA
    pio_remove_program(jamma_pio, &jammaj_program, jamma_offset2);
#endif
  }

  // reset pins
  uint8_t lut[] = {GPIO_RP2U_XLOAD, GPIO_RP2U_XDATA, GPIO_RP2U_XSCK};
  for (int i=0; i<sizeof lut / sizeof lut[0]; i++) {
    gpio_init(lut[i]);
    gpio_set_dir(lut[i], GPIO_IN);
  }

  inited = 0;
#endif
}

void jamma_Init() {
  jamma_InitDB9();
}

uint8_t jamma_GetMisterMode() {
  return inited == 2 ? 0x01 : inited == 1 ? 0x00 : 0xff ;
}

void jamma_InitEx(uint8_t mister) {
  debug(("jamma_InitEx: mister = %d\n", mister));
  if (!mister) {
    jamma_InitUSB();
  } else if (mister != 0xff) {
    jamma_InitDB9();
  }
}

void jamma_SetData(uint8_t inst, uint32_t data) {
  debug(("jamma_SetData: inst %d data %08X\n", inst, data));
  joydata[inst] = data & 0xff;
  reload_data = (joydata[1] << 8) | joydata[0];
}

uint32_t jamma_GetData(uint8_t inst) {
  debug(("jamma_GetData: %d returns %02X\n", inst, joydata[inst]));
  return joydata[inst];
}

int jamma_HasChanged() {
  uint8_t changed = db9_Changed;
  db9_Changed = 0;
  return changed;
}

uint32_t jamma_GetDataAll() {
  return db9_data >> jamma_bitshift;
}

#ifdef JAMMA_JAMMA
uint32_t jamma_GetJamma() {
  return jamma_data >> jamma_bitshift;
}

uint8_t jamma_GetDepth() {
  return 32 - jamma_bitshift - 1;
}

int jamma_HasChangedJamma() {
  uint8_t changed = jamma_Changed;
  jamma_Changed = 0;
  return changed;
}
#endif
