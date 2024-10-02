#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include <string.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "pins.h"
#include "ps2.h"
#include "fifo.h"
// #define DEBUG
#include "debug.h"
#include "keymap.h"


// #define N_ROWS  9
// #define N_COLS  6

typedef enum { zx, cpc, msx, c64, at8, bbc, aco, ap2, vic, ori, sam, jup, fus, pc, pcxt, kbext } KBMODE;

static uint16_t keyheld_map[COLS];
static struct repeating_timer kbd_timer;
static int col = 0;
static fifo_t kbd_fifo;
static uint8_t kbd_fifo_buf[64];
static fifo_t ps2_fifo;
static uint8_t ps2_fifo_buf[64];
static uint8_t opqa_cursors = 0;

static uint8_t shift_held = 0;
static uint8_t ext_held = 0;
static uint8_t sym_held = 0;
static uint8_t fn_held = 0;
static uint8_t xfn_held = 0;

//#define WAIT  0xff
#define WAIT PS2_WAIT_SCANCODE

const uint8_t spacedot[] = { 0xf0, 0x12, 0xf0, 0x14, KEY_SPACE, WAIT, 0xf0, KEY_SPACE, KEY_PUNTO, WAIT, 0xf0, KEY_PUNTO, WAIT, WAIT}; 

const uint8_t nomZX[] = { 2,KEY_Z,KEY_X,1 }; //Numero de Letras,(letras[n],,),CKm)
const uint8_t nomCPC[] = { 3,KEY_C,KEY_P,KEY_C,4 };
const uint8_t nomMSX[] = { 3,KEY_M,KEY_S,KEY_X,4 };
const uint8_t nomC64[] = { 3,KEY_C,KEY_6,KEY_4,4 };
const uint8_t nomAT8[] = { 5,KEY_A,KEY_T,KEY_A,KEY_R,KEY_I,4 };
const uint8_t nomBBC[] = { 3,KEY_B,KEY_B,KEY_C,4 };
const uint8_t nomACO[] = { 5,KEY_A,KEY_C,KEY_O,KEY_R,KEY_N,4 };
const uint8_t nomVIC[] = { 3,KEY_V,KEY_I,KEY_C,4 };
const uint8_t nomORI[] = { 4,KEY_O,KEY_R,KEY_I,KEY_C,4 };
const uint8_t nomSAM[] = { 3,KEY_S,KEY_A,KEY_M,4 };
const uint8_t nomJUP[] = { 7,KEY_J,KEY_U,KEY_P,KEY_I,KEY_T,KEY_E,KEY_R,4 };
const uint8_t nomAP2[] = { 5,KEY_A,KEY_P,KEY_P,KEY_L,KEY_E,4 };
const uint8_t nomPC[] = { 2,KEY_P,KEY_C,4 };
const uint8_t nomPCXT[] = { 4,KEY_P,KEY_C,KEY_X,KEY_T,4 };
const uint8_t nomKBEXT[] = { 5,KEY_K,KEY_B,KEY_E,KEY_X,KEY_T,4 };
const uint8_t nomFUS[] = { 6,KEY_F,KEY_U,KEY_S,KEY_I,KEY_O,KEY_N,1 };

const uint8_t *modeCode[] = {
  nomZX, nomCPC, nomMSX, nomC64, nomAT8, nomBBC, nomACO, nomAP2, nomVIC, nomPCXT, nomORI, nomSAM, nomJUP, nomFUS, nomPC, nomKBEXT
};
const uint8_t mode_scancodes[] = {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_A, KEY_B, KEY_C};

static uint8_t next_key_mode = 0;


static uint8_t codeset_xt = 0;
static KBMODE kbmode = zx;

static bool kbd_timer_callback(struct repeating_timer *t) {
  uint16_t rows = gpio_get_all() & 0x1ff;
  if (keyheld_map[col] ^ rows) {
    uint16_t mask = 0x100;
    uint16_t row_nr = 0;
    while (mask) {
      // key changed state
      if ((keyheld_map[col]&mask) ^ (rows&mask)) {
        uint8_t scancode = row_nr | (col << 4);
        if (keyheld_map[col]&mask) {
          fifo_Put(&kbd_fifo, scancode | 0x80);
        } else {
          fifo_Put(&kbd_fifo, scancode);
        }
      }
      mask >>= 1;
      row_nr ++;
    }
    keyheld_map[col] = rows;
  }
  
  gpio_put(GPIO_KCOL(col), 0);
  col ++;
  if (col >= COLS) col = 0;
  gpio_put(GPIO_KCOL(col), 1);
  return true;
}


static uint64_t waittime;

