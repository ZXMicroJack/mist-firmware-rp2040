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
#define DEBUG
#include "drivers/debug.h"


typedef struct {
  FIL file;
  uint32_t size;
  uint8_t error;
} configFpga;

static uint8_t read_next_block(void *ud, uint8_t *data) {
  UINT br;
  configFpga *cf = (configFpga *)ud;
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

//MJ TODO remove
int ResetFPGA() {
  /* initialise fpga */
  fpga_initialise();
  fpga_claim(true);

  /* now configure */
  return fpga_reset();
}

//MJ TODO remove
unsigned char ConfigureFpgaEx(const char *bitfile, uint8_t fatal, uint8_t reset) {
  configFpga cf;

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
      printf("Failed: FPGA reset returns %d\n", r);
      return 0;
    }
  }

#ifdef XILINX
  if (f_open(&cf.file, bitfile ? bitfile : "CORE.BIT", FA_READ) != FR_OK) {
#else
  if (f_open(&cf.file, bitfile ? bitfile : "CORE.RBF", FA_READ) != FR_OK) {
#endif
    iprintf("No FPGA configuration file found %s!\r", bitfile);
    if (fatal) FatalError(4);
    else return 0;
  }

  cf.size = f_size(&cf.file);
  cf.error = 0;
  printf("cf.size = %ld\n", cf.size);
  iprintf("FPGA bitstream file %s opened, file size = %ld\r", bitfile, cf.size);

  /* initialise fpga */
  fpga_initialise();
  fpga_claim(true);

  /* now configure */
  if (!reset) {
    int r = ResetFPGA();
    if (r) {
      printf("Failed: FPGA reset returns %d\n", r);
      return 0;
    }
  }

  fpga_configure(&cf, read_next_block, cf.size);
  fpga_claim(false);

  f_close(&cf.file);

  // returns 1 if success / 0 on fail
  return !cf.error;
}

unsigned char ConfigureFpga(const char *bitfile) {
  return ConfigureFpgaEx(bitfile, true, false);
}

