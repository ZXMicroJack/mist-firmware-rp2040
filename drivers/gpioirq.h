#ifndef _GPIOIRQ_H
#define _GPIOIRQ_H

#define IRQ_JAMMA       0
#define IRQ_RESET       1
#define IRQ_MAX         2

typedef void (*gpioirq_Callback)(uint gpio, uint32_t events);

void gpioirq_Init();
void gpioirq_SetCallback(uint8_t irq, gpioirq_Callback gpio_callback);
void gpioirq_Kill();

#endif