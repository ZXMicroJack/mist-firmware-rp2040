#ifndef HARDWARE_H
#define HARDWARE_H

#include <inttypes.h>

#define MCLK 48000000
#define FWS 1 // Flash wait states
#define FLASH_PAGESIZE 256

// TODO MJ - MB has no LEDs for disks - maybe can route one through the FPGA cores?
static uint8_t diskled = 0;
#define DISKLED       1
#define DISKLED_ON    diskled = DISKLED;
#define DISKLED_OFF   diskled = DISKLED;

#define MMC_SEL       AT91C_PIO_PA31
#define USB_SEL       AT91C_PIO_PA11
#define USB_PUP       AT91C_PIO_PA16
#define SD_WP         AT91C_PIO_PA1
#define SD_CD         AT91C_PIO_PA0


// TODO MJ - not sure what all this is extra to the stuff below?
// fpga programming interface
static uint8_t AT91C_PIOA_OER, AT91C_PIOA_SODR, AT91C_PIOA_CODR, AT91C_PIOA_PDSR;
#define FPGA_OER      AT91C_PIOA_OER
#define FPGA_SODR     AT91C_PIOA_SODR
#define FPGA_CODR     AT91C_PIOA_CODR
#define FPGA_PDSR     AT91C_PIOA_PDSR
#define FPGA_DONE_PDSR  FPGA_PDSR
#define FPGA_DATA0_CODR FPGA_CODR
#define FPGA_DATA0_SODR FPGA_SODR


// MJ - TODO done pin is also not connected
#ifdef EMIST
// xilinx programming interface
#define XILINX_DONE   AT91C_PIO_PA4
#define XILINX_DIN    AT91C_PIO_PA9
#define XILINX_INIT_B AT91C_PIO_PA8
#define XILINX_PROG_B AT91C_PIO_PA7
#define XILINX_CCLK   AT91C_PIO_PA15
#else
static uint8_t altera_done, altera_data0, altera_nconfig, altera_nstatus, altera_dclk;
// altera programming interface
#define ALTERA_DONE    altera_done
#define ALTERA_DATA0   altera_data0
#define ALTERA_NCONFIG altera_nconfig
#define ALTERA_NSTATUS altera_nstatus
#define ALTERA_DCLK    altera_dclk

#define ALTERA_NCONFIG_SET   FPGA_SODR = ALTERA_NCONFIG
#define ALTERA_NCONFIG_RESET FPGA_CODR = ALTERA_NCONFIG
#define ALTERA_DCLK_SET      FPGA_SODR = ALTERA_DCLK
#define ALTERA_DCLK_RESET    FPGA_CODR = ALTERA_DCLK
#define ALTERA_DATA0_SET     FPGA_DATA0_SODR = ALTERA_DATA0;
#define ALTERA_DATA0_RESET   FPGA_DATA0_CODR = ALTERA_DATA0;

#define ALTERA_NSTATUS_STATE (FPGA_PDSR & ALTERA_NSTATUS)
#define ALTERA_DONE_STATE    (FPGA_DONE_PDSR & ALTERA_DONE)

#endif

// db9 joystick ports
// TODO MJ DB9 joysticks are not connected
#define JOY1_UP        AT91C_PIO_PA28
#define JOY1_DOWN      AT91C_PIO_PA27
#define JOY1_LEFT      AT91C_PIO_PA26
#define JOY1_RIGHT     AT91C_PIO_PA25
#define JOY1_BTN1      AT91C_PIO_PA24
#define JOY1_BTN2      AT91C_PIO_PA23
#define JOY1  (JOY1_UP|JOY1_DOWN|JOY1_LEFT|JOY1_RIGHT|JOY1_BTN1|JOY1_BTN2)

#define JOY0_UP        AT91C_PIO_PA22
#define JOY0_DOWN      AT91C_PIO_PA21
#define JOY0_LEFT      AT91C_PIO_PA20
#define JOY0_RIGHT     AT91C_PIO_PA19
#define JOY0_BTN1      AT91C_PIO_PA18
#define JOY0_BTN2      AT91C_PIO_PA17
#define JOY0  (JOY0_UP|JOY0_DOWN|JOY0_LEFT|JOY0_RIGHT|JOY0_BTN1|JOY0_BTN2)

