#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "errors.h"
#include "usb/usb.h"


//TODO MJ non USB stuff here
void MCUReset() {}

void PollADC() {
}

void InitADC() {
}

static uint8_t mac[] = {1,2,3,4,5,6};
uint8_t *get_mac() {
  return mac;
}

//TODO MJ USB storage
void storage_control_poll() {
}

//TODO MJ No WiFi present at the moment - would need routing through fpga
bool eth_present = 0;

// #if defined(USBFAKE) || !defined(USB)
//TODO MJ PL2303 is a non CDC serial port over USB - maybe can use?
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
// #endif

#ifndef USB
// return number of joysticks
uint8_t joystick_count() { return 0; }

void hid_set_kbd_led(unsigned char led, bool on) {
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

void hid_joystick_button_remap_init() {}

char hid_joystick_button_remap(char *s, char action, int tag) {
  return 0;
}
#endif

// TODO MJ - USB stuff
void usb_init() {}

void usb_poll() {
}

void usb_hw_init() {}

// TODO MJ - firmware updating
void WriteFirmware(char *name) {
  printf("WriteFirmware: name:%s\n", name);
}

const static char firmwareVersion[] = "v999.999abcd";

char *GetFirmwareVersion(char *name) {
  return firmwareVersion;
}

unsigned char CheckFirmware(char *name) {
  printf("CheckFirmware: name:%s\n", name);
  // returns 1 if all ok else 0, setting Error to one of ollowing states
  // ERROR_NONE
  // ERROR_FILE_NOT_FOUND
  // ERROR_INVALID_DATA
  return 1;
}


//// keyboard handling....
//TODO MJ Add detection of this in the PS2 keyboard
// PAUSE -> 6F
// ps2rx E0
// ps2rx 12
// ps2rx E0
// ps2rx 7C
// ps2rx E0
// ps2rx F0
// ps2rx 7C
// ps2rx E0
// ps2rx F0
// ps2rx 12

