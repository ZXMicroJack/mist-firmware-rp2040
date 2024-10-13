#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "attrs.h"
#include "hardware.h"
#include "user_io.h"
#include "xmodem.h"
#include "ikbd.h"
#include "usb.h"
#include "common.h"
#include "drivers/ps2.h"
#include "drivers/fifo.h"
#include "drivers/ipc.h"
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


uint8_t const mod2ps2[] = { 0x14, 0x12, 0x11, 0x1f, 0x14, 0x59, 0x11, 0x27 };
uint8_t const hid2ps2[] = {
  0x00, 0x00, 0xfc, 0x00, 0x1c, 0x32, 0x21, 0x23, 0x24, 0x2b, 0x34, 0x33, 0x43, 0x3b, 0x42, 0x4b,
  0x3a, 0x31, 0x44, 0x4d, 0x15, 0x2d, 0x1b, 0x2c, 0x3c, 0x2a, 0x1d, 0x22, 0x35, 0x1a, 0x16, 0x1e,
  0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46, 0x45, 0x5a, 0x76, 0x66, 0x0d, 0x29, 0x4e, 0x55, 0x54,
  0x5b, 0x5d, 0x5d, 0x4c, 0x52, 0x0e, 0x41, 0x49, 0x4a, 0x58, 0x05, 0x06, 0x04, 0x0c, 0x03, 0x0b,
  0x83, 0x0a, 0x01, 0x09, 0x78, 0x07, 0x7c, 0x7e, 0x7e, 0x70, 0x6c, 0x7d, 0x71, 0x69, 0x7a, 0x74,
  0x6b, 0x72, 0x75, 0x77, 0x4a, 0x7c, 0x7b, 0x79, 0x5a, 0x69, 0x72, 0x7a, 0x6b, 0x73, 0x74, 0x6c,
  0x75, 0x7d, 0x70, 0x71, 0x61, 0x2f, 0x37, 0x0f, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40,
  0x48, 0x50, 0x57, 0x5f
};

uint8_t const e0mask = 0xd8;

static uint8_t kbdkeys[6];
static uint8_t modifier = 0;
static uint8_t ps2ext = 0;
static uint8_t ps2rel = 0;

static uint8_t firsttime = 1;

static uint8_t prev_modifier = 0;


static uint32_t pressed[256/32];
static uint8_t hidreport[6];

static int isExtended(uint8_t data) {
  return (data == 0x46 ||
     data >= 0x48 && data <= 0x52 ||
     data == 0x54 || data == 0x58 ||
     data == 0x65 || data == 0x66 ||
     data >= 0x81);
}

static uint8_t curr_legacy_mode = DEFAULT_MODE;

static int mousex = 0, mousey = 0, mouseb = 0;
static int mouseindex = -1;
static char mousereport[3];

void usb_ToPS2(uint8_t modifier, uint8_t keys[6]) {
  debug(("usb_ToPS2: modifier %02X keys %02X %02X %02X %02X %02X %02X legacy %d\n",
    modifier, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], curr_legacy_mode));

  if (curr_legacy_mode != LEGACY_MODE) return;

  static int firsttime = 1;
  uint32_t newpressed[8] = {0,0,0,0,0,0,0,0};

  uint8_t ps2[33];
  uint8_t nrps2 = 0;

  ps2[nrps2++] = 0; // keyboard

  // first time initialise
  if (firsttime) {
    firsttime = 0;
    memset(pressed, 0, sizeof pressed);
    memset(hidreport, 0, sizeof hidreport);
  }

  // detect newly pressed
  for (int i=0; i<6; i++) {
    uint8_t off = keys[i] >> 5;
    uint32_t bit = 1 << (keys[i] & 0x1f);

    if ((pressed[off] & bit) == 0) {
      // not previously pressed
      pressed[off] |= bit;

      // indicate pressed
      if (isExtended(keys[i])) ps2[nrps2++] = 0xe0;
      ps2[nrps2++] = hid2ps2[keys[i]];
    }
    newpressed[off] |= bit;
  }

  for (int i=0; i<6; i++) {
    uint8_t off = hidreport[i] >> 5;
    uint32_t bit = 1 << (hidreport[i] & 0x1f);

    if ((newpressed[off] & bit) == 0) {
      // indicate released
      pressed[off] &= ~bit;

      if (isExtended(hidreport[i])) ps2[nrps2++] = 0xe0;
      ps2[nrps2++] = 0xf0;
      ps2[nrps2++] = hid2ps2[hidreport[i]];
    }
  }

  // copy latest report
  memcpy(hidreport, keys, sizeof hidreport);

  // handle modifiers
  uint8_t m = 0x01;
  for (int i=0; i<8; i++) {
      if (!(prev_modifier&m) && (modifier&m)) {
        if (e0mask&m) ps2[nrps2++] = 0xe0;
        ps2[nrps2++] = mod2ps2[i];
      }
      if ((prev_modifier&m) && !(modifier&m)) {
        if (e0mask&m) ps2[nrps2++] = 0xe0;
        ps2[nrps2++] = 0xf0;
        ps2[nrps2++] = mod2ps2[i];
      }
      m <<= 1;
  }
  prev_modifier = modifier;

