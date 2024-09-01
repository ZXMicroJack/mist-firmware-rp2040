#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hardware/pio.h"

#include "pins.h"
#include "fpga.h"
#include "bitfile.h"
//#define DEBUG
#include "debug.h"
#include "fat32.h"

#include "fpga.pio.h"
#include "pio_spi.h"


void fpga_boot_from_flash() {
  
//   fpga_init();
  gpio_init(0);
  gpio_init(1);
  
  fpga_claim(true);
  fpga_reset();
}

static void fpga_closing_fn(void *spi) {
  printf("fpga_closing_fn: killing SPI\n");
  sd_hw_kill((pio_spi_inst_t *)spi);
}

#define BUFFERED_READ

#ifdef BUFFERED_READ
static buffered_read_lba_t rl;
#else
static read_lba_t rl;
#endif

// load bitfile
#ifdef TEST_SD_READS
#define nada()  do {} while (0)
#define fpga_init() nada()
#define fpga_claim(a) nada()
#define fpga_reset() (0)

#define gpio_get(a) (1)
#define pio_sm_is_rx_fifo_empty(a, b) (1)
#define pio_sm_set_enabled(a,b,c) nada()
#endif

void fpga_load_bitfile(void *_spi, int lba, char *fn) {
  pio_spi_inst_t *spi = (pio_spi_inst_t *)_spi;
  fat32_t *fat32 = NULL;

  /* initialise fpga */
  fpga_init();
  fpga_claim(true);

  /* now configure */
  int r = fpga_reset();
  if (r) {
    printf("Failed: FPGA reset returns %d\n", r);
    return;
  }

  /* now make contact with SD card */
  if (sd_init_card(spi)) {
    printf("Failed to initialise SD card\n");
    fpga_claim(false);
    return;
  }

  /* initialise file system */
  if (init_fs()) {
    printf("Failed to initialise file system\n");
    fpga_claim(false);
    return;
  }

  /* set up SD card read */
  uint32_t filesize;
  if (fn != NULL) {
    int part = 0;
    uint32_t cluster;

    printf("Looking for bitfile %s\n", fn);
    fat32 = get_partition(part);
    while (fat32 && (cluster = get_cluster_from_filename(fat32, fn, &filesize)) == 0) {
      fat32 = get_partition(++part);
    }
    
    printf("cluster = %08x\n", cluster);
    if (cluster) {
      lba = get_lba_from_cluster(fat32, cluster);
    }
    
    printf("lba = %08x\n", lba);
    if (!cluster || !lba) {
      fpga_claim(false);
      return;
    }
  }
#ifdef BUFFERED_READ
  get_next_lba_init_buffered(fat32, &rl, lba, NR_BLOCKS(filesize));
  rl.closing_fn = fpga_closing_fn;
  rl.closing_user = _spi;
#else
  get_next_lba_init(fat32, &rl, lba, NR_BLOCKS(filesize));
#endif
  
#if 0
  /* now configure */
  r = fpga_reset();
  if (r) {
    printf("Failed: FPGA reset returns %d\n", r);
    return;
  }
#endif
  
#ifdef BUFFERED_READ
  fpga_configure(&rl, get_next_lba_buffered, filesize);
#else
  fpga_configure(&rl, get_next_lba, filesize);
#endif
  fpga_claim(false);
}

#ifdef TEST_SD_READS
#undef gpio_get
#endif

#ifdef BOOT_CORE
static uint32_t core_detection = 0;
static uint64_t last_core_detection = 0;
static uint8_t core_detection_cancelled = 0;
#define MAX_CORE_DETECTION  10
#define CHECK_INTERVAL      1000000
#endif

#ifdef DETECT_ERROR
static uint64_t last_fatal_error_detection = 0;
#ifndef CHECK_INTERVAL
#define CHECK_INTERVAL      1000000
#endif
static uint32_t fatal_error_detection = 0;
// static uint8_t fatal_error_occurred = 0;
#define MAX_FATAL_ERROR_DETECTION  4
#endif

void fpga_detect_init() {
  gpio_init(PICO_FPGA_MONITOR_PIN);
  gpio_init(PICO_FPGA_BOOT_SD);
}


int fpga_detect_error() {
  int boot_recover = 0;
#ifdef BOOT_CORE
  if (!core_detection_cancelled && time_us_64() > (last_core_detection + CHECK_INTERVAL)) {
    printf("gpio_get(PICO_FPGA_MONITOR_PIN) = %d\n", gpio_get(PICO_FPGA_MONITOR_PIN));
    if (!gpio_get(PICO_FPGA_MONITOR_PIN)) {
      core_detection_cancelled = 1;
      printf("cancelled core detection\n");
    } else {
      core_detection ++;
      if (core_detection >= MAX_CORE_DETECTION) {
        // boot boot.bit
        core_detection_cancelled = 1;
        boot_recover = 1;
      }
    }
    last_core_detection = time_us_64();
  }
#endif
#ifdef DETECT_ERROR
  if (/*!fatal_error_occurred && */time_us_64() > (last_fatal_error_detection + CHECK_INTERVAL)) {
    if (!gpio_get(PICO_FPGA_MONITOR_PIN) && gpio_get(PICO_FPGA_BOOT_SD)) {
      fatal_error_detection ++;
      if (fatal_error_detection >= MAX_FATAL_ERROR_DETECTION) {
        // boot boot.bit
        boot_recover = 1;
//         fatal_error_occurred = 1; // can only happen once
      }
    } else {
      fatal_error_detection = 0;
    }
    last_fatal_error_detection = time_us_64();
  }
#endif
  return boot_recover;
}
