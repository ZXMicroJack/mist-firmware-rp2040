#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "pico/stdlib.h"

#include "pins.h"
#include "ps2.h"
#include "fifo.h"
// #define DEBUG
#include "debug.h"

// #define OLDPS2

#ifdef OLDPS2
enum {
  PS2_IDLE,
  PS2_SUPRESS,
  PS2_SIGNALTRANSMIT,
  PS2_TRANSMIT,
  PS2_TRANSMITKBDETECT,
  PS2_RECEIVE,
  PS2_PAUSE,
  PS2_TRANSMITPREAMBLE
};

#define NR_PS2  2
#define PS2_GUARDTIME PS2_RX_STATES
#define PS2_PAUSETIME (1000)
#define PS2_RX_STATES 23

typedef struct {
  int ps2_state;
  struct repeating_timer ps2timer;
  struct repeating_timer ps2timer_rx;
  int ps2_data;
  int ps2_states;
  int gpio_clk;
  int gpio_data;
  uint8_t channel;
  fifo_t fifo;
  uint8_t fifobuf[64];
  fifo_t fifo_rx;
  uint8_t fiforxbuf[64];
  uint8_t hostMode;
  uint64_t lastAction;
  uint8_t ps2_connected;
} ps2_t;

ps2_t ps2port[NR_PS2];

/* calculate odd parity */
static uint8_t parity(uint8_t d) {
#if 0 /* keeping this here as a reminder that no-one is immune to stupidity */
//   d ^= d << 4;
//   d ^= d << 2;
//   d ^= d << 1;
#endif
  d ^= d >> 4;
  d ^= d >> 2;
  d ^= d >> 1;
  return (d & 1) ^ 1;
}

static void ps2_KickTx(ps2_t *ps2, uint8_t data) {
  if (!ps2->hostMode) {
    if (ps2->channel == 0 && data == 0xff) {
      ps2->ps2_states = PS2_RX_STATES;
      ps2->ps2_state = PS2_PAUSE;
    } else {
      ps2->ps2_data = (data << 1) | (parity(data) << 9) | 0x400;
      ps2->ps2_states = 0;
      ps2->ps2_state = PS2_RECEIVE;
    }

    gpio_put(ps2->gpio_clk, 1);
    gpio_put(ps2->gpio_data, 1);
    gpio_set_dir(ps2->gpio_clk, GPIO_OUT);
    gpio_set_dir(ps2->gpio_data, GPIO_OUT);

  } else {
    ps2->ps2_data = data | (parity(data) << 8) | 0x200;
    ps2->ps2_states = 11;
    ps2->ps2_state = PS2_TRANSMITPREAMBLE;
  }
}

// drive clock and receive data from host
static bool ps2_timer_callback(struct repeating_timer *t) {
  ps2_t *ps2 = (ps2_t *)t->user_data;

  if (ps2->hostMode) return false;

  if (ps2->ps2_state != PS2_TRANSMIT) {
    return false;
  }
  
  if (ps2->ps2_states > 2) {
    if (!(ps2->ps2_states & 1)) ps2->ps2_data = (ps2->ps2_data >> 1) | (gpio_get(ps2->gpio_data) ? 0x200 : 0);
  } else {
    gpio_set_dir(ps2->gpio_data, GPIO_OUT);
    gpio_put(ps2->gpio_data, 0);
  }
  gpio_put(ps2->gpio_clk, ps2->ps2_states & 1);
  
  ps2->ps2_states --;
  if (ps2->ps2_states == 0) {
    ps2->ps2_state = PS2_IDLE;
    gpio_set_dir(ps2->gpio_clk, GPIO_IN);
    gpio_set_dir(ps2->gpio_data, GPIO_IN);
    
    fifo_Put(&ps2->fifo_rx, ps2->ps2_data & 0xff);
    ps2->ps2_data = 0;
  }
  return true;
}

