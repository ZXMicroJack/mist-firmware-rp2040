#include "spi.h"
#include "hardware.h"
#include "user_io.h"
#include "tos.h"
#include "osd.h"
#include "screen.h"

// TODO MJ interface to FPGA via SPI and all the nCS for different bits.

void updateScreenCallback();

void spi_init() {
#if 0
   // Enable the peripheral clock in the PMC
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI;

    // Enable SPI interface
    *AT91C_SPI_CR = AT91C_SPI_SPIEN;

    // SPI Mode Register
    *AT91C_SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS  | (0x01 << 16);

    // SPI CS register
    AT91C_SPI_CSR[0] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (2 << 8) | (0x04 << 16) | (0x00 << 24); // USB
    AT91C_SPI_CSR[1] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (48 << 8) | (0x04 << 16) | (0x00 << 24); // MMC/CONF_DATA
    AT91C_SPI_CSR[2] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (2 << 8) | (0x04 << 16) | (0x00 << 24); // Data IO
    AT91C_SPI_CSR[3] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (2 << 8) | (0x04 << 16) | (0x00 << 24); // OSD

    // Configure pins for SPI use
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA14_SPCK | AT91C_PA13_MOSI | AT91C_PA12_MISO | AT91C_PA11_NPCS0 | AT91C_PA10_NPCS2 | AT91C_PA3_NPCS3;
    AT91C_BASE_PIOA->PIO_BSR = AT91C_PA9_NPCS1 | AT91C_PA10_NPCS2 | AT91C_PA3_NPCS3;

    // PA9 (CONF_DATA0) and PA31 (MMC) are both NPCS1. Give them to the SPI only when transfer is required.
    // Set them to high level by default.
    *AT91C_PIOA_SODR = FPGA0 | MMC_SEL;
#endif
  initScreen(updateScreenCallback);
}

RAMFUNC void spi_wait4xfer_end() {
}

#define COREID 0xa4

static uint8_t cmd[4096];
static uint8_t io = 0;
static uint8_t fpga = 0;
static uint8_t osd = 0;
static int cmd_pos = 0;
static uint8_t next_spi = 0xff;
static uint32_t system_control = 0;
static uint8_t butt_state = 0;

const uint16_t features = FEAT_MENU;
const char *CONF_STR = 
        "MENU;;"
        "O1,Video mode,PAL,NTSC;"
        "O23,Rotate,Off,Left,Right;"
        "V,12341234";
;
char *confptr;
static uint8_t io_status = 0;
static uint32_t io_status2 = 0;

uint8_t react_io() {

	if (cmd_pos > 1) {
		switch (cmd[0]) {
			case UIO_GET_SDSTAT:
			case UIO_KEYBOARD_IN:
			case UIO_MOUSE_IN:
			case UIO_SET_MOD:
				return 0x00;
			case UIO_SIO_IN:
				return 0x80;
			case UIO_GET_STRING:
				if (cmd_pos == 2) {
					confptr = CONF_STR;
				}
				return *confptr++;
			case UIO_SET_STATUS:
				io_status = cmd[1];
				printf("IO: Status set %02X\n", cmd[1]);
				break;
			case UIO_SET_STATUS2:
				io_status2 = (cmd[1]<<24)|(cmd[2]<<16)|(cmd[3]<<8)|cmd[4];
				printf("IO: Status set %08X\n", io_status2);
				break;
			case UIO_BUT_SW:
				butt_state = cmd[1];
				printf("IO: Button state set to %02X\n", butt_state);
				break;
			case UIO_GET_FEATS:
        if (cmd_pos == 2) return 0x80;
        if (cmd_pos == 3) return features >> 24;
				if (cmd_pos == 4) return features >> 16;
        if (cmd_pos == 5) return features >> 8;
				else return features & 0xff;
				break;


			default:
				printf("IO: Unknown command %02X\n", cmd[0]);
				return 0x00;
		}
	} else {
		return COREID;
	}
//	return 0xff;
}

