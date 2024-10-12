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
#include "jamma.h"
// #define DEBUG
#include "debug.h"

#include "fpga.pio.h"


#define RESET_TIMEOUT 100000

uint8_t inited = 0;

PIO fpga_pio = FPGA_PIO;
unsigned fpga_sm = FPGA_SM;

int fpga_initialise() {
  if (inited) return 0;
  
#ifdef ALTERA_FPGA
  gpio_init(GPIO_FPGA_NCONFIG);
  gpio_put(GPIO_FPGA_NCONFIG, 1);
  gpio_set_dir(GPIO_FPGA_NCONFIG, GPIO_OUT);
#ifndef QMTECH
  gpio_init(GPIO_FPGA_MSEL1);
  gpio_put(GPIO_FPGA_MSEL1, MSEL1_AS);
  gpio_set_dir(GPIO_FPGA_MSEL1, GPIO_OUT);
#endif
#else
  gpio_init(GPIO_FPGA_M1M2);
  gpio_put(GPIO_FPGA_M1M2, 0);
  gpio_set_dir(GPIO_FPGA_M1M2, GPIO_OUT);
  gpio_put(GPIO_FPGA_RESET, 1);
#endif

#ifdef MB2
#ifdef FPGA_OFFSET
  pio_add_program_at_offset(fpga_pio, &fpga_program, FPGA_OFFSET);
  fpga_program_init(fpga_pio, fpga_sm, FPGA_OFFSET, 0);
#else
  uint offset = pio_add_program(fpga_pio, &fpga_program);
  fpga_program_init(fpga_pio, fpga_sm, offset, 0);
#endif
  pio_sm_clear_fifos(fpga_pio, fpga_sm);
#endif
  inited = 1;

  return 0;
}

int fpga_claim(uint8_t claim) {
#if defined(ALTERA_FPGA)
#ifndef QMTECH
  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
  gpio_init(GPIO_FPGA_MSEL1);
  gpio_put(GPIO_FPGA_MSEL1, claim ? MSEL1_PS : MSEL1_AS);
  gpio_set_dir(GPIO_FPGA_MSEL1, GPIO_OUT);
#endif
#else
  gpio_init(GPIO_FPGA_M1M2);
  gpio_put(GPIO_FPGA_M1M2, claim ? 1 : 0);
  gpio_set_dir(GPIO_FPGA_M1M2, GPIO_OUT);
#endif
  return 0;
}

void fpga_holdreset() {
#if !defined(ALTERA_FPGA)
  gpio_init(GPIO_FPGA_RESET);
  gpio_set_dir(GPIO_FPGA_RESET, GPIO_OUT);
  gpio_put(GPIO_FPGA_RESET, 0);
  // reset_button_readable = 0;
#endif
}

int fpga_reset() {
#ifdef ALTERA_FPGA
  uint64_t timeout;

  gpio_init(GPIO_FPGA_DATA0);
  gpio_init(GPIO_FPGA_DCLK);

  gpio_init(GPIO_FPGA_CONF_DONE);
  gpio_init(GPIO_FPGA_NSTATUS);
  gpio_init(GPIO_FPGA_NCONFIG);

  gpio_put(GPIO_FPGA_NCONFIG, 1);
  gpio_set_dir(GPIO_FPGA_NCONFIG, GPIO_OUT);

  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
  gpio_put(GPIO_FPGA_NCONFIG, 0);
  timeout = time_us_64() + RESET_TIMEOUT;
  while (gpio_get(GPIO_FPGA_NSTATUS) && time_us_64() < timeout)
    tight_loop_contents();

  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
  gpio_put(GPIO_FPGA_NCONFIG, 1);
  if (time_us_64() >= timeout) {
    gpio_init(GPIO_FPGA_NCONFIG);
    // reset_button_readable = 1;
    return 1;
  }

  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
  timeout = time_us_64() + RESET_TIMEOUT;
  while (!gpio_get(GPIO_FPGA_NSTATUS) && time_us_64() < timeout)
    tight_loop_contents();

  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
  if (time_us_64() >= timeout) {
    gpio_init(GPIO_FPGA_NCONFIG);
    // reset_button_readable = 1;
    return 2;
  }
  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
#else
  uint64_t timeout;

  gpio_init(GPIO_FPGA_DATA);
  gpio_init(GPIO_FPGA_CLOCK);

  gpio_init(GPIO_FPGA_INITB);
  gpio_init(GPIO_FPGA_RESET);
  gpio_set_dir(GPIO_FPGA_RESET, GPIO_OUT);

  gpio_put(GPIO_FPGA_RESET, 0);
  timeout = time_us_64() + RESET_TIMEOUT;
  while (gpio_get(GPIO_FPGA_INITB) && time_us_64() < timeout)
    tight_loop_contents();
  
  gpio_put(GPIO_FPGA_RESET, 1);
  if (time_us_64() >= timeout) {
    gpio_init(GPIO_FPGA_INITB);
    gpio_init(GPIO_FPGA_RESET);
    // reset_button_readable = 1;
    return 1;
  }

  timeout = time_us_64() + RESET_TIMEOUT;
  while (!gpio_get(GPIO_FPGA_INITB) && time_us_64() < timeout)
    tight_loop_contents();

  if (time_us_64() >= timeout) {
    gpio_init(GPIO_FPGA_INITB);
    gpio_init(GPIO_FPGA_RESET);
    // reset_button_readable = 1;
    return 2;
  }
  
  gpio_init(GPIO_FPGA_INITB);
  gpio_init(GPIO_FPGA_RESET);
#endif
  // reset_button_readable = 1;
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

#ifndef ALTERA_FPGA
static uint8_t fpgaType = A200T;
static uint8_t fpgaTypeSet = AXXXT_TYPES;

/* return set type if set, otherwise last detected */
uint8_t fpga_GetType() {
  return fpgaTypeSet == AXXXT_TYPES ? fpgaType : fpgaTypeSet;
}

/* confirm that the core is actually running - this detection should remain */
void fpga_ConfirmType() {
  if (fpgaTypeSet == AXXXT_TYPES) fpgaTypeSet = fpgaType;
}

void fpga_SetType(uint8_t type) {
  if (type < AXXXT_TYPES) {
    fpgaType = type;
  }
}
#endif

int fpga_configure(void *user_data, uint8_t (*next_block)(void *, uint8_t *), uint32_t assumelength) {
  uint8_t bits[512];
  uint32_t data;
  uint32_t len;
  uint32_t thislen;
  int j;

  initCRC();
  uint32_t crc = 0xffffffff;

  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));

  if (!next_block(user_data, bits)) {
    // couldn't read data
    return 1;
  }
  
