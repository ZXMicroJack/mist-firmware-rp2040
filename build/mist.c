#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef DEVKIT_DEBUG
#include <stdarg.h>
#endif

#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/spi.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "hardware/pio.h"

#include "drivers/fpga.h"
#include "drivers/flash.h"
#include "drivers/pio_spi.h"
#include "drivers/sdcard.h"
#include "drivers/ps2.h"
#include "drivers/fifo.h"
#include "drivers/ipc.h"
#include "drivers/kbd.h"
#define DEBUG
#include "drivers/debug.h"

#include "mistmain.h"
// #include "usbdev.h"

#if defined(USB) && !defined (USBFAKE)
#include "bsp/board.h"
#include "tusb.h"
#endif


void FatalError(unsigned long error) {
  unsigned long i;

  printf("Fatal error: %lu\r", error);
  sleep_ms(2000);

//   while (1) {
//     for (i = 0; i < error; i++) {
//       DISKLED_ON;
//       WaitTimer(250);
//       DISKLED_OFF;
//       WaitTimer(250);
//     }
//     WaitTimer(1000);
//     MCUReset();
//   }
  reset_usb_boot(0, 0);
}

#if PICO_NO_FLASH
static void enable_xip(void) {
  rom_connect_internal_flash_fn connect_internal_flash = (rom_connect_internal_flash_fn)rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH);
    rom_flash_exit_xip_fn flash_exit_xip = (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
    rom_flash_flush_cache_fn flash_flush_cache = (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);
    rom_flash_enter_cmd_xip_fn flash_enter_cmd_xip = (rom_flash_enter_cmd_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_ENTER_CMD_XIP);

    connect_internal_flash();    // Configure pins
    flash_exit_xip();            // Init SSI, prepare flash for command mode
    flash_flush_cache();         // Flush & enable XIP cache
    flash_enter_cmd_xip();       // Configure SSI with read cmd
}
#endif


// KBD INSERT
// tuh_hid_mount_cb(dev_addr:1 inst:0)
// report: 0x05,0x01,0x09,0x06,0xA1,0x01,0x05,0x07,0x19,0xE0,0x29,0xE7,0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,0x75,0x08,0x81,0x01,0x95,0x05,,
//         vid 0A81 pid 0101
// tuh_hid_mount_cb(dev_addr:1 inst:1)
// report: 0x05,0x01,0x09,0x80,0xA1,0x01,0x85,0x02,0x09,0x81,0x09,0x82,0x09,0x83,0x25,0x01,0x15,0x00,0x75,0x01,0x95,0x03,0x81,0x06,0x75,0x05,0x95,0x01,0x81,0x01,,
//         vid 0A81 pid 0101
// tuh_hid_umount_cb(dev_addr:1 inst:0)
// tuh_hid_umount_cb(dev_addr:1 inst:1)
// tuh_hid_mount_cb(dev_addr:1 inst:0)
// report: 0x05,0x01,0x09,0x06,0xA1,0x01,0x05,0x07,0x19,0xE0,0x29,0xE7,0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,0x75,0x08,0x81,0x01,0x95,0x05,,
//         vid 0A81 pid 0101
// tuh_hid_mount_cb(dev_addr:1 inst:1)
// report: 0x05,0x01,0x09,0x80,0xA1,0x01,0x85,0x02,0x09,0x81,0x09,0x82,0x09,0x83,0x25,0x01,0x15,0x00,0x75,0x01,0x95,0x03,0x81,0x06,0x75,0x05,0x95,0x01,0x81,0x01,,
//         vid 0A81 pid 0101

// KBD ACTION1
// tuh_hid_report_received_cb(dev_addr:1 inst:0)
// report: 0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,
// tuh_hid_report_received_cb(dev_addr:1 inst:0)
// report: 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
// tuh_hid_report_received_cb(dev_addr:1 inst:0)
// report: 0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,
// tuh_hid_report_received_cb(dev_addr:1 inst:0)
// report: 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,



