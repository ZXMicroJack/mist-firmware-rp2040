#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/resets.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "hardware/pio.h"

#include "spi.pio.h"
#include "pio_spi.h"
#include "sdcard.h"
#include "crc16.h"

#include "pins.h"
// #define DEBUG
#include "debug.h"

static int is_sdhc = 0;

int sd_is_sdhc() {
  return is_sdhc;
}

static void sd_set_highspeed(pio_spi_inst_t *spi, int on);
static int sd_init_card_nosel(pio_spi_inst_t *spi);

static uint8_t sd_spin(pio_spi_inst_t *spi) {
  uint8_t ff = 0xff;
  uint8_t out = 0xff;
  pio_spi_write8_read8_blocking(spi, &ff, &out, 1);
  return out;
}

static void sd_presignal(pio_spi_inst_t *spi) {
  uint8_t sd_presignal_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  gpio_put(spi->cs_pin, 1);
  pio_spi_write8_blocking(spi, sd_presignal_data, sizeof sd_presignal_data);
}

static void pio_spi_read8_blocking_align(const pio_spi_inst_t *spi, uint8_t *dst, size_t len) {
  *dst = 0xff;
  pio_spi_read8_blocking_ex(spi, dst, len);
  if ((*dst) == 0xff) {
    return;
  }

#ifdef DEBUG
  printf("raw read\n");
  hexdump(dst, len);
#endif
}

static uint8_t get_next_byte(pio_spi_inst_t *spi) {
  uint8_t buf[1];
  pio_spi_read8_blocking(spi, buf, sizeof buf);
  return buf[0];
}

static uint8_t wait_ready(pio_spi_inst_t *spi) {
  uint8_t ff = 0xff;
  uint8_t buf = 0x00;
  int timeout = 2000;
  do {
    pio_spi_write8_read8_blocking(spi, &ff, &buf, 1);
    timeout --;
  } while (buf != 0xff && timeout);
  return buf;
}

static void get_bytes(pio_spi_inst_t *spi, uint8_t *block, int len) {
  pio_spi_read8_blocking(spi, block, len);
}

static void sd_select(pio_spi_inst_t *spi, uint8_t ncs) {
  uint8_t spin;
  spin = sd_spin(spi); debug(("spin1 %02X\n", spin));
  gpio_put(spi->cs_pin, ncs);
  spin = sd_spin(spi); debug(("spin2 %02X\n", spin));
}

#define NOCS    1
#define CS      2


static uint8_t sd_cmd(pio_spi_inst_t *spi, uint8_t cmd[], int cmdlen, uint8_t buf[], int buflen, uint16_t good, int retries, int nocs) {
  uint8_t spin = 0xff, ch;
  uint8_t good1 = good & 0xff;
  uint8_t good2 = good >> 8;
  
  good2 = good2 ? good2 : good1;
  while (retries --) {
    sd_select(spi, 0);
    
    if ((ch=wait_ready(spi)) != 0xff) {
      gpio_put(spi->cs_pin, 1);
      debug(("Retry %02X\n", ch));
      continue;
    }
    
    pio_spi_write8_blocking(spi, cmd, cmdlen);
    pio_spi_read8_blocking_align(spi, buf, buflen);

    if (buf[0] == 0xff || buf[0] == good1 || buf[0] == good2) break;
    if ((nocs & CS) == CS) sd_select(spi, 1);
  }

  if (buf == NULL) {
    return 0x00;
  }
  if ((nocs & NOCS) != NOCS || (nocs & CS) == CS) {
    gpio_put(spi->cs_pin, 1);
  }

#ifdef DEBUG
  hexdump(cmd, cmdlen);
  printf(" - ");
  hexdump(buf, buflen);
#endif
  return buf[0];
}

#define DEFAULT_RETRIES   10


