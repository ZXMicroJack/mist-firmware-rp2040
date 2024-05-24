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
// #include "usbrtc.h"

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

//TODO MJ has time expired? - in ms
unsigned long CheckTimer(unsigned long time)
{
  return (time_us_64() / 1000) >= time;
}

//TODO wait for
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
int menu = 0; // TODO remove me
unsigned char Buttons() {
  return 0;
  // return menu ? 0x04 : 0;
}

unsigned char MenuButton() {
  return 0;
  // return menu ? 0x05 : 0;
}

unsigned char UserButton() {
  return 0;
}

// poll db9 joysticks
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
#else
const static uint8_t joylut[] = {0, JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_BTN1, JOY_BTN2, 0};
char GetDB9(char index, unsigned char *joy_map) {
  // *joy_map is set to a combination of the following bitmapped values
  // JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_BTN1, JOY_BTN2

  uint32_t d = jamma_GetData(index);
  uint8_t mask = 0x80;
  uint8_t ndx = 0;
  char j = 0;

  while (mask) {
    if (d & mask) j |= joylut[ndx];
    ndx++;
    mask >>= 1;
  }

  static uint8_t lastdb9[2];
  if (lastdb9[index] != d) {
    printf("GetDB9: d = %02X returns %02X\n", d, j);
    lastdb9[index] = d;
  }
  
  *joy_map = d == 0xff ? 0 : j;
  return 1;
}
#endif

#define DB9_UP          0x80
#define DB9_DOWN        0x40
#define DB9_LEFT        0x20
#define DB9_RIGHT       0x10
#define DB9_BTN1        0x08
#define DB9_BTN2        0x04
#define DB9_BTN3        0x02
#define DB9_BTN4        0x01

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

#ifdef ZXUNO
#else
  jamma_SetData(joy_num & 1, joydata);
#endif
}

// TODO MJ implement RTC
char GetRTC(unsigned char *d) {
  // implemented as d[0-7] -
  //   [y-100] [m] [d] [H] [M] [S] [Day1-7]
  d[0] = 23;
  d[1] = 12;
  d[2] = 12;
  d[3] = 23;
  d[4] = 58;
  d[5] = 34;
  d[6] = 4;
  return 0;
}

// TODO MJ implement RTC
char SetRTC(unsigned char *d) {
  return 0;
}
