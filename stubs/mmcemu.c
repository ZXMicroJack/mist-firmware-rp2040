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



#define CARDFILE    "card.pt"


int read_sector(int sector, uint8_t *buff) {
  FILE *f = fopen(CARDFILE, "rb");
  if (!f) return 1;
  fseeko64(f, (uint64_t)sector*(uint64_t)512, SEEK_SET);
  fread(buff, 1, 512, f);
  fclose(f);
  return 0;
}

int write_sector(int sector, uint8_t *buff) {
  FILE *f = fopen(CARDFILE, "wb+");
  fseek(f, sector*512, SEEK_SET);
  fwrite(buff, 1, 512, f);
  fclose(f);
  return 0;
}



// variables
static unsigned char crc;
static unsigned long timeout;
static unsigned char response;
static unsigned char CardType = CARDTYPE_SD;
static uint32_t capacity = 0;

// internal functions
static void MMC_CRC(unsigned char c) RAMFUNC;
static unsigned char MMC_Command(unsigned char cmd, unsigned long arg) RAMFUNC;
static unsigned char MMC_CMD12(void);

RAMFUNC unsigned char MMC_CheckCard() {
	printf("MMC_CheckCard\n");
  return 1; 
}

static RAMFUNC char check_card() {
  return 1;
}

// init memory card
unsigned char MMC_Init(void)
{
	printf("MMC_Init\n");
  FILE *f = fopen(CARDFILE, "rb");
  if (f) {
	  fseeko64(f, 0, SEEK_END);
	  capacity = ftello64(f);
	  fclose(f);
	  CardType = CARDTYPE_SD;
  } else {
	  CardType = CARDTYPE_NONE;
  }
    return CardType;
}

static unsigned char MMC_GetCXD(unsigned char cmd, unsigned char *ptr) {
  int i;
  EnableCard();
  
  if (MMC_Command(cmd,0)) {
    iprintf("CMD%d (GET_C%cD): invalid response 0x%02X \r", 
	    (cmd==CMD9)?9:10, (cmd==CMD9)?'S':'I', response);
    DisableCard();
    return(0);
  }
  
  // now we are waiting for data token, it takes around 300us
  timeout = 0;
  while ((SPI(0xFF)) != 0xFE) {
    if (timeout++ >= 1000000) { // we can't wait forever
      iprintf("CMD%d (GET_C%cD): no data token!\r", 
	      (cmd==CMD9)?9:10, (cmd==CMD9)?'S':'I');
      DisableCard();
      return(0);
    }
  }
  
  for (i = 0; i < 16; i++)
    ptr[i]=SPI(0xFF);
  
  DisableCard();
  
  return(1);
}


// Read CSD register
unsigned char MMC_GetCSD(unsigned char *data) {
	memset(data, 0, 16);
	data[0] = 0x40;
	data[1] = 0x0e;
	data[3] = 0x32;
	data[4] = 0x5b;
	data[5] = 0x59;
	data[6] = 0x90;
	data[7] = (capacity >> 26) & 0xff;
	data[8] = (capacity >> 18) & 0xff;
	data[9] = (capacity >> 10) & 0xff;
	data[10] = 0x5f;
	data[11] = 0xc0;
	return 1;
}

// Read CID register
unsigned char MMC_GetCID(unsigned char *cid) {
	memset(cid, 0, 16);
	return 1;
}

// MMC get capacity
unsigned long MMC_GetCapacity()
{
	return capacity;
}

RAMFUNC static unsigned char MMC_WaitBusy(unsigned long timeout)
{
	return 1;
}

// Read single 512-byte block
RAMFUNC unsigned char MMC_Read(unsigned long lba, unsigned char *pReadBuffer)
{
	return read_sector(lba, pReadBuffer) ? 0 : 1;
}

// read multiple 512-byte blocks
unsigned char MMC_ReadMultiple(unsigned long lba, unsigned char *pReadBuffer, unsigned long nBlockCount)
{
	for (int i=0; i<nBlockCount; i++) {
		if (read_sector(lba, pReadBuffer)) return 0;
		pReadBuffer += 512;
		lba ++;
	}
	return 1;
}

// write 512-byte block
unsigned char MMC_Write(unsigned long lba, const unsigned char *pWriteBuffer)
{
	return write_sector(lba, pWriteBuffer) ? 0 : 1;
}

// write 512-byte block
unsigned char MMC_WriteMultiple(unsigned long lba, const unsigned char *pWriteBuffer, unsigned long nBlockCount)
{
	for (int i=0; i<nBlockCount; i++) {
		if (read_sector(lba, pWriteBuffer)) return 0;
		pWriteBuffer += 512;
		lba ++;
	}
	return 1;
}

// MMC command
static RAMFUNC unsigned char MMC_Command(unsigned char cmd, unsigned long arg)
{
  unsigned char c,b;

    crc = 0;

    // flush spi, give card a moment to wake up (needed for old 2GB Panasonic card)
    //    spi_n(0xff, 8);  // this is not flash save if not in ram
    // (wait for busy instead)
    //for(b=0;b<8;b++) SPI(0xff); 
    if (!MMC_WaitBusy(1000)) {
        return(0x80); // busy forever?
    }
    SPI(cmd);
    MMC_CRC(cmd);

#if 1
    // code 100 bytes smaller than below
    for(b=0;b<4;b++) {
      c = ((unsigned char*)&arg)[3];
      SPI(c);
      MMC_CRC(c);
      arg <<= 8;
    }
#else
    c = (unsigned char)(arg >> 24);
    SPI(c);
    MMC_CRC(c);

    c = (unsigned char)(arg >> 16);
    SPI(c);
    MMC_CRC(c);
    
    c = (unsigned char)(arg >> 8);
    SPI(c);
    MMC_CRC(c);
    
    c = (unsigned char)(arg);
    SPI(c);
    MMC_CRC(c);
#endif
    
    crc <<= 1;
    crc++;
    SPI(crc);

    unsigned char Ncr = 100;  // Ncr = 0..8 (SD) / 1..8 (MMC)
    do
        response = SPI(0xFF); // get response
    while (response == 0xFF && Ncr--);

    return response;
}


// stop multi block data transmission
static unsigned char MMC_CMD12(void)
{
    SPI(CMD12); // command
    SPI(0x00);
    SPI(0x00);
    SPI(0x00);
    SPI(0x00);
    SPI(0x61); // real CRC7
    SPI(0xFF); // skip stuff byte

    unsigned char Ncr = 100;  // Ncr = 0..8 (SD) / 1..8 (MMC)
    do
    {    response = SPI(0xFF); // get response
//        RS232(response);
    } while (response == 0xFF && Ncr--);

    // Let the firmware proceed while the card is busy
/*
    timeout = 0;
    while ((SPI(0xFF)) == 0x00) // wait until the card is not busy
    {   // RS232('+');
        if (timeout++ >= 1000000)
        {
	  //            iprintf("CMD12 (STOP_TRANSMISSION): busy wait timeout!\r");
            DisableCard();
            return(0);
        }
    }
*/
    return response;
}


// MMC CRC calc
static RAMFUNC void MMC_CRC(unsigned char c)
{
    unsigned char i;

    for (i = 0; i < 8; i++)
    {
        crc <<= 1;
        if (c & 0x80)
            crc ^= 0x09;
        if (crc & 0x80)
            crc ^= 0x09;
        c <<= 1;
    }
}

unsigned char MMC_IsSDHC(void) {
  return(CardType == CARDTYPE_SDHC);
}
