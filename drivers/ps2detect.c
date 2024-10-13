#include <stdio.h>

#include "pico/time.h"
#include "hardware/gpio.h"
#include "ps2.h"
// #define DEBUG
#include "debug.h"

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

static void ps2_InitPin(uint8_t pin) {
  gpio_init(pin);
  gpio_put(pin, 0);
  gpio_set_dir(pin, GPIO_IN);
  // gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA);
  gpio_pull_up(pin);
  // gpio_pull_down(pin);
  // gpio_disable_pulls(pin);
}

static void ps2_gpio_put(uint8_t gpio, uint8_t state) {
#if 0
  if (state) {
    gpio_set_dir(gpio, GPIO_IN);
  } else {
    // gpio_put(gpio, 0);
    gpio_set_dir(gpio, GPIO_OUT);
  }
#else
  gpio_put(gpio, state);
#endif
}

void ps2_set_dir(uint8_t gpio, uint8_t dir) {
#if 1
  gpio_set_dir(gpio, dir);
#endif
}


static int ps2_IssueCmd(uint8_t gpio_clk, uint8_t gpio_data, uint8_t data) {
  debug(("ps2_IssueCmd\n"));
  ps2_InitPin(gpio_clk);
  ps2_InitPin(gpio_data);
  // gpio_init(gpio_data);
  // gpio_set_dir(gpio_data, GPIO_OUT);
  // gpio_disable_pulls(gpio_clk);
  // gpio_disable_pulls(gpio_data);
  // gpio_pull_down(gpio_clk);
  // gpio_pull_down(gpio_data);

  uint32_t states = (data << 1) | (parity(data) << 9) | 0x400;
  uint64_t timeout = time_us_64() + 30000;

  ps2_set_dir(gpio_clk, GPIO_OUT);
  ps2_set_dir(gpio_data, GPIO_OUT);

  ps2_gpio_put(gpio_clk, 1);
  ps2_gpio_put(gpio_data, 1);
  sleep_ms(10);

  // ps2_gpio_put(gpio_clk, 0);
  // sleep_ms(1);
  // ps2_gpio_put(gpio_data, 0);
  // 
  // ps2_gpio_put(gpio_clk, 0);
  ps2_gpio_put(gpio_data, 1);
  ps2_gpio_put(gpio_clk, 0);
  // sleep_ms(1);
  sleep_us(500);

  ps2_gpio_put(gpio_clk, 1);
  ps2_gpio_put(gpio_data, 0);
  
  ps2_set_dir(gpio_clk, GPIO_IN);

  for (int i=0; i<11; i++) {
    ps2_gpio_put(gpio_data, states & 1);
    // gpio_put(gpio_data, states & 1);
    while (gpio_get(gpio_clk) && time_us_64() < timeout)
      tight_loop_contents();
    while (!gpio_get(gpio_clk) && time_us_64() < timeout)
      tight_loop_contents();
    states >>= 1;

    if (time_us_64() >= timeout) {
      debug(("timeout at %d\n", i));
      break;
    }
  }

  // ps2_gpio_put(gpio_clk, 1);
  // ps2_gpio_put(gpio_data, 1);
  gpio_init(gpio_clk);
  gpio_init(gpio_data);


  if (time_us_64() >= timeout) {
    debug(("Timed out!\n"));
    return 1;
  } else {
    debug(("Sent ok!\n"));
    return 0;
  }
}

void ps2_AttemptDetect(uint8_t clk, uint8_t data) {
  int timeout = 5;
  while (timeout--) {
    sleep_ms(350);
    if (!ps2_IssueCmd(clk, data, 0x77))
      break;
    if (!ps2_IssueCmd(clk, data, 0xff))
      break;
    if (!ps2_IssueCmd(clk, data, 0x77))
      break;
  }

  if (timeout > 0) {
    debug(("YES!!!.\n"));
  } else {
    debug(("SORRY!\n"));
  }
}



