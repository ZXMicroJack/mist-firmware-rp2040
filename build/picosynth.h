#ifndef _PICOSYNTH_H
#define _PICOSYNTH_H

void picosynth_Init();
void picosynth_Loop();
void picosynth_Suspend(uint8_t state);
int picosynth_GetStatus();

#endif
