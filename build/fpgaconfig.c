#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "pico/time.h"

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
#include "rtc.h"
#include "common.h"
#include "usb/joymapping.h"

#include "drivers/fpga.h"
#include "drivers/bitfile.h"
//#define DEBUG
#include "drivers/debug.h"

// NB: These for production
#ifndef PICO_NO_FLASH
#define BUFFER_SIZE     16 // PACKETS
#define BUFFER_SIZE_MASK  0xf
#else
// NB: Use these for debugging mechanism.
#define BUFFER_SIZE     4 // PACKETS
#define BUFFER_SIZE_MASK  0x3
#endif
#define NR_BLOCKS(a) (((a)+511)>>9)

typedef struct {
  FIL file;
  uint32_t size;
  uint8_t error;
#ifdef BUFFER_FPGA
  uint8_t buff[BUFFER_SIZE][512];
  uint8_t l, r, c;
#endif
} configFpga;

static uint8_t read_next_block(void *ud, uint8_t *data) {
  UINT br;
  configFpga *cf = (configFpga *)ud;

#ifdef BUFFER_FPGA
  debug(("read_next_block cf->c = %d cf->size = %d\n", cf->c, cf->size));
#endif
  if (f_read(&cf->file, data, cf->size > 512 ? 512 : cf->size, &br) != FR_OK) {
    cf->error = 1;
    return 0;
  }

  if (br > cf->size) {
    cf->error = 0;
    f_close(&cf->file);
    return 0;
  }
  cf->size -= br;
  return 1;
}

#ifdef BUFFER_FPGA
#define brl_inc(a) (((a) + 1) & BUFFER_SIZE_MASK)

static void read_next_block_buffered_fill(configFpga *brl) {
  uint8_t result;

  if (!brl->size) return; // no more blocks don't try to read more.
  if (brl->error) return; // error encountered
  
  while (brl->c < BUFFER_SIZE) {
     result = read_next_block(brl, brl->buff[brl->r]);
     if (result) {
       brl->r = brl_inc(brl->r);
       brl->c ++;
     } else {
      // close the file here if needed
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
#endif

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

#ifdef XILINX
uint8_t chipType = A200T;

uint32_t GotoBitstreamOffset(FIL *f, uint32_t len) {
  // uint8_t *blk;
  uint8_t blk[512];
  UINT br;

  // blk = (uint8_t *)malloc(512);
  if (f_read(f, blk, 512, &br) == FR_OK) {
    if (blk[0] == 'M' && blk[1] == 'J' && blk[2] == 0x01 && blk[3] == 0x00) {
      int o = 4 + fpga_GetType() * 4;
      uint32_t offset = blk[o] | (blk[o+1]<<8) | (blk[o+2]<<16) | (blk[o+3]<<24);
      uint32_t next_offset = blk[o+4] | (blk[o+5]<<8) | (blk[o+6]<<16) | (blk[o+7]<<24);
      f_lseek(f, offset);
      len = next_offset == 0 ? (len - offset) : (next_offset - offset);
    } else {
      f_lseek(f, 0);
    }
  }
  // free(blk);
  return len;
}
#endif

unsigned char ConfigureFpga(const char *bitfile) {
  // configFpga cf;
  uint32_t fileSize;
  configFpga *cf;

  debug(("ConfigureFpgaEx: %s\n", bitfile ? bitfile : "null"));

  inhibit_reset = 1;

  /* handle JTAGMODE - sleep for 60s and poll for recognisable core, 
     otherwise, reset back to core menu. */
  if (bitfile && !strncmp(bitfile, "JTAGMODE.", 9)) {
    fpga_initialise();
    fpga_claim(true);
    fpga_reset();

    int n = 60;
    while (n--) {
      sleep_ms(1000);
      user_io_detect_core_type();
      if (user_io_core_type() != CORE_TYPE_UNKNOWN) {
        break;
      }
    }

    // If core was detected, then return SUCCESS, otherwise
    // load the default core.
    if (n>0) {
      inhibit_reset = 0;
      return 1;
    } else {
      bitfile = NULL;
    }
  }

  /* boot from flash */
#ifdef XILINX // go to flash boot
  if (bitfile && !strcmp(bitfile, "ZXTRES.BIT")) {
    fpga_initialise();
    fpga_claim(false);
    fpga_reset();
    inhibit_reset = 0;
    return 1;
  }
#endif

  cf = (configFpga *)malloc(sizeof (configFpga));

#ifdef XILINX
  if (f_open(&cf->file, bitfile ? bitfile : "CORE.BIT", FA_READ) != FR_OK) {
#else
  if (f_open(&cf->file, bitfile ? bitfile : "CORE.RBF", FA_READ) != FR_OK) {
#endif
    iprintf("No FPGA configuration file found %s!\r", bitfile);
    free(cf);
#ifdef BOOT_FLASH_ON_ERROR
    printf("!!! booting from flash!!!\n");
    BootFromFlash();
    inhibit_reset = 0;
    return 1;
#else
    FatalError(4);
#endif
  }

  fileSize = cf->size = f_size(&cf->file);
  cf->error = 0;

  debug(("cf.size = %ld\n", cf->size));
  iprintf("FPGA bitstream file %s opened, file size = %ld\r", bitfile, cf->size);

#ifdef XILINX
  if (bitfile && !strcasecmp(&bitfile[strlen(bitfile)-4], ".ZXT")) {
    debug(("!!! it's a ZXT format chiptype is %d\n", fpga_GetType()));
    fileSize = cf->size = GotoBitstreamOffset(&cf->file, fileSize);
  }
#endif

  /* initialise fpga */
  fpga_initialise();

#ifdef BUFFER_FPGA
  cf->l = cf->r = cf->c = 0;
  read_next_block_buffered_fill(cf);

#ifdef XILINX
  // try to figure out actual bitstream size
  uint32_t bslen = bitfile_get_length(cf->buff[0], 0, &chipType);
  if (bslen && bslen != 0xffffffff) {
    cf->size = bslen - (BUFFER_SIZE * 512);
    debug(("ConfigureFpga: corrected bitlen to %d\n", cf->size));
  }
#endif
#endif

    int r = ResetFPGA();
    if (r) {
      debug(("Failed: FPGA reset returns %d\n", r));
      f_close(&cf->file);
      free(cf);
      inhibit_reset = 0;
      return 0;
    }

#ifdef BUFFER_FPGA
  fpga_configure(cf, read_next_block_buffered, fileSize);
#else
  fpga_configure(cf, read_next_block, fileSize);
#endif
  fpga_claim(false);

  int result = !cf->error;
  free(cf);

  // returns 1 if success / 0 on fail
  inhibit_reset = 0;
  return result;
}

