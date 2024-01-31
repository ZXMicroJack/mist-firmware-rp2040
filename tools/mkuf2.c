#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define static_assert(a, b)
#include "uf2.h"

#define BLOCK_SIZE  256
// #define BLOCK_SIZE sizeof blk.data

uint32_t getSize(char *s) {
  FILE *fin = fopen(s, "rb");
  uint32_t size = 0;
  if (fin) {
    fseek(fin, 0, SEEK_END);
    size = ftell(fin);
    rewind(fin);
    fclose(fin);
  }
  return size;
}

int main(int argc, char **argv) {
  int i;
  
  if (sizeof (struct uf2_block) != 512) {
    printf("No bloody use!\n");
    return 1;
  }
  
  if (argc < 4) {
    printf("Usage: mkuf2 <output uf2> <input binary> <start_address>\n");
    if (argv[1][0] == '-') {
      struct uf2_block blk;
      FILE *f = fopen(&argv[1][1], "rb");
      while (!feof(f)) {
        fread(&blk, 1, sizeof blk, f);
        printf("m0:%08X m1:%08X fl:%X ta:%08X ps:%X bn:%d nb:%d fs:%08X me:%08X\n",
               blk.magic_start0, blk.magic_start1, blk.flags, blk.target_addr, 
               blk.payload_size, blk.block_no, blk.num_blocks, blk.file_size,
               blk.magic_end);
        for (int i=0; i<476; i+=16) {
          int j;
          for (j=0; j<16 && (i+j)<476; j++) {
            printf("%02X ", blk.data[i+j]);
          }
          for (; j<16; j++) {
            printf("   ");
          }
          for (j=0; j<16 && (i+j)<476; j++) {
            printf("%c", (blk.data[i+j] >= ' ' && blk.data[i+j] < 128) ? blk.data[i+j] : '.');
          }
          printf("\n");
        }
      }
    }
    return 1;
  }
  
  uint32_t nrBlocks = 0;
  
  for (i=2; i<argc; i+=2) {
    uint32_t imageSize = getSize(argv[i]);
    if (imageSize == 0) {
      printf("Error: Couldn't open image %s\n", argv[i]);
      return 1;
    }
    nrBlocks += (imageSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
  }
  
  struct uf2_block blk;
  memset(&blk, 0, sizeof blk);
  blk.magic_start0 = UF2_MAGIC_START0;
  blk.magic_start1 = UF2_MAGIC_START1;
  blk.magic_end = UF2_MAGIC_END;
  blk.file_size = RP2040_FAMILY_ID;
  blk.flags = UF2_FLAG_FAMILY_ID_PRESENT;
  blk.num_blocks = nrBlocks;
  
  
  FILE *fin, *fout;

  fout = fopen(argv[1], "wb");
  if (fout) {
    for (i=2; i<argc; i+=2) {
      fin = fopen(argv[i], "rb");
      if (fin) {
        // find length of file
        fseek(fin, 0, SEEK_END);
        uint32_t imageSize = ftell(fin);
        rewind(fin);
        
        // read begin address
        blk.target_addr = strtol(argv[i+1], NULL, 16);
        
        printf("Info: Writing image of size %d to address 0x%08X\n",
              imageSize, blk.target_addr);
        
        while (imageSize) {
          uint32_t thisSize = imageSize > BLOCK_SIZE ? BLOCK_SIZE : imageSize;
          memset(&blk.data[thisSize], 0, sizeof blk.data-thisSize);
          fread(blk.data, 1, thisSize, fin);
          blk.payload_size = BLOCK_SIZE;
          
          fwrite(&blk, 1, sizeof blk, fout);
          blk.block_no++;
//           printf("target_addr = %08X\n", blk.target_addr);
          blk.target_addr += thisSize;
          imageSize -= thisSize;
        }
        fclose(fin);
        
      } else {
        fclose(fout);
        printf("Error: Couldnt open input file %s\n", argv[i]);
        return 1;
      }
    }
    fclose(fout);
      
  } else {
    printf("Error: Couldnt open output file\n");
    return 1;
  }
  
	return 0;

}
