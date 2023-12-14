#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pins.h"
#include "ps2.h"
#include "jamma.h"
#define DEBUG
#include "debug.h"

#define DB9

#ifdef DB9
#include "jammadb9.pio.h"
#else
#include "jamma.pio.h"
#endif

uint16_t reload_data = 0x0000;
uint8_t joydata[2];

PIO jamma_pio = pio0;
unsigned jamma_sm = 2;

#ifndef DB9
static void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == GPIO_RP2U_XLOAD) {
    while (!pio_sm_is_tx_fifo_full(jamma_pio, jamma_sm))
      pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data); // 
  }
}
#else
static uint32_t data;
static void pio_callback() {
  pio_interrupt_clear (jamma_pio, 0);
  data = pio_sm_get_blocking(jamma_pio, jamma_sm);
}
#endif


#ifdef DB9
void jamma_Init() {
  gpio_init(GPIO_RP2U_XLOAD);
  gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_OUT);

  gpio_init(GPIO_RP2U_XSCK);
  gpio_set_dir(GPIO_RP2U_XSCK, GPIO_OUT);

  gpio_init(GPIO_RP2U_XDATA);
  gpio_set_dir(GPIO_RP2U_XDATA, GPIO_IN);

  uint offset = pio_add_program(jamma_pio, &jammadb9_program);
  jammadb9_program_init(jamma_pio, jamma_sm, offset, GPIO_RP2U_XLOAD, GPIO_RP2U_XDATA);
  pio_sm_clear_fifos(jamma_pio, jamma_sm);

  irq_set_exclusive_handler (PIO0_IRQ_0, pio_callback);
  pio_set_irq0_source_enabled(jamma_pio, jamma_sm, true);
  irq_set_enabled (PIO0_IRQ_0, true);
}
#else
void jamma_Init() {
  gpio_init(GPIO_RP2U_XLOAD);
  gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_IN);

  gpio_init(GPIO_RP2U_XSCK);
  gpio_set_dir(GPIO_RP2U_XSCK, GPIO_IN);

  gpio_init(GPIO_RP2U_XDATA);
  gpio_set_dir(GPIO_RP2U_XDATA, GPIO_OUT);

  uint offset = pio_add_program(jamma_pio, &jamma_program);
  jamma_program_init(jamma_pio, jamma_sm, offset, GPIO_RP2U_XDATA);
  pio_sm_clear_fifos(jamma_pio, jamma_sm);
  
  
  ps2_SetGPIOListener(gpio_callback);
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, true);
  pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data);
  pio_interrupt_clear (jamma_pio, 0);
}
#endif

void jamma_SetData(uint8_t inst, uint32_t data) {
  joydata[inst] = data & 0xff;
  reload_data = (joydata[1] << 8) | joydata[0];
}

uint32_t jamma_GetData(uint8_t inst) {
  return data;
}
