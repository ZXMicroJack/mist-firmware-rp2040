#ifndef _RTC_H
#define _RTC_H

// MiST level functions
char GetRTC(unsigned char *d);
char SetRTC(unsigned char *d);

// functions to access internal RTC chip on AVillena hardware
uint8_t rtc_SetInternal();
uint8_t rtc_GetInternal();

void rtc_Init();
void rtc_AttemptSync();

#endif
