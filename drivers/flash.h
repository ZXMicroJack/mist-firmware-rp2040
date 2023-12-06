#ifndef _FLASH_H
#define _FLASH_H

int flash_EraseProgram(uint32_t addr, uint8_t *codeBuff, uint32_t rlen);
void system_Reset();
uint16_t crc16(const uint8_t* data_p, uint32_t length);
uint16_t crc16iv(const uint8_t* data_p, uint32_t length, uint16_t iv);

#endif
