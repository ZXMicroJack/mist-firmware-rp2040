#include <ctype.h>
#include <stdio.h>
#include "attrs.h"
#include "hardware.h"
#include "user_io.h"
#include "xmodem.h"
#include "ikbd.h"
#include "usb.h"

#include "drivers/jamma.h"
#include "drivers/ps2.h"
#include <pico/time.h>
#include "hardware/gpio.h"
#include "usb/joymapping.h"
#include "mist_cfg.h"
#include "user_io.h"

// #define DEBUG
#include "drivers/debug.h"
// #include "usbrtc.h"

#include "common.h"

uint32_t systimer;

//TODO MJ - can XMODEM be removed?  Serial to core is fine.
// A buffer of 256 bytes makes index handling pretty trivial
#if 0
volatile static unsigned char tx_buf[256];
volatile static unsigned char tx_rptr, tx_wptr;

volatile static unsigned char rx_buf[256];
volatile static unsigned char rx_rptr, rx_wptr;
#endif

//TODO MJ - re-integrate XMODEM and UART with core.
#if 0
void Usart0IrqHandler(void) {
      	// Read USART status
  unsigned char status = AT91C_BASE_US0->US_CSR;

  // received something?
  if(status & AT91C_US_RXRDY) {
    // read byte from usart
    unsigned char c = AT91C_BASE_US0->US_RHR;

    // only store byte if rx buffer is not full
    if((unsigned char)(rx_wptr + 1) != rx_rptr) {
      // there's space in buffer: use it
      rx_buf[rx_wptr++] = c;
    }
  }
    
  // ready to transmit further bytes?
  if(status & AT91C_US_TXRDY) {

    // further bytes to send in buffer? 
    if(tx_wptr != tx_rptr)
      // yes, simply send it and leave irq enabled
      AT91C_BASE_US0->US_THR = tx_buf[tx_rptr++];
    else
      // nothing else to send, disable interrupt
      AT91C_BASE_US0->US_IDR = AT91C_US_TXRDY;
  }
}
#endif

void USART_Poll(void) {}

// check usart rx buffer for data
#if 0
uint8_t xmodem_on = 0;
void USART_Poll(void) {
  int c;
  uint8_t ch;

  if (xmodem_on) {
    xmodem_poll();
  }

  while ((c = usb_cdc_getc()) >= 0) {
    // data available -> send via user_io to core
    ch = c;
    if (xmodem_on) {
      xmodem_rx_byte(ch);
    } else {
      user_io_serial_tx(&ch, 1);
    }
  }
}

void USART_Write(unsigned char c) {
  usb_cdc_putc(c);
}

void USART_Init(unsigned long baudrate) {
}
#endif

unsigned long CheckButton(void)
{
#ifdef BUTTON
    return((~*AT91C_PIOA_PDSR) & BUTTON);
#else
    return MenuButton();
#endif
}

void InitRTTC() {
}

int GetRTTC() {
  return time_us_64() / 1000;
}

//TODO MJ 1 ms precision timer to 4s
// 12 bits accuracy at 1ms = 4096 ms 
unsigned long GetTimer(unsigned long offset)
{
  return (time_us_64() / 1000) + offset;
}

unsigned long CheckTimer(unsigned long time)
{
  return (time_us_64() / 1000) >= time;
}

void WaitTimer(unsigned long time)
{
    time = GetTimer(time);
    while (!CheckTimer(time))
      tight_loop_contents();
}


//TODO GetSPICLK just for display in debug - not really important - can be removed when main integrated.
int GetSPICLK() {
  return 0;
}

// TODO MJ There are no switches or buttons on NeptUno
// user, menu, DIP1, DIP2
unsigned char Buttons() {
  return 0;
}

unsigned char MenuButton() {
  return 0;
}

unsigned char UserButton() {
  return 0;
}

#ifdef ZXUNO
#define GPIO_JRT        28
#define GPIO_JLT        15
#define GPIO_JDN        14
#define GPIO_JUP        12
#define GPIO_JF1        11

void InitDB9() {
  uint8_t lut[] = { GPIO_JRT, GPIO_JLT, GPIO_JDN, GPIO_JUP, GPIO_JF1 };
  for (int i=0; i<sizeof lut; i++) {
    gpio_init(lut[i]);
    gpio_set_dir(lut[i], GPIO_IN);
  }
}

#define JOY_ALL   (JOY_RIGHT|JOY_LEFT|JOY_UP|JOY_DOWN|JOY_BTN1)

char GetDB9(char index, unsigned char *joy_map) {
  char data = 0;

  if (!index) {
    data |= gpio_get(GPIO_JRT) ? 0 : JOY_RIGHT;
    data |= gpio_get(GPIO_JLT) ? 0 : JOY_LEFT;
    data |= gpio_get(GPIO_JDN) ? 0 : JOY_DOWN;
    data |= gpio_get(GPIO_JUP) ? 0 : JOY_UP;
    data |= gpio_get(GPIO_JF1) ? 0 : JOY_BTN1;
  }

  if ((data & JOY_ALL) == JOY_ALL) {
    /* pins pobably not reflected */
    return 0;
  }

  *joy_map = data;
  return 1;
}

static uint8_t db9_legacy_mode = 0;
static void SetGpio(uint8_t usbjoy, uint8_t mask, uint8_t gpio) {
  gpio_put(gpio, (usbjoy & mask) ? 0 : 1);
  gpio_set_dir(gpio, (usbjoy & mask) ? GPIO_OUT : GPIO_IN);
}