#ifdef USBFAKE

#define JOY3_DN   3
#define JOY2_DN   2
#define JOY1_DN   1
#define KBD_DN   0

#define FAKE_KBD
#define FAKE_JOY1
#define FAKE_JOY2
#define FAKE_JOY3

#ifdef FAKE_KBD
const static uint8_t kbd_rd[] = {0x05,0x01,0x09,0x06,0xA1,0x01,0x05,0x07,0x19,0xE0,0x29,0xE7,0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,0x75,0x08,0x81,0x01,0x95,0x05,0x75,0x01,0x05,0x08,0x19,0x01,0x29,0x05,0x91,0x02,0x95,0x03,0x75,0x01,0x91,0x01,0x95,0x06,0x75,0x08,0x15,0x00,0x26,0xFF,0x00,0x05,0x07,0x19,0x00,0x2A,0xFF,0x00,0x81,0x00,0xC0};
const static uint8_t dd0[18] = {0x12,0x01,0x10,0x01,0x00,0x00,0x00,0x08,0x81,0x0A,0x01,0x01,0x10,0x01,0x01,0x02,0x00,0x01};
const static uint8_t cfg0[] = {0x09,0x02,0x3B,0x00,0x02,0x01,0x00,0xA0,0x32,0x09,0x04,0x00,0x00,0x01,0x03,0x01,0x01,0x00,0x09,0x21,0x10,0x01,0x21,0x01,0x22,0x41,0x00,0x07,0x05,0x81,0x03,0x08,0x00,0x0A,0x09,0x04,0x01,0x00,0x01,0x03,0x00,0x00,0x00,0x09,0x21,0x10,0x01,0x21,0x01,0x22,0x66,0x00,0x07,0x05,0x82,0x03,0x08,0x00,0x0A};
#endif

#ifdef FAKE_JOY1
const static uint8_t joy1_rd[] = {0x05,0x01,0x09,0x04,0xA1,0x01,0xA1,0x02,0x75,0x08,0x95,0x05,0x15,0x00,0x26,0xFF,0x00,0x35,0x00,0x46,0xFF,0x00,0x09,0x30,0x09,0x31,0x09,0x32,0x09,0x32,0x09,0x35,0x81,0x02,0x75,0x04,0x95,0x01,0x25,0x07,0x46,0x3B,0x01,0x65,0x14,0x09,0x39,0x81,0x42,0x65,0x00,0x75,0x01,0x95,0x0C,0x25,0x01,0x45,0x01,0x05,0x09,0x19,0x01,0x29,0x0C,0x81,0x02,0x06,0x00,0xFF,0x75,0x01,0x95,0x08,0x25,0x01,0x45,0x01,0x09,0x01,0x81,0x02,0xC0,0xA1,0x02,0x75,0x08,0x95,0x07,0x46,0xFF,0x00,0x26,0xFF,0x00,0x09,0x02,0x91,0x02,0xC0,0xC0};
const static uint8_t dd1[] = {0x12,0x01,0x00,0x01,0x00,0x00,0x00,0x08,0x79,0x00,0x06,0x00,0x07,0x01,0x01,0x02,0x00,0x01};
const static uint8_t cfg1[] = {0x09,0x02,0x29,0x00,0x01,0x01,0x00,0x80,0xFA,0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x09,0x21,0x10,0x01,0x21,0x01,0x22,0x65,0x00,0x07,0x05,0x81,0x03,0x08,0x00,0x0A,0x07,0x05,0x01,0x03,0x08,0x00,0x0A};
#endif


