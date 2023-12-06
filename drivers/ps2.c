#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
// #include "hardware/flash.h"
// #include "hardware/resets.h"

// #include "pico/bootrom.h"
#include "pico/stdlib.h"
// #include "pico/bootrom.h"

#include "pins.h"
#include "ps2.h"
#include "fifo.h"
#define DEBUG
#include "debug.h"

enum {
  PS2_IDLE,
  PS2_SUPRESS,
  PS2_SIGNALTRANSMIT,
  PS2_TRANSMIT,
  PS2_RECEIVE,
  PS2_PAUSE
};

#define NR_PS2  2
#define PS2_GUARDTIME PS2_RX_STATES
#define PS2_PAUSETIME (1000)
#define PS2_RX_STATES 22

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
  fifo_t fifo_rx;
  
} ps2_t;

ps2_t ps2port[NR_PS2];

static bool ps2_timer_callback(struct repeating_timer *t) {
  ps2_t *ps2 = (ps2_t *)t->user_data;
  if (ps2->ps2_state != PS2_TRANSMIT) {
    return false;
  }
  
  if (ps2->ps2_states > 2) {
    if (ps2->ps2_states & 1) ps2->ps2_data = (ps2->ps2_data >> 1) | (gpio_get(ps2->gpio_data) ? 0x200 : 0);
  } else {
    gpio_set_dir(ps2->gpio_data, GPIO_OUT);
    gpio_put(ps2->gpio_data, 0);
  }
  gpio_put(ps2->gpio_clk, !(ps2->ps2_states & 1));
  
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
}

static bool ps2_timer_callback_rx(struct repeating_timer *t) {
  ps2_t *ps2 = (ps2_t *)t->user_data;
  
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
    add_repeating_timer_us(40, ps2_timer_callback_rx, &ps2port[ch], &ps2port[ch].ps2timer_rx);
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

static void gpio_callback(uint gpio, uint32_t events) {
  if (gpio_cb) gpio_cb(gpio, events);
  
  if (events & 0x4) { // fall
    for (int i=0; i<NR_PS2; i++) {
      if (gpio == ps2port[i].gpio_clk && ps2port[i].ps2_state == PS2_IDLE) {
        ps2port[i].ps2_state = PS2_SUPRESS;
      } else if (gpio == ps2port[i].gpio_data && ps2port[i].ps2_state == PS2_SUPRESS) {
        ps2port[i].ps2_state = PS2_SIGNALTRANSMIT;
      }
    }
  }
    
  if (events & 0x8) { // fall
    for (int i=0; i<NR_PS2; i++) {
      if (gpio == ps2port[i].gpio_clk) {
        if (ps2port[i].ps2_state == PS2_SIGNALTRANSMIT) {
          ps2port[i].ps2_state = PS2_TRANSMIT;
          ps2port[i].ps2_states = 22;
          gpio_set_dir(ps2port[i].gpio_clk, GPIO_OUT);
          add_repeating_timer_us(40, ps2_timer_callback, &ps2port[i], &ps2port[i].ps2timer);
        
        } else if (ps2port[i].ps2_state == PS2_SUPRESS) {
          ps2port[i].ps2_state = PS2_IDLE;
        }
      }
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
  ps2port[1].gpio_clk = GPIO_PS2_CLK2;
  ps2port[1].gpio_data = GPIO_PS2_DATA2;
  ps2port[0].channel = 0;
  ps2port[1].channel = 1;
  fifo_Init(&ps2port[0].fifo);
  fifo_Init(&ps2port[1].fifo);
  fifo_Init(&ps2port[0].fifo_rx);
  fifo_Init(&ps2port[1].fifo_rx);
  
  for (int i=0; i<sizeof lut; i++) {
    gpio_init(lut[i]);
    gpio_set_dir(lut[i], GPIO_IN);
  }
  
  gpio_set_irq_callback(gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);
}

void ps2_EnablePort(uint8_t ch, bool enabled) {
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

int ps2_GetChar(uint8_t ch) {
  return fifo_Get(&ps2port[ch].fifo_rx);
}

#ifdef DEBUG
void ps2_Debug() {
  debug(("ps2port[0].ps2_states = %d .ps2_state = %d\n", ps2port[0].ps2_states, ps2port[0].ps2_state));
  debug(("ps2port[1].ps2_states = %d .ps2_state = %d\n", ps2port[1].ps2_states, ps2port[1].ps2_state));
  for (int i=0; i<NR_PS2; i++) {
    int ch;
    while ((ch = ps2_GetChar(i)) >= 0) {
      printf("[RX%d:%02X]", i, ch);
    }
  }
}
#endif
