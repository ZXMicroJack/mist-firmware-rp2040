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
#include "jammaj.pio.h"

uint16_t reload_data = 0x0000;
uint8_t joydata[2];

static PIO jamma_pio = JAMMA_PIO;
static unsigned jamma_sm = JAMMA_SM;
static unsigned jamma2_sm = JAMMA2_SM;
static uint jamma_offset;
static uint jamma_offset2;
static uint8_t detect_jamma_bitshift = 0;
static uint8_t jamma_bitshift = 0;

static uint32_t data;
static uint8_t jamma_Changed = 0;
static uint32_t debounce = 0;
static uint32_t debounce_data;

static uint32_t jamma_data = 0;
static uint32_t debounce2 = 0;
static uint32_t debounce2_data;
static uint8_t jamma_ChangedJamma = 0;

#define DEBOUNCE_COUNT    120

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
    
#if 1
    while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma2_sm)) {
      uint32_t _data = pio_sm_get_blocking(jamma_pio, jamma2_sm);
      do_debounce(_data, &jamma_data, &debounce2_data, &debounce2, &jamma_ChangedJamma);
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

static void pio_callback() {
  uint32_t _data;
  pio_interrupt_clear (jamma_pio, 0);

  if (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma_sm)) {
    _data = pio_sm_get_blocking(jamma_pio, jamma_sm);
    do_debounce(_data, &data, &debounce_data, &debounce, &jamma_Changed);
#if 0
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
#endif
    joydata[0] = ~(data >> (jamma_bitshift + 8));
    joydata[1] = ~(data >> jamma_bitshift);
  }

  if (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma2_sm)) {
    _data = pio_sm_get_blocking(jamma_pio, jamma2_sm);
    do_debounce(_data, &jamma_data, &debounce2_data, &debounce2, &jamma_ChangedJamma);
#if 0
    if (debounce2_data != _data) {
      debounce2 = DEBOUNCE_COUNT;
      debounce2_data = _data;
    }

    if (debounce2) {
      debounce2--;
      if (!debounce2) {
        jamma_ChangedJamma = _data != jamma_data;
        jamma_data = _data;
      }
    }
#endif
  }
}

static uint8_t inited = 0;

void jamma_InitDB9() {
  /* don't need to detect shifting */
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

  gpio_init(GPIO_RP2U_XDATAJAMMA);
  gpio_set_dir(GPIO_RP2U_XDATAJAMMA, GPIO_IN);

#ifdef JAMMA_OFFSET
  pio_add_program_at_offset(jamma_pio, &jammadb9_program, JAMMA_OFFSET);
  jamma_offset = JAMMA_OFFSET;
#else
  jamma_offset = pio_add_program(jamma_pio, &jammadb9_program);
#endif
  jammadb9_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XLOAD, GPIO_RP2U_XDATA);
  jammadb9_program_init(jamma_pio, jamma2_sm, jamma_offset, GPIO_RP2U_XLOAD, GPIO_RP2U_XDATAJAMMA);

  pio_sm_clear_fifos(jamma_pio, jamma_sm);
  pio_sm_clear_fifos(jamma_pio, jamma2_sm);

  irq_set_exclusive_handler (JAMMA_PIO_IRQ, pio_callback);
  pio_set_irq0_source_enabled(jamma_pio, jamma_sm, true);
  pio_set_irq0_source_enabled(jamma_pio, jamma2_sm, true);
  irq_set_enabled (JAMMA_PIO_IRQ, true);
  inited = 2;
#endif
}

#define d printf("[%d]\n", __LINE__);

void jamma_InitUSB() {
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

  gpio_init(GPIO_RP2U_XDATAJAMMA);
  gpio_set_dir(GPIO_RP2U_XDATAJAMMA, GPIO_IN);

#ifdef JAMMA_OFFSET
  pio_add_program_at_offset(jamma_pio, &jamma_program, JAMMA_OFFSET);d
  jamma_offset = JAMMA_OFFSET;d
  pio_add_program_at_offset(jamma_pio, &jammaj_program, JAMMA_OFFSET + JAMMAU_INSTR);d
  jamma_offset2 = JAMMA_OFFSET + JAMMAU_INSTR;d
#else
  jamma_offset = pio_add_program(jamma_pio, &jamma_program);
#endif
  jamma_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XDATA);d
  pio_sm_clear_fifos(jamma_pio, jamma_sm);d
  jammaj_program_init(jamma_pio, jamma2_sm, jamma_offset2, GPIO_RP2U_XDATAJAMMA);d
  pio_sm_clear_fifos(jamma_pio, jamma2_sm);d
  
  gpioirq_SetCallback(IRQ_JAMMA, gpio_callback);d
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, true);d

  // pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data);d
  pio_interrupt_clear (jamma_pio, 0);d
  inited = 1;
