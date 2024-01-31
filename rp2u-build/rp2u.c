#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/pio.h"

#include "flash.h"
#include "ps2.h"
#include "jamma.h"
#ifdef USB
#include "joypad.h"
#endif

#include "fifo.h"
#include "ipc.h"
#include "pins.h"
// #define printf uprintf
// #define DEBUG
#include "debug.h"

#ifdef USB
#include "bsp/board.h"
#include "tusb.h"
#endif

#include "kbd.h"
#include "cookie.h"

#include "version.h"

#define STANDARD_SYS_CLOCK_KHZ 48000

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100
#endif
#ifndef SAMPLE_LEN
#define SAMPLE_LEN 1024
#endif

#ifdef MAINDEBUG
#define DEBUG
#endif

#ifdef SYSEXDEBUG
#define SDEBUG
#endif


#ifdef DEBUG
#define debug(a) uprintf a
#else
#define debug(a)
#endif

#ifdef SDEBUG
#define sdebug(a) uprintf a
#else
#define sdebug(a)
#endif

#ifdef DEBUG
uint8_t serial_buffer[1024];
fifo_t serial_buffer_fifo;

int xboard_getchar() {
  return fifo_Get(&serial_buffer_fifo);
}

static char str[256];
int uprintf(const char *fmt, ...) {
  int i;
  va_list argp;
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);

  i=0;
  while (str[i]) {
#if 0
    if (str[i] == '\n') uart_putc(uart1, '\r');
    uart_putc(uart1, str[i]);
#endif
    fifo_Put(&serial_buffer_fifo, str[i]);
    i++;
  }
  return i;
}
#endif

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

uint8_t flash_data[4096];
uint16_t flash_crc;
uint8_t reboot = 0;
uint8_t stop_watchdog = 0;

uint8_t mistMode = 0;
// uint8_t mistMode = 1;
uint8_t previousMistMode = 0xff;

struct repeating_timer watchdog_timer;
static int watchdog_Callback(struct repeating_timer *t) {
  if (!stop_watchdog) watchdog_update();
  return true;
}

