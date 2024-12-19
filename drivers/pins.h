#ifndef _PINS_H
#define _PINS_H

/* for development with zxuno on picosynth board
 * clkps2 -> p57 (normally dabd) -> gp3
 * dataps2 -> p46 (normally wsbd) -> gp4
*/

/* what gpios to what fpga pins...
 * seems to be;
0   fclk - l12 - sj2 - cclk - altera fpga_dclk
1   fmiso - r22 - sj1 - altera fpga_data0
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
22  w4 - MIST_SS4 (AE9 on neptuno2)
--23  y6 - signal high when error state to boot /BOOT.BIT
--23  y6 - debug output
23  y6 - xdata-jamma
24  y9 - MIST_SS4 - altera FPGA_CONF_DONE (neptuno1++)
25  r19_pal  - altera FPGA_MSEL1 (neptuno1++)     FPGA_CONF_DONE (neptuno2)
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


#define GPIO_MIST_CSN    17 // user io
#define GPIO_MIST_SS2    20 // data io
#define GPIO_MIST_SS3    21 // osd
#define GPIO_MIST_SS4    22 // dmode?

#define GPIO_MIST_MISO   16
#define GPIO_MIST_MOSI   18
#define GPIO_MIST_SCK    19



#ifdef USBDEV
#define GPIO_PS2_CLK      3
#define GPIO_PS2_DATA     4
#define GPIO_PS2_CLK2     1
#define GPIO_PS2_DATA2    0
#else

#ifdef ZXUNO
#define GPIO_PS2_CLK2     26
#define GPIO_PS2_DATA2    27
#define GPIO_PS2_CLK      23
#define GPIO_PS2_DATA     24
#else
#define GPIO_PS2_CLK      11
#define GPIO_PS2_DATA     12
#define GPIO_PS2_CLK2     14
#define GPIO_PS2_DATA2    15
#endif
#endif

#define GPIO_RP2U_PS2_CLK   GPIO_RP2M_COM4
#define GPIO_RP2U_PS2_DATA  GPIO_RP2M_COM5
#define GPIO_RP2U_PS2_CLK2  GPIO_RP2M_COM6
#define GPIO_RP2U_PS2_DATA2 GPIO_RP2M_COM7


// Xilinx pins
#define GPIO_FPGA_INITB    13
#define GPIO_FPGA_M1M2    25
#define GPIO_FPGA_RESET   10
#define GPIO_FPGA_CLOCK 0
#define GPIO_FPGA_DATA 1

// Altera pins
#define GPIO_FPGA_DCLK    0
#define GPIO_FPGA_DATA0   1
#define GPIO_FPGA_MSEL1   25

// ZXUno+
#define GPIO_JTAG_TCK     0 //p32
#define GPIO_JTAG_TDO     25 //p67
#define GPIO_JTAG_TDI     1 //p30
#define GPIO_JTAG_TMS     13 //p29

// #define GPIO_JTAG_TCK     13
// #define GPIO_JTAG_TDO     1
// #define GPIO_JTAG_TDI     25
// #define GPIO_JTAG_TMS     0


#define MSEL1_AS          1
#define MSEL1_PS          0

#ifdef ALTERA_FPGA_DEV
//TODO remove - its just for dev
#define GPIO_FPGA_NCONFIG 8
#define GPIO_FPGA_CONF_DONE 7
#define GPIO_FPGA_NSTATUS 6
#else
#define GPIO_FPGA_NCONFIG 10
#ifdef QMTECH
#define GPIO_FPGA_CONF_DONE 25
#else
#define GPIO_FPGA_CONF_DONE 24
#endif
#define GPIO_FPGA_NSTATUS 13
#endif

#ifdef ALTERA_FPGA
#define GPIO_RESET_FPGA GPIO_FPGA_NCONFIG
#else
#define GPIO_RESET_FPGA GPIO_FPGA_RESET
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

#define MIST_CSN    17 // user io
#define MIST_SS2    20 // data io
#define MIST_SS3    21 // osd
// #define MIST_SS4    24 // dmode?
#define MIST_SS4    22 // dmode?

// #define GPIO_IPCM_I2C_CLK   15
// #define GPIO_IPCM_I2C_DAT   14
// #define GPIO_IPCS_I2C_CLK   23
// #define GPIO_IPCS_I2C_DAT   22
#define GPIO_IPCM_I2C_CLK   27
#define GPIO_IPCM_I2C_DAT   26
#define GPIO_IPCS_I2C_CLK   25
#define GPIO_IPCS_I2C_DAT   24
#endif

#define GPIO_DEBUG_TX_PIN   23

// RP2USB

// Keyboard matrix - row0-8, column 0-5
#define GPIO_KROW(n) n
#define GPIO_KCOL(n) (16+n)

#define GPIO_RP2U_BCDATAREADY   GPIO_RP2U_COM2
#define GPIO_RP2M_BCDATAREADY   GPIO_RP2M_COM2


#define GPIO_RP2U_COM0    24 //I2C1 SDA
#define GPIO_RP2U_COM1    25 //I2C1 SCL
#define GPIO_RP2U_COM2    29
#define GPIO_RP2U_COM3    9
#define GPIO_RP2U_COM4    23 // PS2 CLK1
#define GPIO_RP2U_COM5    22 // PS2 DATA1
#define GPIO_RP2U_COM6    10 // PS2 CLK2
#define GPIO_RP2U_COM7    13 // PS2 DATA2

#define GPIO_RP2M_COM0    26 // I2C1 SCL
#define GPIO_RP2M_COM1    27 // I2C1 SDA
#define GPIO_RP2M_COM2    28
#define GPIO_RP2M_COM3    29
#define GPIO_RP2M_COM4    11
#define GPIO_RP2M_COM5    12
#define GPIO_RP2M_COM6    14
#define GPIO_RP2M_COM7    15

#define GPIO_RP2M_UART0_TX    16 // V8
#define GPIO_RP2M_UART0_RX    17 // W9

#define GPIO_RP2U_XLOAD       26 // AA4
#define GPIO_RP2U_XSCK        27 // AB5
#define GPIO_RP2U_XDATA       28 // AA6
#define GPIO_RP2U_XDATAJAMMA  23 // Y6

// Note: This set incorrect after query of 18/01/2024
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


// Note: New set.
/*                                 USB          MIDI
  (SPI1 RX) (UART1 TX) (I2C0 SDA) GP24 - COM0 - GP26 (SPI1 SCK) (UART1 CTS) (I2C1 SDA)
 (SPI1 CSN) (UART1 RX) (I2C0 SCL) GP25 - COM1 - GP27 (SPI1 TX) (UART1 RTS) (I2C1 SCL)
 (SPI1 CSN) (UART0 RX) (I2C0 SCL) GP29 - COM2 - GP28 (SPI1 RX) (UART0 TX) (I2C0 SDA)
  (UART1 RX) (I2C0 SCL) (SPI1 CSN) GP9 - COM3 - GP29 (SPI1 CSn) (UART0 RX) (I2C0 SCL)
 (SPI0 TX) (UART1 RTS) (I2C1 SCL) GP23 - COM4 - GP11 (I2C1 SCL) (SPI1 TX) (UART1 RTS)
(SPI0 SCK) (UART1 CTS) (I2C1 SDA) GP22 - COM5 - GP12 (I2C0 SDA) (SPI1 RX) (UART0 TX)
(UART1 CTS) (I2C1 SDA) (SPI1 SCK) GP10 - COM6 - GP14 (SPI1 SCK) (I2C1 SDA) 
 (SPI1 CSN) (I2C0 SCL) (UART0 RX) GP13 - COM7 - GP15 (SPI1 TX)  (I2C1 SCL)
*/


