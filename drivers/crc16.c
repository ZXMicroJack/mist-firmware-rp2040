#include <stdint.h>
#include "crc16.h"

uint16_t crc16(const uint8_t* data_p, uint32_t length) {
  return crc16iv(data_p, length, 0xffff);
}

uint16_t crc16iv(const uint8_t* data_p, uint32_t length, uint16_t iv) {
    uint8_t x;
    uint16_t crc = iv;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}



