#include <ctype.h>
#include <stdio.h>
#include "attrs.h"
#include "hardware.h"
#include "user_io.h"
#include "xmodem.h"
#include "ikbd.h"
#include "usb.h"

#include "drivers/jamma.h"
#include <pico/time.h>
#include "hardware/gpio.h"
#include "usb/joymapping.h"
#include "mist_cfg.h"

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
void Usart0IrqHandler(void) {
#if 0
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
#endif
}

// check usart rx buffer for data
void USART_Poll(void) {
#if 0
  if(Buttons() & 2)
    xmodem_poll();

  while(rx_wptr != rx_rptr) {
    // this can a little be optimized by sending whole buffer parts 
    // at once and not just single bytes. But that's probably not
    // worth the effort.
    char chr = rx_buf[rx_rptr++];

    if(Buttons() & 2) {
      // if in debug mode use xmodem for file reception
      xmodem_rx_byte(chr);
    } else {
      iprintf("USART RX %d (%c)\n", chr, chr);

      // data available -> send via user_io to core
      user_io_serial_tx(&chr, 1);
    }
  }
#endif
}

void USART_Write(unsigned char c) {
#if 0
#if 0
  while(!(AT91C_BASE_US0->US_CSR & AT91C_US_TXRDY));
  AT91C_BASE_US0->US_THR = c;
#else
  if((AT91C_BASE_US0->US_CSR & AT91C_US_TXRDY) && (tx_wptr == tx_rptr)) {
    // transmitter ready and buffer empty? -> send directly
    AT91C_BASE_US0->US_THR = c;
  } else {
    // transmitter is not ready: block until space in buffer
    while((unsigned char)(tx_wptr + 1) == tx_rptr);

    // there's space in buffer: use it
    tx_buf[tx_wptr++] = c;
  }

  AT91C_BASE_US0->US_IER = AT91C_US_TXRDY;  // enable interrupt
#endif
#endif
}

void USART_Init(unsigned long baudrate) {
#if 0
    	// Configure PA5 and PA6 for USART0 use
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;

    // Enable the peripheral clock in the PMC
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_US0;

    // Reset and disable receiver & transmitter
    AT91C_BASE_US0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;

    // Configure USART0 mode
    AT91C_BASE_US0->US_MR = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | 
      AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT | AT91C_US_CHMODE_NORMAL;

    // Configure USART0 rate
    AT91C_BASE_US0->US_BRGR = MCLK / 16 / baudrate;

    // Enable receiver & transmitter
    AT91C_BASE_US0->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;

    // tx buffer is initially empty
    tx_rptr = tx_wptr = 0;

    // and so is rx buffer
    rx_rptr = rx_wptr = 0;

    // Set the USART0 IRQ handler address in AIC Source
    AT91C_BASE_AIC->AIC_SVR[AT91C_ID_US0] = (unsigned int)Usart0IrqHandler; 
    AT91C_BASE_AIC->AIC_IECR = (1<<AT91C_ID_US0);

    AT91C_BASE_US0->US_IER = AT91C_US_RXRDY;  // enable rx interrupt
#endif
}

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
const static uint8_t joylut[] = {JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_BTN1, JOY_BTN2, 0, 0};

#ifdef JAMMA_JAMMA
const static uint16_t jammalut[2][24] = {
  {
    JOY_Y, JOY_X, JOY_A, JOY_B, JOY_RIGHT, JOY_LEFT, JOY_DOWN, JOY_UP,
    0, 0, 0, 0, JOY_R, JOY_L, JOY_BTN3, JOY_BTN4, 
    0, 0, 0, 0, 0, 0, 0, 0
  }, {
    0, 0, 0, 0, 0, 0, 0, 0,
    JOY_RIGHT, JOY_LEFT, JOY_DOWN, JOY_UP, 0, 0, 0, 0,
    JOY_R, JOY_L, JOY_BTN3, JOY_BTN4, JOY_Y, JOY_X, JOY_A, JOY_B
  }
};

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
  uint8_t ndx = 0;
  uint8_t j = 0;
  uint8_t depth = jamma_GetDepth();

  for (uint8_t index=0; index<2; index++) {
    uint32_t d = ~jamma_GetJamma();
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
}
#endif

char GetDB9(char index, unsigned char *joy_map) {
  // *joy_map is set to a combination of the following bitmapped values
  // JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_BTN1, JOY_BTN2

  uint32_t d = jamma_GetData(index);
  uint32_t mask = 0x80;
  uint8_t ndx = 0;
  uint16_t j = 0;

  if (d != 0xff) { // joystick is properly set up
    while (mask) {
      if (d & mask) j |= joylut[ndx];
      ndx++;
      mask >>= 1;
    }
  }

#ifdef JAMMA_JAMMA
  d = ~jamma_GetJamma();
  uint8_t depth = jamma_GetDepth();
  for (ndx = 0; ndx < depth; ndx++) {
    j |= (d & 1) ? jammalut[index][ndx] : 0;
    d >>= 1;
  }
#endif
  
#ifndef JAMMA_JAMMA
#ifdef DEBUG
  static uint16_t lastdb9[2];
  if (lastdb9[index] != j) {
    debug(("GetDB9: index %d joy %04x\n", index, j));
    lastdb9[index] = j;
  }
#endif

  *joy_map = d == 0xff ? 0 : j;
  return 1;
#else
  uint16_t joy_map2;

  joy_map2 = virtual_joystick_mapping(0x00db, index, j);

  uint8_t idx = mist_cfg.joystick_db9_fixed_index ? user_io_joystick_renumber(index) : joystick_count() + index;
  if (!user_io_osd_is_visible()) user_io_joystick(idx, joy_map2);
  StateJoySet(joy_map2, idx); // send to OSD
  StateJoySetExtra( joy_map2>>8, idx);
  virtual_joystick_keyboard_idx(idx, joy_map2);
#ifdef DEBUG
  static uint16_t lastdb9[2];
  if (lastdb9[index] != j) {
    printf("GetDB9: index %d (->%d) joy %04x map %04x\n", index, idx, j, joy_map2);
    lastdb9[index] = j;
  }
#endif
  return 0;
#endif
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
