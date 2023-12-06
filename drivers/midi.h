#ifndef _UART_H
#define _UART_H

void midi_init();
int midi_get();
int midi_isdata();
int midi_get(uint8_t *d, int len);
void midi_setbaud(uint32_t baud);

#endif