uint8_t react_fpga() {
	if (cmd_pos > 1) {
		switch (cmd[0]) {
			case MIST_SET_CONTROL:
				if (cmd_pos == 5) {
					system_control = (cmd[1]<<24)|(cmd[2]<<16)|(cmd[3]<<8)|cmd[4];
					printf("Mist: System control %08x\n", system_control);
				}
				return 0x00;
			default:
				printf("IO: Unknown command %02X\n", cmd[0]);
				return 0x00;
		}
	}
	return 0xff;
}

uint8_t react_osd() {
	if (cmd_pos > 1) {
		switch (cmd[0]) {
			case MM1_OSDCMDENABLE:
//				printf("OSD: Enable OSD\n");
				return 0x00;

			case MM1_OSDCMDDISABLE:
//				printf("OSD: Disable OSD\n");
				return 0x00;
		
			default:
				if ((cmd[0] & 0xe0) == MM1_OSDCMDWRITE) {
//					printf("OSD: Write to OSD\n");
				} else {
//					printf("OSD: Unknown command %02X\n", cmd[0]);
				}
				return 0x00;
		}
	}
	return 0xff;
}

extern void putpixel(int x, int y, uint32_t pixel);

void write_to_screen() {
  updateScreen();
}

void updateScreenCallback() {
  int line = (cmd[0] & 0x1f) * 8;

  for (int x=1; x<cmd_pos; x++) {
    for (int y=0; y<8; y++) {
      uint8_t mask = 0x80;
      for (int b=0; b<8; b++) {
        putpixel(x, line+(8-b), (cmd[x] & mask) ? 0xffffff : 0x000000);
        mask >>= 1;
      }
    }
  }
  // printf("cmd_pos %d: ", cmd_pos);
  // for (int i=0; i<cmd_pos; i++) {
  //   printf("%02X ", cmd[i]);
  // }
  // printf("\n)");
}

void react_osd_end() {
	switch (cmd[0]) {
		case MM1_OSDCMDENABLE|DISABLE_KEYBOARD:
			printf("OSD: Enable OSD (disabling keyboard)\n");
			break;

		case MM1_OSDCMDDISABLE:
			printf("OSD: Disable OSD\n");
			break;
	
		default:
			if ((cmd[0] & 0xe0) == MM1_OSDCMDWRITE) {
				if ((cmd[0] & 0x1f) == 0x18) {
					printf("OSD: Clear\n");
				} else {
					printf("OSD: Write to OSD line %d\n", cmd[0] & 0x1f);
          write_to_screen();
				}
			} else {
				printf("OSD: Unknown command %02X\n", cmd[0]);
			}
			return 0x00;
	}

}



unsigned char SPI(unsigned char outByte) {
//	uint8_t ret = 0xff;

//	ret = cmd_pos == 0 ? 0xb4 : next_spi;

	if (io || fpga || osd)
	{
		if (cmd_pos < sizeof cmd) {
			cmd[cmd_pos++] = outByte;
		} else printf("Error: Overwriting command buffer\n");

		if (io) return react_io();
		if (fpga) return react_fpga();
		if (osd) return react_osd();
	}


//	printf("SPI(%02X)\n", outByte);
	return 0xff;
}

void EnableFpga()
{
	fpga = 1;
	cmd_pos = 0;
	printf("EnableFPGA();\n");
}

void DisableFpga()
{
	fpga = 0;
	printf("DisableFPGA();\n");
    spi_wait4xfer_end();
}

void EnableOsd()
{
	osd = 1;
	cmd_pos = 0;
	printf("EnableOsd();\n");
}

void DisableOsd()
{
	osd = 0;
	react_osd_end();
	printf("DisableOsd();\n");
    spi_wait4xfer_end();
}

void EnableIO() {
	io = 1;
	cmd_pos = 0;
	next_spi = 0xff;
//	printf("EnableIO();\n");
}

void DisableIO() {
	io = 0;
//	printf("DisableIO();\n");
    spi_wait4xfer_end();
}