void DB9SetLegacy(uint8_t on) {
  DB9Update(0, 0);
  db9_legacy_mode = on;
}

void DB9Update(uint8_t joy_num, uint8_t usbjoy) {
  if (!db9_legacy_mode) return;
  SetGpio(usbjoy, JOY_UP, GPIO_JUP);
  SetGpio(usbjoy, JOY_DOWN, GPIO_JDN);
  SetGpio(usbjoy, JOY_LEFT, GPIO_JLT);
  SetGpio(usbjoy, JOY_RIGHT, GPIO_JRT);
  SetGpio(usbjoy, JOY_BTN1, GPIO_JF1);
}
#else
void InitDB9() {}
/*
JAMMA
  P1
  TST COI STA UP_ DWN LFT RTG BT1 BT2 BT3 BT4

  P2
  SVC COI STA UP_ DWN LFT RTG BT1 BT2 BT3 BT4
*/

#ifdef JAMMA_JAMMA
const static uint8_t jammadb9lut[2][24] = {
  {
    DB9_BTN4, DB9_BTN3, DB9_BTN2, DB9_BTN1, DB9_RIGHT, DB9_LEFT, DB9_DOWN, DB9_UP,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }, {
    0, 0, 0, 0, 0, 0, 0, 0,
    DB9_RIGHT, DB9_LEFT, DB9_DOWN, DB9_UP, 0, 0, 0, 0,
    0, 0, 0, 0, DB9_BTN4, DB9_BTN3, DB9_BTN2, DB9_BTN1
  }
};
#endif

#ifdef JAMMA_JAMMA
static uint8_t jammadb9[2] = {0x00, 0x00};
static uint8_t usbdb9[2] = {0x00, 0x00};

void JammaToDB9() {
#if 1
  uint8_t ndx = 0;
  uint8_t j = 0;
  uint8_t depth = jamma_GetDepth();

  for (uint8_t index=0; index<2; index++) {
    uint32_t d = jamma_GetJamma();
    for (ndx = 0; ndx < depth; ndx++) {
      j |= (d & 1) ? jammadb9lut[index][ndx] : 0;
      d >>= 1;
    }
    if (jammadb9[index] != j) {
      jammadb9[index] = j;
      jamma_SetData(index, j | usbdb9[index]);
      debug(("JammaToDB9: %02X (usb %02X)\n", j, usbdb9[index]));
      debug(("jamma %X: depth: %d\n", jamma_GetJamma(), depth));
    }
  }
#endif
}

void KeypressJamma(uint16_t prev, uint16_t curr, uint16_t mask, uint8_t keyscan) {
  if (!user_io_osd_is_visible() && ((prev ^ curr) & mask)) {
    if (curr & mask) {
      ps2_FakeKey(0, keyscan);
    } else {
      ps2_FakeKey(0, 0xf0);
      ps2_FakeKey(0, keyscan);
    }
  }
}
#endif

char GetDB9(char index, unsigned char *joy_map) {
  uint32_t d = jamma_GetData(index);
  static uint16_t lastdb9[2];
  if (lastdb9[index] != d) {
    uint16_t joy_map2 = virtual_joystick_mapping(0x00db, index, d);
    uint8_t idx = (index ^ mist_cfg.joystick_db9_swap) & 1;
    idx = mist_cfg.joystick_db9_fixed_index ? user_io_joystick_renumber(idx) : joystick_count() + idx;

    if (!user_io_osd_is_visible()) user_io_joystick(idx, joy_map2);
    StateJoySet(joy_map2, idx); // send to OSD
    StateJoySetExtra( joy_map2>>8, idx);
    virtual_joystick_keyboard_idx(idx, joy_map2);

    /* detect and handle coin button */
    /* P1COIN send also '5' + 'X'
       P2COIN send also '6' + 'Y' */
    /* P1START send also kb '1'
       P2START send also kb '2' */

#ifndef MB2
    if (jamma_GetMode() == MODE_JAMMA) {
      KeypressJamma(lastdb9[index], d, JOY_L, index ? 0x36 : 0x2e); //5/6
      KeypressJamma(lastdb9[index], d, JOY_L, index ? 0x35 : 0x22); //X/Y;
      KeypressJamma(lastdb9[index], d, JOY_BTN3, index ? 0x36 : 0x2e); //5/6
      KeypressJamma(lastdb9[index], d, JOY_BTN3, index ? 0x35 : 0x22); //X/Y
      KeypressJamma(lastdb9[index], d, JOY_BTN4, index ? 0x1e : 0x16); //1/2
    }
#endif
    lastdb9[index] = d;
  }
  return 0;
}

const static uint8_t inv_joylut[] = {DB9_BTN4, DB9_BTN3, DB9_BTN2, DB9_BTN1, DB9_UP, DB9_DOWN, DB9_LEFT, DB9_RIGHT};
void DB9Update(uint8_t joy_num, uint8_t usbjoy) {
  uint8_t mask = 0x80;
  uint8_t ndx = 0;
  uint8_t joydata = 0;

  while (mask) {
    if (usbjoy & mask) joydata |= inv_joylut[ndx];
    ndx++;
    mask >>= 1;
  }

#ifdef JAMMA_JAMMA
  usbdb9[joy_num & 1] = joydata;
  jamma_SetData(joy_num & 1, joydata | jammadb9[joy_num&1]);
#else
  jamma_SetData(joy_num & 1, joydata);
#endif
}

void DB9SetLegacy(uint8_t on) {
}
#endif
