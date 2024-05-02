#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "pico/time.h"
#include "hardware/gpio.h"
#include "jtag.h"
#include "pins.h"
// #include "gpio.h"
// #include "huffman.h"
// #include "rle.h"
// #include "output.h"

#define TDO GPIO_JTAG_TDO
#define TDI GPIO_JTAG_TDI
#define TCK GPIO_JTAG_TCK
#define TMS GPIO_JTAG_TMS

#define TCKWAIT 1

#define NO_COMPRESS

#define o_putsln(ch, s) printf("%s\n", s)
#define o_putln(ch) printf("\n")
#define o_puts(ch, s) printf("%s", s)
#define o_puthex(ch, h) printf("%08X", h)


///////////////////////////////////////////////////////
// CRC32 specific
#define CRC32_POLY 0x04c11db7   /* AUTODIN II, Ethernet, & FDDI */
#define CRC32(crc, byte) \
        crc = (crc << 8) ^ CRC32_lut[(crc >> 24) ^ byte]

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
static int jtag_preinslen = 0;

int jtag_init(int ch, int preinslen) {
  // cable gpiodirect tdo=9 tdi=10 tck=11 tms=25
  o_putsln(ch, "Info: setting up GPIO pins");


  gpio_init(TDO);
  gpio_init(TDI);
  gpio_init(TCK);
  gpio_init(TMS);

  gpio_set_dir(TDO, GPIO_IN);
  gpio_set_dir(TDI, GPIO_OUT);
  gpio_set_dir(TCK, GPIO_OUT);
  gpio_set_dir(TMS, GPIO_OUT);
	
	// jtag_preinslen = preinslen;
  jtag_preinslen = 0;

  return 0;
}

