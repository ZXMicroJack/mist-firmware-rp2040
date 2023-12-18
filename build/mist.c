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
  for(;;) {
    int c = getchar_timeout_us(2);
//     int c = getchar();
//     if (forceexit) break;
    if (c == 'q') break;
    if (c == 'm') {
      extern int menu;
      menu = !menu;
    }
    mist_loop();
  }

  reset_usb_boot(0, 0);
	return 0;
}