#define GPIO_JRT        28
#define GPIO_JLT        15
#define GPIO_JDN        14
#define GPIO_JUP        12
#define GPIO_JF1        11


#define RP2X_MIDIAPP    0x1CE07AC1
#define RP2X_USBAPP     0xE31F853E
#define RP2X_MIDILUTS   0xed27055a
#define RP2X_MIDILUTS2  0xc58d1f47
#define RP2X_MIDISF1    0x7d2413d1
#define RP2X_MIDISF2    0xd582ad64
#define RP2M_BOOTSTRAP  0x4835b955
#define RP2U_BOOTSTRAP  0x9a584fc0


#define FLASH_BOOTSTRAP       0x10000000
#define FLASH_APPSIGNATURE    0x1000F000
#define FLASH_APPLICATION     0x10010000
#define MAX_APP_SIZE          0x200000 // 2mb
#define RP2M_LUTS_POS         0x10040000
#define RP2M_LUTS2_POS        0x10060000
#define RP2M_SOUNDFONT_POS    0x100a0000
#define RP2M_SOUNDFONT2_POS   0x10200000

#define FPGA_IMAGE_POS        0x100A0000


/* SM / PIO allocation */
#define FPGA_PIO            pio1
#define FPGA_SM             0
#define FPGA_OFFSET         AUDIO_I2S_INSTR
#define FPGA_INSTR          5

#define JAMMA_PIO           pio1
#define JAMMA_SM            2
#define JAMMA_OFFSET        AUDIO_I2S_INSTR
#define JAMMA_INSTR         12
#define JAMMAU_INSTR        5
#define JAMMA_PIO_IRQ       PIO1_IRQ_0
#define JAMMA2_SM           3

// NOTE - PIO number not changed on PIO not changed from this, but is here for reference sake
#define AUDIO_I2S_PIO       pio1
#define AUDIO_I2S_SM        1
#define AUDIO_I2S_OFFSET    0
#define AUDIO_I2S_INSTR     20

// debug PIO is only on dual for now
// #define DEBUG_PIO       pio0
#define DEBUG_PIO       pio1
#define DEBUG_SM        2
// #define DEBUG_OFFSET    0
#define DEBUG_OFFSET AUDIO_I2S_INSTR
#define DEBUG_INSTR     4

#define PS2HOST_PIO pio0
#define PS2HOST_OFFSET  0
#define PS2HOST_SM 2
#define PS2HOST2_SM 3
#define PS2HOST_INSTR 30

#define SDCARD_PIO pio0
#define SDCARD_SM 1
#define SDCARD_OFFSET  30




/*

Planned PIOs
------------
pio1      - 32
====        ==
ps2       - 30 | *1
ps2tx     - 27 | *1
spi       - 2

pio0      - 41
====        ==
audio_i2s - 20
jammadb9  - 12 | *2
jamma     - 6  | *2
fpga      - 5
debug     - 4

current PIOs
------------
pio1      - 32
ps2       - 30 | *1
ps2tx     - 27 | *1
debug     - 4

pio0      - 39
spi       - 2
audio_i2s - 20
jammadb9  - 12 | *2
jamma     - 6  | *2
fpga      - 5
*/

#endif