#ifdef FAKE_JOY2
const static uint8_t joy2_rd[] = {0x05,0x01,0x09,0x04,0xA1,0x01,0xA1,0x02,0x85,0x01,0x75,0x08,0x95,0x01,0x15,0x00,0x26,0xFF,0x00,0x81,0x03,0x75,0x01,0x95,0x0D,0x15,0x00,0x25,0x01,0x35,0x00,0x45,0x01,0x05,0x09,0x19,0x01,0x29,0x0D,0x81,0x02,0x75,0x01,0x95,0x03,0x06,0x00,0xFF,0x81,0x03,0x05,0x01,0x25,0x07,0x46,0x3B,0x01,0x75,0x04,0x95,0x01,0x65,0x14,0x09,0x39,0x81,0x42,0x65,0x00,0x75,0x01,0x95,0x0C,0x06,0x00,0xFF,0x81,0x03,0x15,0x00,0x26,0xFF,0x00,0x05,0x01,0x09,0x01,0xA1,0x00,0x75,0x08,0x95,0x04,0x15,0x00,0x15,0x00,0x15,0x00,0x35,0x00,0x35,0x00,0x46,0xFF,0x00,0x09,0x30,0x09,0x31,0x09,0x32,0x09,0x35,0x81,0x02,0xC0,0x05,0x01,0x75,0x08,0x95,0x27,0x09,0x01,0x81,0x02,0x75,0x08,0x95,0x30,0x09,0x01,0x91,0x02,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xA1,0x02,0x85,0x02,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xA1,0x02,0x85,0xEE,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xA1,0x02,0x85,0xEF,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xC0};
const static uint8_t dd2[] = {0x12,0x01,0x00,0x02,0x00,0x00,0x00,0x08,0x79,0x00,0x06,0x00,0x00,0x01,0x01,0x02,0x00,0x01};
const static uint8_t cfg2[] = {0x09,0x02,0x29,0x00,0x01,0x01,0x00,0x80,0xFA,0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x09,0x21,0x11,0x01,0x00,0x01,0x22,0xB8,0x00,0x07,0x05,0x02,0x03,0x40,0x00,0x01,0x07,0x05,0x81,0x03,0x40,0x00,0x01};
#endif

#ifdef FAKE_JOY3
const static uint8_t joy3_rd[] = {0x05,0x01,0x09,0x04,0xA1,0x01,0xA1,0x02,0x85,0x01,0x75,0x08,0x95,0x01,0x15,0x00,0x26,0xFF,0x00,0x81,0x03,0x75,0x01,0x95,0x13,0x15,0x00,0x25,0x01,0x35,0x00,0x45,0x01,0x05,0x09,0x19,0x01,0x29,0x13,0x81,0x02,0x75,0x01,0x95,0x0D,0x06,0x00,0xFF,0x81,0x03,0x15,0x00,0x26,0xFF,0x00,0x05,0x01,0x09,0x01,0xA1,0x00,0x75,0x08,0x95,0x04,0x35,0x00,0x46,0xFF,0x00,0x09,0x30,0x09,0x31,0x09,0x32,0x09,0x35,0x81,0x02,0xC0,0x05,0x01,0x75,0x08,0x95,0x27,0x09,0x01,0x81,0x02,0x75,0x08,0x95,0x30,0x09,0x01,0x91,0x02,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xA1,0x02,0x85,0x02,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xA1,0x02,0x85,0xEE,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xA1,0x02,0x85,0xEF,0x75,0x08,0x95,0x30,0x09,0x01,0xB1,0x02,0xC0,0xC0};
const static uint8_t dd3[] = {0x12,0x01,0x00,0x02,0x00,0x00,0x00,0x40,0x4C,0x05,0x68,0x02,0x00,0x01,0x01,0x02,0x00,0x01};
const static uint8_t cfg3[] = {0x09,0x02,0x29,0x00,0x01,0x01,0x00,0x80,0xFA,0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x09,0x21,0x11,0x01,0x00,0x01,0x22,0x94,0x00,0x07,0x05,0x02,0x03,0x40,0x00,0x01,0x07,0x05,0x81,0x03,0x40,0x00,0x01};
#endif

