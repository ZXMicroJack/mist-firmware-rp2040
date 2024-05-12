#ifndef _BITFILE_H
#define _BITFILE_H

#define A35T        0
#define A100T       1
#define A200T       2
#define AXXXT_TYPES 3
#define UNKNOWN     0xff

uint32_t bitfile_get_length(uint8_t *data, uint32_t filesize, uint8_t *chipType);

#endif