#ifdef MB2
  if (nrps2 > 1) mb2_SendPS2(ps2, nrps2);
#else
  if (nrps2 > 1) {
    for (int i=1; i<nrps2; i++) {
      ps2_SendChar(0, ps2[i]);
    }
  }
#endif
}

void usb_ToPS2Mouse(uint8_t report[], uint16_t len) {
  if (curr_legacy_mode != LEGACY_MODE) return;
  
  if (len >= 3) {
      uint8_t ps2[4];
      ps2[0] = 1;
      ps2[1] = (report[0] & 7) | 0x08;
      ps2[2] = report[1]; // x
      ps2[3] = -report[2]; // y

#ifdef MB2
      mb2_SendPS2(ps2, 4);
#else
      for (int i=1; i<4; i++) {
        ps2_SendChar(1, ps2[i]);
      }
#endif
  }
}


#ifndef MB2
uint64_t mouse_enable_at = 0;
int mousestandoff = 0;
#define MOUSE_POST_RESET_ENABLE   1000000
#endif

void ps2_Poll() {
  int k;

  if (firsttime) {
    ps2_Init();
    modifier = 0;
    memset(kbdkeys, 0, sizeof kbdkeys);
    firsttime = 0;
  }

  if (curr_legacy_mode != legacy_mode) {
    if (legacy_mode == LEGACY_MODE) {
      ps2_EnablePortEx(0, false, 1);
      ps2_EnablePortEx(0, true, 0);
      ps2_EnablePortEx(1, false, 1);
      ps2_EnablePortEx(1, true, 0);
      ps2_SwitchMode(0);
    } else {
      ps2_EnablePortEx(0, false, 0);
      ps2_EnablePortEx(0, true, 1);
      ps2_EnablePortEx(1, false, 0);
      ps2_EnablePortEx(1, true, 1);
      ps2_SwitchMode(1);
#ifndef MB2
      // reset
      ps2_SendChar(0, 0xff);
      ps2_SendChar(1, 0xff);
      mouse_enable_at = time_us_64() + MOUSE_POST_RESET_ENABLE;
#endif
    }
    curr_legacy_mode = legacy_mode;
  }

#ifndef MB2
  /* issue mouse enable */
  if (mouse_enable_at != 0 && time_us_64() > mouse_enable_at) {
    ps2_SendChar(1, 0xf4);
    mouse_enable_at = 0;
    mousestandoff = 10;
  }
#endif

#if defined(MB2) && !defined(CORE2_IPC_TICKS)
  ipc_MasterTick();
#endif

  int changed = 0;
  while ((k = ps2_GetChar(0)) >= 0) {
	  debug(("[K%02X]\n", k));
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
          debug(("release {%03X}\n", d));
          for (int i=0; i<6; i++) {
            if (kbdkeys[i] == (d & 0xff)) {
              kbdkeys[i] = 0;
              changed = 1;
            }
          }
        } else {
          debug(("pressed {%03X}\n", d));
          for (int i=0; i<6; i++) {
            if (kbdkeys[i] == (d & 0xff)) break;
            if (kbdkeys[i] == 0) {
              kbdkeys[i] = d & 0xff;
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
    for (int i = 0; i<6; i++) printf("%02X ", kbdkeys[i]);
    printf("\n");
#endif
    uint8_t keys[6];
    memcpy(keys, kbdkeys, 6);
    user_io_kbd(modifier, keys, UIO_PRIORITY_KEYBOARD, 0, 0);
  }

  /* scan mouse messages */
  while ((k = ps2_GetChar(1)) >= 0) {
	  debug(("[M%02X]\n", k));
#ifndef MB2
    if (mousestandoff) mousestandoff --;
    else 
#endif
    if (mouseindex == -1) {
      // find bit that is always set
      if (k & 0x08) {
        mousereport[0] = k;
        mouseindex = 1;
      }
    } else {
      mousereport[mouseindex++] = k;
      /* sync bit lost reset sync */
      if ((mousereport[0] & 0x08) != 0x08 )
        mouseindex = -1;
      else if (mouseindex >= 3) {
        /* got full report process */
        mouseindex = 0;
        debug(("mouse x %d y %d b %x\n", mousereport[1], mousereport[2], mousereport[0] & 7));

        user_io_mouse(0, mousereport[0] & 7, mousereport[1], -mousereport[2], 0);
      }
    }
  }

#ifndef MB2
  ps2_HealthCheck();
#endif
  // void user_io_kbd(unsigned char m, unsigned char *k, uint8_t priority, unsigned short vid, unsigned short pid);
  // m = modifier, k = buffer of 6 keycodes; priority, vid = 0, pid = 0

}





