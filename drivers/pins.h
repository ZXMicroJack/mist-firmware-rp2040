#ifndef _PINS_H
#define _PINS_H

/* for development with zxuno on picosynth board
 * clkps2 -> p57 (normally dabd) -> gp3
 * dataps2 -> p46 (normally wsbd) -> gp4
*/

/* what gpios to what fpga pins...
 * seems to be;
0   fclk - l12 - sj2 - cclk
1   fmiso - r22 - sj1 - 
2   aa8_clkdb \
3   y8_wsdb   |- midi
4   ab8_dabd  |
5   ab7_out   /
6   m5_miso \
7   l4_mosi |-- sdcard
8   m6_sck  |
9   l5_cs   /
10  reset
11  u1_kclk \  keyboard
12  t1_kdat /
13  init_b  - sj3
14  p4_mclk \  mouse
15  n5_mdat /
16  v8_miso \   uart0 tx, SPI0RX
17  w9_mosi |-- sdcard high level / uart0 rx, SPI0CSN
18  w7_sck  |   SPI0SCK
19  v7_cs   /   SPI0TX
20  w5 - MIST_SS2
21  w6 - MIST_SS3
22  w4 - when core is running, it detects as low, and when in reset detects as high
23  y6 - signal high when error state to boot /BOOT.BIT
24  y9 - MIST_SS4
25  r19_pal
26  aa4_xload
27  ab5_xsck
28  aa6_xdata
29  nc
*/

#define PICO_SD_CLK_PIN 8
#define PICO_SD_CMD_PIN 7
#define PICO_SD_DAT0_PIN 6
#define DPICO_SD_CS_PIN 9

#undef PICO_DEFAULT_SPI_SCK_PIN
#undef PICO_DEFAULT_SPI_TX_PIN
#undef PICO_DEFAULT_SPI_RX_PIN
#undef PICO_DEFAULT_SPI_CSN_PIN

#define PICO_DEFAULT_SPI_SCK_PIN 8
#define PICO_DEFAULT_SPI_TX_PIN 7
#define PICO_DEFAULT_SPI_RX_PIN 6
#define PICO_DEFAULT_SPI_CSN_PIN 9

// routed to W4 on the fpga, when core is running, it pulls the pin up,
// when reset, it pulls it down.  Rudamentry core detection.
#define PICO_FPGA_MONITOR_PIN 22
#define PICO_FPGA_BOOT_SD     23

#ifdef USBDEV
#define GPIO_PS2_CLK      3
#define GPIO_PS2_DATA     4
#define GPIO_PS2_CLK2     1
#define GPIO_PS2_DATA2    0
#else
#define GPIO_PS2_CLK      11
#define GPIO_PS2_DATA     12
#define GPIO_PS2_CLK2     14
#define GPIO_PS2_DATA2    15
#endif

// Xilinx pins
#define GPIO_FPGA_INITB    13
#define GPIO_FPGA_M1M2    25
#define GPIO_FPGA_RESET   10
#define GPIO_FPGA_CLOCK 0
#define GPIO_FPGA_DATA 1

// Altera pins
#define GPIO_FPGA_DCLK    0
#define GPIO_FPGA_DATA0   1
#if 0
#define GPIO_FPGA_NCONFIG 10
#define GPIO_FPGA_CONF_DONE 25
#define GPIO_FPGA_NSTATUS 13
#else
//TODO remove - its just for dev
#define GPIO_FPGA_NCONFIG 8
#define GPIO_FPGA_CONF_DONE 7
#define GPIO_FPGA_NSTATUS 6
#endif

#ifdef IPCDEV
#define GPIO_IPCM_I2C_CLK   4
#define GPIO_IPCM_I2C_DAT   5
#define GPIO_IPCS_I2C_CLK   2
#define GPIO_IPCS_I2C_DAT   3
#else
/*                                 USB          MIDI
  (SPI1 RX) (UART1 TX) (I2C0 SDA) GP24 - COM0 - GP26 (SPI1 SCK) (UART1 CTS) (I2C1 SDA)
 (SPI1 CSN) (UART1 RX) (I2C0 SCL) GP25 - COM1 - GP27 (SPI1 TX) (UART1 RTS) (I2C1 SCL)
*/