#ifndef ALTERA_FPGA
  uint32_t totallen = bitfile_get_length(bits, assumelength, &fpgaType);
  len = totallen == 0xffffffff ? assumelength : totallen;
  debug(("fpga_get_length: returns %u\n", len));
  if (!len) {
    // couldn't find data length
    return 1;
  }
#else
  uint32_t totallen = assumelength;
  len = assumelength;
#endif
  
  // probably this should not be here, this makes some space in PIO memory to
  // load the FPGA routines.
#ifndef MB2
  uint8_t old_jamma_mode = jamma_GetMisterMode();
  jamma_Kill();
  pio_add_program_at_offset(fpga_pio, &fpga_program, FPGA_OFFSET);
  fpga_program_init(fpga_pio, fpga_sm, FPGA_OFFSET, 0);
  pio_sm_clear_fifos(fpga_pio, fpga_sm);
#endif

  debug(("fpga_status: done %u nstatus %u\n", gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS)));
  fpga_program_enable(fpga_pio, fpga_sm, 0, true);

  do {
    thislen = len > 512 ? 512 : len;
    crc = crc32(crc, bits, thislen);
    
    // clock data out
    for (int i=0; i<thislen; i+=4) {
#ifdef ALTERA_FPGA
      for (j=i; j<(i+4) && j<thislen; j++) {
        data = (data >> 8) | (bits[j] << 24);
      }
      for (;j<(i+4); j++) {
        data = (data >> 8);
      }
#else
      for (j=i; j<(i+4) && j<thislen; j++) {
        data = (data << 8) | bits[j];
      }
      for (;j<(i+4); j++) {
        data = (data << 8); // | 0xff;
      }
#endif
      pio_sm_put_blocking(fpga_pio, fpga_sm, data);
    }
    len -= thislen;

    debug(("fpga_configure: remaining %u / %u %u %u %08X %08X %08X\n",
           len,
           totallen,
#ifdef ALTERA_FPGA
           gpio_get(GPIO_FPGA_CONF_DONE), gpio_get(GPIO_FPGA_NSTATUS),
#else
           gpio_get(GPIO_FPGA_INITB), 0,
#endif
           crc, crc32(0xffffffff, bits, thislen), gpio_get_all()));
#ifdef ALTERA_FPGA
#ifdef ALTERA_DONT_CHECK_DONE
  } while (len > 0 && next_block(user_data, bits) && gpio_get(GPIO_FPGA_NSTATUS));
#else
  } while (len > 0 && next_block(user_data, bits) && !gpio_get(GPIO_FPGA_CONF_DONE) && gpio_get(GPIO_FPGA_NSTATUS));
#endif
#else
  } while (len > 0 && next_block(user_data, bits) && gpio_get(GPIO_FPGA_INITB));
#endif

  // wait until fifo is empty
#ifndef ALTERA_DONT_CHECK_DONE
  for (int i=0; i<32 && !gpio_get(GPIO_FPGA_CONF_DONE); i++) {
#else
  for (int i=0; i<32; i++) {
#endif
    pio_sm_put_blocking(fpga_pio, fpga_sm, 0xffffffff);
  }
  while (!pio_sm_is_rx_fifo_empty(fpga_pio, fpga_sm))
    tight_loop_contents();


  pio_sm_set_enabled(fpga_pio, fpga_sm, false);
  fpga_program_enable(fpga_pio, fpga_sm, 0, false);

#ifndef MB2
  pio_remove_program(fpga_pio, &fpga_program, FPGA_OFFSET);
  jamma_InitEx(old_jamma_mode);
#endif

#ifdef ALTERA_FPGA
  debug(("fpga_configure: crc %08X %d\n", crc, gpio_get(GPIO_FPGA_CONF_DONE)));
#else
  debug(("fpga_configure: crc %08X %d\n", crc, gpio_get(GPIO_FPGA_INITB)));
#endif

#if defined (ALTERA_FPGA) && !defined (ALTERA_DONT_CHECK_DONE)
  return (gpio_get(GPIO_FPGA_NSTATUS) && gpio_get(GPIO_FPGA_CONF_DONE)) ? 0 : 1;
#else
  return 0;
#endif
}