static uint8_t sd_cxd(pio_spi_inst_t *spi, uint8_t cmd[], uint8_t buf[]) {
  uint8_t buf1[1];
	pio_spi_select(spi, 1);
  if (sd_cmd(spi, cmd, 6, buf1, sizeof buf1, 0x00, DEFAULT_RETRIES, 1) != 0x00) {
    sd_select(spi, 1);
		pio_spi_select(spi, 0);
    return 1;
  }

  int timeout = 20;
  while (timeout--) {
    uint8_t status = get_next_byte(spi);
    debug(("status %02X\n", status));
    if (status == 0xfe) {
      break;
    }
  }

  if (timeout <= 0) {
    sd_select(spi, 1);
		pio_spi_select(spi, 0);
    return 1;
  }

  get_bytes(spi, buf, 16);
	pio_spi_select(spi, 0);
  return 0;
}

uint8_t sd_cmd9(pio_spi_inst_t *spi, uint8_t buf[]) { // csd
  uint8_t cmd[] = {0x49, 0x00, 0x00, 0x00, 0x00, 0xaf};
  return sd_cxd(spi, cmd, buf);
}

uint8_t sd_cmd10(pio_spi_inst_t *spi, uint8_t buf[]) {
  uint8_t cmd[] = {0x4a, 0x00, 0x00, 0x00, 0x00, 0x1b};
  return sd_cxd(spi, cmd, buf);
}

static uint8_t sd_cmd1(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0x50, 0x00, 0x00, 0x02, 0x00, 0xff};
  uint8_t buf[1];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x00, DEFAULT_RETRIES, 0);
}

static uint8_t sd_reset(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
  uint8_t buf[1];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x01, DEFAULT_RETRIES, 0);
}

static uint8_t sd_init(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0xff, 0x41, 0x00, 0x00, 0x00, 0x00, 0xff};
  uint8_t buf[1];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x01, DEFAULT_RETRIES, 0);
}

static uint8_t sd_cmd8(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0xff, 0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
  uint8_t buf[5];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x01, DEFAULT_RETRIES, 0);
}

static uint8_t sd_cmd55(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0xff, 0x77, 0x00, 0x00, 0x00, 0x00, 0xff};
  uint8_t buf[1];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x00, DEFAULT_RETRIES, 0);
}

static uint8_t spi_spin = 0xff;

static uint8_t sd_cmd41(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0xff, 0x69, 0x40, 0x00, 0x00, 0x00, 0x87};
  uint8_t buf[1];

  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x00, 100, 0);
}


static uint8_t sd_cmd58(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0xff, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x75};
  uint8_t buf[5];
  if (sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x00, DEFAULT_RETRIES, 0) == 0x00) {
    return buf[1];
  }
  return 0xff;
}

static uint8_t sd_cmd59(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0x7b, 0x00, 0x00, 0x00, 0x00, 0xff};
  uint8_t buf[8];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x01, DEFAULT_RETRIES, 0);
}

#if 0
static uint8_t sd_cmd9(pio_spi_inst_t *spi) {
  uint8_t cmd[] = {0x49, 0x00, 0x00, 0x00, 0x00, 0xff};
  uint8_t buf[8];
  return sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x01, DEFAULT_RETRIES, 0);
}
#endif

uint8_t sd_writesector(pio_spi_inst_t *spi, uint32_t lba, uint8_t *data) {
  uint8_t cmd[] = {0xff, 0x58, 0x00, 0x00, 0x00, 0x00, 0xff};
  uint8_t writegap[] = {0xff, 0xfe};
  uint8_t crc[] = {0xff, 0xff, 0xff};
  uint8_t buf[1];

  hexdump(data, 512);

  if (!is_sdhc) lba <<= 9;

  cmd[2] = (lba >> 24);
  cmd[3] = (lba >> 16) & 0xff;
  cmd[4] = (lba >> 8) & 0xff;
  cmd[5] = lba & 0xff;

	pio_spi_select(spi, 1);
  sd_select(spi, 0);
  
  if (sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x00, DEFAULT_RETRIES, 1)!=0x00) {
    sd_select(spi, 1);
		pio_spi_select(spi, 0);
    debug(("Write failed 1 %02X\n", buf[0]));
    return 1;
  }
  
  pio_spi_write8_blocking(spi, writegap, sizeof writegap);
  pio_spi_write8_blocking(spi, data, 512);
  pio_spi_write8_blocking(spi, crc, sizeof crc);

  int timeout = 100000;
  while (timeout--) {
    uint8_t status = get_next_byte(spi);
    debug(("status %02X\n", status));
    if (status != 0x00) {
      break;
    }
  }
  
  sd_select(spi, 1);
	pio_spi_select(spi, 0);
  return timeout > 0 ? 0 : 1;
}  