void kbd_Init() {
  for (int i=0; i<COLS; i++) {
    gpio_init(GPIO_KCOL(i));
    gpio_set_dir(GPIO_KCOL(i), GPIO_OUT);
    gpio_put(GPIO_KCOL(i), 0);
  }

  for (int i=0; i<ROWS; i++) {
    gpio_init(GPIO_KROW(i));
    gpio_set_dir(GPIO_KROW(i), GPIO_IN);
  }

  fifo_Init(&kbd_fifo, kbd_fifo_buf, sizeof kbd_fifo_buf);
  fifo_Init(&ps2_fifo, ps2_fifo_buf, sizeof ps2_fifo_buf);
  memset(keyheld_map, 0, sizeof keyheld_map);
  add_repeating_timer_us(200, kbd_timer_callback,
                         NULL, &kbd_timer);
}

static int get_mode(uint8_t scancode) {
  for (int i=0; i<sizeof mode_scancodes; i++) {
    if (scancode == mode_scancodes[i])
      return i;
  }
  return -1;
}

#define KBD_WAIT_TIME     100000

int kbd_Get() {
  int r = -1;
  if (!fifo_Empty(&ps2_fifo)) {
    uint64_t now = time_us_64();
    if (!waittime || waittime < now) {
      int c = fifo_Get(&ps2_fifo);
      if (c == 0xff) {
        waittime = now + KBD_WAIT_TIME;
      } else {
        r = c;
      }
    }
  }
  return r;
}

static void send_key_raw(uint16_t key, uint8_t pressed) {
  if (!key || key == NADA) return;
  if (codeset_xt) {
    if (pressed) {
      if (key & E) fifo_Put(&ps2_fifo, 0xE0);
      if (key & S) fifo_Put(&ps2_fifo, KS1_LSHIFT);
      fifo_Put(&ps2_fifo, key & 0xff);
    } else {
      if (key & E) fifo_Put(&ps2_fifo, 0xE0);
      fifo_Put(&ps2_fifo, KS1_RELEASE | key & 0xff);
      if (key & S) fifo_Put(&ps2_fifo, KS1_RELEASE | KS1_LSHIFT);
    }
  } else {
    if ((key & S) && pressed) {
      fifo_Put(&ps2_fifo, KEY_LSHIFT);
    }
    if (key & E) fifo_Put(&ps2_fifo, 0xe0);
    if (!pressed) fifo_Put(&ps2_fifo, 0xf0);
    fifo_Put(&ps2_fifo, key & 0xff);
    if ((key & S) && !pressed) {
      fifo_Put(&ps2_fifo, 0xf0);
      fifo_Put(&ps2_fifo, KEY_LSHIFT);
    }
  }
}

static void send_key(uint16_t code, uint8_t pressed) {
  uint32_t keys;
  
  uint8_t momentary_or_released = ((!pressed && !(code & M)) || (pressed && (code & M)));
  
  if (code & K) { // key combination
    int kc = code & 0x7f;
    if (pressed) {
      for (int i=0; i<MAX_KEY_COMB; i++) {
        send_key_raw(kc_lut[kc][i], true);
      }
    }
    if (code & M) fifo_Put(&ps2_fifo, WAIT);
    if (momentary_or_released) {
      for (int i=MAX_KEY_COMB-1; i>=0; i--) {
        send_key_raw(kc_lut[kc][i], false);
      }
    }
  } else {
    if (pressed) send_key_raw(code, true);
    if (code & M) fifo_Put(&ps2_fifo, WAIT);
    if (momentary_or_released) send_key_raw(code, false);
  }
}

static void update_shifts(uint8_t scancode, uint8_t pressed) {
  if (scancode == 0x03) shift_held = pressed;
  if (scancode == 0x20) fn_held = pressed;
  if (scancode == 0x11) sym_held = pressed;
  if (scancode == 0x30) ext_held = pressed;
  xfn_held = fn_held || (shift_held && sym_held);
}

