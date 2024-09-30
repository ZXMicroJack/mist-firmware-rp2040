#include "pico/stdlib.h"

#include "gpioirq.h"

static gpioirq_Callback callback[IRQ_MAX];

static void gpio_callback(uint gpio, uint32_t events) {
  for (int i=0; i<IRQ_MAX; i++) {
    if (callback[i]) callback[i](gpio, events);
  }
}

void gpioirq_Init() {
  for (int i=0; i<IRQ_MAX; i++) {
    callback[i] = NULL;
  }
  gpio_set_irq_callback(gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);
}

void gpioirq_SetCallback(uint8_t irq, gpioirq_Callback gpio_callback) {
  if (irq < IRQ_MAX) {
    callback[irq] = gpio_callback;
  }
}


void gpioirq_Kill() {
  gpio_set_irq_callback(NULL);
  irq_set_enabled(IO_IRQ_BANK0, false);
}
