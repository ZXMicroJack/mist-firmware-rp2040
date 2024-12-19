#ifndef _JAMMA_H
#define _JAMMA_H

/*
jamma FFFFFE00 - p1 button #4
jamma FFFFFD00 - p1 button #3
jamma FFFFFB00 - p1 button #2
jamma FFFFF700 - p1 button #1
jamma FFFFEF00 - p1 right
jamma FFFFDF00 - p1 left
jamma FFFFBF00 - p1 down
jamma FFFF7F00 - p1 up
jamma FF7FFF00 - p1 start
jamma FFDFFF00 - coin switch #1
jamma FFEFFF00 - test
jamma FFFFFF00 - video ground

jamma EFFFFF00 - p2 button #4
jamma DFFFFF00 - p2 button #3
jamma BFFFFF00 - p2 button #2
jamma 7FFFFF00 - p2 button #1
jamma FFFEFF00 - p2 right
jamma FFFDFF00 - p2 left
jamma FFFBFF00 - p2 down
jamma FFF7FF00 - p2 up
jamma F7FFFF00 - p2 start
jamma FDFFFF00 - coin switch #2
jamma FFFFFF00 - tilt switch
jamma FEFFFF00 - service
*/

// poll db9 joysticks
#define DB9_UP          0x80
#define DB9_DOWN        0x40
#define DB9_LEFT        0x20
#define DB9_RIGHT       0x10
#define DB9_BTN1        0x08
#define DB9_BTN2        0x04
#define DB9_BTN3        0x02
#define DB9_BTN4        0x01

uint8_t jamma_GetMisterMode();
void jamma_InitEx(uint8_t mister);
void jamma_Init();
void jamma_SetData(uint8_t inst, uint32_t data);
uint32_t jamma_GetData(uint8_t inst);
uint32_t jamma_GetDataAll();
int jamma_HasChanged();
void jamma_Kill();

uint32_t jamma_GetJamma();
uint8_t jamma_GetDepth();

// MJTODO remove
#if 0
void jamma_DetectPoll(uint8_t reset);
#endif

#endif
