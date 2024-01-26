#include <stdio.h>
#include <stdint.h>
#include "bitfile.h"
#define DEBUG
#include "debug.h"

#define BLOCKSIZE 512

uint32_t bitfile_get_length(uint8_t *data, uint32_t filesize) {
  int i = 0;
  uint8_t tag;
  uint16_t len;
  
//   i = 3;
  /* is ZX3 file? */
  if (data[0] == 0xff && data[1] == 0xff) {
    return filesize ? filesize : 0xffffffff;
  }
  
  /* initial header */
  i += (data[i+0] << 8) | data[i+1];
  i += 2; // skip over length

  /* check version tag */
  len = (data[i+0] << 8) | data[i+1];
  if (len != 0x0001) {
    debug(("bitfile_get_length: unknown version %04X\n", len));
	  /* unknown version */
	  return 0;
  }
  i += 2; // skip over version

  while (i < (BLOCKSIZE-3)) {
    tag = data[i+0];
    if (tag == 'e') len = 4;
    else len = (data[i+1] << 8) | data[i+2];

    debug(("bitfile_get_length: tag (%02X) %c - %u (%04X)\n", tag, tag, len, len));

    if (tag == 'e' && i <= (BLOCKSIZE-7)) {
      uint32_t len = ((data[i+1]<<24) | (data[i+2]<<16) |
        (data[i+3]<<8) | data[i+4]) + i + 5;
      debug(("bitfile_get_length: detected len %d\n", len));
      return len;
    }
    i += len + (tag == 'e' ? 1 : 3);
  }
  debug(("bitfile_get_length: failed to detect len\n"));
  return 0;
}

#ifdef TEST
int main(int argc, char **argv) {
	FILE *f;
	uint8_t block[512];
  
  uint32_t filesize = 0;
  
	for(int i=1; i<argc; i++) {
		f = fopen(argv[i], "rb");

    if (f) {
      fseek(f, 0, SEEK_END);
      filesize = ftell(f);
      rewind(f);
      
      fread(block, 1, sizeof block, f);
      fclose(f);

      uint32_t len = bitfile_get_length(block, filesize);
      printf("%s[%u (%08X)] - %u (%08X)\n", argv[i], filesize, filesize, len, len);
    } else {
      printf("%s not found\n", argv[i]);
    }
	}
	return 0;
}
#endif

