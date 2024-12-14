#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
// #include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/spi.h"

#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "hardware/pio.h"
#include "hardware/watchdog.h"

#include "drivers/fpga.h"
#include "drivers/flash.h"
#include "drivers/pio_spi.h"
#include "drivers/sdcard.h"
#include "drivers/ps2.h"
#include "drivers/fifo.h"
#include "drivers/ipc.h"
#include "drivers/kbd.h"
#include "drivers/cookie.h"
// #define DEBUG
#include "drivers/debug.h"

#include "mistmain.h"

#ifdef PICOSYNTH
#include "picosynth.h"
#include "wtsynth.h"
#endif

#include "drivers/pins.h"

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

void usb_poll() {
}

#ifdef MB2
uint8_t stop_watchdog = 0;

struct repeating_timer watchdog_timer;
static bool watchdog_Callback(struct repeating_timer *t) {
  if (!stop_watchdog) watchdog_update();
  return true;
}
#endif

static void usb_and_audio_core() {
#ifdef PICOSYNTH
  picosynth_Init();
#endif

  for(;;) {
#ifdef PICOSYNTH
    picosynth_Loop();
#endif
#ifdef USB
    tuh_task();
#endif
  }
}

int main() {
  // hold FPGA in reset until we decide what to do with it - (ZXTRES only)
#ifndef ZXUNO
  fpga_holdreset();
#endif

  stdio_init_all();

#ifdef MB2
  cookie_Reset();
  watchdog_enable(4000, true);
  add_repeating_timer_us(2000000, watchdog_Callback, NULL, &watchdog_timer);
#endif

#if PICO_NO_FLASH
  sleep_ms(2000); // usb settle delay
#endif
  pio_spi_inst_t *spi = NULL;

#if PICO_NO_FLASH
  enable_xip();
#endif

  printf("Drivertest Microjack\'23\n");

#if defined(USB) && !defined (USBFAKE)
  board_init();
  tusb_init();
#endif

#ifdef MB2
  ipc_InitMaster();
#endif

  /* initialise MiST software */
  mist_init();

  /* start usb and sound process */
#if defined(PICOSYNTH) || defined(USB)
  multicore_reset_core1();
  multicore_launch_core1(usb_and_audio_core);
#endif


  char lastch = 0;
  for(;;) {
    int c = getchar_timeout_us(2);
//     if (forceexit) break;
    if (c == 'q') break;
    if (c == 'w') {
      fpga_init("ZX-Spectrum-Legacy.rbf");
      reset_usb_boot(0, 0);
    }
    if (c == 'e') {
      fpga_init("zxspectrum_20201222.np1");
      // fpga_init("Adam_Computer.rbf");
      reset_usb_boot(0, 0);
    }
    if (c == 'r') {
      gpio_init(GPIO_FPGA_CONF_DONE);
      gpio_init(GPIO_FPGA_NSTATUS);
      gpio_init(GPIO_FPGA_NCONFIG);

      gpio_put(GPIO_FPGA_NCONFIG, 1);
      gpio_set_dir(GPIO_FPGA_NCONFIG, GPIO_OUT);
      gpio_put(GPIO_FPGA_NCONFIG, 0);
      sleep_us(100);
      gpio_put(GPIO_FPGA_NCONFIG, 1);

      gpio_init(GPIO_FPGA_CONF_DONE);
      gpio_init(GPIO_FPGA_NSTATUS);
      gpio_init(GPIO_FPGA_NCONFIG);
      reset_usb_boot(0, 0);
    }

#ifdef ZXUNO
    if (c == 'j') {
      void ConfigureFPGAStdin();
      ConfigureFPGAStdin();
      while (getchar_timeout_us(2) >= 0);
    }
#endif
#if 0 /* press SELECT with no joypad connected */
    if (c == 'w' || c == 'W') {
      user_io_digital_joystick(0, c == 'W' ? 0 : 0x80);
      user_io_digital_joystick_ext(0, c == 'W' ? 0 : 0x80);
      user_io_digital_joystick(1, c == 'W' ? 0 : 0x80);
      user_io_digital_joystick_ext(1, c == 'W' ? 0 : 0x80);
      printf("Pressing start realase status = %d\n", c == 'W');
    }
#endif


    mist_loop();
    usb_poll();
  }

  reset_usb_boot(0, 0);
	return 0;
}
