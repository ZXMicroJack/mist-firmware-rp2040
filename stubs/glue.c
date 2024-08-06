#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#include "usb/usb.h"


//TODO MJ where shall debug go?
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

//TODO MJ non USB stuff here
void MCUReset() {
	exit(1);
}

void PollADC() {
}

void InitADC() {
}

void InitRTTC() {}


#ifdef EMU
int GetRTTC() { 
   struct timeval tv;
   static uint32_t offset_time = 0;
   static uint32_t base_time = 0;
   static uint32_t last_time = 0;


   gettimeofday(&tv, NULL);

   if (base_time == 0) base_time = tv.tv_sec;
   tv.tv_sec -= base_time;

   uint32_t curr_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

   if (last_time > curr_time)
   {
      offset_time = last_time;
   }
   last_time = curr_time;
   return curr_time + offset_time;
}
#else
int GetRTTC() { return 0; }
#endif

void arch_irq_disable() {}

static uint8_t mac[] = {1,2,3,4,5,6,7,8};
uint8_t *get_mac() {
  return mac;
}

//TODO MJ USB storage
void storage_control_poll() {
}

// uint8_t mmc_inserted = 0;
//TODO MJ No WiFi present at the moment - would need routing through fpga
bool eth_present = 0;

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

// TODO MJ - USB stuff
void usb_init() {}

void SPIN() {}

// return number of joysticks
#ifndef EMU
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
#endif
void usb_hw_init() {}

#ifndef EMU
// uint32_t timer_get_msec() { return 0; }
void hid_joystick_button_remap_init() {}

// void hid_joystick_button_remap_init(void) {
// }
char hid_joystick_button_remap(char *s, char action, int tag) {
  return 0;
}
#endif

void dmb() {}

// TODO MJ - firmware updating
void WriteFirmware(char *name) {
}

const static char firmwareVersion[] = "v999.999 - TBD FAKE ";

char *GetFirmwareVersion(char *name) {
  return firmwareVersion;
}

unsigned char CheckFirmware(char *name) {
  // returns 1 if all ok else 0, setting Error to one of ollowing states
  // ERROR_NONE
  // ERROR_FILE_NOT_FOUND
  // ERROR_INVALID_DATA
  return 0;
}

unsigned char ConfigureFpga(const char*) {
  // returns 1 if success / 0 on fail
  return 1;
}

void DB9Update(int n, uint8_t d) {}

