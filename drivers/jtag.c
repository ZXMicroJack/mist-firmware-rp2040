#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "pico/time.h"
#include "hardware/gpio.h"
#include "jtag.h"
#include "pins.h"
#define DEBUG
#include "debug.h"

#define TDO GPIO_JTAG_TDO
#define TDI GPIO_JTAG_TDI
#define TCK GPIO_JTAG_TCK
#define TMS GPIO_JTAG_TMS

#define TCKWAIT 1

#define BLOCKSIZE 512

///////////////////////////////////////////////////////
// CRC32 specific
static int jtag_preinslen = 0;

int jtag_init() {
  // cable gpiodirect tdo=9 tdi=10 tck=11 tms=25
  printf("Info: setting up GPIO pins\n");

  gpio_init(TDO);
  gpio_init(TDI);
  gpio_init(TCK);
  gpio_init(TMS);

  gpio_set_dir(TDO, GPIO_IN);
  gpio_set_dir(TDI, GPIO_OUT);
  gpio_set_dir(TCK, GPIO_OUT);
  gpio_set_dir(TMS, GPIO_OUT);
	
	return 0;
}

void jtag_kill(void) {
}

///////////////////////////////////////////////////////
// JTAG pin functions
// set TMS to value, cycle TCK
void jtag_tms(int value) {
    gpio_put(TMS, value ? 1 : 0);
    jtag_tck();
}

// cycle TCK, set TDI to value, sample TDO
int jtag_tdi(int value) {
    jtag_tck();
    gpio_put(TDI, value ? 1 : 0);
    return gpio_get(TDO);
}

// multi-bit version of tdi()
uint8_t jtag_tdin(int n, uint8_t bits) {
    uint8_t tmp=0, res=0;
    int i;

    // shift bits and push into tmp lifo-order
    for(i=0; i<n; i++) {
        tmp = tmp<<1 | jtag_tdi(bits & 1);
        bits >>= 1;
    }

    // reverse bit order tmp->res
    for(i=0; i<n; i++) {
        res = res<<1 | tmp&1;
        tmp >>= 1;
    }

    return res;
}

// multi-bit version of tdi()
uint8_t jtag_tdin_rev(int n, uint8_t bits) {
    uint8_t tmp=0, res=0;
    int i;

    // shift bits and push into tmp lifo-order
    for(i=0; i<n; i++) {
        tmp = tmp<<1 | jtag_tdi(bits >> 7);
        bits <<= 1;
    }

    // reverse bit order tmp->res
    for(i=0; i<n; i++) {
        res = res<<1 | tmp&1;
        tmp >>= 1;
    }

    return res;
}

void jtag_tck(void) {
    gpio_put(TCK, 1);
    // sleep_us(TCKWAIT);
    gpio_put(TCK, 0);
    // sleep_us(TCKWAIT);
}

void jtag_reset() {
	int i;
	for (i=0; i<5; i++) jtag_tms(1);
}

void jtag_idle() {
	int i;
	for (i=0; i<5; i++) jtag_tms(1);
	jtag_tms(0);
}

// uint8_t (*next_block)(void *, uint8_t *)

// uint16_t (*next_block)(void *, uint8_t *)


// uint8_t (*next_block)(void *, uint8_t *)


uint32_t jtag_ins_ex_cb(uint8_t ins, uint16_t (*next_block)(void *, uint8_t *), int len, void *user_data) {
	uint32_t result = 0;
	uint8_t tmp;
  uint8_t buff[512];

	jtag_tms(1);
	jtag_tms(1);
	jtag_tms(0);

	jtag_tdin(6, ins);
	jtag_tms(1); // exit ir
	jtag_tms(1); // updir

  jtag_tms(1); // select dr
  jtag_tms(0); // capture dr

  // get dr
  while (len > 0) {
    uint16_t n = next_block(user_data, buff);
    uint8_t *data = buff;
    n = n > len ? len : n;
    len -= n;

    while (n>0) {
      tmp = jtag_tdin_rev(8, *data++);
      n--;
      result = (tmp << 24) | (result >> 8);
    }
  }

  jtag_tms(1); // exit1 dr
  jtag_tms(1); // update dr
  jtag_tms(0); // idle
  jtag_tms(0); // idle

	return result;
}


