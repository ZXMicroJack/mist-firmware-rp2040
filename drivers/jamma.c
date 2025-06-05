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

/* copied from user-io */
#define JOY_RIGHT       0x01
#define JOY_LEFT        0x02
#define JOY_DOWN        0x04
#define JOY_UP          0x08
#define JOY_BTN_SHIFT   4
#define JOY_BTN1        0x10
#define JOY_BTN2        0x20
#define JOY_BTN3        0x40
#define JOY_BTN4        0x80
#define JOY_MOVE        (JOY_RIGHT|JOY_LEFT|JOY_UP|JOY_DOWN)

#define BUTTON1         0x01
#define BUTTON2         0x02
#define SWITCH1         0x04
#define SWITCH2         0x08

// virtual gamepad buttons
#define JOY_A      JOY_BTN1
#define JOY_B      JOY_BTN2
#define JOY_SELECT JOY_BTN3
#define JOY_START  JOY_BTN4
#define JOY_X      0x100
#define JOY_Y      0x200
#define JOY_L      0x400
#define JOY_R      0x800
#define JOY_L2     0x1000
#define JOY_R2     0x2000
#define JOY_L3     0x4000
#define JOY_R3     0x8000

static PIO jamma_pio = JAMMA_PIO;
static unsigned jamma_sm = JAMMA_SM;
#ifdef JAMMA_JAMMA
static unsigned jamma2_sm = JAMMA2_SM;
#endif
static uint jamma_offset;
#ifdef JAMMA_JAMMA
static uint jamma_offset2;
#endif

static uint16_t reload_data = 0x0000;
static uint16_t joydata_md[2];
static uint16_t joydata[2];
static uint8_t detect_jamma_bitshift;
static uint8_t jamma_bitshift;
static uint32_t db9_data;

#ifdef JAMMA_JAMMA
static uint32_t jamma_data;
static uint32_t jamma_debounce;
static uint32_t jamma_debounce_data;
static uint8_t jamma_Changed;
#endif

#define DEBOUNCE_COUNT    120

static uint8_t is_md[2] = {0,0};
static uint8_t md_prev[2] = {0,0};

static JAMMA_MODE mode = MODE_DB9;
static struct repeating_timer read_timer;
static uint8_t inited = 0;

static void init_jamma() {
  db9_data = 0;
  
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

void jamma_SetMode(JAMMA_MODE new_mode) {
  if (new_mode == mode) return;
  
  if (inited == 2) {
    jamma_Kill();
    // printf("setting mode to from %d to %d\n", mode, new_mode);
    mode = new_mode;
    jamma_InitEx(1);
  } else {
    mode = new_mode;
  }
}

JAMMA_MODE jamma_GetMode() {
  return mode;
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

static uint8_t db9_translate(uint8_t d) {
  return (d >> 4) | ((d & 8) << 1) | ((d & 4)) << 3;
}

#ifdef MDDEBUG
#define mddebug(a) printf a
#else
#define mddebug(a)
#endif

static uint8_t process_frame_stick(uint8_t stick, uint8_t this) {
  uint8_t nr = 0;

  mddebug(("stick %d: md_prev[stick] %02X this %02X\n", stick, md_prev[stick], this));
  if ((md_prev[stick] & 0xf0) == 0xf0) {
    mddebug(("6 button this %02X\n", this));
    joydata_md[stick] &= ~(JOY_SELECT|JOY_X|JOY_Y|JOY_L);
    joydata_md[stick] |= ((this & 0xe0) << 3) | ((this & 0x10) << 2);
    nr ++;
  } else if ((md_prev[stick] & 0x30) == 0x30) {
    mddebug(("3a button this %02X\n", this));
    joydata_md[stick] &= ~(JOY_B | JOY_R);
    joydata_md[stick] |= ((this & 0x08) << 2) | ((this & 0x04) << 9);
    nr ++;
  } else if ((this & 0x30) == 0x30) {
    mddebug(("3b button this %02X\n", this));
    joydata_md[stick] &= ~(JOY_START | JOY_A);
    joydata_md[stick] |= ((this & 0x04) << 5) | ((this & 0x08) << 1);
    nr ++;
  } else if ((this & 0x30) != 0x30 && (md_prev[stick] & 0x30) != 0x30) {
    mddebug(("regular button this %02X\n", this));
    if (is_md[stick]) {
      joydata_md[stick] &= ~(JOY_B | JOY_MOVE);
      joydata_md[stick] |= (this >> 4) | 
        ((this & 0x08) << 1);
    } else {
      joydata[stick] = db9_translate(this);
    }
  }

  md_prev[stick] = this;
  return nr;
}

static void process_frame(uint8_t nr[2], uint32_t data) {
  nr[1] += process_frame_stick(1, ~data >> 8);
  nr[0] += process_frame_stick(0, ~data >> 16);
}

static void pio_callback_md() {
  int i = 0;
  uint8_t nr[2] = {0,0};

  md_prev[0] = md_prev[1] = 0x00;
  while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma_sm)) {
    process_frame(nr, pio_sm_get_blocking(jamma_pio, jamma_sm));
  }
  is_md[0] = is_md[0] << 1 | (nr[0] ? 1 : 0);
  is_md[1] = is_md[1] << 1 | (nr[1] ? 1 : 0);
}

