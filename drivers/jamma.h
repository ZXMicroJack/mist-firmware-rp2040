#ifndef _JAMMA_H
#define _JAMMA_H

void jamma_InitEx(uint8_t mister);
void jamma_Init();
void jamma_SetData(uint8_t inst, uint32_t data);
uint32_t jamma_GetData(uint8_t inst);
uint32_t jamma_GetDataAll();
int jamma_HasChanged();
void jamma_Kill();

#endif
