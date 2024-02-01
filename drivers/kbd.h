#ifndef _KBD_H
#define _KBD_H

void kbd_InitEx(uint8_t _mistMode);
void kbd_Process();
void kbd_SetMistMode(uint8_t _mistMode);
void ps2_SendCharX(uint8_t ch, uint8_t data);

#endif

