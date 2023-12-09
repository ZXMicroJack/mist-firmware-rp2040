#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb/usb.h"

static char str[256];
int iprintf(const char *fmt, ...) {
  int i;
  va_list argp;
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);

  printf("Output: %s\n", str);
  return 0;
}

void siprintf(char *str, const char *fmt, ...) {
  int i;
  va_list argp;
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);
}

// uint8_t mmc_inserted = 0;
bool eth_present = 0;


int8_t pl2303_present(void) {
  return 0;
}

void pl2303_settings(uint32_t rate, uint8_t bits, uint8_t parity, uint8_t stop) {}
void pl2303_tx(uint8_t *data, uint8_t len) {}
void pl2303_tx_byte(uint8_t byte) {}
uint8_t pl2303_rx_available(void) {
  return 0;
}
uint8_t pl2303_rx(void) {
  return 0;
}
int8_t pl2303_is_blocked(void) {
  return 0;
}
uint8_t get_pl2303s(void) {
  return 0;
}

void hid_set_kbd_led(unsigned char led, bool on) {
}

void SPIN() {}

uint8_t pl2303_init(struct usb_device_entry *, struct usb_device_descriptor *) { return 0; }
uint8_t pl2303_release(struct usb_device_entry *) { return 0; }
uint8_t pl2303_poll(struct usb_device_entry *) { return 0; }

const usb_device_class_config_t usb_pl2303_class = {
  pl2303_init, pl2303_release, pl2303_poll };

const usb_device_class_config_t usb_hub_class = {
  pl2303_init, pl2303_release, pl2303_poll };


const usb_device_class_config_t usb_hid_class = {
  pl2303_init, pl2303_release, pl2303_poll };
const usb_device_class_config_t usb_xbox_class = {
  pl2303_init, pl2303_release, pl2303_poll };
const usb_device_class_config_t usb_asix_class = {
  pl2303_init, pl2303_release, pl2303_poll };
const usb_device_class_config_t usb_usbrtc_class = {
  pl2303_init, pl2303_release, pl2303_poll };

/* Control transfer. Sets address, endpoint, fills control packet */
/* with necessary data, dispatches control packet, and initiates */
/* bulk IN transfer, depending on request. Actual requests are defined */
/* as inlines                   */
/* return codes:                */
/* 00       =   success         */
/* 01-0f    =   non-zero HRSLT  */

uint8_t usb_ctrl_req(usb_device_t *dev, uint8_t bmReqType,
                    uint8_t bRequest, uint8_t wValLo, uint8_t wValHi,
                    uint16_t wInd, uint16_t nbytes, uint8_t* dataptr) {
  return 0;
}

/* IN transfer to arbitrary endpoint. Assumes PERADDR is set. Handles multiple packets */
/* if necessary. Transfers 'nbytes' bytes. Keep sending INs and writes data to memory area */
/* pointed by 'data' */
/* rcode 0 if no errors. rcode 01-0f is relayed from dispatchPkt(). Rcode f0 means RCVDAVIRQ error, */
/* fe USB xfer timeout */
uint8_t usb_in_transfer( usb_device_t *dev, ep_t *ep, uint16_t *nbytesptr, uint8_t* data) {
  return 0;
}

void PollADC() {
}

void InitADC() {
}

void usb_poll() {
}

int8_t hid_keyboard_present(void) {
  return 0;
}

unsigned char get_keyboards(void) {
  return 0;
}

unsigned char get_mice() {
  return 0;
}

void MCUReset() {}

void usb_hw_init() {}

// uint32_t timer_get_msec() { return 0; }

int GetRTTC() { return 0; }

void hid_joystick_button_remap_init() {}

void arch_irq_disable() {}

static uint8_t mac[] = {1,2,3,4,5,6,7,8};
uint8_t *get_mac() {
  return mac;
}

// void hid_joystick_button_remap_init(void) {
// }
char hid_joystick_button_remap(char *s, char action, int tag) {
  return 0;
}

void dmb() {}