// #define SD_NO_CRC

// #define debug(a) printf a
static uint8_t sd_readsector_ll(pio_spi_inst_t *spi, uint32_t lba, uint8_t *sector) {
  uint8_t cmd[] = {0x51, 0x00, 0x05, 0x00, 0x00, 0xff};
  uint8_t buf[1];
  uint8_t spin;

  if (!is_sdhc) lba <<= 9;
  
  cmd[1] = (lba >> 24);
  cmd[2] = (lba >> 16) & 0xff;
  cmd[3] = (lba >> 8) & 0xff;
  cmd[4] = lba & 0xff;

  // also 0x04 as expected code
  sd_select(spi, 0);
  uint8_t result = sd_cmd(spi, cmd, sizeof cmd, buf, sizeof buf, 0x00, DEFAULT_RETRIES, 1);
  if (result != 0x00 && result != 04) {
    sd_select(spi, 1);
    debug(("read failed 1 %02X\n", result));
    return 1;
  }
  
  int timeout = 50000;
  while (get_next_byte(spi) != 0xfe && timeout--)
    ;
  debug(("timeout after read = %d\n", timeout));
  
  if (timeout <= 1) {
    sd_select(spi, 1);
    debug(("Read failed 2\n"));
    return 1;
  }
  
#ifdef SD_DIRECT_MODE_GPIO
  if (!sector) gpio_put(SD_DIRECT_MODE_GPIO, 0);
#endif
  get_bytes(spi, sector, 512);
  
  uint8_t crc[2];
  get_bytes(spi, crc, sizeof crc);
#ifdef SD_DIRECT_MODE_GPIO
  if (!sector) gpio_put(SD_DIRECT_MODE_GPIO, 1);
#endif
  sd_select(spi, 1);
  
  uint16_t crcw = (crc[0] << 8) | crc[1];
  
#ifndef SD_NO_CRC
  if (sector != NULL && crc16iv(sector, 512, 0) != crcw) {
    debug(("Read failed crc 3 [%02X %02X] != %04X\n", crc[0], crc[1], crc16iv(sector, 512, 0)));
    hexdump(sector, 512);
    return 1;
  }
#endif

#ifdef DEBUG
  debug(("crc: %02X%02X\n", crc[0], crc[1]));
//   hexdump(sector, 512);
#endif
  
  return 0;
}
// #undef debug

#ifdef TEST_RETRIES
int xretryused = 0;
int xretry2used = 0;
#endif

uint8_t sd_readsector(pio_spi_inst_t *spi, uint32_t lba, uint8_t *sector) {
  int retries, resets = 3;
  
  debug(("SD: lba %08X ", lba));

  pio_spi_select(spi, 1);
  do {
    retries = 10;
    while (sd_readsector_ll(spi, lba, sector) && retries --) {
#ifdef TEST_RETRIES
      xretry2used ++;
#endif
      tight_loop_contents();
    }
  
    if (retries >= 0) {
			pio_spi_select(spi, 0);
      debug(("\n"));
      return 0;
    }
#ifdef TEST_RETRIES
    xretryused ++;
#endif
    // 3 retries failed, try to reset card and try again
    sd_init_card_nosel(spi);
  } while (resets--);

  // failed
	pio_spi_select(spi, 0);
  debug((" Failed!\n"));
  return 1;
}

// 694000000077

