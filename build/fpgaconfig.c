#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "errors.h"
#include "hardware.h"
#include "fdd.h"
#include "user_io.h"
#include "config.h"
#include "boot.h"
#include "osd.h"
#include "fpga.h"
#include "tos.h"
#include "mist_cfg.h"
#include "settings.h"
#include "usb/joymapping.h"

#include "drivers/fpga.h"
// #define DEBUG
#include "drivers/debug.h"

#define BUFFER_SIZE     16 // PACKETS
#define BUFFER_SIZE_MASK  0xf
#define NR_BLOCKS(a) (((a)+511)>>9)

#define BUFFER_FPGA

typedef struct {
  FIL file;
  uint32_t size;
  uint8_t error;
#ifdef BUFFER_FPGA
  uint8_t buff[BUFFER_SIZE][512];
  uint8_t l, r, c;
//   void (*closing_fn)(void *);
//   void *closing_user;
#endif
} configFpga;

static uint8_t read_next_block(void *ud, uint8_t *data) {
  UINT br;
  configFpga *cf = (configFpga *)ud;

  debug(("read_next_block cf->c = %d\n", cf->c));
  if (f_read(&cf->file, data, 512, &br) != FR_OK) {
    cf->error = 1;
    return 0;
  }

  if (br >= cf->size) {
    cf->error = 0;
    return 0;
  }
  cf->size -= br;
  return 1;
}

// #ifdef BUFFER_FPGA

// typedef struct {
//   read_lba_t rl;
//   uint8_t buff[BUFFER_SIZE][512];
//   uint8_t l, r, c;
// } buffered_read_lba_t;

#define brl_inc(a) (((a) + 1) & BUFFER_SIZE_MASK)

static void read_next_block_buffered_fill(configFpga *brl) {
  uint8_t result;

  if (!brl->size) return; // no more blocks don't try to read more.
  
  while (brl->c < BUFFER_SIZE) {
     result = read_next_block(brl, brl->buff[brl->r]);
     if (result) {
       brl->r = brl_inc(brl->r);
       brl->c ++;
     } else {
#if 0
       if (brl->closing_fn) {
         brl->closing_fn(brl->closing_user);
         brl->closing_fn = NULL;
       }
#endif
       break;
    }
  }
}

uint8_t read_next_block_buffered(void *user_data, uint8_t *block) {
  uint8_t result = 0;
  
  configFpga *brl = (configFpga *)user_data;
  debug(("read_next_block_buffered cf->c = %d\n", brl->c));
  if (brl->c) {
    memcpy(block, brl->buff[brl->l], 512);
    brl->c --;
    brl->l = brl_inc(brl->l);
    result = 1;
  }
  read_next_block_buffered_fill(brl);
  return result;
}

//MJ TODO remove
int ResetFPGA() {
  /* initialise fpga */
  fpga_initialise();
  fpga_claim(true);

  /* now configure */
  return fpga_reset();
}

#ifdef BOOT_FLASH_ON_ERROR
void BootFromFlash() {
  debug(("BootFromFlash!\n"));
  fpga_initialise();
  fpga_claim(false);
  int r = fpga_reset();
  debug(("fpga_reset returns %d\n", r));
}
#endif

//MJ TODO remove
unsigned char ConfigureFpgaEx(const char *bitfile, uint8_t fatal, uint8_t reset) {
  configFpga cf;
  debug(("ConfigureFpgaEx: %s\n", bitfile ? bitfile : "null"));

#ifdef XILINX // go to flash boot
  if (bitfile && !strcmp(bitfile, "ZXTRES.BIT")) {
    fpga_initialise();
    fpga_claim(false);
    fpga_reset();
    return 1;
  }
#endif

  /* now configure */
  if (reset) {
    int r = ResetFPGA();
    if (r) {
      debug(("Failed: FPGA reset returns %d\n", r));
      return 0;
    }
  }

#ifdef XILINX
  if (f_open(&cf.file, bitfile ? bitfile : "CORE.BIT", FA_READ) != FR_OK) {
#else
  if (f_open(&cf.file, bitfile ? bitfile : "CORE.RBF", FA_READ) != FR_OK) {
#endif
    iprintf("No FPGA configuration file found %s!\r", bitfile);
#ifdef BOOT_FLASH_ON_ERROR
    printf("!!! booting from flash!!!\n");
    BootFromFlash();
    return 1;
#else
    if (fatal) FatalError(4);
    else return 0;
#endif
  }

  cf.size = f_size(&cf.file);
  cf.error = 0;
  debug(("cf.size = %ld\n", cf.size));
  iprintf("FPGA bitstream file %s opened, file size = %ld\r", bitfile, cf.size);

  /* initialise fpga */
  fpga_initialise();
  fpga_claim(true);

#ifdef BUFFER_FPGA
#if 0
  brl->closing_fn = NULL;
  brl->closing_user = NULL;
#endif  
  cf.l = cf.r = cf.c = 0;
  read_next_block_buffered_fill(&cf);
#endif

  /* now configure */
  if (!reset) {
    int r = ResetFPGA();
    if (r) {
      debug(("Failed: FPGA reset returns %d\n", r));
      f_close(&cf.file);
      return 0;
    }
  }



#ifdef BUFFER_FPGA
  fpga_configure(&cf, read_next_block_buffered, cf.size);
#else
  fpga_configure(&cf, read_next_block, cf.size);
#endif
  fpga_claim(false);

  f_close(&cf.file);

  // returns 1 if success / 0 on fail
  return !cf.error;
}

unsigned char ConfigureFpga(const char *bitfile) {
  return ConfigureFpgaEx(bitfile, true, false);
}

