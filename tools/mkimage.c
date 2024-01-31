#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RP2X_MIDIAPP    0x1CE07AC1
#define RP2X_USBAPP     0xE31F853E
#define RP2X_MIDILUTS   0xed27055a
#define RP2X_MIDISF1    0x7d2413d1
#define RP2X_MIDISF2    0xd582ad64
#define RP2M_BOOTSTRAP  0x4835b955
#define RP2U_BOOTSTRAP  0x9a584fc0

uint32_t img[] = {
  RP2X_USBAPP,
  RP2X_MIDIAPP,
  RP2X_MIDILUTS,
  RP2X_MIDISF1,
  RP2X_MIDISF2,
  RP2M_BOOTSTRAP,
  RP2U_BOOTSTRAP
};


uint16_t version;

uint16_t crc16(const uint8_t* data_p, uint32_t length) {
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

int main(int argc, char **argv) {
  int midi = 0;
  int blank = 0;
  char *inputfile = NULL;
  char *outputfile = NULL;
  FILE *fin, *fout;
  
  uint8_t blk[4096];
  
  for (int i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'v': // version
          version = strtol(argv[i+1], NULL, 16);
          break;
        case 'i': // input
          inputfile = argv[i+1];
          i++;
          break;
          
        case 'o': // output
          outputfile = argv[i+1];
          i++;
          break;

        case 'm': // is midi rp2
          midi = 1;
          break;

        case 'g': // set image
          midi = atoi(argv[i+1]);
          i++;
          break;
        
        case 'b': // create blank image
          blank = 1;
          break;
        
      }
    }
  }
  
  if (!inputfile || !outputfile) {
    printf("%s: -i inputbinary -o output -v version [-m (is midi)]\n", argv[0]);
    return 1;
  }
  
  fin = fopen(inputfile, "rb");
  if (!fin) {
    printf("%s: cannot open input file %s\n", argv[0], inputfile);
    return 1;
  }

  fout = fopen(outputfile, "wb");
  if (!fin) {
    printf("%s: cannot open input file %s\n", argv[0], inputfile);
    fclose(fin);
    return 1;
  }
  
  
  
  memset(blk, 0, sizeof blk);
  uint32_t *info = (uint32_t *)blk;
//   info[0] = midi ? RP2X_MIDIAPP : RP2X_USBAPP;
  info[0] = img[midi];
  
  fseek(fin, 0, SEEK_END);
  info[1] = ftell(fin);
  rewind(fin);
  
  uint8_t *img = (uint8_t *)malloc(info[1]);
  fread(img, 1, info[1], fin);
  fclose(fin);
  
  info[2] = crc16(img, info[1]);
  info[3] = version;

  if (blank) {
    info[2] = 0;
    info[1] = 4096;
    memset(img, 0xff, 4096);
  }
  
  fwrite(blk, 1, sizeof blk, fout);
  fwrite(img, 1, info[1], fout);
  fclose(fout);

  printf("%s: image written %d bytes crc %04X\n", argv[0], info[1], info[2]);
  
  return 0;
}