void jtag_kill(void) {
  // gpio_kill();
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

void jtag_tck(void) {
    gpio_put(TCK, 1);
    sleep_us(TCKWAIT);
    gpio_put(TCK, 0);
    sleep_us(TCKWAIT);
}

///////////////////////////////////////////////////////
// JTAG higher level

// subfunction: enumerate the JTAG chain
void jtag_enumdevs(int ch)
{
    uint8_t n7, n6, n5, n4, n3, n2, lsb;

    // Test-Logic-Reset
    jtag_tms(1); jtag_tms(1); jtag_tms(1); jtag_tms(1); jtag_tms(1);

    // Capture-DR
    jtag_tms(0); jtag_tms(1); jtag_tms(0);

    for(;;) {
        lsb = jtag_tdi(1);
        if(lsb == 0) {
            o_putsln(ch, "0         [device has no idcode]");
            break;
        } else {
            // read manufacturer code (7+4 bits)
            lsb |= jtag_tdin(7, 0xff) << 1;
            n2 = jtag_tdin(4, 0xf);

						// printf("lsb %02X n2 = %X\n", lsb, n2);

            if(lsb == 0xff && n2 == 0xf) {
                // no such manufacturer, must be our own 1s
                // NB: actually IEEE 1149.1-2001 reserves 0000 1111111
                //     as the invalid manufacturer code, but i'm too
                //     lazy to track when to insert the zeros, so let's
                //     assume 1111 1111111 will not be used either.
                break;
            }

            // read product code
            n3 = jtag_tdin(4, 0xf);
            n4 = jtag_tdin(4, 0xf);
            n5 = jtag_tdin(4, 0xf);
            n6 = jtag_tdin(4, 0xf);
            // read product version
            n7 = jtag_tdin(4, 0xf);

            // print in the format VPPPPMMM
						o_puts(ch, "Id: ");
            o_puthex(ch, (n7<<28)|(n6<<24)|(n5<<20)|(n4<<16)|(n3<<12)|(n2<<8)|lsb);
            o_putln(ch);
        }
    }

    // Run-Test/Idle
    jtag_tms(1); jtag_tms(1); jtag_tms(0);
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

uint32_t jtag_ins(uint8_t ins, uint8_t *data, int n) {
  return jtag_ins_crc(ins, data, n, NULL);
}

///////////////////////////////////////////////////////
// compression glue
#if 0
static uint8_t *huff_inbuff;
static uint32_t huff_inlen;

int huff_in(void) {
	if (huff_inlen) {
		huff_inlen --;
		return *huff_inbuff++;
	} else {
		return -1;
	}
}

int rle_in(void) {
  return huff_get();
}
#endif

uint32_t jtag_ins_crc(uint8_t ins, uint8_t *data, int n, uint32_t *crcCalc) {
	uint32_t result = 0;
  uint32_t crc = 0xffffffff;
	uint8_t tmp;

	jtag_tms(1);
	jtag_tms(1);
	jtag_tms(0);
	// if (jtag_preinslen) jtag_tdin(jtag_preinslen, 0xff);
	jtag_tdin(6, ins);
	jtag_tms(1); // exit ir
	jtag_tms(1); // updir
	if (n == 0) {
		jtag_tms(0); // idle
		data = 0;
	} else {
		jtag_tms(1); // select dr
		jtag_tms(0); // capture dr
		// if (jtag_preinslen) jtag_tdi(0);

		// get dr
#ifndef NO_COMPRESS
    if (n < 128) {
#endif
  		while (n>0) {
        if (crcCalc) CRC32(crc, *data);
  			tmp = jtag_tdin(n > 8 ? 8 : n, *data++);
  			n -= 8;
  			result = (tmp << 24) | (result >> 8);
      }
#ifndef NO_COMPRESS
    } else {
      huff_reset();
      rle_reset();
      huff_inbuff = data;
      huff_inlen = (n+7)/8;

      int c = rle_get();
      while (c >= 0) {
        if (crcCalc) CRC32(crc, c);
        tmp = jtag_tdin(n >= 8 ? 8 : n, c);

  			result = (tmp << 24) | (result >> 8);
        c = rle_get();
				n -= 8;
  		}
    }
#endif
		// printf("\n");

		jtag_tms(1); // exit1 dr
		jtag_tms(1); // update dr
		jtag_tms(0); // idle
		jtag_tms(0); // idle
	}

  if (crcCalc) *crcCalc = crc;
	return result;
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

void jtag_detect(int ch) {
	uint8_t no_data[] = {0xff, 0xff, 0xff, 0xff};
	jtag_idle();
	uint32_t idcode = jtag_ins(INS_IDCODE, no_data, 32);
  printf("Info : idcode = %08X\n", idcode);
	// o_puts(ch, "Info : idcode = "); o_puthex(ch, idcode); o_putln(ch);
}

static void jtag_start_xilinx(int ch, uint8_t *image, uint32_t imageSize, uint32_t device, uint32_t devicemask, uint32_t crc) {
	uint32_t idcode;
	uint8_t no_data[] = {0xff, 0xff, 0xff, 0xff};
	uint8_t rst_config[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint32_t crcCalc = 0xffffffff;

  initCRC();

	o_putsln(ch, "Info: Putting jtag in idle...\n");
	jtag_idle();

	o_putsln(ch, "Info: grabbing idcode...\n");
// 	idcode = jtag_ins(INS_IDCODE, no_data, 32)>>1;
	idcode = jtag_ins(INS_IDCODE, no_data, 32);
	o_puts(ch, "Info : idcode = "); o_puthex(ch, idcode); o_putln(ch);
  if ((idcode & devicemask) != (device & devicemask)) {
		o_putsln(ch, "Error: Incorrect device");
		return;
	}

	o_putsln(ch, "Info: BYPASS...");
	jtag_ins(INS_BYPASS, no_data, 0);
	o_putsln(ch, "Info: JPROGRAM...");
	jtag_ins(INS_JPROGRAM, no_data, 0);
	o_putsln(ch, "Info: CONFIGIN...");
	jtag_ins(INS_CONFIGIN, no_data, 0);
	// RUNTEST 10000 TCK;
  sleep_us(14000);

// 	o_putsln(ch, "Info: CONFIGIN...");
// 	jtag_ins(INS_CONFIGIN, no_data, 0);

// 	jtag_idle();
	o_putsln(ch, "Info: CONFIGIN...");
	jtag_ins(INS_CONFIGIN, rst_config, 95);
//   gpio_sleep_us(10000);
	o_putsln(ch, "Info: CONFIGIN...");
	jtag_ins_crc(INS_CONFIGIN, image, imageSize, &crcCalc);
  if (crcCalc != crc) {
    o_puts(ch, "Error: CRC doesn't match (");
    o_puthex(ch, crc);
    o_puts(ch, " != ");
    o_puthex(ch, crcCalc);
    o_putln(ch);
    return;
  }
  o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);

		jtag_idle();
		sleep_us(10000);
//   o_putsln(ch, "Info: JSTART...");
// 	jtag_ins(INS_JSTART, no_data, 0);
#if 0
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);
	o_putsln(ch, "Info: BYPASS...");
	jtag_ins(INS_BYPASS, no_data, 0);
	o_putsln(ch, "Info: BYPASS...");
	jtag_ins(INS_BYPASS, no_data, 0);
	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);

	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);
	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);
	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);
	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);
	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);
	o_putsln(ch, "Info: JSTART...");
	jtag_ins(INS_JSTART, no_data, 0);
	// RUNTEST 24 TCK;
  gpio_sleep_us(13);


#endif
	o_putsln(ch, "Info: BYPASS...");
	jtag_ins(INS_BYPASS, no_data, 1);
}

void jtag_start(int ch, uint8_t *image, uint32_t imageSize, uint32_t device, uint32_t devicemask, uint32_t crc) {
	switch (device & devicemask) {
		case XILINX_SPARTAN6_XL16:
		case XILINX_SPARTAN6_XL25:
		case XILINX_SPARTAN3E_500:
			jtag_start_xilinx(ch, image, imageSize, device, devicemask, crc);
			break;
		default:
			o_putsln(ch, "Error: Device not supported.");
	}
}