static bool ps2_timer_callback_rx(struct repeating_timer *t) {
  ps2_t *ps2 = (ps2_t *)t->user_data;

  if (ps2->hostMode) return false;
  
  if (ps2->ps2_state != PS2_RECEIVE && ps2->ps2_state != PS2_PAUSE) {
    gpio_set_dir(ps2->gpio_clk, GPIO_IN);
    gpio_set_dir(ps2->gpio_data, GPIO_IN);
    return false;
  }
  
  if (ps2->ps2_states < PS2_RX_STATES) {
    if ((ps2->ps2_states & 1) == 1) {
      gpio_put(ps2->gpio_data, ps2->ps2_data & 1);
      ps2->ps2_data >>= 1;
    }
    gpio_put(ps2->gpio_clk, (ps2->ps2_states & 1) ^ 1);
  }
  
  // continue counting beyond end of send to provide some guardtime
  ps2->ps2_states ++;
  if ((ps2->ps2_state == PS2_RECEIVE && ps2->ps2_states == (PS2_RX_STATES + PS2_GUARDTIME)) ||
      (ps2->ps2_state == PS2_PAUSE && ps2->ps2_states >= PS2_PAUSETIME)) {
    int ch = fifo_Get(&ps2->fifo);
    if (ch >= 0) {
      ps2_KickTx(ps2, ch);
    } else {
      ps2->ps2_state = PS2_IDLE;
    }
  }
  return true;
}

void ps2_SendChar(uint8_t ch, uint8_t data) {
  if (ps2port[ch].ps2_state == PS2_IDLE) {
    ps2_KickTx(&ps2port[ch], data);
    if (!ps2port[ch].hostMode) {
      add_repeating_timer_us(40, ps2_timer_callback_rx, &ps2port[ch], &ps2port[ch].ps2timer_rx);
    }
  } else {
    fifo_Put(&ps2port[ch].fifo, data);
  }
}


// due to pre-Christmas piss up at PICO HQ, someone forgot to implement separate
// callbacks for GPIO lines, so this is a kludge to get around yet another SDK issue.
// Don't want to seem like I'm moaning - but these kind of things cost time and frustration.
// happy to discuss...
static void (*gpio_cb)(uint gpio, uint32_t events) = (void (*)(uint, uint32_t))NULL;

void ps2_SetGPIOListener(void (*cb)(uint gpio, uint32_t events)) {
  gpio_cb = cb;
}


#define IDLE_RESET_PERIOD_US 10000
static uint32_t stored_gpios = 0;
static void gpio_handle_host(ps2_t *ps2, uint gpio, uint32_t events) {
  // unstall interface if its out of sync
  uint64_t now;

  now = time_us_64();

  if (events & 0x8) { // rising
    if (gpio == ps2->gpio_clk) {

      // handle falling clock
      if (ps2->ps2_state == PS2_RECEIVE) {
        uint8_t data = (stored_gpios >> ps2->gpio_data) & 1;
        ps2->ps2_data = (ps2->ps2_data >> 1) | (data ? 0x200 : 0);
        ps2->ps2_states --;

        /* received a char */
        if (!ps2->ps2_states) {
          ps2->ps2_state = PS2_IDLE;
          ps2->lastAction = now;
          fifo_Put(&ps2->fifo_rx, ps2->ps2_data & 0xff);
        }
      } else if (ps2->ps2_state == PS2_TRANSMIT) {
        /* finished transmitting char */
        if (!ps2->ps2_states) {
          ps2->ps2_state = PS2_IDLE;
          ps2->lastAction = now;
        }
      }
    }
  }

  if (events & 0x4) { // falling
    if (gpio == ps2->gpio_data) {
      // handle falling data

      /* data fell in idle mode, keyboard wants to send a char */
      if (ps2->ps2_state == PS2_IDLE) {
        ps2->ps2_states = 11;
        ps2->ps2_state = PS2_RECEIVE;
        ps2->ps2_data = 0;
        ps2->lastAction = now;
      }
    } else if (gpio == ps2->gpio_clk) {
      /* clock fell, put data bit on data line */
      if (ps2->ps2_state == PS2_TRANSMIT) {
        gpio_put(ps2->gpio_data, ps2->ps2_data & 1);
        ps2->ps2_data >>= 1;
        ps2->ps2_states --;

        /* got to end, let keyboard ack */
        if (!ps2->ps2_states) {
          gpio_put(ps2->gpio_data, 1);
          gpio_set_dir(ps2->gpio_data, GPIO_IN);
          ps2->lastAction = now;
        }
      }      
    }
  }
}

