/*
Copyright 2005, 2006, 2007 Dennis van Weeren
Copyright 2008, 2009 Jakub Bednarski

This file is part of Minimig

Minimig is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Minimig is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// --== based on the work by Dennis van Weeren and Jan Derogee ==--
// 2008-10-03 - adaptation for ARM controller
// 2009-07-23 - clean-up and some optimizations
// 2009-11-22 - multiple sector read implemented


// FIXME - get capacity from SD card

//1GB:
//CSD:
//0000: 00 7f 00 32 5b 59 83 bc f6 db ff 9f 96 40 00 93   ...2[Y.��.�.@.�
//CID:
//0000: 3e 00 00 34 38 32 44 00 00 73 2f 6f 93 00 c7 cd   >..482D..s/o�...


#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "spi.h"

#include "mmc.h"

#include "drivers/pio_spi.h"
#include "drivers/sdcard.h"
// #define DEBUG
#include "drivers/debug.h"


static pio_spi_inst_t *spi = NULL;

unsigned char MMC_Init(void) {
  if (spi == NULL) spi = sd_hw_init();

  if (sd_init_card(spi)) {
    return CARDTYPE_NONE;
  }

  return sd_is_sdhc() ? CARDTYPE_SDHC : CARDTYPE_SD;
}

unsigned char MMC_Read(unsigned long lba, unsigned char *pReadBuffer) {
  debug(("MMC_Read: lba %d pReadBuffer %x\n", lba, pReadBuffer));
  if (!sd_readsector(spi, lba, (uint8_t *)pReadBuffer)) {
      return 1;
  }
  return 0;
}

unsigned char MMC_Write(unsigned long lba, const unsigned char *pWriteBuffer) {
  debug(("MMC_Write: lba %d pReadBuffer %x\n", lba, pWriteBuffer));
  if (!sd_writesector(spi, lba, (uint8_t *)pWriteBuffer)) {
      return 1;
  }
  return 0;
}

unsigned char MMC_ReadMultiple(unsigned long lba, unsigned char *pReadBuffer, unsigned long nBlockCount) {
  debug(("MMC_ReadMultiple: lba %d pReadBuffer %x nBlockCount %d\n", lba, pReadBuffer, nBlockCount));
  while (nBlockCount --) {
    if (sd_readsector(spi, lba, (uint8_t *)pReadBuffer)) {
      return 0;
    }
    lba++;
    if (pReadBuffer != NULL) pReadBuffer += 512;
  }
  return 1;
}

unsigned char MMC_WriteMultiple(unsigned long lba, const unsigned char *pWriteBuffer, unsigned long nBlockCount) {
  debug(("MMC_WriteMultiple: lba %d pReadBuffer %x nBlockCount %d\n", lba, pWriteBuffer, nBlockCount));
  while (nBlockCount--) {
      if (sd_writesector(spi, lba, (uint8_t *)pWriteBuffer)) {
          return 0;
      }
      lba ++;
      pWriteBuffer += 512;
  }
  return 1;
}

unsigned char MMC_GetCSD(unsigned char *b) {
  return sd_cmd9(spi, b);
}

unsigned char MMC_GetCID(unsigned char *b) {
  return sd_cmd10(spi, b);
}

// frequently check if card has been removed
unsigned char MMC_CheckCard() {
  return 1; // cannot hot remove SD card on NeptUno / ZXTres
}

unsigned char MMC_IsSDHC() {
  return sd_is_sdhc();
}

// MMC get capacity
unsigned long MMC_GetCapacity() {
	unsigned long result=0;
	unsigned char CSDData[16];
 
	MMC_GetCSD(CSDData);

	if ((CSDData[0] & 0xC0)==0x40)   //CSD Version 2.0 - SDHC
	{
	  result=(CSDData[7]&0x3f)<<26;
	  result|=CSDData[8]<<18;
	  result|=CSDData[9]<<10;
	  result+=1024;
			return(result);
	}
	else
	{    
	  int blocksize=CSDData[5]&15;	// READ_BL_LEN
	  blocksize=1<<(blocksize-9);		// Now a scalar:  physical block size / 512.
	  result=(CSDData[6]&3)<<10;
	  result|=CSDData[7]<<2;
	  result|=(CSDData[8]>>6)&3;		// result now contains C_SIZE
	  int cmult=(CSDData[9]&3)<<1;
	  cmult|=(CSDData[10]>>7) & 1;
	  ++result;
	  result<<=cmult+2;
	  result*=blocksize;	// Scale by the number of 512-byte chunks per block.
	  return(result);
	}
}

//assume MMC is inserted
char mmc_inserted() {
  return 1;
}
