#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pins.h"
#include "ps2.h"
#include "jamma.h"
// #define DEBUG
#include "debug.h"



#include "jammadb9.pio.h"
#include "jamma.pio.h"

uint16_t reload_data = 0x0000;
uint8_t joydata[2];

static PIO jamma_pio = JAMMA_PIO;
static unsigned jamma_sm = JAMMA_SM;
static uint jamma_offset;

static void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == GPIO_RP2U_XLOAD) {
    while (!pio_sm_is_tx_fifo_full(jamma_pio, jamma_sm))
      pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data); // 
  }
}

static uint32_t data;
static uint8_t jamma_Changed = 0;
static uint32_t debounce = 0;
static uint32_t debounce_data;

#define DEBOUNCE_COUNT    120

static void pio_callback() {
  uint32_t _data;
  pio_interrupt_clear (jamma_pio, 0);

  _data = pio_sm_get_blocking(jamma_pio, jamma_sm);
  if (debounce_data != _data) {
    debounce = DEBOUNCE_COUNT;
    debounce_data = _data;
  }

  if (debounce) {
    debounce--;
    if (!debounce) {
      jamma_Changed = _data != data;
      data = _data;
    }
  }

  joydata[0] = ~(data >> 24);
  joydata[1] = ~(data >> 16);
}

static uint8_t inited = 0;

void jamma_InitDB9() {
#ifndef NO_JAMMA
  debug(("jamma_Init: DB9 mode\n"));
  if (inited) return;
  gpio_init(GPIO_RP2U_XLOAD);
  gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_OUT);

  gpio_init(GPIO_RP2U_XSCK);
  gpio_set_dir(GPIO_RP2U_XSCK, GPIO_OUT);

  gpio_init(GPIO_RP2U_XDATA);
  gpio_set_dir(GPIO_RP2U_XDATA, GPIO_IN);

#ifdef JAMMA_OFFSET
  pio_add_program_at_offset(jamma_pio, &jammadb9_program, JAMMA_OFFSET);
  jamma_offset = JAMMA_OFFSET;
#else
  jamma_offset = pio_add_program(jamma_pio, &jammadb9_program);
#endif
  jammadb9_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XLOAD, GPIO_RP2U_XDATA);

  pio_sm_clear_fifos(jamma_pio, jamma_sm);

  irq_set_exclusive_handler (JAMMA_PIO_IRQ, pio_callback);
  pio_set_irq0_source_enabled(jamma_pio, jamma_sm, true);
  irq_set_enabled (JAMMA_PIO_IRQ, true);
  inited = 2;
#endif
}

void jamma_InitUSB() {
#ifndef NO_JAMMA
  debug(("jamma_Init: USB mode\n"));
  if (inited) return;
  gpio_init(GPIO_RP2U_XLOAD);
  gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_IN);

  gpio_init(GPIO_RP2U_XSCK);
  gpio_set_dir(GPIO_RP2U_XSCK, GPIO_IN);

  gpio_init(GPIO_RP2U_XDATA);
  gpio_set_dir(GPIO_RP2U_XDATA, GPIO_OUT);

#ifdef JAMMA_OFFSET
  pio_add_program_at_offset(jamma_pio, &jamma_program, JAMMA_OFFSET);
  jamma_offset = JAMMA_OFFSET;
#else
  jamma_offset = pio_add_program(jamma_pio, &jamma_program);
#endif
  jamma_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XDATA);
  pio_sm_clear_fifos(jamma_pio, jamma_sm);
  
#ifdef OLD_PS2
  ps2_SetGPIOListener(gpio_callback);
#else
  gpio_set_irq_callback(gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);
#endif
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, true);

  pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data);
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

  // remove handlers
#ifdef OLD_PS2
  ps2_SetGPIOListener(NULL);
#else
  irq_set_exclusive_handler (PIO0_IRQ_0, NULL);
#endif

  // shutdown sm
  pio_sm_set_enabled(jamma_pio, jamma_sm, false);
  pio_remove_program(jamma_pio, inited == 2 ? &jammadb9_program : &jamma_program, jamma_offset);

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

void jamma_InitEx(uint8_t mister) {
  debug(("jamma_InitEx: mister = %d\n", mister));
  if (mister) {
    jamma_InitDB9();
  } else {
    jamma_InitUSB();
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
  uint8_t changed = jamma_Changed;
  jamma_Changed = 0;
  return changed;
}

uint32_t jamma_GetDataAll() {
  return data;
}