static void pio_callback_jamma() {
  uint32_t d;
  uint16_t w;

  while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma_sm)) {
    d = ~pio_sm_get_blocking(jamma_pio, jamma_sm);

    w = (d >> 8) & 0xf0ff;
    joydata[0] = 
      ((w & 0xc000) >> 8) | // btn4/btn3
      ((w & 0x2000) >> 3) | // joy l
      ((w & 0x1000) >> 1) | // joy r
      ((w & 0x00f0) >> 4) | // directions
      ((w & 0x000c) << 2) | // button b / a
      ((w & 0x0002) << 7) | // button x
      ((w & 0x0001) << 9); // button y

    w = (d >> 16) & 0xff0f;
    joydata[1] = 
      ((w & 0x0c00) >> 4) | // btn4/btn3
      ((w & 0x0200) << 1) | // joy l
      ((w & 0x0100) << 3) | // joy r
      (w & 0x000f) | // directions
      ((w & 0xc000) >> 10) | // button b / a
      ((w & 0x2000) >> 5) | // button x
      ((w & 0x1000) >> 3); // button y
  }
}

static void pio_callback_db9() {
  uint32_t d;
  while (!pio_sm_is_rx_fifo_empty(jamma_pio, jamma_sm)) {
    d = ~pio_sm_get_blocking(jamma_pio, jamma_sm);
    joydata[0] = db9_translate((d >> 16) & 0xff);
    joydata[1] = db9_translate((d >> 8) & 0xff);
  }
}

typedef void (*jamma_cb_t)();

static jamma_cb_t jamma_cb[MODE_MAX] = {
  pio_callback_db9,
  pio_callback_jamma,
  pio_callback_md
};

static void pio_callback() {
  jamma_cb[mode]();
  pio_interrupt_clear (jamma_pio, 0);
}

static bool ReadKick(struct repeating_timer *t) {
  pio_interrupt_clear (jamma_pio, 7);
  return inited == 2;
}

void jamma_InitDB9() {
  /* don't need to detect shifting */
  init_jamma();

  memset(&is_md, 0, sizeof is_md);
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
  pio_add_program_at_offset(jamma_pio, 
    mode == MODE_DB9 ? &jammadb9ns_program : &jammadb9_program,
    JAMMA_OFFSET);
  jamma_offset = JAMMA_OFFSET;
#else
  if (mode == MODE_DB9) {
    jamma_offset = pio_add_program(jamma_pio, &jammadb9_program);
  } else {
    jamma_offset = pio_add_program(jamma_pio, &jammadb9ns_program);
  }
#endif

  jammadb9_program_init(jamma_pio, jamma_sm, jamma_offset, GPIO_RP2U_XLOAD, 
    mode == MODE_JAMMA ? GPIO_RP2U_XDATAJAMMA : GPIO_RP2U_XDATA, mode != MODE_DB9);

  pio_sm_clear_fifos(jamma_pio, jamma_sm);
  irq_set_exclusive_handler (JAMMA_PIO_IRQ, pio_callback);
  pio_set_irq0_source_enabled(jamma_pio, pis_interrupt0, true);
  irq_set_enabled (JAMMA_PIO_IRQ, true);

  inited = 2;
  add_repeating_timer_ms(20, ReadKick, NULL, &read_timer);
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

  pio_interrupt_clear (jamma_pio, 0);
  inited = 1;
#endif
}

void jamma_Kill() {
#ifndef NO_JAMMA
  if (!inited) return;

  if (inited == 2) cancel_repeating_timer(&read_timer);

  // disable interrupts
  gpio_set_irq_enabled(GPIO_RP2U_XLOAD, GPIO_IRQ_EDGE_FALL, false);
  pio_set_irq0_source_enabled(jamma_pio, jamma_sm, false);
#ifdef JAMMA_JAMMA
  if (inited == 1) pio_set_irq1_source_enabled(jamma_pio, jamma2_sm, false);
#endif
  gpioirq_SetCallback(IRQ_JAMMA, NULL);

  // shutdown sm
  pio_sm_set_enabled(jamma_pio, jamma_sm, false);
#ifdef JAMMA_JAMMA
  if (inited == 1) pio_sm_set_enabled(jamma_pio, jamma2_sm, false);
#endif
  if (inited == 2) {
    pio_remove_program(jamma_pio,
      mode == MODE_DB9 ? &jammadb9ns_program : &jammadb9_program,
      jamma_offset);
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
  if (mode == MODE_DB9) {
    return joydata[inst];
  } else if (mode == MODE_JAMMA) {
    return joydata[inst];
  } else if (mode == MODE_MEGADRIVE) {
    return is_md[inst] ? joydata_md[inst] : joydata[inst];
  }
  return 0;
}

#ifdef JAMMA_JAMMA
uint32_t jamma_GetJamma() {
  return jamma_data >> jamma_bitshift;
}

uint8_t jamma_GetDepth() {
  return 32 - jamma_bitshift - 1;
}
#endif