// #define GPIO_IPCM_I2C_CLK   15
// #define GPIO_IPCM_I2C_DAT   14
// #define GPIO_IPCS_I2C_CLK   23
// #define GPIO_IPCS_I2C_DAT   22
#define GPIO_IPCM_I2C_CLK   27
#define GPIO_IPCM_I2C_DAT   26
#define GPIO_IPCS_I2C_CLK   25
#define GPIO_IPCS_I2C_DAT   24
#endif

// RP2USB

// Keyboard matrix - row0-8, column 0-5
#define GPIO_KROW(n) n
#define GPIO_KCOL(n) (16+n)

#define GPIO_RP2U_COM0    24
#define GPIO_RP2U_COM1    25
#define GPIO_RP2U_COM2    29
#define GPIO_RP2U_COM3    9
#define GPIO_RP2U_COM4    10
#define GPIO_RP2U_COM5    13
#define GPIO_RP2U_COM6    22 //I2C1 SDA
#define GPIO_RP2U_COM7    23 //I2C1 SCL

#define GPIO_RP2M_COM0    26
#define GPIO_RP2M_COM1    27
#define GPIO_RP2M_COM2    28
#define GPIO_RP2M_COM3    29
#define GPIO_RP2M_COM4    11
#define GPIO_RP2M_COM5    12
#define GPIO_RP2M_COM6    14 // I2C1 SDA
#define GPIO_RP2M_COM7    15 // I2C1 SCL

#define GPIO_RP2M_UART0_TX    16 // V8
#define GPIO_RP2M_UART0_RX    17 // W9

#define GPIO_RP2U_XLOAD       26 // AA4
#define GPIO_RP2U_XSCK        27 // AB5
#define GPIO_RP2U_XDATA       28 // AA6

/*                                 USB          MIDI
  (SPI1 RX) (UART1 TX) (I2C0 SDA) GP24 - COM0 - GP26 (SPI1 SCK) (UART1 CTS) (I2C1 SDA)
 (SPI1 CSN) (UART1 RX) (I2C0 SCL) GP25 - COM1 - GP27 (SPI1 TX) (UART1 RTS) (I2C1 SCL)
 (SPI1 CSN) (UART0 RX) (I2C0 SCL) GP29 - COM2 - GP28 (SPI1 RX) (UART0 TX) (I2C0 SDA)
  (UART1 RX) (I2C0 SCL) (SPI1 CSN) GP9 - COM3 - GP29 (SPI1 CSn) (UART0 RX) (I2C0 SCL)
(UART1 CTS) (I2C1 SDA) (SPI1 SCK) GP10 - COM4 - GP11 (I2C1 SCL) (SPI1 TX) (UART1 RTS)
 (SPI1 CSN) (I2C0 SCL) (UART0 RX) GP13 - COM5 - GP12 (I2C0 SDA) (SPI1 RX) (UART0 TX)
(SPI0 SCK) (UART1 CTS) (I2C1 SDA) GP22 - COM6 - GP14 (SPI1 SCK) (I2C1 SDA) 
 (SPI0 TX) (UART1 RTS) (I2C1 SCL) GP23 - COM7 - GP15 (SPI1 TX)  (I2C1 SCL)
*/

#define RP2X_MIDIAPP    0x1CE07AC1
#define RP2X_USBAPP     0xE31F853E
#define RP2X_MIDILUTS   0xed27055a
#define RP2X_MIDISF1    0x7d2413d1
#define RP2X_MIDISF2    0xd582ad64
#define RP2M_BOOTSTRAP  0x4835b955
#define RP2U_BOOTSTRAP  0x9a584fc0


#define FLASH_BOOTSTRAP       0x10000000
#define FLASH_APPSIGNATURE    0x1000F000
#define FLASH_APPLICATION     0x10010000
#define MAX_APP_SIZE          0x200000 // 2mb
#define RP2M_LUTS_POS         0x10040000
#define RP2M_SOUNDFONT_POS    0x100a0000
#define RP2M_SOUNDFONT2_POS   0x10200000

#endif
