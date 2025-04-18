#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"

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
#include "usb/joymapping.h"

#include "drivers/fpga.h"
#include "drivers/bitfile.h"
#include "drivers/bitstore.h"
#include "drivers/pins.h"
#include "drivers/jtag.h"

#ifdef PICOSYNTH
#include "picosynth.h"
#endif

// #define DEBUG
#include "drivers/debug.h"
#include "common.h"

typedef struct {
  FIL file;
  uint32_t size;
  uint32_t block_nr;
  uint32_t n_blocks;
  uint8_t error;
} configFpga;


static uint8_t read_next_block(void *ud, uint8_t *data) {
  UINT br;
  configFpga *cf = (configFpga *)ud;

  if (f_read(&cf->file, data, 512, &br) != FR_OK) {
    cf->error = 1;
    return 0;
  }

  if (br > cf->size) {
    cf->error = 0; 
    f_close(&cf->file);
    return 0;
  }
  cf->size -= br;
  debug(("read_next_block: cf->size = %d br = %d\n", cf->size, br));
  return br > 0;
}

uint8_t read_next_block_stdin(void *user_data, uint8_t *data) {
  configFpga *b = (configFpga *)user_data;
  if (b->block_nr >= b->n_blocks) return false;

  for (int i=0; i<512; i++) {
    data[i] = getchar();
  }

  b->block_nr ++;
  return true;
}

static uint8_t bitstore_GetBlockJTAG(void *user_data, uint8_t *blk) {
  return bitstore_GetBlock(blk) ? false : true;
}

uint32_t fpga_image_size;

static uint8_t test_fpga_get_next_block(void *user_data, uint8_t *data) {
  int j;
  int thislen;
  int o = 0;
  configFpga *b = (configFpga *)user_data;
  

  if ((b->block_nr * 512) > fpga_image_size) {
    return false;
  }

  uint8_t *bits = (uint8_t *)(FPGA_IMAGE_POS + b->block_nr * 512);

  memcpy(data, bits, 512);
  b->block_nr ++;
  return true;
}

void ConfigureFPGAFlash()
{
  configFpga cf;
  uint32_t size, offset;
  core_detect = 1;
  int result = jtag_get_length((uint8_t *)FPGA_IMAGE_POS, 512, &size, &offset);

  debug(("result %d size %d offset %d\n", result, size, offset));

  if (result) {
    fpga_image_size = size;
    memset(&cf, 0x00, sizeof cf);
    jtag_init();
    result = jtag_configure(&cf, test_fpga_get_next_block, size);
    debug(("fpga_program returns %d\n", result));
  }
}

void ConfigureFPGAStdin() {
  configFpga cf;
  core_detect = 1;

  while (getchar_timeout_us(2) >= 0)
    ;

  printf("Size of block = ");
  int len;
  scanf("%d", &len);
  printf("%d\n", len);

  memset(&cf, 0x00, sizeof cf);
  cf.n_blocks = (len + 511) / 512;

  jtag_init();
  printf("fpga_program returns %d\n", jtag_configure(&cf, read_next_block_stdin, len));
  rtc_AttemptSync();
}

#ifdef BOOT_FLASH_ON_ERROR
void BootFromFlash() {
  gpio_put(GPIO_FPGA_RESET, 1);
  gpio_init(GPIO_FPGA_RESET);
  gpio_set_dir(GPIO_FPGA_RESET, GPIO_OUT);
  gpio_put(GPIO_FPGA_RESET, 0);
  sleep_ms(100);
  gpio_put(GPIO_FPGA_RESET, 1);
}
#endif

unsigned char ConfigureFpga(const char *bitfile) {
  configFpga cf;
  uint32_t size;
  core_detect = 1;

#if PICO_NO_FLASH
  printf("ConfigureFpga: bitfile=%s\n", bitfile);
  if (!bitfile || !strcmp(bitfile, "CORE.BIT")) {
    ConfigureFPGAFlash();
    return 1;
  }

  if (bitfile && !strcmp(bitfile, "ZXUNO.BIT")) {
    BootFromFlash();
    return 1;
  }

  ConfigureFPGAStdin();
#else

  if (bitfile && !strcmp(bitfile, "JTAGMODE.BIT")) {
    ConfigureFPGAStdin();
    return 1;
  }

  if (bitfile && !strcmp(bitfile, "FLSHMENU.BIT")) {
    /* already done earlier on */
    return 1;
  }

  if (bitfile && !strcmp(bitfile, "ZXUNO.BIT")) {
    BootFromFlash();
    return 1;
  }

  if (f_open(&cf.file, bitfile ? bitfile : "CORE.BIT", FA_READ) != FR_OK) {
    ConfigureFPGAFlash();
    return 1; // return to menu
//  FatalError(4);
  }

  size = cf.size = f_size(&cf.file);
  cf.error = 0;

  /* first create some memory */
#ifdef PICOSYNTH
  picosynth_Suspend(1);
#endif

  /* initialise fpga */
  debug(("ConfigureFpga: Initialising bitstore\n"));
  bitstore_InitRetrieve();
  debug(("ConfigureFpga: Storing bitfile\n"));
  int chunks = bitstore_Store(&cf, read_next_block);

  /* initialise jtag */
  debug(("ConfigureFpga: Initialising JTAG\n"));
  jtag_init();

  debug(("ConfigureFpga: setting up receive\n"));
  bitstore_InitRetrieve();
  debug(("ConfigureFpga: programming\n"));
  jtag_configure(NULL, bitstore_GetBlockJTAG, size);

#ifdef PICOSYNTH
  picosynth_Suspend(0);
#endif

#endif
  return 1;
}

int ResetFPGA() {
  return 0;
}
