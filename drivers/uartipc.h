#ifndef _UARTIPC_H
#define _UARTIPC_H

uint8_t uart_ProcessPacket(uint8_t cmd, uint8_t *data, uint16_t len);
void uart_Loop();
void uart_Init();

#endif
