#ifndef _CRC16_H
#define _CRC16_H

uint16_t crc16(const uint8_t* data_p, uint32_t length);
uint16_t crc16iv(const uint8_t* data_p, uint32_t length, uint16_t iv);


#endif


