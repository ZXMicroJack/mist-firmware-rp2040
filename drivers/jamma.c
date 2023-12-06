#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

// #include "hardware/clocks.h"
// #include "hardware/structs/clocks.h"
#include "pico/stdlib.h"

#include "pins.h"
#include "ps2.h"
#include "jamma.h"
#define DEBUG
#include "debug.h"


#include "jamma.pio.h"

// uint16_t shift_data = 0xf9f9;
// uint16_t reload_data = 0x0008;
uint16_t reload_data = 0x0000;
uint8_t joydata[2];

PIO jamma_pio = pio0;
unsigned jamma_sm = 2;

static void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == GPIO_RP2U_XLOAD) {
//     shift_data = reload_data;
    
//     pio_sm_put_blocking(jamma_pio, jamma_sm, 0x0000ffff); // nothing
    while (!pio_sm_is_tx_fifo_full(jamma_pio, jamma_sm))
      pio_sm_put_blocking(jamma_pio, jamma_sm, reload_data); // 
  }
  
//   if (gpio == GPIO_RP2U_XSCK) {
//     gpio_put(GPIO_RP2U_XDATA, shift_data & 1);
//     shift_data >>= 1;
//   }
}

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
//   gpio_set_irq_enabled(GPIO_RP2U_XSCK, GPIO_IRQ_EDGE_RISE, true);

//   gpio_pull_up(GPIO_RP2U_XSCK);
//   gpio_pull_up(GPIO_RP2U_XLOAD);
  
//   jamma_program_init(kPIO pio, uint sm, uint offset, uint data_pin)  
}

void jamma_SetData(uint8_t inst, uint32_t data) {
  joydata[inst] = data & 0xff;
  reload_data = (joydata[1] << 8) | joydata[0];
}