uint32_t jtag_ins(uint8_t ins, uint8_t *data, int n) {
	uint32_t result = 0;
	uint8_t tmp;

	jtag_tms(1);
	jtag_tms(1);
	jtag_tms(0);

	jtag_tdin(6, ins);
	jtag_tms(1); // exit ir
	jtag_tms(1); // updir
	if (n == 0) {
		jtag_tms(0); // idle
		data = 0;
	} else {
		jtag_tms(1); // select dr
		jtag_tms(0); // capture dr

		// get dr
    while (n>0) {
      tmp = jtag_tdin(n > 8 ? 8 : n, *data++);
      n -= 8;
      result = (tmp << 24) | (result >> 8);
    }

		jtag_tms(1); // exit1 dr
		jtag_tms(1); // update dr
		jtag_tms(0); // idle
		jtag_tms(0); // idle
	}

	return result;
}

void jtag_ins_start(uint8_t ins) {
	jtag_tms(1);
	jtag_tms(1);
	jtag_tms(0);

	jtag_tdin(6, ins);
	jtag_tms(1); // exit ir
	jtag_tms(1); // updir

  jtag_tms(1); // select dr
  jtag_tms(0); // capture dr
}

void jtag_ins_end() {
  jtag_tms(1); // exit1 dr
  jtag_tms(1); // update dr
  jtag_tms(0); // idle
  jtag_tms(0); // idle
}



///////////////////////////////////////////////////////
// JTAG Xilinx specific
#ifndef INS_BYPASS
#define INS_BYPASS 0x3f
#define INS_IDCODE 0x09
#define INS_JPROGRAM 0x0B
#define INS_CONFIGIN 0x05
#define INS_JSTART 0x0c
#endif
#define NO_DATA	0xffffffff

void jtag_detect() {
	uint8_t no_data[] = {0xff, 0xff, 0xff, 0xff};
	jtag_idle();
	uint32_t idcode = jtag_ins(INS_IDCODE, no_data, 32);
  printf("Info : idcode = %08X\n", idcode);
}


int jtag_get_length(uint8_t *data, uint32_t filesize, uint32_t *size, uint32_t *offset) {
  int i = 0;
  uint8_t tag;
  uint16_t len;
  
  /* is ZX3 file? */
  if (data[0] == 0xff && data[1] == 0xff) {
    *size = filesize;
    *offset = 0;
    printf("This is a ZX3 file\n");
    return 1;
  }
  
  printf("hello\n");

  /* initial header */
  i += (data[i+0] << 8) | data[i+1];
  i += 2; // skip over length

  /* check version tag */
  len = (data[i+0] << 8) | data[i+1];
  if (len != 0x0001) {
    debug(("bitfile_get_length: unknown version %04X\n", len));
	  /* unknown version */
	  return 0;
  }
  i += 2; // skip over version

  while (i < (BLOCKSIZE-3)) {
    tag = data[i+0];
    if (tag == 'e') len = 4;
    else len = (data[i+1] << 8) | data[i+2];

    debug(("bitfile_get_length: tag (%02X) %c - %u (%04X)\n", tag, tag, len, len));

    if (tag == 'e' && i <= (BLOCKSIZE-7)) {
      *size = ((data[i+1]<<24) | (data[i+2]<<16) |
        (data[i+3]<<8) | data[i+4]) + i + 5;
      *offset = i+len+1;
      return 1;
    }
    i += len + (tag == 'e' ? 1 : 3);
  }
  debug(("bitfile_get_length: failed to detect len\n"));
  return 0;
}


