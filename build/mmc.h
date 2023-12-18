#ifndef MMC_H
#define MMC_H

#include "spi.h"
#include "hardware.h"

#define CARDTYPE_NONE 0
#define CARDTYPE_MMC  1
#define CARDTYPE_SD   2
#define CARDTYPE_SDHC 3

unsigned char MMC_Init(void);
unsigned char MMC_Read(unsigned long lba, unsigned char *pReadBuffer) RAMFUNC;
unsigned char MMC_Write(unsigned long lba, const unsigned char *pWriteBuffer);
unsigned char MMC_ReadMultiple(unsigned long lba, unsigned char *pReadBuffer, unsigned long nBlockCount);
unsigned char MMC_WriteMultiple(unsigned long lba, const unsigned char *pWriteBuffer, unsigned long nBlockCount);
unsigned char MMC_GetCSD(unsigned char *);
unsigned char MMC_GetCID(unsigned char *);
unsigned long MMC_GetCapacity(); // Returns the capacity in 512 byte blocks
unsigned char MMC_CheckCard();   // frequently check if card has been removed
unsigned char MMC_IsSDHC();

#endif