static bool ps2_timer_callback_keyboard_timeout(struct repeating_timer *t) {
  ps2_t *ps2 = (ps2_t *)t->user_data;

  // no keyboard found, drive the clock line
  if (ps2->ps2_state == PS2_TRANSMITKBDETECT) {
    // printf("!\n");
    ps2->ps2_state = PS2_TRANSMIT;
    gpio_set_dir(ps2->gpio_clk, GPIO_OUT);
    add_repeating_timer_us(40, ps2_timer_callback, ps2, &ps2->ps2timer);
  }
  return false;
}

static void gpio_handle_normal(ps2_t *ps2, uint gpio, uint32_t events) {
  if (events & 0x4) { // fall
    if (gpio == ps2->gpio_clk && ps2->ps2_state == PS2_IDLE) {
      ps2->ps2_state = PS2_SUPRESS;
    } else if (gpio == ps2->gpio_data && ps2->ps2_state == PS2_SUPRESS) {
      ps2->ps2_state = PS2_SIGNALTRANSMIT;
    }
  }

  if (events & 0x8) { // fall
    if (gpio == ps2->gpio_clk) {
      if (ps2->ps2_state == PS2_SIGNALTRANSMIT) {
        if (ps2->ps2_connected) {
          ps2->ps2_state = PS2_TRANSMITKBDETECT;
          ps2->ps2_states = 22;
        } else {
          ps2->ps2_state = PS2_IDLE;
          ps2->ps2_states = 0;
        }

        // wait for keyboard
        add_repeating_timer_us(2000, ps2_timer_callback_keyboard_timeout, ps2, &ps2->ps2timer);

      } else if (ps2->ps2_state == PS2_SUPRESS) {
        ps2->ps2_state = PS2_IDLE;
      }
    }
  }
}

static void gpio_callback(uint gpio, uint32_t events) {
  stored_gpios = gpio_get_all();
  if (gpio_cb) gpio_cb(gpio, events);
  for (int i=0; i<NR_PS2; i++) {
    if (gpio == ps2port[i].gpio_clk || gpio == ps2port[i].gpio_data) {
      // keyboard is detected - drop out
      if (ps2port[i].ps2_state == PS2_TRANSMITKBDETECT) {
        ps2port[i].ps2_connected = true;
        // printf("?\m");
        ps2port[i].ps2_state = PS2_IDLE;
        ps2port[i].ps2_states = 0;
      } else if (ps2port[i].hostMode) {
        gpio_handle_host(&ps2port[i], gpio, events);
      } else {
        gpio_handle_normal(&ps2port[i], gpio, events);
      }
      break;
    }
  }
}

void ps2_Init() {
  uint8_t lut[] = {
    GPIO_PS2_CLK,
    GPIO_PS2_CLK2,
    GPIO_PS2_DATA,
    GPIO_PS2_DATA2
  };

  memset(&ps2port, 0x00, sizeof ps2port);
  ps2port[0].gpio_clk = GPIO_PS2_CLK;
  ps2port[0].gpio_data = GPIO_PS2_DATA;
  ps2port[0].channel = 0;
  ps2port[1].gpio_clk = GPIO_PS2_CLK2;
  ps2port[1].gpio_data = GPIO_PS2_DATA2;
  ps2port[1].channel = 1;
  fifo_Init(&ps2port[0].fifo, ps2port[0].fifobuf, sizeof ps2port[0].fifobuf);
  fifo_Init(&ps2port[0].fifo_rx, ps2port[0].fiforxbuf, sizeof ps2port[0].fiforxbuf);
  fifo_Init(&ps2port[1].fifo, ps2port[1].fifobuf, sizeof ps2port[1].fifobuf);
  fifo_Init(&ps2port[1].fifo_rx, ps2port[0].fiforxbuf, sizeof ps2port[0].fiforxbuf);
  
  for (int i=0; i<sizeof lut; i++) {
    gpio_init(lut[i]);
    gpio_set_dir(lut[i], GPIO_IN);
  }
  
  gpio_set_irq_callback(gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);
}

