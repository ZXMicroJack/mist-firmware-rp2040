#include <stdio.h>
#include <stdint.h>

uint8_t block[] = {
0x50, 0x52, 0x49, 0x56, 0x41, 0x54, 0x45, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x00, 0x18, 0xA0,
0x23, 0x56, 0x23, 0x56, 0x00, 0x00, 0x18, 0xA0, 0x23, 0x56, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
0x41, 0x56, 0x46, 0x5F, 0x49, 0x4E, 0x46, 0x4F, 0x20, 0x20, 0x20, 0x13, 0x00, 0x64, 0x18, 0xA0,
0x23, 0x56, 0x23, 0x56, 0x00, 0x00, 0x18, 0xA0, 0x23, 0x56, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00,
0x53, 0x46, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x42, 0x49, 0x4E, 0x20, 0x00, 0xA7, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xE4, 0x9E, 0xA0, 0x92, 0x73, 0x00,
0x53, 0x46, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x42, 0x49, 0x4E, 0x20, 0x00, 0x7B, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xBE, 0x9E, 0xDC, 0xEB, 0x12, 0x00,
0x52, 0x50, 0x32, 0x55, 0x42, 0x4F, 0x4F, 0x54, 0x42, 0x49, 0x4E, 0x20, 0x00, 0x6D, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xBC, 0x9E, 0xB4, 0x89, 0x00, 0x00,
0x41, 0x62, 0x00, 0x69, 0x00, 0x74, 0x00, 0x66, 0x00, 0x69, 0x00, 0x0F, 0x00, 0x92, 0x6C, 0x00,
0x65, 0x00, 0x73, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x42, 0x49, 0x54, 0x46, 0x49, 0x4C, 0x45, 0x53, 0x20, 0x20, 0x20, 0x10, 0x00, 0x33, 0xD8, 0x44,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xD8, 0x44, 0x24, 0x57, 0xCA, 0x4C, 0x00, 0x00, 0x00, 0x00,
0x52, 0x50, 0x32, 0x55, 0x41, 0x50, 0x50, 0x20, 0x42, 0x49, 0x4E, 0x20, 0x00, 0x5F, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xBA, 0x9E, 0x74, 0xDA, 0x00, 0x00,
0x52, 0x50, 0x32, 0x4D, 0x42, 0x4F, 0x4F, 0x54, 0x42, 0x49, 0x4E, 0x20, 0x00, 0x51, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xB8, 0x9E, 0xF4, 0xCA, 0x00, 0x00,
0x52, 0x50, 0x32, 0x4D, 0x41, 0x50, 0x50, 0x20, 0x42, 0x49, 0x4E, 0x20, 0x00, 0x43, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xB5, 0x9E, 0xF8, 0x2A, 0x01, 0x00,
0x4C, 0x55, 0x54, 0x53, 0x20, 0x20, 0x20, 0x20, 0x42, 0x49, 0x4E, 0x20, 0x00, 0x33, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0xAD, 0x9E, 0x40, 0x94, 0x03, 0x00,
0x42, 0x4F, 0x4F, 0x54, 0x20, 0x20, 0x20, 0x20, 0x42, 0x49, 0x54, 0x20, 0x00, 0x12, 0xDD, 0x53,
0x24, 0x57, 0x24, 0x57, 0x01, 0x00, 0xDD, 0x53, 0x24, 0x57, 0x8B, 0x9E, 0xC5, 0xA2, 0x10, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint16_t crc = 0x60FB;

uint16_t crcab[] = {0xFB, 0x60};

uint16_t crc16(const uint8_t* data_p, uint32_t length) {
    uint8_t x;
//    uint16_t crc = 0xFFFF;
    uint16_t crc = 0x60fb;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

int main(int argc, char **argv) {
    printf("%04X\n", crc16(crcab, sizeof crcab));
    return 0;
}