void kbd_Process() {
  int ch = fifo_Get(&kbd_fifo);
  while (ch >= 0) {
    int row = 8 - (ch & 0xf);
    int col = (ch&0x7f) >> 4;
    int pressed = 1 ^ (ch >> 7);
    int scancode = ch&0x7f;
    
    debug(("row %d col %d\n", row, col));

    if (!pressed) {
      update_shifts(scancode, pressed);
    }
    
    // detect change mode
    if (next_key_mode && pressed) {
      // TODO possibly unengage shifts.
      debug(("change mode \n"));
      int new_mode = get_mode(mapZX[row][col]);
      if (new_mode >= 0) {
        kbmode = (KBMODE)new_mode;
        codeset_xt = new_mode == pcxt;
        
        //TODO should be sending in XT scancodes?
        //TODO set mode setting from AT?
        // send mode key
        for (int i=0; i<sizeof spacedot; i++) {
          fifo_Put(&ps2_fifo, spacedot[i]);
        }
        
        uint8_t *keys = modeCode[kbmode];
        for (int i=1; i<=keys[0]; i++) {
          fifo_Put(&ps2_fifo, keys[i]);
          fifo_Put(&ps2_fifo, WAIT);
          fifo_Put(&ps2_fifo, 0xf0);
          fifo_Put(&ps2_fifo, keys[i]);
          fifo_Put(&ps2_fifo, WAIT);
        }
      }
      next_key_mode = 2;
    }
    else if (next_key_mode == 2 && !pressed) {
      next_key_mode = 0;
      
    }
    else if (xfn_held) { // handle FN special keys
      int key = mapZX[row][col];
      switch(key) {
        case KEY_X: // save
          if (pressed) {
            // TODO: save config to flash?
#ifdef DEV_BUILD
            reset_usb_boot(0, 0);
#endif
          }
          break;
        case KEY_V: // display firmware version
          if (pressed) {
            // TODO: do this or not?
          }
          break;
        case KEY_U:
          if (pressed) {
            next_key_mode = 1;
          }
          break;
        case KEY_C:
          if (pressed) {
            opqa_cursors = !opqa_cursors;
          }
          break;
#if 0 // for testing jamma
        case KEY_5:
          if (pressed) {
            jamma_SetData(0, 0x80);
          } else {
            jamma_SetData(0, 0x00);
          }
          break;
#endif          
        default:
          
          if (mapFN[row][col] && !codeset_xt) {
            debug(("ps2(fn): %02X\n", mapFN[row][col]));
            send_key(mapFN[row][col], pressed);
          } else if (mapFN1[row][col] && codeset_xt) {
            debug(("ps2(fn): %02X\n", mapFN1[row][col]));
            send_key(mapFN1[row][col], pressed);
          }

          break;
      }
    }
    else if (kbmode == zx) {
      // press normal key
      if (mapZX[row][col]) {
        debug(("ps2(normal): %02X\n", mapZX[row][col]));
        send_key(mapZX[row][col], pressed);
      }
    }
    else if (kbmode == cpc) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == msx) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapMSX_sym[row][col]));
        send_key(mapMSX_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == c64) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapC64_sym[row][col]));
        send_key(mapC64_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == at8) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapAT8_sym[row][col]));
        send_key(mapAT8_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == bbc) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapBBC_sym[row][col]));
        send_key(mapBBC_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == aco) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapACO_sym[row][col]));
        send_key(mapACO_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == ap2) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapAP2_sym[row][col]));
        send_key(mapAP2_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == vic) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapVIC_sym[row][col]));
        send_key(mapVIC_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == ori) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapORI_sym[row][col]));
        send_key(mapORI_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == sam) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapSAM_sym[row][col]));
        send_key(mapSAM_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == jup) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapJUP_sym[row][col]));
        send_key(mapJUP_sym[row][col], pressed);
      } else if (shift_held && mapCPC_shifted[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC_shifted[row][col], pressed);
      } else if (mapCPC[row][col]) {
        debug(("ps2(normal): %02X\n", mapCPC[row][col]));
        send_key(mapCPC[row][col], pressed);
      }
    }
    else if (kbmode == fus) {
      if (sym_held) {
        debug(("ps2(normal): %02X\n", mapFUS_sym[row][col]));
        send_key(mapFUS_sym[row][col], pressed);
      } else if (shift_held && mapFUS[row][col]) {
        debug(("ps2(normal): %02X\n", mapFUS[row][col]));
        send_key(mapFUS[row][col], pressed);
      } else if (mapFUS[row][col]) {
        debug(("ps2(normal): %02X\n", mapFUS[row][col]));
        send_key(mapFUS[row][col], pressed);
      }
    }
    else if (kbmode == pc || kbmode == kbext || kbmode == pcxt) {
      if (codeset_xt) {
        if (mapXT1[row][col] && sym_held) {
          send_key(mapXT1[row][col], pressed);
        } else if (shift_held && mapEXT1[row][col]) {
          send_key(mapEXT1[row][col], pressed);
        } else if (mapSET1[row][col]) {
          send_key(opqa_cursors ? mapSET1opqa[row][col] : mapSET1[row][col], pressed);
        }
      } else {
        if (sym_held) {
          debug(("ps2(normal): %02X\n", mapPC_sym[row][col]));
          send_key(mapPC_sym[row][col], pressed);
        } else if (shift_held && mapPC[row][col]) {
          debug(("ps2(normal): %02X\n", mapPC[row][col]));
          send_key(mapPC_shifted[row][col], pressed);
        } else if (mapPC[row][col]) {
          debug(("ps2(normal): %02X\n", mapPC[row][col]));
          send_key(opqa_cursors ? mapPCopqa[row][col] : mapPC[row][col], pressed);
        }
      }
    }

    if (pressed) {
      update_shifts(scancode, pressed);
    }
    debug(("Scancode %02X\n", ch));
    ch = fifo_Get(&kbd_fifo);
  }
}