void EnableDMode() {
	printf("EnableDMode();\n");
}

void DisableDMode() {
	printf("DisableDMode();\n");
}

RAMFUNC void EnableCard() {
	printf("EnableCard();\n");
}

RAMFUNC void DisableCard() {
	printf("DisableCard();\n");
}

void spi_max_start() {
}

void spi_max_end() {
    spi_wait4xfer_end();
}

void spi_block(unsigned short num) {
#if 0
      	unsigned short i;
  unsigned long t;

  for (i = 0; i < num; i++) {
    while (!(*AT91C_SPI_SR & AT91C_SPI_TDRE)); // wait until transmiter buffer is empty
    *AT91C_SPI_TDR = 0xFF; // write dummy spi data
  }
  while (!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY)); // wait for transfer end
  t = *AT91C_SPI_RDR; // dummy read to empty receiver buffer for new data
#endif
}

RAMFUNC void spi_read(char *addr, uint16_t len) {
#if 0
      	*AT91C_PIOA_SODR = AT91C_PA13_MOSI; // set GPIO output register
  *AT91C_PIOA_OER = AT91C_PA13_MOSI;  // GPIO pin as output
  *AT91C_PIOA_PER = AT91C_PA13_MOSI;  // enable GPIO function
  
  // use SPI PDC (DMA transfer)
  *AT91C_SPI_TPR = (unsigned long)addr;
  *AT91C_SPI_TCR = len;
  *AT91C_SPI_TNCR = 0;
  *AT91C_SPI_RPR = (unsigned long)addr;
  *AT91C_SPI_RCR = len;
  *AT91C_SPI_RNCR = 0;
  *AT91C_SPI_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN; // start DMA transfer
  // wait for tranfer end
  while ((*AT91C_SPI_SR & (AT91C_SPI_ENDTX | AT91C_SPI_ENDRX)) != (AT91C_SPI_ENDTX | AT91C_SPI_ENDRX));
  *AT91C_SPI_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS; // disable transmitter and receiver

  *AT91C_PIOA_PDR = AT91C_PA13_MOSI; // disable GPIO function
#endif
}

RAMFUNC void spi_block_read(char *addr) {
  spi_read(addr, 512);
}

void spi_write(const char *addr, uint16_t len) {
#if 0
      	// use SPI PDC (DMA transfer)
  *AT91C_SPI_TPR = (unsigned long)addr;
  *AT91C_SPI_TCR = len;
  *AT91C_SPI_TNCR = 0;
  *AT91C_SPI_RCR = 0;
  *AT91C_SPI_PTCR = AT91C_PDC_TXTEN; // start DMA transfer
  // wait for tranfer end
  while (!(*AT91C_SPI_SR & AT91C_SPI_ENDTX));
  *AT91C_SPI_PTCR = AT91C_PDC_TXTDIS; // disable transmitter
#endif
}

void spi_block_write(const char *addr) {
  spi_write(addr, 512);
}

static unsigned char spi_speed;

void spi_slow() {
#if 0
      	AT91C_SPI_CSR[1] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (4 << 16) | (SPI_SLOW_CLK_VALUE << 8) | (2 << 24); // init clock 100-400 kHz
  spi_speed = SPI_SLOW_CLK_VALUE;
#endif
}

void spi_fast() {
#if 0
      	// set appropriate SPI speed for SD/SDHC card (max 25 Mhz)
  AT91C_SPI_CSR[1] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (4 << 16) | (SPI_SDC_CLK_VALUE << 8); // 24 MHz SPI clock
  spi_speed = SPI_SDC_CLK_VALUE;
#endif
}

void spi_fast_mmc() {
#if 0
      	// set appropriate SPI speed for MMC card (max 20Mhz)
  AT91C_SPI_CSR[1] = AT91C_SPI_CPOL | AT91C_SPI_CSAAT | (4 << 16) | (SPI_MMC_CLK_VALUE << 8); // 16 MHz SPI clock
  spi_speed = SPI_MMC_CLK_VALUE;
#endif
}

