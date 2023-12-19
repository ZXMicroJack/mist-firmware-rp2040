#include <stdio.h>
#include <stdint.h>
#include <string.h>
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
#include "drivers/ipc.h"
#include "drivers/kbd.h"
#define DEBUG
#include "drivers/debug.h"

#include "mistmain.h"
#include "usbdev.h"


void FatalError(unsigned long error) {
  unsigned long i;

  iprintf("Fatal error: %lu\r", error);
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



#if 0
usb_data_t *k = NULL;

const static uint8_t kbd_rd[] = {0x05,0x01,0x09,0x06,0xA1,0x01,0x05,0x07,0x19,0xE0,0x29,0xE7,0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,0x75,
  0x08,0x81,0x01,0x95,0x05};

const static uint8_t kbd_rep1[] = {0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00};
const static uint8_t kbd_rep2[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const static uint8_t kbd_rep3[] = {0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00};
const static uint8_t kbd_rep4[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void kbd_usbaction(char c) {
  switch(c) {
    case 'i': // insert
      printf("insert kb\n");
      if (!k) k = usb_get_handle(0x0a81, 0x0101);
      usb_attached(k, 0, kbd_rd, sizeof kbd_rd);
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
}

// void mouse_usbaction(char c) {
// }
//
// void joypad1_usbaction(char c) {
// }
#endif

int main() {
  stdio_init_all();
//   test_block_read_t fbrt;
  sleep_ms(2000); // usb settle delay
  pio_spi_inst_t *spi = NULL;

  // set up error led
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

#if PICO_NO_FLASH
  enable_xip();
#endif

  printf("Drivertest Microjack\'23\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");
  printf("Running test\n");

  mist_init();
#ifdef USB
  mist_usb_init();
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
#if 0
    if (c == 'k') {
      printf("mode = keyboard\n");
      lastch = 'k';
    } else if (lastch == 'k') {
      kbd_usbaction(c);
    }
#endif

//     if (c == 'm') {
//       printf("mode = mouse\n");
//       lastch = 'm';
//     } else if (lastch == 'm') {
//       mouse_usbaction(c);
//     }
    mist_loop();
#ifdef USB
    mist_usb_loop();
#endif
  }

  reset_usb_boot(0, 0);
	return 0;
}