// chip selects for FPGA communication
#define FPGA0 AT91C_PIO_PA10
#define FPGA1 AT91C_PIO_PA3
#define FPGA2 AT91C_PIO_PA2

#define FPGA3         AT91C_PIO_PA9   // same as ALTERA_DATA0

#define VBL           AT91C_PIO_PA7

#define USB_LOAD_VAR         *(int*)(0x0020FF04)
#define USB_LOAD_VALUE       12345678

#define DEBUG_MODE_VAR       *(int*)(0x0020FF08)
#define DEBUG_MODE_VALUE     87654321
#define DEBUG_MODE           (DEBUG_MODE_VAR == DEBUG_MODE_VALUE)

#define VIDEO_KEEP_VALUE     0x87654321
#define VIDEO_KEEP_VAR       (*(int*)0x0020FF10)
#define VIDEO_ALTERED_VAR    (*(uint8_t*)0x0020FF14)
#define VIDEO_SD_DISABLE_VAR (*(uint8_t*)0x0020FF15)
#define VIDEO_YPBPR_VAR      (*(uint8_t*)0x0020FF16)

#define USB_BOOT_VALUE       0x8007F007
#define USB_BOOT_VAR         (*(int*)0x0020FF18)

#define SECTOR_BUFFER_SIZE   4096

// TODO MJ - no sense switches for these two
char mmc_inserted(void);
char mmc_write_protected(void);

// TODO MJ easily can port - but where do these go to?
void USART_Init(unsigned long baudrate);
void USART_Write(unsigned char c);
unsigned char USART_Read(void);
void USART_Poll(void);

// TODO MJ no buttons
unsigned long CheckButton(void);

// TODO MJ timers can be ported.
void Timer_Init(void);
unsigned long GetTimer(unsigned long offset);
unsigned long CheckTimer(unsigned long t);
void WaitTimer(unsigned long time);

// TODO MJ real time clock is present on RP2040 - needs to be synced with RTC on middleboard - no direct connection
#if 0
void InitRTTC();
int inline GetRTTC() {/* return (int)(AT91C_BASE_RTTC->RTTC_RTVR);*/ return 0; }
#endif
char GetRTC(unsigned char *d);
char SetRTC(unsigned char *d);
// void hid_set_kbd_led(unsigned char led, bool on);

// TODO MJ reset CPU
// void inline MCUReset() {/* *AT91C_RSTC_RCR = 0xA5 << 24 | AT91C_RSTC_PERRST | AT91C_RSTC_PROCRST | AT91C_RSTC_EXTRST; */}
void MCUReset();

// TODO MJ spi - just for display
int GetSPICLK();

// TODO MJ no buttons or user buttons.
void InitADC(void);
void PollADC();
// user, menu, DIP2, DIP1
unsigned char Buttons();
unsigned char MenuButton();
unsigned char UserButton();

// TODO MJ no DB9s unless routed
void InitDB9();
char GetDB9(char index, unsigned char *joy_map);

// TODO MJ we have far better way of upgrading
void UnlockFlash();
void WriteFlash(int page);

#ifdef FPGA3
// the MiST has the user inout on the arm controller
void EnableIO(void);
void DisableIO(void);
#endif

#define DEBUG_FUNC_IN() 
#define DEBUG_FUNC_OUT() 

#ifdef __GNUC__
void __init_hardware(void);
#endif

int8_t pl2303_is_blocked(void);
uint8_t *get_mac();
int iprintf(const char *fmt, ...);
void siprintf(char *str, const char *fmt, ...);
int8_t pl2303_present(void);
void pl2303_settings(uint32_t rate, uint8_t bits, uint8_t parity, uint8_t stop);
void pl2303_tx(uint8_t *data, uint8_t len);
void pl2303_tx_byte(uint8_t byte);
uint8_t pl2303_rx_available(void);
uint8_t pl2303_rx(void);
int8_t pl2303_is_blocked(void);
uint8_t get_pl2303s(void);
void usb_dev_reconnect(void);
int GetRTTC();
void FatalError(unsigned long error);
void SPIN();

#endif // HARDWARE_H
