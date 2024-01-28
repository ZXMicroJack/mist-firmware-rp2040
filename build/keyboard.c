#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "attrs.h"
#include "hardware.h"
#include "user_io.h"
#include "xmodem.h"
#include "ikbd.h"
#include "usb.h"
#include "drivers/ps2.h"
// #define DEBUG
#include "drivers/debug.h"

// remap modifiers to each other if requested
//  bit  0     1      2    3    4     5      6    7
//  key  LCTRL LSHIFT LALT LGUI RCTRL RSHIFT RALT RGUI


// #define RGUI  0x100
// #define EXT   0x200
#define MOD   0x400

#define LCTRL 0x01
#define LSHIFT 0x02
#define LALT 0x04
#define LGUI 0x08
#define RCTRL 0x10
#define RSHIFT 0x20
#define RALT 0x40
#define RGUI 0x80

static const uint16_t ps2usb_lut[] = {
0x03, // 00: ErrorUndefined
0x42, // 01: F9
0x00, // 02: NoEvent
0x3E, // 03: F5
0x3C, // 04: F3
0x3A, // 05: F1
0x3B, // 06: F2
0x45, // 07: F12 (OSD)
0x00, // 08: NoEvent
0x43, // 09: F10
0x41, // 0a: F8
0x3F, // 0b: F6
0x3D, // 0c: F4
0x2B, // 0d: Tab
0x35, // 0e: `
0x67, // 0f: KP =
0x00, // 10: NoEvent
0x00, // 11: NoEvent
MOD|LSHIFT, // 12: NoEvent
MOD|LALT, // 13: NoEvent
MOD|LCTRL, // 14: NoEvent
0x14, // 15: q
0x1E, // 16: 1
0x00, // 17: NoEvent
0x6A, // 18: F15
0x00, // 19: NoEvent
0x1D, // 1a: z
0x16, // 1b: s
0x04, // 1c: a
0x1A, // 1d: w
0x1F, // 1e: 2
0x00, // 1f: NoEvent
0x00, // 20: NoEvent
0x06, // 21: c
0x1B, // 22: x
0x07, // 23: d
0x08, // 24: e
0x21, // 25: 4
0x20, // 26: 3
0x00, // 27: NoEvent
0x00, // 28: NoEvent
0x2C, // 29: Space
0x19, // 2a: v
0x09, // 2b: f
0x17, // 2c: t
0x15, // 2d: r
0x22, // 2e: 5
0x00, // 2f: NoEvent
0x00, // 30: NoEvent
0x11, // 31: n
0x05, // 32: b
0x0B, // 33: h
0x0A, // 34: g
0x1C, // 35: y
0x23, // 36: 6
0x00, // 37: NoEvent
0x00, // 38: NoEvent
0x00, // 39: NoEvent
0x10, // 3a: m
0x0D, // 3b: j
0x18, // 3c: u
0x24, // 3d: 7
0x25, // 3e: 8
0x00, // 3f: NoEvent
0x00, // 40: NoEvent
0x36, // 41: ,
0x0E, // 42: k
0x0C, // 43: i
0x12, // 44: o
0x27, // 45: 0
0x26, // 46: 9
0x00, // 47: NoEvent
0x00, // 48: NoEvent
0x37, // 49: .
0x38, // 4a: /
0x0F, // 4b: l
0x33, // 4c: ;
0x13, // 4d: p
0x2D, // 4e: -
0x00, // 4f: NoEvent
0x00, // 50: NoEvent
0x00, // 51: NoEvent
0x34, // 52: '
0x00, // 53: NoEvent
0x2F, // 54: [
0x2E, // 55: =
0x00, // 56: NoEvent
0x00, // 57: NoEvent
0x39, // 58: Caps Lock
MOD|RSHIFT, // 59: NoEvent
0x28, // 5a: Return
0x30, // 5b: ]
0x00, // 5c: NoEvent
0x32, // 5d: Europe 1
0x00, // 5e: NoEvent
0x00, // 5f: NoEvent
0x00, // 60: NoEvent
0x64, // 61: Europe 2
0x00, // 62: NoEvent
0x00, // 63: NoEvent
0x00, // 64: NoEvent
0x00, // 65: NoEvent
0x2A, // 66: Backspace
0x00, // 67: NoEvent
0x00, // 68: NoEvent
0x59, // 69: KP 1
0x00, // 6a: NoEvent
0x5C, // 6b: KP 4
0x6C, // 6c: F17
0x00, // 6d: NoEvent
0x00, // 6e: NoEvent
0x48, // 6f: Pause (special key handled inside user_io)
0x6F, // 70: F20
0x63, // 71: KP .
0x5A, // 72: KP 2
0x5D, // 73: KP 5
0x5E, // 74: KP 6
0x6D, // 75: F18
0x29, // 76: Escape
0x68, // 77: Num Lock
0x44, // 78: F11
0x57, // 79: KP +
0x5B, // 7a: KP 3
0x56, // 7b: KP -
0x55, // 7c: KP *
0x6E, // 7d: F19
0x69, // 7e: Scroll Lock
0x00, // 7f: NoEvent
0x00, // 80: NoEvent
0x00, // 81: NoEvent
0x00, // 82: NoEvent
0x40, // 83: F7
};
static const uint16_t ps2usb_ext_lut[] = {
0x00, // 00: NoEvent
0x00, // 01: NoEvent
0x00, // 02: NoEvent
0x00, // 03: NoEvent
0x00, // 04: NoEvent
0x00, // 05: NoEvent
0x00, // 06: NoEvent
0x00, // 07: NoEvent
0x00, // 08: NoEvent
0x00, // 09: NoEvent
0x00, // 0a: NoEvent
0x00, // 0b: NoEvent
0x00, // 0c: NoEvent
0x00, // 0d: NoEvent
0x00, // 0e: NoEvent
0x00, // 0f: NoEvent
0x00, // 10: NoEvent
MOD|RALT, // 11: NoEvent
0x00, // 12: NoEvent
0x00, // 13: NoEvent
MOD|RCTRL, // 14: NoEvent
0x00, // 15: NoEvent
0x00, // 16: NoEvent
0x00, // 17: NoEvent
0x00, // 18: NoEvent
0x00, // 19: NoEvent
0x00, // 1a: NoEvent
0x00, // 1b: NoEvent
0x00, // 1c: NoEvent
0x00, // 1d: NoEvent
0x00, // 1e: NoEvent
MOD|LGUI, // 1f: NoEvent
0x00, // 20: NoEvent
0x00, // 21: NoEvent
0x00, // 22: NoEvent
0x00, // 23: NoEvent
0x00, // 24: NoEvent
0x00, // 25: NoEvent
0x00, // 26: NoEvent
0x00, // 27: NoEvent
0x00, // 28: NoEvent
0x00, // 29: NoEvent
0x00, // 2a: NoEvent
0x00, // 2b: NoEvent
0x00, // 2c: NoEvent
0x00, // 2d: NoEvent
0x00, // 2e: NoEvent
0x65, // 2f: App
0x00, // 30: NoEvent
0x00, // 31: NoEvent
0x00, // 32: NoEvent
0x00, // 33: NoEvent
0x00, // 34: NoEvent
0x00, // 35: NoEvent
0x00, // 36: NoEvent
0x66, // 37: Power
0x00, // 38: NoEvent
0x00, // 39: NoEvent
0x00, // 3a: NoEvent
0x00, // 3b: NoEvent
0x00, // 3c: NoEvent
0x00, // 3d: NoEvent7
0x00, // 3e: NoEvent
0x00, // 3f: NoEvent
0x00, // 40: NoEvent
0x00, // 41: NoEvent
0x00, // 42: NoEvent
0x00, // 43: NoEvent
0x00, // 44: NoEvent
0x00, // 45: NoEvent
0x00, // 46: NoEvent
0x00, // 47: NoEvent
0x00, // 48: NoEvent
0x00, // 49: NoEvent
0x54, // 4a: KP /
0x00, // 4b: NoEvent
0x00, // 4c: NoEvent
0x00, // 4d: NoEvent
0x00, // 4e: NoEvent
0x00, // 4f: NoEvent
0x00, // 50: NoEvent
0x00, // 51: NoEvent
0x00, // 52: NoEvent
0x00, // 53: NoEvent
0x00, // 54: NoEvent
0x00, // 55: NoEvent
0x00, // 56: NoEvent
0x00, // 57: NoEvent
0x00, // 58: NoEvent
0x00, // 59: NoEvent
0x58, // 5a: KP Enter
0x00, // 5b: NoEvent
0x00, // 5c: NoEvent
0x00, // 5d: NoEvent
0x00, // 5e: NoEvent
0x00, // 5f: NoEvent
0x00, // 60: NoEvent
0x00, // 61: NoEvent
0x00, // 62: NoEvent
0x00, // 63: NoEvent
0x00, // 64: NoEvent
0x00, // 65: NoEvent
0x00, // 66: NoEvent
0x00, // 67: NoEvent
0x00, // 68: NoEvent
0x4D, // 69: End
0x00, // 6a: NoEvent
0x50, // 6b: Left Arrow
0x4A, // 6c: Home
0x00, // 6d: NoEvent
0x00, // 6e: NoEvent
0x00, // 6f: NoEvent
0x6B, // 70: insert (for keyrah)
0x4C, // 71: Delete
0x51, // 72: Down Arrow
0x00, // 73: NoEvent
0x4F, // 74: Right Arrow
0x52, // 75: Up Arrow
0x00, // 76: NoEvent
0x00, // 77: NoEvent
0x00, // 78: NoEvent
0x00, // 79: NoEvent
0x4E, // 7a: Page Down
0x00, // 7b: NoEvent
0x46, // 7c: Print Screen
0x4B, // 7d: Page Up
};
void user_io_kbd(unsigned char m, unsigned char *k, uint8_t priority, unsigned short vid, unsigned short pid);