uint8_t ipc_GotCommand(uint8_t cmd, uint8_t *data, uint8_t len) {
  printf("ipc_GotCommand: cmd %02X\n", cmd);
  hexdump(data, len);
  switch(cmd) {
    case IPC_FLASHDATA:
      memcpy(&flash_data[(data[0] & 0x1f) * 128], data + 1, 128);
      return 0x01;
    case IPC_FLASHCOMMIT: {
      uint32_t addr = (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
      uint16_t crc = (data[4]<<8) | data[5];
      uint16_t calcCrc = crc16(flash_data, sizeof flash_data);
      
      printf("addr %08X crc %04X calcCrc %04X\n", addr, crc, calcCrc);
      if (crc != calcCrc) {
        return 2;
      } else {
        return flash_EraseProgram(addr, flash_data, sizeof flash_data);
      }
    }
    
    case IPC_VERSIONMAJOR:
      return VERSION_MAJOR;
    case IPC_VERSIONMINOR:
      return VERSION_MINOR;
    case IPC_RUNTIME:
      return 'u';
    case IPC_REBOOT:
      reboot = data[0];
      return 0x00;

    case IPC_BOOTSTRAP:
      cookie_Set();
      stop_watchdog = 1;
      return 0xde;
//       watchdog_enable(1, 1);
//       for(;;);
//       break;

    case IPC_CHECKINTEGRITY:
      return 1; // of course its fine because we're running it now
    
#ifdef USB
    case IPC_TRAINJOYPAD:
      return joypad_TrainingAPI(data, len);
#endif

    case IPC_SETMISTER: {
      mistMode = data[0] == MIST_MODE;
      cookie_Set2(data[0]);
#ifdef USB
      HID_setMistMode(mistMode);
#endif
      break;
    }

    case IPC_SETFASTMODE: {
      ipc_SetFastMode(data[0]);
      return 0;
    }

    // messages to the PS2 devices - keyboard and mouse
    case IPC_SENDPS2: {
      for (int i = 1; i<len; i++) {
        ps2_SendChar(data[0], data[i]);
      }
      break;
    }

    // to jamma interface from main core - when USB is used on main core.
    case IPC_SENDJAMMA:
      jamma_SetData(data[0], data[1]);
      break;

#ifdef USB_ON_RP2U
    case IPC_USB_SETCONFIG: {
      tuh_configuration_set(data[0], data[1], NULL, NULL);
      return 0;
    }
#endif

    case IPC_APPDATA: {
      uint32_t *sig = (uint32_t *)FLASH_APPSIGNATURE;
      switch(data[0]) {
        case 0: return (sig[3] >> 8) & 0xff; //vh
        case 1: return sig[3] & 0xff; //vl
        case 2: return (sig[2] >> 8) & 0xff; // crch
        case 3: return sig[2] & 0xff; // crcl
        case 4: return sig[1] >> 24; // img size
        case 5: return (sig[1] >> 16) & 0xff;
        case 6: return (sig[1] >> 8) & 0xff;
        case 7: return sig[1] & 0xff;
      }
    } // fall through
      
    default:
      return 0xfd;
  }
}

#ifdef MATRIX_KEYBOARD
void kbd_core() {
  ps2_Init();
  ipc_InitSlave();

  kbd_InitEx(mistMode);

  for(;;) {
    if (previousMistMode != mistMode) {
      jamma_Kill();
      jamma_InitEx(mistMode);
      ps2_EnablePortEx(0, false, mistMode);
      ps2_EnablePortEx(0, true, mistMode);
      kbd_SetMistMode(mistMode);
      previousMistMode = mistMode;
    }

    kbd_Process();

    if (mistMode) {
      if (jamma_HasChanged()) {
        uint32_t data = jamma_GetDataAll();
        ipc_SendData(IPC_UPDATE_JAMMA, &data, sizeof data);
        printf("Jamma updated\n");
      }

      uint8_t data[16];
      int i = 0, c;
      while (i<16 && (c = ps2_GetChar(0)) != -1) {
        data[i++] = c;
#ifdef DEBUG_PS2
        printf("[%02X]", c);
#endif
      }
      if (i) {
#ifdef DEBUG_PS2
        printf("\n");
#endif
        ipc_SendData(IPC_PS2_DATA, data, i);
      }
    }

    ipc_SlaveTick();
  }
}
#endif

void HID_setMistMode(uint8_t on);
int main()
{
#ifdef DEBUG
  fifo_Init(&serial_buffer_fifo, serial_buffer, sizeof serial_buffer);
#endif


 // set_sys_clock_khz(STANDARD_SYS_CLOCK_KHZ, true);
#ifndef USB
  stdio_init_all();
  for (int i=0; i<100; i++) {
    printf("USB UART on\n");
  }
#endif
  // default on
#if PICO_NO_FLASH
  uint8_t ramCookie = cookie_IsPresent2();
  mistMode = ramCookie == MIST_MODE;
#endif

#ifdef USB
  HID_setMistMode(mistMode);
#endif

  sleep_ms(1000); // usb settle delay

  cookie_Reset();
  watchdog_enable(4000, true);
  add_repeating_timer_us(2000000, watchdog_Callback, NULL, &watchdog_timer);

#ifdef MATRIX_KEYBOARD
  multicore_reset_core1();
  multicore_launch_core1(kbd_core);
#endif

#ifdef USB
  board_init();
  tusb_init();
#endif

#ifdef USB
  joypad_Init();
#endif
  
#if PICO_NO_FLASH
  enable_xip();
#endif

  for(;;) {
#ifndef USB
    if (getchar_timeout_us(2) == 'q') break;
#endif
//     printf("in loop\n");
#ifdef USB
    tuh_task();
#endif
#if CFG_TUH_HID
    hid_app_task();
#endif

#if CFG_TUH_CDC
    cdc_app_task();
#endif

    if (reboot == 0xaa) {
      break;
    }
//     printf("gpio: %d\n", gpio_get(GPIO_RP2U_XLOAD));
  }
  
  reset_usb_boot(0, 0);
	return 0;
}
