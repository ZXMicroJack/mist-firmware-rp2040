#include <stdint.h>

static inline uint16_t swab16(uint16_t value)
{
        uint32_t result;

        result = ((value & 0xff00) >> 8) |
                 ((value & 0x00ff) << 8);

        return result;
}

static inline uint32_t swab32(uint32_t value)
{
        uint32_t result;

        result =  ((value & 0x000000ff)<<24) |
                  ((value & 0x0000ff00)<<8) |
                  ((value & 0x00ff0000)>>8) |
                  ((value & 0xff000000)>>24);
        return result;
}

