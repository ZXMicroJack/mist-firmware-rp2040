#ifndef _PS2_H
#define _PS2_H

#define PS2_WAIT_SCANCODE		0xEC

void ps2_InitEx(int mister);
void ps2_Init();
void ps2_SendChar(uint8_t ch, uint8_t data);
void ps2_SwitchMode(int hostMode); // TODO: replaces enable port - maybe remove
void ps2_EnablePortEx(uint8_t ch, bool enabled, uint8_t hostMode);
void ps2_EnablePort(uint8_t ch, bool enabled);
int ps2_GetChar(uint8_t ch);
void ps2_InsertChar(uint8_t ch, uint8_t data);
void ps2_SetGPIOListener(void (*cb)(uint gpio, uint32_t events));
void ps2_HealthCheck();
void ps2_DebugQueues();
void ps2_AttemptDetect(uint8_t clk, uint8_t data);

#endif
