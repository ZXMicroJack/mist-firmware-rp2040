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
// #define DEBUG
#include "debug.h"

#include "fpga.pio.h"

#define RESET_TIMEOUT 100000

uint8_t inited = 0;

PIO fpga_pio = pio0;
unsigned fpga_sm = 0;

int fpga_init() {
  if (inited) return 0;
  
  gpio_init(GPIO_FPGA_M1M2);
  gpio_put(GPIO_FPGA_M1M2, 0);
  gpio_set_dir(GPIO_FPGA_M1M2, GPIO_OUT);
  gpio_put(GPIO_FPGA_RESET, 1);
  
  uint offset = pio_add_program(fpga_pio, &fpga_program);

  fpga_program_init(fpga_pio, fpga_sm, offset, 0);
  pio_sm_clear_fifos(fpga_pio, fpga_sm);
  inited = 1;

  return 0;
}

int fpga_claim(uint8_t claim) {
  gpio_init(GPIO_FPGA_M1M2);
  gpio_put(GPIO_FPGA_M1M2, claim ? 1 : 0);
  gpio_set_dir(GPIO_FPGA_M1M2, GPIO_OUT);
  return 0;
}

int fpga_reset() {
  uint64_t timeout;

  gpio_init(GPIO_FPGA_INITB);
  gpio_init(GPIO_FPGA_RESET);
  gpio_set_dir(GPIO_FPGA_RESET, GPIO_OUT);

//   gpio_init(GPIO_FPGA_INITB);
//   gpio_init(GPIO_FPGA_RESET);
//   gpio_put(GPIO_FPGA_RESET, 1);
//   gpio_set_dir(GPIO_FPGA_RESET, GPIO_OUT);

  gpio_put(GPIO_FPGA_RESET, 0);
  timeout = time_us_64() + RESET_TIMEOUT;
  while (gpio_get(GPIO_FPGA_INITB) && time_us_64() < timeout)
    tight_loop_contents();
  
  gpio_put(GPIO_FPGA_RESET, 1);
  if (time_us_64() >= timeout) {
    gpio_init(GPIO_FPGA_INITB);
    gpio_init(GPIO_FPGA_RESET);
    return 1;
  }

  timeout = time_us_64() + RESET_TIMEOUT;
  while (!gpio_get(GPIO_FPGA_INITB) && time_us_64() < timeout)
    tight_loop_contents();

  if (time_us_64() >= timeout) {
    gpio_init(GPIO_FPGA_INITB);
    gpio_init(GPIO_FPGA_RESET);
    return 2;
  }
  
  gpio_init(GPIO_FPGA_INITB);
  gpio_init(GPIO_FPGA_RESET);
  return 0;
}

#define CRC32_POLY 0x04c11db7   /* AUTODIN II, Ethernet, & FDDI */
#define CRC32(crc, byte) \
        crc = (crc << 8) ^ CRC32_lut[(crc >> 24) ^ byte]

#define PADDING       32

static uint32_t CRC32_lut[256] = {0};
static void initCRC(void) {
  int i, j;
  unsigned long c;
	if (CRC32_lut[0]) return;

  for (i = 0; i < 256; ++i) {
    for (c = i << 24, j = 8; j > 0; --j)
      c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
    CRC32_lut[i] = c;
  }
}


static uint32_t crc32(uint32_t crc, uint8_t *blk, uint32_t len) {
  while (len--) {
    CRC32(crc, *blk);
    blk++;
  }
  return crc;
}

#ifdef TEST_SD_READS
#define nada()  do {} while (0)
#define bitfile_get_length(a,b) b
#define fpga_program_enable(a,b,c,d) nada()
#define pio_sm_put_blocking(a,b,c) nada()
#define gpio_get(a) (1)
#define pio_sm_is_rx_fifo_empty(a, b) (1)
#define pio_sm_set_enabled(a,b,c) nada()
#endif

int fpga_configure(void *user_data, uint8_t (*next_block)(void *, uint8_t *), uint32_t assumelength) {
  uint8_t bits[512];
  uint32_t data;  
  uint32_t len;
  uint32_t thislen;
  int j;

  initCRC();
  uint32_t crc = 0xffffffff;
  
  if (!next_block(user_data, bits)) {
    // couldn't read data
    return 1;
  }
  
  uint32_t totallen = bitfile_get_length(bits, assumelength);
  len = totallen == 0xffffffff ? assumelength : totallen;
  debug(("fpga_get_length: returns %u\n", len));
  if (!len) {
    // couldn't find data length
    return 1;
  }
  
  fpga_program_enable(fpga_pio, fpga_sm, 0, true);
  do {
    thislen = len > 512 ? 512 : len;
    crc = crc32(crc, bits, thislen);
    
    // clock data out
    for (int i=0; i<thislen; i+=4) {
      for (j=i; j<(i+4) && j<thislen; j++) {
        data = (data << 8) | bits[j];
      }
      for (;j<(i+4); j++) {
        data = (data << 8); // | 0xff;
      }
      pio_sm_put_blocking(fpga_pio, fpga_sm, data);
    }
    len -= thislen;
    debug(("fpga_configure: remaining %u / %u %u %08X %08X\n", len, totallen, gpio_get(GPIO_FPGA_INITB), 
           crc, crc32(0xffffffff, bits, thislen)));
  } while (len > 0 && next_block(user_data, bits) && gpio_get(GPIO_FPGA_INITB));

  // wait until fifo is empty
  while (!pio_sm_is_rx_fifo_empty(fpga_pio, fpga_sm))
    tight_loop_contents();

  pio_sm_set_enabled(fpga_pio, fpga_sm, false);
  fpga_program_enable(fpga_pio, fpga_sm, 0, false);
  
  debug(("fpga_configure: crc %08X %d\n", crc, gpio_get(GPIO_FPGA_INITB)));
  return 0;
}

