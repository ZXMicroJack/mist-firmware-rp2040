#ifndef _COMMON_H
#define _COMMON_H

extern uint8_t legacy_mode;
extern uint8_t inhibit_reset;
extern uint8_t core_detect;

void mb2_SendPS2(uint8_t *data, uint8_t len);
void mb2_SendMessages();

int ResetFPGA();

void DB9Update(uint8_t joy_num, uint8_t usbjoy);
void platform_UpdateDebugMode();

#endif