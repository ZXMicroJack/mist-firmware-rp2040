#ifndef _JAMMA_H
#define _JAMMA_H

/* build option - for now keep in, but if it creates issues remove it */
// #define JAMMA_JAMMA

/*
JAMMA
  P1
  TST COI STA UP_ DWN LFT RTG BT1 BT2 BT3 BT4

  P2
  SVC COI STA UP_ DWN LFT RTG BT1 BT2 BT3 BT4
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

typedef enum {
  MODE_DB9,
  MODE_JAMMA,
  MODE_MEGADRIVE,
  MODE_MAX
} JAMMA_MODE;


void jamma_SetMode(JAMMA_MODE new_mode);
JAMMA_MODE jamma_GetMode();

uint8_t jamma_GetMisterMode();
void jamma_InitEx(uint8_t mister);
void jamma_Init();
void jamma_SetData(uint8_t inst, uint32_t data);
uint32_t jamma_GetData(uint8_t inst);
uint32_t jamma_GetDataMD(uint8_t inst);

uint32_t jamma_GetDataAll();
int jamma_HasChanged();
void jamma_Kill();

#ifdef JAMMA_JAMMA
uint32_t jamma_GetJamma();
uint8_t jamma_GetDepth();
#endif

#endif
