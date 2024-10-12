#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "errors.h"
#include "usb/usb.h"
#include "version.h"
#include "drivers/fifo.h"
#include "drivers/pins.h"
#include "drivers/ipc.h"
#include "drivers/crc16.h"
#include "FatFs/ff.h"

//#define DEBUG
#include "drivers/debug.h"

#include "pico/bootrom.h"
#include "pico/time.h"

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

void usb_hw_init() {}

#ifdef MB2
static uint8_t update_magic[] = {0x3e, 0x85, 0x1f, 0xe3};

int InvokeBootstrap() {
  int timeout = 80;
  int result;
  while (timeout && (result = ipc_Command(IPC_BOOTSTRAP, NULL, 0)) != 0xfd) {
    sleep_ms(100);
    printf("result = %d\n", result);
    timeout --;
  }
  printf("_result = %d\n", result);
  return (result == 0xfd) ? 0 : 1;
}

void ReturnToNormal() {
  uint8_t reboot = 0x55;
  ipc_Command(IPC_REBOOT, &reboot, 1);
}

#define FLASH_PAGE    4096
int WriteToUSBFlash(uint32_t addr, uint8_t *data) {
  uint8_t cmd[128+1];
  int result;

  for (int i=0; i<32; i++) {
    cmd[0] = i;
    memcpy(&cmd[1], &data[i*128], 128);
    printf("ipc_Command returns %d\n", ipc_Command(IPC_FLASHDATA, cmd, 129));
  }

  uint16_t crc = crc16(data, FLASH_PAGE);
  cmd[0] = (addr >> 24) & 0xff;
  cmd[1] = (addr >> 16) & 0xff;
  cmd[2] = (addr >> 8) & 0xff;
  cmd[3] = addr & 0xff;
  cmd[4] = crc>>8;
  cmd[5] = crc&0xff;

  result = ipc_Command(IPC_FLASHCOMMIT, cmd, 6);

  debug(("ipc_Command returns %d\n", result));
  return result;
}

int UpdateFirmwareUSB() {
  FIL file;
  uint32_t size;
  UINT br;
  uint8_t *data;
  uint32_t addr = FLASH_APPSIGNATURE;

  //ERROR_INVALID_DATA
  //ERROR_UPDATE_FAILED

  if (f_open(&file, "/RP2UAPP.BIN", FA_READ) == FR_OK) {
    size = f_size(&file);

    int result = InvokeBootstrap();
    debug(("InvokeBootstrap(); returns %d\n", result));
    if (result) {
      f_close(&file);
      return ERROR_UPDATE_FAILED;
    }
    
    data = (uint8_t *)malloc(FLASH_PAGE);
    while (size) {
      uint32_t this_read = size > FLASH_PAGE ? FLASH_PAGE : size;
      
      memset(data, 0xff, FLASH_PAGE);
      if (f_read(&file, data, FLASH_PAGE, &br) == FR_OK) {
        /* check first block */
        if (addr == FLASH_APPSIGNATURE) {
          if (memcmp(data, update_magic, sizeof update_magic)) {
            debug(("Error: magic is %02X%02X%02X%02X\n", data[0], data[1], data[2], data[3]));
            free(data);
            f_close(&file);
            ReturnToNormal();
            return ERROR_INVALID_DATA;
          }
        }
        WriteToUSBFlash(addr, data);
      }
      size -= this_read;
      addr += FLASH_PAGE;
    }

    free(data);
    f_close(&file);
    ReturnToNormal();

    return 0;
  } else {
    debug(("failed to open file\n"));
    return ERROR_FILE_NOT_FOUND;
  }
}

#else
int UpdateFirmwareUSB() {
  return ERROR_UPDATE_FAILED;
}
#endif

void WriteFirmware(char *name) {
  debug(("WriteFirmware: name:%s\n", name));
  reset_usb_boot(0, 0);
}

const char *GetUSBVersion() {
#ifdef MB2
  uint8_t major, minor;
  static char version[] = "v_.___";

  debug(("Get version\n"));

  major = ipc_Command(IPC_VERSIONMAJOR, NULL, 0);
  minor = ipc_Command(IPC_VERSIONMINOR, NULL, 0);

  version[1] = major + '0';
  sprintf(&version[3], "%d", minor);
  return (const char *)version;
#else
  return NULL;
#endif
}


#ifdef XILINX
#ifdef ZXUNO
#define ARCH "XZX1"
#elif defined(MB2)
#define ARCH "XMB2"
#else
#define ARCH "XMB1"
#endif
#else
#if defined(MB2)
#define ARCH "AN+2"
#else
#define ARCH "AN+"
#endif
#endif



const char firmwareVersion[] = "v" VERSION ARCH;

char *GetFirmwareVersion(char *name) {
  return (char *)firmwareVersion;
}

unsigned char CheckFirmware(char *name) {
  debug(("CheckFirmware: name:%s\n", name));
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

