#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/spi.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "hardware/pio.h"

#include "fpga.h"
#include "flash.h"
#include "pio_spi.h"
#include "sdcard.h"
#include "ps2.h"
#include "fifo.h"
#include "ipc.h"
#include "kbd.h"
#define DEBUG
#include "debug.h"

#define TEST_PS2
// #define TEST_PS2_HOST
// #define TEST_IPC
// #define TEST_SDCARD_SPI
// #define TEST_FPGA
// #define TEST_MATRIX
// #define TEST_FLASH
// #define TEST_USERIO
// #define TEST_JAMMA

// KEY ACTION ALLOCATION
// aAgGHjJlMNoOqQrRTuUVwWxXyYzZ
// aAgGHJlMNqQrRTVwWxXYZ
// IPC      -=[]'#
// FLASH    PBI
// MATRIXK  KL
// PS2      kheEdD
// PS2      0123456789 - press key
// FPGA     pfFcC
// HELP     ?
// SDCARD   vbnm,isS
// USERIO   uUV


typedef struct {
  int block_nr;
  int dumps;
} test_block_read_t;

// #define FPGA_IMAGE_POS  0x100a0000
// #define FPGA_IMAGE_SIZE 786789
#define FPGA_IMAGE_POS 0x100b0000
#define FPGA_IMAGE_SIZE 1340672


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

bool test_fpga_get_next_block(void *user_data, uint8_t *data) {
  int j;
  int thislen;
  int o = 0;
  test_block_read_t *b = (test_block_read_t *)user_data;
  
  if ((b->block_nr * 512) > FPGA_IMAGE_SIZE) {
    return false;
  }

  uint8_t *bits = (uint8_t *)(FPGA_IMAGE_POS + b->block_nr * 512);
  
//   printf("bits = %p\n", bits);
//   if (b->dumps < 10) {
//     hexdump(bits, 512);
//     b->dumps++;
//   }

  
  memcpy(data, bits, 512);
  b->block_nr ++;
  return true;
}

#define TEST_WRITE_BLOCK 0x80010

void test_sector_read(pio_spi_inst_t *spi) {
  uint8_t sect[512];
  printf("------ READ SECTORS ------\n");
  sd_readsector(spi, 0x20000, sect);
  hexdump(sect, 512);
  sd_readsector(spi, 0x20001, sect);
  hexdump(sect, 512);
  sd_readsector(spi, 0x20002, sect);
  hexdump(sect, 512);
}

void test_sector_write1(pio_spi_inst_t *spi) {
  uint8_t sect[512];
  printf("------ READ TEST WRITE SECTOR ------\n");
  sd_readsector(spi, TEST_WRITE_BLOCK, sect);
  hexdump(sect, 512);
}

void test_sector_write2(pio_spi_inst_t *spi) {
  uint8_t sect[512];
  printf("------ WRITE TEST WRITE SECTOR 55 ------\n");
  memset(sect, 0x55, sizeof sect);
  sd_writesector(spi, TEST_WRITE_BLOCK, sect);

  printf("------ READBACK TEST WRITE SECTOR 55 ------\n");
  memset(sect, 0x00, sizeof sect);
  sd_readsector(spi, TEST_WRITE_BLOCK, sect);
  hexdump(sect, 512);
}

void test_sector_write3(pio_spi_inst_t *spi) {
  uint8_t sect[512];
  printf("------ WRITE AND READ TEST WRITE SECTOR AA ------\n");
  memset(sect, 0xaa, sizeof sect);
  sd_writesector(spi, TEST_WRITE_BLOCK, sect);
  memset(sect, 0x00, sizeof sect);
  sd_readsector(spi, TEST_WRITE_BLOCK, sect);
  hexdump(sect, 512);
    
}

void test_sector_write4(pio_spi_inst_t *spi) {
  uint8_t sect[512];
  printf("------ RESET TEST WRITE SECTOR 00 ------\n");
  memset(sect, 0x00, sizeof sect);
  sd_writesector(spi, TEST_WRITE_BLOCK, sect);
  memset(sect, 0x55, sizeof sect);
  sd_readsector(spi, TEST_WRITE_BLOCK, sect);
  hexdump(sect, 512);
}