void ps2_EnablePortEx(uint8_t ch, bool enabled, uint8_t hostMode) {
  ps2port[ch].hostMode = hostMode;
  if (enabled) {
    gpio_set_irq_enabled(ps2port[ch].gpio_clk,
      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ps2port[ch].gpio_data,
      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_pull_up(ps2port[ch].gpio_data);
    gpio_pull_up(ps2port[ch].gpio_clk);
  } else {
    gpio_set_irq_enabled(ps2port[ch].gpio_clk,
      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    gpio_set_irq_enabled(ps2port[ch].gpio_data,
      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);

    gpio_init(ps2port[ch].gpio_data);
    gpio_init(ps2port[ch].gpio_clk);

    ps2port[ch].ps2_state = PS2_IDLE;
    ps2port[ch].ps2_states = 0;
  }
}

void ps2_EnablePort(uint8_t ch, bool enabled) {
  ps2_EnablePortEx(ch, enabled, 0);
}

void ps2_InsertChar(uint8_t ch, uint8_t data) {
  fifo_Put(&ps2port[ch].fifo_rx, data);
}

int ps2_GetChar(uint8_t ch) {
  return fifo_Get(&ps2port[ch].fifo_rx);
}

void ps2_HealthCheck() {
  uint64_t now = time_us_64();
  for (int i=0; i<NR_PS2; i++) {
    if (ps2port[i].hostMode) {
      /* if receive is stalled due to missing a clock transition, then return it to normal */
      if (ps2port[i].ps2_state == PS2_RECEIVE && (ps2port[i].lastAction < now) && (now - ps2port[i].lastAction) > IDLE_RESET_PERIOD_US) {
        debug(("!\n"));
        ps2port[i].ps2_state = PS2_IDLE;
        gpio_put(ps2port[i].gpio_clk, 1);
        gpio_set_dir(ps2port[i].gpio_clk, GPIO_IN);
        gpio_put(ps2port[i].gpio_data, 1);
        gpio_set_dir(ps2port[i].gpio_data, GPIO_IN);

      } else if (ps2port[i].ps2_state == PS2_TRANSMITPREAMBLE) {
        /* signal send command */
        gpio_put(ps2port[i].gpio_clk, 0);
        gpio_set_dir(ps2port[i].gpio_clk, GPIO_OUT);
        sleep_us(500);
        gpio_put(ps2port[i].gpio_data, 0);
        gpio_set_dir(ps2port[i].gpio_data, GPIO_OUT);
        sleep_us(500);
        gpio_put(ps2port[i].gpio_clk, 1);
        gpio_set_dir(ps2port[i].gpio_clk, GPIO_IN);
        sleep_us(10);
        ps2port[i].lastAction = now;
        ps2port[i].ps2_state = PS2_TRANSMIT;

      } else if ( ps2port[i].ps2_state == PS2_IDLE && (now - ps2port[i].lastAction) > IDLE_RESET_PERIOD_US ) {
        int ch = fifo_Get(&ps2port[i].fifo);
        if (ch >= 0) ps2_KickTx(&ps2port[i], ch);
      }
    }
  }
}

void ps2_SwitchMode(int hostMode) {
  // does nothing in the old world
}

#ifdef TESTBUILD
void ps2_DebugQueues() {
  int n = 0;
  for (int i=0; i<NR_PS2; i++) {
    int ch;
    while ((ch = ps2_GetChar(i)) >= 0) {
      printf("[RX%d:%02X]", i, ch);
      n++;
    }
  }
  if (n) printf("\n");
}

void ps2_Debug() {
  printf("ps2port[0].ps2_states = %d .ps2_state = %d\n", ps2port[0].ps2_states, ps2port[0].ps2_state);
  printf("ps2port[1].ps2_states = %d .ps2_state = %d\n", ps2port[1].ps2_states, ps2port[1].ps2_state);
}
#endif
#else

/*************************************************************************************************************/
// NEW PS2 using PIO

#include "hardware/pio.h"

#include "ps2.pio.h"
#include "ps2tx.pio.h"

static PIO ps2host_pio = PS2HOST_PIO;
// static int hostMode = -1;

#define NR_PS2  2

typedef struct {
  fifo_t fifo;
  uint8_t fifobuf[64];
  fifo_t fifo_rx;
  uint8_t fiforxbuf[64];
  uint sm;
} ps2_t;

static ps2_t ps2port[NR_PS2];
static int hostMode = -1;
static uint ps2_offset = 0;

/* calculate odd parity */
static uint8_t parity(uint8_t d) {
#if 0 /* keeping this here as a reminder that no-one is immune to stupidity */
//   d ^= d << 4;
//   d ^= d << 2;
//   d ^= d << 1;
#endif
  d ^= d >> 4;
  d ^= d >> 2;
  d ^= d >> 1;
  return (d & 1) ^ 1;
}

static int decode(uint32_t x) {
  uint32_t val = 0;
  for (int i=0; i<11; i++) {
    val = (val>>1) | ((x&2) << 10);
    x >>= 2;
  }
  return (val >> 2) & 0xff;
}


static int readPs2(PIO pio, uint sm) {
  int c = -1;
  if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
    uint32_t x = pio_sm_get_blocking(pio, sm);
    c = decode(x);
    debug(("fifo: %08x (%08x)\n", x, c));
  }
  return c;
}

static void writePs2(PIO pio, uint sm, uint8_t data, int hostMode) {
  if (!pio_sm_is_tx_fifo_full(pio, sm)) {
    if (hostMode) {
      pio_sm_put_blocking(pio, sm, data | (parity(data) << 8) | 0x200);
    } else {
      pio_sm_put_blocking(pio, sm, (data << 1) | (parity(data) << 9) | 0x400);
    }
    debug(("Sent data %02X\n", data));
  } else {
    debug(("Couldn't send data %02X\n", data));
  }
}

#ifdef PIOINTS
static int nrints = 0;
static void pio_callback() {
  uint32_t _data;
  pio_interrupt_clear (ps2host_pio, 0);

  int c;
  int ch = 0;

  while ((c = readPs2(ps2host_pio, ps2host_sm)) >= 0) {
    fifo_Put(&ps2port[ch].fifo_rx, c);
  }

  while (!pio_sm_is_tx_fifo_full(ps2host_pio, ps2host_sm) && (c = fifo_Get(&ps2port[ch].fifo)) >= 0) {
    writePs2(ps2host_pio, ps2host_sm, c);
  }

  nrints++;
}

#define PS2_PIO_IRQ   PIO0_IRQ_1
#endif

void ps2_Init() {
  static int started = 0;
  if (started) return;

  fifo_Init(&ps2port[0].fifo, ps2port[0].fifobuf, sizeof ps2port[0].fifobuf);
  fifo_Init(&ps2port[0].fifo_rx, ps2port[0].fiforxbuf, sizeof ps2port[0].fiforxbuf);
  fifo_Init(&ps2port[1].fifo, ps2port[1].fifobuf, sizeof ps2port[1].fifobuf);
  fifo_Init(&ps2port[1].fifo_rx, ps2port[1].fiforxbuf, sizeof ps2port[1].fiforxbuf);
  ps2port[0].sm = PS2HOST_SM;
  ps2port[1].sm = PS2HOST2_SM;

#ifdef PIOINTS
  irq_set_exclusive_handler (PS2_PIO_IRQ, pio_callback);
  pio_set_irq0_source_enabled(ps2host_pio, ps2host_sm, true);
  irq_set_enabled (PS2_PIO_IRQ, true);
#endif
  started = 1;
}

void ps2_SwitchMode(int _hostMode) {
  if (_hostMode != hostMode) {
    if (hostMode >= 0) {
      pio_sm_set_enabled(ps2host_pio, ps2port[0].sm, false);
      pio_sm_set_enabled(ps2host_pio, ps2port[1].sm, false);
      pio_remove_program(ps2host_pio, hostMode ? &ps2_program : &ps2tx_program, ps2_offset);
    }

#ifdef PS2_OFFSET
    if (_hostMode ) {
      ps2_offset = pio_add_program(ps2host_pio, &ps2_program);
    } else {
      ps2_offset = pio_add_program(ps2host_pio, &ps2tx_program);
    }
#else
    ps2_offset = PS2HOST_OFFSET;
    if (_hostMode ) {
      pio_add_program_at_offset(ps2host_pio, &ps2_program, PS2HOST_OFFSET);
    } else {
      pio_add_program_at_offset(ps2host_pio, &ps2tx_program, PS2HOST_OFFSET);
    }
#endif
    if (_hostMode ) {
      ps2_program_init(ps2host_pio, ps2port[0].sm, ps2_offset, GPIO_PS2_CLK);
      ps2_program_init(ps2host_pio, ps2port[1].sm, ps2_offset, GPIO_PS2_CLK2);
    } else {
#ifndef NO_LEGACY_MODE
      ps2tx_program_init(ps2host_pio, ps2port[0].sm, ps2_offset, GPIO_PS2_CLK);
      ps2tx_program_init(ps2host_pio, ps2port[1].sm, ps2_offset, GPIO_PS2_CLK2);
#endif
    }

    hostMode = _hostMode;
  }
}

void ps2_EnablePortEx(uint8_t ch, bool enabled, uint8_t _hostMode) {
}

int ps2_GetChar(uint8_t ch) {
  return fifo_Get(&ps2port[ch].fifo_rx);
}

void ps2_InsertChar(uint8_t ch, uint8_t data) {
#ifndef NO_PS2_TX
  fifo_Put(&ps2port[ch].fifo, data);
#endif
}

void ps2_SendChar(uint8_t ch, uint8_t data) {
#ifndef NO_PS2_TX
  fifo_Put(&ps2port[ch].fifo, data);
  ps2_HealthCheck();
#endif
}

void ps2_HealthCheck() {
  int c;
  int ch = 0;

  for (int ch = 0; ch < NR_PS2; ch ++) {
    while ((c = readPs2(ps2host_pio, ps2port[ch].sm)) >= 0) {
      fifo_Put(&ps2port[ch].fifo_rx, c);
    }

#ifndef NO_PS2_TX
    while (!pio_sm_is_tx_fifo_full(ps2host_pio, ps2port[ch].sm) && (c = fifo_Get(&ps2port[ch].fifo)) >= 0) {
      writePs2(ps2host_pio, ps2port[ch].sm, c, hostMode);
    }
#endif
  }
}


void ps2_DebugQueues() {
  int n = 0;
  for (int i=0; i<NR_PS2; i++) {
    int ch;
    while ((ch = ps2_GetChar(i)) >= 0) {
      printf("[RX%d:%02X]", i, ch);
      n++;
    }
  }
  if (n) printf("\n");
#ifdef PIOINTS
  if (nrints) { printf("nrints = %d\n", nrints); nrints = 0; }
#endif
}

void ps2_EnablePort(uint8_t ch, bool enabled) {
  ps2_EnablePortEx(ch, enabled, 0);
}

void ps2_Debug() {}
#endif
