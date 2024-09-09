#ifndef _KBD_H
#define _KBD_H

void kbd_Init();
void kbd_Process();
void kbd_SetMistMode(uint8_t _mistMode);
//TODO clean
//void ps2_SendCharX(uint8_t ch, uint8_t data);
// fifo_t *kbd_GetFifo();
int kbd_Get();

#endif