#if 0
const static uint8_t cfg0[] = {0x09,0x02,0x29,0x00,0x01,0x01,0x00,0x80,0xFA,0x09,0x04,0x00,0x00,0x02,0x03,0x00,0x00,0x00,0x09,0x21,0x10,0x01,0x21,0x01,0x22,0x65,0x00,0x07,0x05,0x81,0x03,0x08,0x00,0x0A,0x07,0x05,0x01,0x03,0x08,0x00,0x0A};
const static uint8_t dd0[] = {0x12,0x01,0x00,0x01,0x00,0x00,0x00,0x08,0x79,0x00,0x06,0x00,0x07,0x01,0x01,0x02,0x00,0x01};
#endif

#define lowest(a,b) ((a) < (b) ? (a) : (b))


uint8_t tuh_descriptor_get_device_sync(uint8_t dev_addr, uint8_t *dd, uint16_t len) {
#ifdef FAKE_KBD
  if (dev_addr == KBD_DN) {
    memcpy(dd, dd0, sizeof dd0);
  }
#endif
#ifdef FAKE_JOY1
  if (dev_addr == JOY1_DN) {
    memcpy(dd, dd1, sizeof dd1);
  }
#endif
#ifdef FAKE_JOY2
  if (dev_addr == JOY2_DN) {
    memcpy(dd, dd2, sizeof dd2);
  }
#endif
#ifdef FAKE_JOY3
  if (dev_addr == JOY3_DN) {
    memcpy(dd, dd3, sizeof dd3);
  }
#endif
  return 0;
}

uint8_t tuh_descriptor_get_configuration_sync(uint8_t dev_addr, uint8_t inst, uint8_t *dd, uint16_t len) {
#ifdef FAKE_KBD
  if (dev_addr == KBD_DN) {
    memcpy(dd, cfg0, lowest(sizeof cfg0, len));
  }
#endif
#ifdef FAKE_JOY1
  if (dev_addr == JOY1_DN) {
    memcpy(dd, cfg1, lowest(sizeof cfg1, len));
  }
#endif
#ifdef FAKE_JOY2
  if (dev_addr == JOY2_DN) {
    memcpy(dd, cfg2, lowest(sizeof cfg2, len));
  }
#endif
#ifdef FAKE_JOY3
  if (dev_addr == JOY3_DN) {
    memcpy(dd, cfg3, lowest(sizeof cfg3, len));
  }
#endif
  return 0;
}

#if 0
uint8_t tuh_descriptor_get_string_sync(uint8_t daddr, uint8_t index, uint16_t language_id, void* buffer, uint16_t len) {
  if (dev_addr == 0) {
    memcpy(dd, cfg0, lowest(sizeof cfg0, len));
  }
  return 0;
}

// Sync (blocking) version of tuh_descriptor_get_manufacturer_string()
// return transfer result
uint8_t tuh_descriptor_get_manufacturer_string_sync(uint8_t daddr, uint16_t language_id, void* buffer, uint16_t len);

// Sync (blocking) version of tuh_descriptor_get_product_string()
// return transfer result
uint8_t tuh_descriptor_get_product_string_sync(uint8_t daddr, uint16_t language_id, void* buffer, uint16_t len);

// Sync (blocking) version of tuh_descriptor_get_serial_string()
// return transfer result
uint8_t tuh_descriptor_get_serial_string_sync(uint8_t daddr, uint16_t language_id, void* buffer, uint16_t len);
#endif

