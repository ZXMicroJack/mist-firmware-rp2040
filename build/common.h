#ifndef _COMMON_H
#define _COMMON_H

extern uint8_t legacy_mode;
extern uint8_t inhibit_reset;

void mb2_SendPS2(uint8_t *data, uint8_t len);
void mb2_SendMessages();


#endif