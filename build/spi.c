#include "spi.h"
#include "hardware.h"

#undef spi_init
#include "hardware/spi.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"

// TODO MJ interface to FPGA via SPI and all the nCS for different bits.

#define MIST_CSN    17
#define MIST_SS2    20
#define MIST_SS3    21
#define MIST_SS4    24


void test_UserIOSPI(uint8_t datain) {
  uint8_t data[6];
// int spi_write_read_blocking (spi_inst_t *spi, const uint8_t *src, uint8_t *dst, size_t len)
//   uint8_t cmd[] = {0x1a, 0x00, 0x00, 0x00, 0xff, 0xff};
  uint8_t cmd[] = {0x02, 0xff};

  gpio_put(MIST_CSN, 0);

  cmd[1] = datain;

  memset(data, 0xff, sizeof data);
  spi_write_read_blocking(spi0, cmd, data, sizeof cmd);
  printf("Returns: ");
  for (int i=0; i<sizeof data; i++) {
    printf("%02X ", data[i]);
  }
  printf("\n");
  gpio_put(MIST_CSN, 1);
}



void test_UserIOInit() {
  gpio_init(MIST_CSN);
  gpio_put(MIST_CSN, 1);
  gpio_set_dir(MIST_CSN, GPIO_OUT);
  uint8_t spi_pins[] = {16, 18, 19};

  for (int i=0; i<sizeof spi_pins; i++) {
    gpio_init(spi_pins[i]);
    gpio_set_function(spi_pins[i], GPIO_FUNC_SPI);
  }
  spi_init(spi0, 500000); // 500khz
  spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

void test_UserIOKill() {
  spi_deinit(spi0);
}


void mist_spi_init() {
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
}

RAMFUNC void spi_wait4xfer_end() {
}

void EnableFpga()
{
}

void DisableFpga()
{
    spi_wait4xfer_end();
}

void EnableOsd()
{
}

void DisableOsd()
{
    spi_wait4xfer_end();
}

void EnableIO() {
}

void DisableIO() {
    spi_wait4xfer_end();
}

void EnableDMode() {
}

void DisableDMode() {
}

RAMFUNC void EnableCard() {
}

RAMFUNC void DisableCard() {
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
