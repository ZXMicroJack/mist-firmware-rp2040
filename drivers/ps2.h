#ifndef _PS2_H
#define _PS2_H

void ps2_Init();
void ps2_SendChar(uint8_t ch, uint8_t data);
void ps2_EnablePort(uint8_t ch, bool enabled);
int ps2_GetChar(uint8_t ch);
void ps2_SetGPIOListener(void (*cb)(uint gpio, uint32_t events));

#endif