#endif
}

void jamma_Kill() {
#ifndef NO_JAMMA
  if (!inited) return;
  // disable interrupts
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, false);
  pio_set_irq0_source_enabled(jamma_pio, jamma_sm, false);
  pio_set_irq0_source_enabled(jamma_pio, jamma2_sm, false);
  gpioirq_SetCallback(IRQ_JAMMA, NULL);

  // shutdown sm
  pio_sm_set_enabled(jamma_pio, jamma_sm, false);
  pio_sm_set_enabled(jamma_pio, jamma2_sm, false);
  if (inited == 2) {
    pio_remove_program(jamma_pio, &jammadb9_program, jamma_offset);
  } else {
    pio_remove_program(jamma_pio, &jamma_program, jamma_offset);
    pio_remove_program(jamma_pio, &jammaj_program, jamma_offset2);
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
  uint8_t changed = jamma_Changed;
  jamma_Changed = 0;
  return changed;
}

uint32_t jamma_GetDataAll() {
  return data >> jamma_bitshift;
}

uint32_t jamma_GetJamma() {
  return jamma_data >> jamma_bitshift;
}

uint8_t jamma_GetDepth() {
  return 32 - jamma_bitshift;
}

int jamma_HasChangedJamma() {
  uint8_t changed = jamma_ChangedJamma;
  jamma_ChangedJamma = 0;
  return changed;
}

#if 0 // MJTODO remove
static uint64_t detect_timeout = 0;
static uint32_t detect_clocks = 0;

static void gpio_callback_detect(uint gpio, uint32_t events) {
  // printf("!\n");
  if (gpio == GPIO_RP2U_XLOAD) { //} || gpio == GPIO_RP2U_XSCK) {
    detect_clocks ++;
  }
}

void jamma_DetectPoll(uint8_t reset) {
  // gpio_set_irq_enabled(GPIO_RP2U_XLOAD);

#if 1
  if (reset) {
    gpio_init(GPIO_RP2U_XLOAD);
    gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_IN);
    gpioirq_SetCallback(IRQ_JAMMA, gpio_callback_detect);
    gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, true);
    detect_timeout = time_us_64() + 100000;
    detect_clocks = 0;
  } else {
    if (time_us_64() > detect_timeout ) {
      /* can now detect  */
      gpioirq_SetCallback(IRQ_JAMMA, NULL);
      printf("Shutting down detect\n");
      if (detect_clocks == 0) {
        /* now try to detect if slave mode is on */
        uint32_t d = 0;
        gpio_init(GPIO_RP2U_XLOAD);
        gpio_init(GPIO_RP2U_XSCK);
        gpio_init(GPIO_RP2U_XDATA);
        gpio_put(GPIO_RP2U_XLOAD, 1);
        gpio_put(GPIO_RP2U_XSCK, 1);
        gpio_set_dir(GPIO_RP2U_XSCK, GPIO_OUT);
        gpio_set_dir(GPIO_RP2U_XLOAD, GPIO_OUT);
        sleep_us(100);
        gpio_put(GPIO_RP2U_XLOAD, 0);
        sleep_us(100);
        gpio_put(GPIO_RP2U_XLOAD, 1);
        sleep_us(100);

        for (int i=0; i<24; i++) {
          d = (d<<1) | (gpio_get(GPIO_RP2U_XDATA) ? 1 : 0);
          gpio_put(GPIO_RP2U_XSCK, 1);
          sleep_us(100);
          gpio_put(GPIO_RP2U_XSCK, 0);
          sleep_us(100);
        }
        printf("read %08X\n", d);
        gpio_init(GPIO_RP2U_XLOAD);
        gpio_init(GPIO_RP2U_XSCK);


      } else {
        printf("Reflection ON, Master mode 16\n");
      }
    }
    printf("detect_clocks = %d\n", detect_clocks);
  }
#endif
}
#endif