static uint8_t keys[6];
static uint8_t modifier = 0;
static uint8_t ps2ext = 0;
static uint8_t ps2rel = 0;

static uint8_t firsttime = 1;

void ps2_Poll() {
  int k;

  // printf("ps2_Poll\n");

  if (firsttime) {
    modifier = 0;
    memset(keys, 0, sizeof keys);
    ps2_Init();
    // ps2_EnablePort(0, true);
    ps2_EnablePortEx(0, true, 1);
    firsttime = 0;
  }

#if defined(MB2) && !defined(CORE2_IPC_TICKS)
  ipc_MasterTick();
#endif

  int changed = 0;
  while ((k = ps2_GetChar(0)) >= 0) {
	  printf("[%02X]\n", k);
    if (k == 0xe0) {
      ps2ext = 1;
    } else if (k == 0xf0) {
      ps2rel = 1;
    } else {
      uint16_t d = ps2ext && k < (sizeof ps2usb_ext_lut / sizeof ps2usb_ext_lut[0]) ? ps2usb_ext_lut[k] :
                   !ps2ext && k < (sizeof ps2usb_lut / sizeof ps2usb_lut[0]) ? ps2usb_lut[k] : 0;

      if (d & MOD) {
        if (ps2rel) modifier &= ~d;
        else        modifier |= d;
        changed = 1;
      } else {
        if (ps2rel) {
          for (int i=0; i<6; i++) {
            if (keys[i] == (d & 0xff)) {
              keys[i] = 0;
              changed = 1;
            }
          }
        } else {
          for (int i=0; i<6; i++) {
            if (keys[i] == (d & 0xff)) break;
            if (keys[i] == 0) {
              keys[i] = d & 0xff;
              changed = 1;
              break;
            }
          }
        }
      }
      ps2rel = 0;
      ps2ext = 0;
    }
  }

  if (changed) {
    debug(("kbd: %08X ", modifier));
#ifdef DEBUG
    for (int i = 0; i<6; i++) printf("%02X ", keys[i]);
#endif
    user_io_kbd(modifier, keys, UIO_PRIORITY_KEYBOARD, 0, 0);
  }

  // void user_io_kbd(unsigned char m, unsigned char *k, uint8_t priority, unsigned short vid, unsigned short pid);
  // m = modifier, k = buffer of 6 keycodes; priority, vid = 0, pid = 0

}





