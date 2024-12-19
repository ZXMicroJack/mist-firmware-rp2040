#ifndef HARDWARE_H
#define HARDWARE_H

#include <inttypes.h>
#define lowest(a,b) ((a) < (b) ? (a) : (b))

// TODO MJ - MB has no LEDs for disks - maybe can route one through the FPGA cores?
#define DISKLED_ON    {}
#define DISKLED_OFF   {}

#define STORE_VARS_SIZE   0x20
#define STORE_VARS_POS    (0x20000000 + (256*1024) - STORE_VARS_SIZE)

// #define STORE_VARS_POS      0x0020FF00
#define USB_LOAD_VAR         *(int*)(STORE_VARS_POS+4)
#define USB_LOAD_VALUE       12345678

#define DEBUG_MODE_VAR       *(int*)(STORE_VARS_POS+8)
#define DEBUG_MODE_VALUE     87654321
#define DEBUG_MODE           (DEBUG_MODE_VAR == DEBUG_MODE_VALUE)

#define VIDEO_KEEP_VALUE     0x87654321
#define VIDEO_KEEP_VAR       (*(int*)(STORE_VARS_POS+0x10))
#define VIDEO_ALTERED_VAR    (*(uint8_t*)(STORE_VARS_POS+0x14))
#define VIDEO_SD_DISABLE_VAR (*(uint8_t*)(STORE_VARS_POS+0x15))
#define VIDEO_YPBPR_VAR      (*(uint8_t*)(STORE_VARS_POS+0x16))

// TODO MJ should never be invoked - because we have no buttons
#define USB_BOOT_VALUE       0x8007F007
// #define USB_BOOT_VAR         (*(int*)0x0020FF18)
#define USB_BOOT_VAR         (*(int*)(STORE_VARS_POS+0x18))


// TODO MJ - increase back to 4096 when USB done.
#ifdef TEST_BUILD
#define SECTOR_BUFFER_SIZE   1024
#else
#define SECTOR_BUFFER_SIZE   4096
#endif

#ifdef BOOT_FLASH_ON_ERROR
void BootFromFlash();
#endif

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
int GetRTTC();
char GetRTC(unsigned char *d);
char SetRTC(unsigned char *d);
// void hid_set_kbd_led(unsigned char led, bool on);

// TODO MJ reset CPU
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
void DB9SetLegacy(uint8_t on);
void JammaToDB9();

// the MiST has the user inout on the arm controller
void EnableIO(void);
void DisableIO(void);

#define DEBUG_FUNC_IN()
#define DEBUG_FUNC_OUT()

int8_t pl2303_is_blocked(void);
uint8_t *get_mac();
int iprintf(const char *fmt, ...);
// void siprintf(char *str, const char *fmt, ...);
int8_t pl2303_present(void);
void pl2303_settings(uint32_t rate, uint8_t bits, uint8_t parity, uint8_t stop);
void pl2303_tx(uint8_t *data, uint8_t len);
void pl2303_tx_byte(uint8_t byte);
uint8_t pl2303_rx_available(void);
uint8_t pl2303_rx(void);
int8_t pl2303_is_blocked(void);
uint8_t get_pl2303s(void);
void usb_dev_reconnect(void);
void FatalError(unsigned long error);
void SPIN();

#endif // HARDWARE_H