unsigned char spi_get_speed() {
  return spi_speed;
}

void spi_set_speed(unsigned char speed) {
  switch (speed) {
    case SPI_SLOW_CLK_VALUE:
      spi_slow();
      break;

    case SPI_SDC_CLK_VALUE:
      spi_fast();
      break;

    default:
      spi_fast_mmc();
  }
}

/* generic helper */
unsigned char spi_in() {
  return SPI(0);
}

void spi8(unsigned char parm) {
  SPI(parm);
}

void spi16(unsigned short parm) {
  SPI(parm >> 8);
  SPI(parm >> 0);
}

void spi16le(unsigned short parm) {
  SPI(parm >> 0);
  SPI(parm >> 8);
}

void spi24(unsigned long parm) {
  SPI(parm >> 16);
  SPI(parm >> 8);
  SPI(parm >> 0);
}

void spi32(unsigned long parm) {
  SPI(parm >> 24);
  SPI(parm >> 16);
  SPI(parm >> 8);
  SPI(parm >> 0);
}

// little endian: lsb first
void spi32le(unsigned long parm) {
  SPI(parm >> 0);
  SPI(parm >> 8);
  SPI(parm >> 16);
  SPI(parm >> 24);
}

void spi_n(unsigned char value, unsigned short cnt) {
  while(cnt--) 
    SPI(value);
}

/* OSD related SPI functions */
void spi_osd_cmd_cont(unsigned char cmd) {
  EnableOsd();
  SPI(cmd);
}

void spi_osd_cmd(unsigned char cmd) {
  spi_osd_cmd_cont(cmd);
  DisableOsd();
}

void spi_osd_cmd8_cont(unsigned char cmd, unsigned char parm) {
  EnableOsd();
  SPI(cmd);
  SPI(parm);
}

void spi_osd_cmd8(unsigned char cmd, unsigned char parm) {
  spi_osd_cmd8_cont(cmd, parm);
  DisableOsd();
}

void spi_osd_cmd32_cont(unsigned char cmd, unsigned long parm) {
  EnableOsd();
  SPI(cmd);
  spi32(parm);
}

void spi_osd_cmd32(unsigned char cmd, unsigned long parm) {
  spi_osd_cmd32_cont(cmd, parm);
  DisableOsd();
}

void spi_osd_cmd32le_cont(unsigned char cmd, unsigned long parm) {
  EnableOsd();
  SPI(cmd);
  spi32le(parm);
}

void spi_osd_cmd32le(unsigned char cmd, unsigned long parm) {
  spi_osd_cmd32le_cont(cmd, parm);
  DisableOsd();
}

/* User_io related SPI functions */
void spi_uio_cmd_cont(unsigned char cmd) {
  EnableIO();
  SPI(cmd);
}

void spi_uio_cmd(unsigned char cmd) {
  spi_uio_cmd_cont(cmd);
  DisableIO();
}

void spi_uio_cmd8_cont(unsigned char cmd, unsigned char parm) {
  EnableIO();
  SPI(cmd);
  SPI(parm);
}

void spi_uio_cmd8(unsigned char cmd, unsigned char parm) {
  spi_uio_cmd8_cont(cmd, parm);
  DisableIO();
}

void spi_uio_cmd32(unsigned char cmd, unsigned long parm) {
  EnableIO();
  SPI(cmd);
  SPI(parm);
  SPI(parm>>8);
  SPI(parm>>16);
  SPI(parm>>24);
  DisableIO();
}

void spi_uio_cmd64(unsigned char cmd, unsigned long long parm) {
  EnableIO();
  SPI(cmd);
  SPI(parm);
  SPI(parm>>8);
  SPI(parm>>16);
  SPI(parm>>24);
  SPI(parm>>32);
  SPI(parm>>40);
  SPI(parm>>48);
  SPI(parm>>56);
  DisableIO();
}