void all_tests(pio_spi_inst_t *spi) {
  if (sd_init_card(spi)) {
    test_sector_read(spi);
    test_sector_write1(spi);
    test_sector_write2(spi);
    test_sector_write3(spi);
    test_sector_write4(spi);
  }
}

// void ps2_GotChar(uint8_t ch, uint8_t data) {
//   printf("ps2_GotChar: ch:%d data:%02X\n", ch, data);
// }

// works
// 0x1e - w4 - 0001 1110
// 0x2e - w4 - 0010 1110
// 0x36 - w4 - 0011 0110
// 0x3d - w5 - 0011 1001

// notworks
// 0x45 - w3 - 0100 1001
// 0x16 - w3 - 0001 0110
// 0x26 - w3 - 0010 0110
// 0x25 - w3 - 
// 0x3e - w5
// 0x46 - w3

const uint8_t kbdlut[] = {0x45, 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46};

#if PICO_NO_FLASH
static void enable_xip(void) {
  rom_connect_internal_flash_fn connect_internal_flash = (rom_connect_internal_flash_fn)rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH);
    rom_flash_exit_xip_fn flash_exit_xip = (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
    rom_flash_flush_cache_fn flash_flush_cache = (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);
    rom_flash_enter_cmd_xip_fn flash_enter_cmd_xip = (rom_flash_enter_cmd_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_ENTER_CMD_XIP);

    connect_internal_flash();    // Configure pins
    flash_exit_xip();            // Init SSI, prepare flash for command mode
    flash_flush_cache();         // Flush & enable XIP cache
    flash_enter_cmd_xip();       // Configure SSI with read cmd
}
#endif

uint8_t flash_data[4096];
uint16_t flash_crc;