static void jtag_start_xilinx(uint8_t *image, uint32_t imageSize, uint32_t device, uint32_t devicemask, uint32_t crc) {
	uint32_t idcode;
	uint8_t no_data[] = {0xff, 0xff, 0xff, 0xff};
	uint8_t rst_config[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // uint8_t jtagheader[] = {0x0c, 0x85, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};
  uint8_t jtagheader[] = {0x30, 0xa1, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};

  uint32_t size, offset;

  if (!jtag_get_length(image, imageSize, &size, &offset)) {
    debug(("bitfile_get_length: problems\n"));
    return;
  }

	printf("Info: Putting jtag in idle...\n");
	jtag_idle();

	printf("Info: grabbing idcode...\n");
	idcode = jtag_ins(INS_IDCODE, no_data, 32);

	printf("Info : idcode = %08X\n", idcode);
  if ((idcode & devicemask) != (device & devicemask)) {
		printf("Error: Incorrect device\n");
		return;
	}

	printf("Info: BYPASS...\n");
	jtag_ins(INS_BYPASS, no_data, 0);
	printf("Info: JPROGRAM...\n");
	jtag_ins(INS_JPROGRAM, no_data, 0);
  
	printf("Info: CONFIGIN...\n");
	jtag_ins(INS_CONFIGIN, no_data, 0);

  sleep_us(14000);

	printf("Info: CONFIGIN...\n");
	jtag_ins(INS_CONFIGIN, rst_config, 95);

	printf("Info: CONFIGIN...\n");
	jtag_ins_ex(INS_CONFIGIN, jtagheader, 8 * sizeof jtagheader, image, imageSize*8, 1);

  printf("Info: JSTART...\n");
	jtag_ins(INS_JSTART, no_data, 0);

  jtag_idle();
  sleep_us(10000);

	printf("Info: BYPASS...\n");
	jtag_ins(INS_BYPASS, no_data, 1);

  printf("Info: offset %d size %d\n", offset, size);
}

void jtag_start(uint8_t *image, uint32_t imageSize, uint32_t device, uint32_t devicemask, uint32_t crc) {
	switch (device & devicemask) {
		case XILINX_SPARTAN6_XL9:
			jtag_start_xilinx(image, imageSize, device, devicemask, crc);
			break;
		default:
			printf("Error: Device not supported.\n");
	}
}


// uint32_t jtag_ins_ex_cb(uint8_t ins, , int len, void *user_data)

#if 0
static uint8_t jtagheader[] = {0x30, 0xa1, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};
typedef struct {
  int n;
  uint8_t (*next_block)(void *, uint8_t *);
  void *user_data;

} jtag_block_status;

static uint16_t jtag_next_block(void *user_data, uint8_t *data) {
  jtag_block_status *status = (jtag_block_status *)user_data;
  if (status->n == 0) {
    status->n++;
    memcpy(data, jtagheader, sizeof jtagheader);
    return sizeof jtagheader;
  } else 
}
#endif

void jtag_tdin_rev_block(uint8_t *data, uint32_t len) {
  while (len--) {
    jtag_tdin_rev(8, *data++);
  }
}


int jtag_configure(void *user_data, uint8_t (*next_block)(void *, uint8_t *), uint32_t assumelength) {
	uint32_t idcode;
  uint8_t buff[512];
	uint8_t no_data[] = {0xff, 0xff, 0xff, 0xff};
	uint8_t rst_config[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // uint8_t jtagheader[] = {0x0c, 0x85, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};
  uint8_t jtagheader[] = {0x30, 0xa1, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};

  uint32_t size, offset;

  if (!next_block(user_data, buff)) {
    printf("Cannot read back data\n");
    return 1;
  }

  printf("%02X %02X %02X %02X\n", buff[0], buff[1], buff[2], buff[3]);

  if (buff[0] != 0x00 || buff[1] != 0x09 || buff[2] != 0x0f || buff[3] != 0xf0)
    return 1;


  if (!jtag_get_length(buff, assumelength, &size, &offset)) {
    debug(("bitfile_get_length: problems\n"));
    return 1;
  }

  printf("size %d offset %d\n", size, offset);

	printf("Info: Putting jtag in idle...\n");
	jtag_idle();

	printf("Info: grabbing idcode...\n");
	idcode = jtag_ins(INS_IDCODE, no_data, 32);

	printf("Info : idcode = %08X\n", idcode);
  if ((idcode & 0xfffffff) != XILINX_SPARTAN6_XL9) {
		printf("Error: Incorrect device\n");
		return 1;
	}

	printf("Info: BYPASS...\n");
	jtag_ins(INS_BYPASS, no_data, 0);
	printf("Info: JPROGRAM...\n");
	jtag_ins(INS_JPROGRAM, no_data, 0);
  
	printf("Info: CONFIGIN...\n");
	jtag_ins(INS_CONFIGIN, no_data, 0);

  sleep_us(14000);

	printf("Info: CONFIGIN...\n");
	jtag_ins(INS_CONFIGIN, rst_config, 95);

	printf("Info: CONFIGIN...\n");
	jtag_ins_start(INS_CONFIGIN);

  jtag_tdin_rev_block(jtagheader, sizeof jtagheader);
  jtag_tdin_rev_block(buff + offset, 512 - offset);
  size -= (512-offset);
  // size -= 512;

  int nr_blocks = 0;
  while (size && next_block(user_data, buff)) {
    int this_len = size > 512 ? 512 : size;
    jtag_tdin_rev_block(buff, this_len);
    size -= this_len;
    nr_blocks ++;
  }
  jtag_ins_end();
  printf("nr_blocks = %d\n", nr_blocks);


  printf("Info: JSTART...\n");
	jtag_ins(INS_JSTART, no_data, 0);

  jtag_idle();
  sleep_us(10000);

	printf("Info: BYPASS...\n");
	jtag_ins(INS_BYPASS, no_data, 1);
  return 0;
}