static int sd_init_card_ll(pio_spi_inst_t *spi) {
  is_sdhc = 0;
  sd_presignal(spi);

  if (wait_ready(spi) != 0xff) {
    return 0;
  }
  
  sd_select(spi, 0);
  if (sd_reset(spi) == 0x01) {
//     if (sd_init(spi) == 0x01) {
      if (sd_cmd8(spi) == 0x01) {
        int retries = 5;
        while (retries --) {
          if (sd_cmd55(spi) == 0x01) {
            if (sd_cmd41(spi) == 0x00) {
              break;
            }
          }
        }
        
        if (retries > 0) {
          if (sd_cmd58(spi) & 0x40) {
            printf("SDHC CARD!\n");
            is_sdhc = 1;
            // it's an sdhc card
            sd_select(spi, 1);
            return 1;
          } else if (sd_cmd1(spi) == 0x00) {
            printf("SD CARD!\n");
            // it's not - so set blocksize
            sd_select(spi, 1);
            return 1;
          }
        }
      }
//     }
  }
  return 0;
}


static int sd_init_card_nosel(pio_spi_inst_t *spi) {
  int timeout = 10;
  sd_set_highspeed(spi, 0);
  while (!sd_init_card_ll(spi) && timeout --)
    ;
  
  if (timeout > 0) {
    sd_set_highspeed(spi, 1);
  }
  
  return timeout > 0 ? 0 : 1;
}

int sd_init_card(pio_spi_inst_t *spi) {
  int r;
  
  pio_spi_select(spi, 1);
  r = sd_init_card_nosel(spi);
  pio_spi_select(spi, 0);
  return r;
}

static uint spi_offset = 0;
void sd_hw_kill(pio_spi_inst_t *spi) {
  pio_sm_set_enabled(spi->pio, spi->sm, false);
  pio_spi_kill(spi->pio, spi->sm, 
                PICO_DEFAULT_SPI_SCK_PIN,
                PICO_DEFAULT_SPI_TX_PIN,
                PICO_DEFAULT_SPI_RX_PIN);
  gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_IN);
}

static pio_spi_inst_t spi = {
    .pio = SDCARD_PIO,
    .sm = SDCARD_SM,
    .cs_pin = PICO_DEFAULT_SPI_CSN_PIN,
    .sck_pin = PICO_DEFAULT_SPI_SCK_PIN,
    .mosi_pin = PICO_DEFAULT_SPI_TX_PIN,
    .miso_pin = PICO_DEFAULT_SPI_RX_PIN
};

pio_spi_inst_t *sd_hw_init() {
  static int firsttime = 1;
  
  gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
  gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
  gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
  
  gpio_pull_up(PICO_DEFAULT_SPI_RX_PIN);
#ifdef PULL_UP_LINES
  gpio_pull_up(PICO_DEFAULT_SPI_CSN_PIN);
  gpio_pull_up(PICO_DEFAULT_SPI_SCK_PIN);
  gpio_pull_up(PICO_DEFAULT_SPI_TX_PIN);
#endif

  if (firsttime) {
    firsttime = 0;
#ifdef SDCARD_OFFSET
   pio_add_program_at_offset(spi.pio, &spi_cpha0_program, SDCARD_OFFSET);
   spi_offset = SDCARD_OFFSET;
#else
    spi_offset = pio_add_program(spi.pio, &spi_cpha0_program);
#endif
    printf("Loaded program at %d\n", spi_offset);
  }

  pio_spi_init(spi.pio, spi.sm, spi_offset,
                8,       // 8 bits per SPI frame
                78.125f, // 400khz @ 125 clk_sys
                false,   // CPOL = 0
                PICO_DEFAULT_SPI_SCK_PIN,
                PICO_DEFAULT_SPI_TX_PIN,
                PICO_DEFAULT_SPI_RX_PIN);
  
  return &spi;
}

static void sd_set_highspeed(pio_spi_inst_t *spi, int on) {
 pio_spi_init(spi->pio, spi->sm, spi_offset,
                8,       // 8 bits per SPI frame
                on ? 4.0f : 78.125f, // 400khz @ 125 clk_sys
                false,   // CPOL = 0
                PICO_DEFAULT_SPI_SCK_PIN,
                PICO_DEFAULT_SPI_TX_PIN,
                PICO_DEFAULT_SPI_RX_PIN);
 }