uint8_t ipc_GotCommand(uint8_t cmd, uint8_t *data, uint8_t len) {
  printf("ipc_GotCommand: cmd %02X\n", cmd);
  hexdump(data, len);
  switch(cmd) {
    case IPC_FLASHDATA:
      memcpy(&flash_data[(data[0] & 0x1f) * 128], data + 1, 128);
      return 0x01;
    case IPC_FLASHCOMMIT: {
      uint32_t addr = (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
      uint16_t crc = (data[4]<<8) | data[5];
      uint16_t calcCrc = crc16(flash_data, sizeof flash_data);
      
      printf("addr %08X crc %04X calcCrc %04X\n", addr, crc, calcCrc);
      if (crc != calcCrc) {
        return 2;
      } else {
        return flash_EraseProgram(addr, flash_data, sizeof flash_data);
      }
    }
    
#if 0
    case IPC_FPGA_CLAIM:
      return fpga_claim(data[0] ? true : false);

    case IPC_FPGA_RESET:
      return fpga_reset();

    case IPC_FPGA_START:
      fpga_configure_start();
      return 1;

    case IPC_FPGA_DATA:
      fpga_configure_data(data, len);
      return 1;

    case IPC_FPGA_END:
      fpga_configure_end();
      return 1;
#endif
    case 0xed:
      return 0x01;
    case 0x12:
      return 0x02;
    default:
      return 0xfd;
  }
}


void keypress(uint8_t ch) {
  printf("kbdsendchar ch:%02X\n", ch);
  ps2_SendChar(0, ch);
  sleep_ms(100);
  ps2_SendChar(0, 0xf0);
  ps2_SendChar(0, ch);
}

void keyledon(uint8_t ledstate) {
  // ps2_SendChar(0, 0xff);
  ps2_SendChar(0, 0xed);
  sleep_ms(100);
  ps2_SendChar(0, ledstate);

  // ps2_SendChar(1, 0xff);
  ps2_SendChar(1, 0xed);
  sleep_ms(100);
  ps2_SendChar(1, ledstate);
}

void keyreset() {
  ps2_SendChar(0, 0xff);
  ps2_SendChar(1, 0xff);
}

void mouseenable(uint8_t ch) {
  ps2_SendChar(ch, 0xf4);
}

// 16  v8_miso \   uart0 tx, SPI0RX
// 17  w9_mosi |-- sdcard high level / uart0 rx, SPI0CSN
// 18  w7_sck  |   SPI0SCK
// 19  v7_cs   /   SPI0TX

#if 0
void test_UserIOSPI() {
  uint8_t data[6];
// int spi_write_read_blocking (spi_inst_t *spi, const uint8_t *src, uint8_t *dst, size_t len)
  uint8_t cmd[] = {0x1a, 0x00, 0x00, 0x00, 0xff, 0xff};

  gpio_put(17, 0);

  memset(data, 0xff, sizeof data);
  spi_write_read_blocking(spi0, cmd, data, sizeof cmd);
  printf("Returns: ");
  for (int i=0; i<sizeof data; i++) {
    printf("%02X ", data[i]);
  }
  printf("\n");
  gpio_put(17, 1);
}
#endif

#define MIST_CSN    17
#define MIST_SS2    20
#define MIST_SS3    21
#define MIST_SS4    24


void ipc_HandleData(uint8_t tag, uint8_t *data, uint16_t len) {
  printf("ipc_HandleData: tag %02X\n", tag);
  hexdump(data, len);
}

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


extern int forceexit;
int main()
{
  stdio_init_all();
  test_block_read_t fbrt;
  sleep_ms(2000); // usb settle delay
  pio_spi_inst_t *spi = NULL;
  uint8_t buf[16];
  int i;
  uint8_t rddata = 0;

  // set up error led
//   gpio_init(PICO_DEFAULT_LED_PIN);
//   gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

#if PICO_NO_FLASH
  enable_xip();
#endif
  
#ifdef TEST_MATRIX
  kbd_Init();
#endif
#ifdef TEST_PS2
  ps2_Init();
  // ps2_InitX();
  ps2_EnablePort(0, true);
#endif
//   ps2_SendChar(0, 0x7e);
//   ps2_SendChar(0, 0xf0);
//   ps2_SendChar(0, 0x7e);
  printf("Drivertest Microjack\'23\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  
//   ipc_InitMaster();
//   ps2_InitEx(1);
  for(;;) {
    int c = getchar_timeout_us(10);

#if defined(TEST_PS2_HOST) || defined(TEST_PS2)
    ps2_HealthCheck();
    ps2_DebugQueues();
    ps2_DebugQueuesX();
#endif

//     ipc_MasterTick();

//     int c = getchar();
    if (forceexit) break;
    if (c == 'q') break;
//     if (c == 'h') printf("Hello\n");
//     kbd_Process();
//     printf("Hello\n");


#ifdef TEST_PS2
    if (c >= '0' && c <= '9') {
      ps2_SendChar(0, kbdlut[c - '0']);
      sleep_ms(100);
      ps2_SendChar(0, 0xf0);
      ps2_SendChar(0, kbdlut[c - '0']);
    }
#endif

    switch(c) {
      // SD CARD SPI
#ifdef TEST_SDCARD_SPI
      case 'v': test_sector_read(spi); break;
      case 'b': test_sector_write1(spi); break;
      case 'n': test_sector_write2(spi); break;
      case 'm': test_sector_write3(spi); break;
      case ',': test_sector_write4(spi); break;
      case 'i': printf("sd_init_card(spi) returns %d\n", sd_init_card(spi)); break;
      case 's': printf("init sdhw\n"); spi = sd_hw_init(); break;
      case 'S': printf("kill sdhw\n"); sd_hw_kill(spi); break;
      case 'o':
        memset(buf, 0, sizeof buf);
        printf("cmd9 returns %02X\n", sd_cmd9(spi, buf)); // CSD
        for (i=0; i<16; i++) printf("%02X ", buf[i]);
        printf("\n");
        break;

      case 'O':
        memset(buf, 0, sizeof buf);
        printf("cmd10 returns %02X\n", sd_cmd10(spi, buf)); // CSD
        for (i=0; i<16; i++) printf("%02X ", buf[i]);
        printf("\n");
        break;
#endif

      // FPGA PROGRAM
#ifdef TEST_FPGA
      case 'p':
        memset(&fbrt, 0x00, sizeof fbrt);
#ifdef ALTERA_FPGA
        initCRC();
//         printf("crc32: %08X\n", crc32(0xffffffff, (uint8_t *)FPGA_IMAGE_POS, FPGA_IMAGE_SIZE));
//         fpga_reset();
        printf("fpga_program returns %d\n", fpga_configure(&fbrt, test_fpga_get_next_block, FPGA_IMAGE_SIZE));
#else
        printf("fpga_program returns %d\n", fpga_configure(&fbrt, test_fpga_get_next_block, 0));
#endif
        break;
      case 'f':
        printf("fpga_initialise(); returns %d\n", fpga_initialise());
        break;
      case 'F':
        printf("fpga_reset(); returns %d\n", fpga_reset());
        break;
      case 'c':
        printf("fpga_claim(false); returns %d\n", fpga_claim(false));
        break;
      case 'C':
        printf("fpga_claim(true); returns %d\n", fpga_claim(true));
        break;
#endif

      // HELP
      case '?':
        printf("\n");
        printf("SDCARD: r(v)0 w(b)1 w(n)2 w(m)3 w(,)4 (i)nit (s)etup (S)hutdown \n");
        printf("FPGA: (p)rogram (f)pga init (F)pga reset (C)laim dis(c)laim\n");
        printf("PS2: (k)ps2init sendc(h)ar (e)n ps0 (E)n ps1 (d)isps0 (D)isps1\n");
        printf("IPC: (-)initslave (=)initmaster ([)cmd1 (])cmd2 (')slavetick (#)debug\n");
        printf("IPC: (;) read fast (:) write 1 byte fast (@) write 1024 bytes fast\n");
        printf("FLASH: (P)rogram (I)pc based program (B)ad crc ipc program\n");
        printf("KBD: (K)bd init (L)kbd process\n");
        printf("USERIO: (u)init (U)close (V)coreid\n");
        printf("JAMMA: (j)init (J)getdata\n");
        break;

      // PS2
#if defined(TEST_PS2) || defined(TEST_PS2_HOST)
      case 'k': printf("ps2init\n"); ps2_Init(); break;
#endif
#if defined(TEST_PS2)
      case 'e': printf("enable ps2 0\n"); ps2_EnablePort(0, true); break;
      case 'E': printf("enable ps2 1\n"); ps2_EnablePort(1, true); break;
      case 'd': printf("disable ps2 0\n"); ps2_EnablePort(0, false); break;
      case 'D': printf("disable ps2 1\n"); ps2_EnablePort(1, false); break;
#endif

#if defined(TEST_PS2_HOST)
      case 'e': printf("enable ps2 0\n"); ps2_EnablePortEx(0, true, true); break;
      case 'E': printf("enable ps2 1\n"); ps2_EnablePortEx(1, true, true); break;
      case 'd': printf("disable ps2 0\n"); ps2_EnablePortEx(0, false, true); break;
      case 'D': printf("disable ps2 1\n"); ps2_EnablePortEx(1, false, true); break;
      case 'm': printf("enable mouse 0\n"); mouseenable(0); break;
      case 'M': printf("enable mouse 1\n"); mouseenable(1); break;
#endif

#ifdef TEST_PS2
      case 'h': keypress(0x7e); break;
      case 'H': keypress(0x58); break;
#endif

      // case 'r': ps2_SendCharX(0, 0xff); break;

#ifdef TEST_PS2_HOST
      case 'h': keyledon(0x00); break;
      case 'H': keyledon(0x07); break;
      case 'r': keyreset(); break;
#endif

      // IPC
#ifdef TEST_IPC
      case '-':
        printf("ipc_InitSlave();\n");
        ipc_InitSlave();
        break;
      case '=':
        printf("ipc_InitMaster();\n");
        ipc_InitMaster();
        break;
      case '[': {
        uint8_t cmddata[20];
        memset(cmddata, 0x55, sizeof cmddata);
        printf("ipc_Command returns %d\n", ipc_Command(0xed, cmddata, sizeof cmddata));
      }
      break;
      case '{': {
        uint8_t cmddata[20];
        memset(cmddata, 0x55, sizeof cmddata);
        ipc_SendData(0xed, cmddata, sizeof cmddata);
        printf("ipc_SendData ED\n");
      }
      break;
      case ']': {
        uint8_t cmddata[128];
        memset(cmddata, 0x45, sizeof cmddata);
        printf("ipc_Command returns %d\n", ipc_Command(0x12, cmddata, sizeof cmddata));
      }
      break;
      case '}': {
        uint8_t cmddata[128];
        memset(cmddata, 0x45, sizeof cmddata);
        ipc_SendData(0x12, cmddata, sizeof cmddata);
        printf("ipc_SendData 1\n");
      }
      break;
      case '\'': {
        printf("int ipc_SlaveTick();\n");
        ipc_SlaveTick();
        ipc_MasterTick();
        break;
      }
      case ';': {
//         uint8_t len = ipc_Command(IPC_READBACKSIZE, NULL, 0);
        uint8_t len = ipc_ReadBackLen();
        uint8_t readbackdata[256];

        printf("ipc_Command returns %d\n", len);
        if (len) {
          ipc_ReadBack(readbackdata, len);
          hexdump(readbackdata, len);
        }
        break;
      }
      case ':': {
        fifo_t *f = ipc_GetFifo();
        fifo_Put(f, rddata++);
        printf("put data len %d\n", fifo_Count(f));
        break;
      }
      case '@': {
        fifo_t *f = ipc_GetFifo();
        for (i=0; i<1024; i++) {
          fifo_Put(f, rddata++);
        }
        printf("put data len %d\n", fifo_Count(f));
        break;
      }
#endif
      case '#':
        ps2_Debug();
        printf("\n");
        ipc_Debug();
        printf("\n");
        break;

      // FLASH
#ifdef TEST_FLASH
      case 'P': {
        uint8_t data[4096];
        memset(data, 0xaa, sizeof data);
        hexdump((uint8_t *)0x101FF000, 4096);
        printf("flash_EraseProgram returns %d\n",
               flash_EraseProgram(0x101FF000, data, sizeof data));
        hexdump((uint8_t *)0x101FF000, 4096);
        break;
      }

      // FLASH IPC
#ifdef TEST_IPC
      case 'B':
      case 'I': {
        uint8_t data[4096];
        uint8_t cmd[128+1];
        memset(data, 0x55, sizeof data);
        hexdump((uint8_t *)0x101FF000, 4096);
        for (int i=0; i<32; i++) {
          cmd[0] = i;
          memcpy(&cmd[1], &data[i*128], 128);
          printf("ipc_Command returns %d\n", ipc_Command(IPC_FLASHDATA, cmd, 129));
        }

        uint16_t crc = crc16(data, sizeof data);
        if (c == 'B') crc = ~crc;
        cmd[0] = 0x10;
        cmd[1] = 0x1f;
        cmd[2] = 0xf0;
        cmd[3] = 0x00;
        cmd[4] = crc>>8;
        cmd[5] = crc&0xff;
        printf("ipc_Command returns %d\n", ipc_Command(IPC_FLASHCOMMIT, cmd, 6));

        hexdump((uint8_t *)0x101FF000, 4096);
        break;
      }
#endif
#endif

#ifdef TEST_JAMMA
      case 'j': jamma_InitEx(0); printf("joypad 0 %X 1 %X\n", jamma_GetData(0), jamma_GetData(1)); break;
      case 'J': jamma_InitEx(1); printf("joypad 0 %X 1 %X\n", jamma_GetData(0), jamma_GetData(1)); break;
      case 'y': printf("jamma %08X\n", jamma_GetDataAll()); break;
      case 'x': jamma_Kill(); printf("kill jamma\n"); break;
#endif

      // MATRIX KEYBOARD
#ifdef TEST_MATRIX
      case 'K':
        kbd_Init();
        break;
      case 'L':
        kbd_Process();
        break;
#endif
#ifdef TEST_USERIO
      case 'u':
        test_UserIOInit();
        break;
      case 'U':
        test_UserIOKill();
        break;
      case 'z':
        test_UserIOSPI(0xaa);
        break;
      case 'x':
        test_UserIOSPI(0x55);
        break;
#endif
    }
    
  }
  
  reset_usb_boot(0, 0);
	return 0;
}
