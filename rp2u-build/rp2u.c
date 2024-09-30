#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/watchdog.h"

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
#include "gpioirq.h"
// #define printf uprintf
// #define DEBUG
// #define DEBUG_PS2
#include "debug.h"

#ifdef USB
#include "bsp/board.h"
#include "tusb.h"
#endif

#include "kbd.h"
#include "cookie.h"
#ifdef USB
#include "hiddev.h"
#endif

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
uint8_t previousMistMode = 0xff;

struct repeating_timer watchdog_timer;
static bool watchdog_Callback(struct repeating_timer *t) {
  if (!stop_watchdog) watchdog_update();
  return true;
}

uint8_t ps2_rp2m_buf[4][64]; // kbd in, mse in, kbd host in, mse host in
fifo_t ps2_rp2m_fifo[4];

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
      if (data[0] != 0xaa) {
	stop_watchdog = 1;
      }
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
      break;
    }

    case IPC_SETFASTMODE: {
      ipc_SetFastMode(data[0]);
      return 0;
    }

    // messages to the PS2 devices - keyboard and mouse
    case IPC_SENDPS2: {
      for (int i = 1; i<len; i++) {
				fifo_Put(&ps2_rp2m_fifo[data[0]], data[i]);
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
// static fifo_t *matkbd_in;
static fifo_t *hid_ps2_in[2];
static fifo_t *hid_ps2_out[2];
static fifo_t mist_ps2_out[2];
static uint8_t mist_ps2_out_buf[2][64];

// from:
//   [MiST]   keyboard command from core
//   [legacy] PS2 host command from core
int ps2_InHost(uint8_t ch) {
  int c;

  // from rp2m in mist mode
  if ((c = fifo_Get(&ps2_rp2m_fifo[ch+2])) < 0 && !mistMode) { 
    c = ps2_GetChar(ch); // from core in legacy mode.
  }

  return c;
}

// to:
//          USB HID as PS2 host command
//   [MiST] PS2 keyboard as command
void ps2_OutHost(uint8_t ch, uint8_t data) {
#ifdef USB
	fifo_Put(hid_ps2_out[ch], data); // to ps2 hid
#endif
	if (mistMode) { // to ps2 keyboard
    ps2_SendChar(ch, data);
  }
}

// from:
//   matrix keyboard
//   hid usb keyboard
//   hid usb keyboard connected to rp2m
//   [MiST only] PS2 keyboard
int ps2_In(uint8_t ch) {
  int c;

	// if (ch == 0 && (c = fifo_Get(matkbd_in)) >= 0) return c; // from matrix
  if (ch == 0 && (c = kbd_Get()) >= 0) return c; // from matrix
#ifdef USB
	if ((c = fifo_Get(hid_ps2_in[ch])) >= 0) return c; // from usb
#endif
	if ((c = fifo_Get(&ps2_rp2m_fifo[ch])) >= 0) return c; // from rp2m in legacy mode
  if (mistMode) {
    c = ps2_GetChar(ch); // from host mode ps2 in mist mode
  }
    return c;
}

// to:
//   [legacy] to core as PS2 scancode
//   [MiST]   to MiST core as PS2 scancode 
void ps2_Out(uint8_t ch, uint8_t data) {
  if (mistMode) {
		fifo_Put(&mist_ps2_out[ch], data);
	} else {
		ps2_SendChar(ch, data);
	}
}

// process mist ps2 queues
void ps2_MistFlush(uint8_t ch) {
  uint8_t data[16];
	int c, j=1;

	data[0] = ch; // channel
  while (j<16 && ((c = fifo_Get(&mist_ps2_out[ch]))) != -1) {
#ifdef DEBUG_PS2
    printf("[%02X]", c);
#endif
    data[j++] = c;
  }

	// if there is data to send
  if (j > 1) {
#ifdef DEBUG_PS2
    printf("\n");
#endif
    ipc_SendData(IPC_PS2_DATA, data, j);
  }
}

uint64_t mouse_enable_at = 0;
uint64_t mouse_last_report = 0;
#define MOUSE_POST_RESET_ENABLE   1000000
#define MOUSE_REPORT_PERIOD       25000

int mouseindex = -1;
int mousereport[3];
int mousestandoff = 0;

char to_8bit_signed(int n) {
  return n > 127 ? 127 :
         n < -128 ? -128 :
         n;
}

void mouse_filter(uint8_t k) {
  if (mousestandoff) {
    ps2_Out(1, k);
    mousestandoff --;
    mouse_last_report = 0;
    return;
  }

  /* scan mouse messages */
  if (mouseindex == -1) {
    // find bit that is always set
    if (k & 0x08) {
      mousereport[0] = k;
      mouseindex = 1;
    }
  } else {
    if (mouseindex == 0)
      mousereport[mouseindex++] = k;
    else
      mousereport[mouseindex++] += (int8_t)k;

    /* sync bit lost reset sync */
    if ((mousereport[0] & 0x08) != 0x08 ) {
      mouseindex = -1;
      mousereport[0] = mousereport[1] = mousereport[2] = 0;
    } else if (mouseindex >= 3) {
      /* got full report process */
      mouseindex = 0;

      uint64_t now = time_us_64();
      if (mouse_last_report == 0 || now > (mouse_last_report + MOUSE_REPORT_PERIOD))
      {
        ps2_Out(1, mousereport[0]);
        ps2_Out(1, to_8bit_signed(mousereport[1]));
        ps2_Out(1, to_8bit_signed(mousereport[2]));

#ifdef DEBUG_MOUSE
        printf("mouse %02X %02X %02X\n", mousereport[0], mousereport[1], mousereport[2]);
#endif
        mousereport[0] = mousereport[1] = mousereport[2] = 0;
        mouse_last_report = now;
      }
    }
  }
}


void kbd_core() {
#ifdef USB
  HID_init();
#endif
	// PS2 IO from MiST to/from devices
  fifo_Init(&ps2_rp2m_fifo[0], ps2_rp2m_buf[0], sizeof ps2_rp2m_buf[0]); // in device
  fifo_Init(&ps2_rp2m_fifo[1], ps2_rp2m_buf[1], sizeof ps2_rp2m_buf[1]);
  fifo_Init(&ps2_rp2m_fifo[2], ps2_rp2m_buf[2], sizeof ps2_rp2m_buf[2]); // in host
  fifo_Init(&ps2_rp2m_fifo[3], ps2_rp2m_buf[3], sizeof ps2_rp2m_buf[3]);

	// PS2 keyboard and mouse channels to MiST core
  fifo_Init(&mist_ps2_out[0], mist_ps2_out_buf[0], sizeof mist_ps2_out_buf[0]);
  fifo_Init(&mist_ps2_out[1], mist_ps2_out_buf[1], sizeof mist_ps2_out_buf[1]);

  ps2_Init();
  ipc_InitSlave();
  kbd_Init();


  // matkbd_in = kbd_GetFifo();
#ifdef USB
	hid_ps2_in[0] = HID_getPS2Fifo(0, 0); // from device
	hid_ps2_in[1] = HID_getPS2Fifo(1, 0); // from device
	hid_ps2_out[0] = HID_getPS2Fifo(0, 1); // to device
	hid_ps2_out[1] = HID_getPS2Fifo(1, 1); // to device
#endif

	int c;
  for(;;) {
    if (previousMistMode != mistMode) {
      jamma_Kill();
      jamma_InitEx(mistMode);
      ps2_EnablePortEx(0, false, mistMode);
      ps2_EnablePortEx(0, true, mistMode);
      ps2_EnablePortEx(1, false, mistMode);
      ps2_EnablePortEx(1, true, mistMode);
      ps2_SwitchMode(mistMode);
      previousMistMode = mistMode;

      /* reset the ps2 keyboard */
      ps2_OutHost(0, 0xff);

      /* reset the ps2 mouse */
      ps2_OutHost(1, 0xff);
      mouse_enable_at = time_us_64() + MOUSE_POST_RESET_ENABLE;
      mousestandoff = 10;
    }

    /* issue mouse enable */
    if (mouse_enable_at != 0 && time_us_64() > mouse_enable_at) {
      ps2_OutHost(1, 0xf4);
      mouse_enable_at = 0;
      mousestandoff = 10;
    }

    kbd_Process();

		// handle PS2 multiplexing
		for (int i=0; i<2; i++) {
			// input keyboard scancodes
      if (i == 1) { // is a mouse
        while ((c = ps2_In(i)) >= 0) {
          mouse_filter(c);
#ifdef DEBUG_PS2
          printf("In[%d]: %02X\n", i, c);
#endif
        }
      } else {
        while ((c = ps2_In(i)) >= 0) {
          ps2_Out(i, c);
#ifdef DEBUG_PS2
          printf("In[%d]: %02X\n", i, c);
#endif
        }
      }
			// input keyboard commands
			while ((c = ps2_InHost(i)) >= 0) {
				ps2_OutHost(i, c);
#ifdef DEBUG_PS2
				printf("Out[%d]: %02X\n", i, c);
#endif
			}
			ps2_MistFlush(i);
		}

		// in MiST mode, pass DB9 joypad changes up to RP2M
    if (mistMode) {
      if (jamma_HasChanged()) {
        uint32_t data = jamma_GetDataAll();
        ipc_SendData(IPC_UPDATE_JAMMA, (uint8_t *)&data, sizeof data);
        printf("Jamma updated\n");
      }
    }

    ps2_HealthCheck();
    ipc_SlaveTick();
  }
}
#endif

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
  uint8_t ramCookie = cookie_IsPresent2();
  mistMode = ramCookie == MIST_MODE;

  // sleep_ms(1000); // usb settle delay

  cookie_Reset();
  gpioirq_Init();
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
		int c = getchar_timeout_us(2);
    if (c == 'q') break;
		if (c == 'm') mistMode = !mistMode;
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
