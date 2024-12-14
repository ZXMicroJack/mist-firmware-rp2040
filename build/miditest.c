#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/resets.h"

#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "hardware/pio.h"

#include "drivers/fifo.h"
#include "drivers/midi.h"
#include "drivers/pins.h"
#define DEBUG
#include "drivers/debug.h"

#include "picosynth.h"
#include "wtsynth.h"
#include "drivers/fpga.h"

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

static int picosynth_inited = 0;

static void usb_and_audio_core() {
  picosynth_Init();

  picosynth_inited = 1;
  for(;;) {
    picosynth_Loop();
  }
}

void midi_loop() {
  unsigned char uartbuff[16];
  int thisread;

  // read and process midi data
  int readable = midi_isdata();
  while (readable) {
    thisread = readable > sizeof uartbuff ? sizeof uartbuff : readable;
    readable -= thisread;
    
    thisread = midi_get(uartbuff, thisread);

#if 1 // disabled for debug
    debug(("MidiIn: "));
    for (int i=0; i<thisread; i++) {
      debug(("%02X %c", uartbuff[i], (uartbuff[i] >= ' ' && uartbuff[i] < 128) ? uartbuff[i] : '?'));
    }
    debug(("\n"));
#endif
    if (picosynth_inited) wtsynth_HandleMidiBlock(uartbuff, thisread);
  }
}

void wtsynth_Sysex(uint8_t data) {}

int main() {
  stdio_init_all();

#if PICO_NO_FLASH
  sleep_ms(2000); // usb settle delay
#endif

#if PICO_NO_FLASH
  enable_xip();
#endif

  printf("MIDItest Microjack\'23\n");

#if 0
  fpga_initialise();
  fpga_claim(false);
  fpga_reset();
#endif
  midi_init();
  multicore_reset_core1();
  multicore_launch_core1(usb_and_audio_core);

  char lastch = 0;
  for(;;) {
    int c = getchar_timeout_us(2);
    if (c == 'q') break;
    else {
	    // extern int nrbuffs;
	    // printf("picosynth status: %d nrbuffs %d\n", picosynth_GetStatus(), nrbuffs);
    }

    midi_loop();
  }

  reset_usb_boot(0, 0);
	return 0;
}