#ifdef FAKE_KBD
const static uint8_t kbd_rep1[] = {0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00};
const static uint8_t kbd_rep2[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const static uint8_t kbd_rep3[] = {0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00};
const static uint8_t kbd_rep4[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void kbd_usbaction(char c) {
#define k KBD_DN
  switch(c) {
    case 'i': // insert
      printf("insert kb\n");
      usb_attached(k, 0, 0x0079, 0x0006, kbd_rd, sizeof kbd_rd);
      break;

    case 'r': // remove
      printf("removed kb\n");
      usb_detached(k);
      break;

    case '1': // action1
      printf("act1\n");
      usb_handle_data(k, kbd_rep1, sizeof kbd_rep1);
      break;
    case '2': // action2
      printf("act2\n");
      usb_handle_data(k, kbd_rep2, sizeof kbd_rep2);
      break;
    case '3': // action1
      printf("act3\n");
      usb_handle_data(k, kbd_rep3, sizeof kbd_rep3);
      break;
    case '4': // action2
      printf("act4\n");
      usb_handle_data(k, kbd_rep4, sizeof kbd_rep4);
      break;
  }
#undef k
}
#endif

#ifdef FAKE_JOY1
const static uint8_t joy1_rep1[] = {0x7F,0x7F,0x87,0x7F,0x7F,0x0F,0x00,0xC0};
const static uint8_t joy1_rep2[] = {0x00,0x7F,0x86,0x7F,0x7F,0x0F,0x00,0xC0};
const static uint8_t joy1_rep3[] = {0x7F,0xFF,0x85,0x7F,0x7F,0x0F,0x00,0xC0};
const static uint8_t joy1_rep4[] = {0x7F,0x00,0x87,0x7F,0x7F,0x0F,0x00,0xC0};


void joy1_usbaction(char c) {
#define k JOY1_DN
  switch(c) {
    case 'i': // insert
      printf("insert joy1\n");
      usb_attached(k, 0, 0x0a81, 0x0101, joy1_rd, sizeof joy1_rd);
      break;

    case 'r': // remove
      printf("removed joy1\n");
      usb_detached(k);
      break;

    case '1': // action1
      printf("act1\n");
      usb_handle_data(k, joy1_rep1, sizeof joy1_rep1);
      break;
    case '2': // action2
      printf("act2\n");
      usb_handle_data(k, joy1_rep2, sizeof joy1_rep2);
      break;
    case '3': // action1
      printf("act3\n");
      usb_handle_data(k, joy1_rep3, sizeof joy1_rep3);
      break;
    case '4': // action2
      printf("act4\n");
      usb_handle_data(k, joy1_rep4, sizeof joy1_rep4);
      break;
  }
#undef k
}
#endif


#ifdef FAKE_JOY2
const static uint8_t joy2_rep1[] = {0x01,0x00,0x00,0x00,0x08,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};
const static uint8_t joy2_rep2[] = {0x01,0x00,0x00,0x00,0x04,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};
const static uint8_t joy2_rep3[] = {0x01,0x00,0x00,0x00,0x06,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};
const static uint8_t joy2_rep4[] = {0x01,0x00,0x00,0x00,0x02,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};


void joy2_usbaction(char c) {
#define k JOY2_DN
  switch(c) {
    case 'i': // insert
      printf("insert joy2\n");
      usb_attached(k, 0, 0x054C, 0x0268, joy2_rd, sizeof joy2_rd);
      break;

    case 'r': // remove
      printf("removed joy2\n");
      usb_detached(k);
      break;

    case '1': // action1
      printf("act1\n");
      usb_handle_data(k, joy2_rep1, sizeof joy2_rep1);
      break;
    case '2': // action2
      printf("act2\n");
      usb_handle_data(k, joy2_rep2, sizeof joy2_rep2);
      break;
    case '3': // action1
      printf("act3\n");
      usb_handle_data(k, joy2_rep3, sizeof joy2_rep3);
      break;
    case '4': // action2
      printf("act4\n");
      usb_handle_data(k, joy2_rep4, sizeof joy2_rep4);
      break;
  }
#undef k
}
#endif

#ifdef FAKE_JOY3
const static uint8_t joy3_rep1[] = {0x01,0x00,0x00,0x00,0x08,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};
const static uint8_t joy3_rep2[] = {0x01,0x00,0x00,0x00,0x04,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};
const static uint8_t joy3_rep3[] = {0x01,0x00,0x00,0x00,0x06,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};
const static uint8_t joy3_rep4[] = {0x01,0x00,0x00,0x00,0x02,0x00,0x7F,0x7F,0x7F,0x7F,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEF,0x14,0x00,0x00,0x00,0x00,0x23,0xFF,0x77,0x01,0x81,0x02,0x00,0x02,0x00,0x01,0xA0,0x02,0x00};


void joy3_usbaction(char c) {
#define k JOY3_DN
  switch(c) {
    case 'i': // insert
      printf("insert joy3\n");
      usb_attached(k, 0, 0x054C, 0x0286, joy3_rd, sizeof joy3_rd);
      break;

    case 'r': // remove
      printf("removed joy3\n");
      usb_detached(k);
      break;

    case '1': // action1
      printf("act1\n");
      usb_handle_data(k, joy3_rep1, sizeof joy3_rep1);
      break;
    case '2': // action2
      printf("act2\n");
      usb_handle_data(k, joy3_rep2, sizeof joy3_rep2);
      break;
    case '3': // action1
      printf("act3\n");
      usb_handle_data(k, joy3_rep3, sizeof joy3_rep3);
      break;
    case '4': // action2
      printf("act4\n");
      usb_handle_data(k, joy3_rep4, sizeof joy3_rep4);
      break;
  }
#undef k
}
#endif

// void mouse_usbaction(char c) {
// }
//
// void joypad1_usbaction(char c) {
// }
#endif

#if defined(USB) && !defined(USBFAKE)
void usb_poll() {
  tuh_task();
  hid_app_task();
}
#else
void usb_poll() {
}
#endif



#ifdef DEVKIT_DEBUG
static char str[256];
int uprintf(const char *fmt, ...) {
  int i;
  va_list argp;
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);

  i=0;
  while (str[i]) {
    if (str[i] == '\n') uart_putc(uart1, '\r');
    uart_putc(uart1, str[i]);
    i++;
  }
  return i;
}

void usetup() {
  uart_init (uart1, 115200);
  gpio_set_function(8, GPIO_FUNC_UART);
  gpio_set_function(9, GPIO_FUNC_UART);
}
#endif


int main() {
  stdio_init_all();
//   test_block_read_t fbrt;
  sleep_ms(2000); // usb settle delay
  pio_spi_inst_t *spi = NULL;

  // set up error led
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

#ifdef DEVKIT_DEBUG
  usetup();
#endif

#if PICO_NO_FLASH
  enable_xip();
#endif

  printf("Drivertest Microjack\'23\n");

#if defined(USB) && !defined (USBFAKE)
  board_init();
  tusb_init();
#endif

  mist_init();

#ifdef MB2
  ipc_InitMaster();
#endif

  char lastch = 0;
  for(;;) {
    int c = getchar_timeout_us(2);
//     int c = getchar();
//     if (forceexit) break;
    if (c == 'q') break;
    if (c == 'm') {
      extern int menu;
      menu = !menu;
    }
#ifdef USBFAKE
#ifdef FAKE_KBD
    if (c == 'k') {
      printf("mode = keyboard\n");
      lastch = 'k';
    } else if (lastch == 'k') {
      kbd_usbaction(c);
    }
#endif
#ifdef FAKE_JOY1
    if (c == 'j') {
      printf("mode = joypad1\n");
      lastch = 'j';
    } else if (lastch == 'j') {
      joy1_usbaction(c);
    }
#endif
#ifdef FAKE_JOY2
    if (c == 'J') {
      printf("mode = joypad2\n");
      lastch = 'J';
    } else if (lastch == 'J') {
      joy2_usbaction(c);
    }
#endif
#ifdef FAKE_JOY3
    if (c == 'h') {
      printf("mode = joypad3\n");
      lastch = 'h';
    } else if (lastch == 'h') {
      joy3_usbaction(c);
    }
#endif
#endif

//     if (c == 'm') {
//       printf("mode = mouse\n");
//       lastch = 'm';
//     } else if (lastch == 'm') {
//       mouse_usbaction(c);
//     }

    mist_loop();
    usb_poll();
  }

  reset_usb_boot(0, 0);
	return 0;
}
