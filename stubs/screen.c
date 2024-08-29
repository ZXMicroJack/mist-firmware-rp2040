/* This file is part of fpga-spec by ZXMicroJack - see LICENSE.txt for moreinfo */
#include <stdint.h>

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
// #include "Z80.h"

#include <stdio.h>
#include <stdint.h>

#include "user_io.h"

SDL_Surface *screen;
uint8_t mem[0x10000];
uint8_t specKeys[8];
int quit = 0;
int debug = 0;

unsigned long zxColours[] = {
  0x000000, 0x0000d7, 0xd70000, 0xd700d7,
  0x00d700, 0x00d7d7, 0xd7d700, 0xd7d7d7,
  0x000000, 0x0000ff, 0xff0000, 0xff00ff,
  0x00ff00, 0x00ffff, 0xffff00, 0xffffff
};


Uint32 colourLut[16];

void (*pupdateScreen)() = (void (*)())NULL;

void initScreenReal();

void pollKeyboard();
void updateScreen() {
  /* Lock the screen for direct access to the pixels */
  if ( SDL_MUSTLOCK(screen) ) {
      if ( SDL_LockSurface(screen) < 0 ) {
          fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
          return;
      }
  }

  pupdateScreen();

  if ( SDL_MUSTLOCK(screen) ) {
      SDL_UnlockSurface(screen);
  }

  /* Update just the part of the display that we've changed */
  SDL_UpdateRect(screen, 0, 0, 256*2, 192*2);

	pollKeyboard();
}

void initScreen(void (*scrUpd)()) {
	pupdateScreen = scrUpd;
	initScreenReal();
}

void initScreenReal() {
  /* Initialize the SDL library */
  if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
     fprintf(stderr,
             "Couldn't initialize SDL: %s\n", SDL_GetError());
     exit(1);
  }

  /* Clean up on exit */
  atexit(SDL_Quit);
  /*
  * Initialize the display in a 640x480 8-bit palettized mode,
  * requesting a software surface
  */
  screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);
  if ( screen == NULL ) {
     fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",
                     SDL_GetError());
     exit(1);
  }

  for (int i=0; i<sizeof zxColours / sizeof zxColours[0]; i++) {
    /* Map the color yellow to this display (R=0xff, G=0xFF, B=0x00)
       Note:  If the display is palettized, you must set the palette first.
    */
    colourLut[i] = SDL_MapRGB(screen->format, zxColours[i]>>16,
      (zxColours[i]>>8) & 0xff, zxColours[i] & 0xff);
  }

  memset(specKeys, 0xff, sizeof specKeys);
  quit = 0;
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel_ll(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

#define SCX 2
#define SCY 4

void putpixel(int x, int y, uint32_t pixel) {
  for (int x1=0; x1<SCX; x1++)
    for (int y1=0; y1<SCY; y1++)
      putpixel_ll(screen, 16+x*SCX+x1, y*SCY+y1, pixel);
}


void updateScreenSpectrum() {
  for (int i=191; i>=0; i--) {
    for (int j=0; j<32; j++) {
      int l = (i/64) * 2048;
      l += (i & 7) * 256;
      l += ((i & 63) / 8) * 32;

      uint8_t d = mem[16384+l+j];
      int y = i / 8;
      uint8_t attr = mem[16384+2048*3+y*32+j];
      Uint32 rgbInk = colourLut[(attr & 7) + ((attr&0x40) ? 8 : 0)];// ^ 0xffffff;
      Uint32 rgbPaper = colourLut[((attr >> 3) & 7) + ((attr&0x40) ? 8 : 0)];// ^ 0xffffff;
      for (int k=0; k<8; k++) {
        putpixel(j*8+k, i, (d & 0x80) ? rgbInk : rgbPaper);
        d <<= 1;
      }
    }
  }
}

//0xfefe  SHIFT, Z, X, C, V            0xeffe  0, 9, 8, 7, 6
//0xfdfe  A, S, D, F, G                0xdffe  P, O, I, U, Y
//0xfbfe  Q, W, E, R, T                0xbffe  ENTER, L, K, J, H
//0xf7fe  1, 2, 3, 4, 5                0x7ffe  SPACE, SYM SHFT, M, N, B
uint8_t scancodeLut[8][5] = {
	{SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v},
	{SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g},
	{SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t},
	{SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5},

	{SDLK_0, SDLK_9, SDLK_8, SDLK_7, SDLK_6},
	{SDLK_p, SDLK_o, SDLK_i, SDLK_u, SDLK_y},
	{SDLK_RETURN, SDLK_l, SDLK_k, SDLK_j, SDLK_h},
	{SDLK_SPACE, SDLK_LCTRL, SDLK_m, SDLK_n, SDLK_b}
};

static uint32_t pressed[256/32];
static uint8_t hidreport[6];
uint8_t kbdkeys[6] = {0};
uint8_t modifier = 0;

void usbaction(char c);

void processKey(uint16_t scancode, int pressed) {
  uint8_t d = 0;
	printf("processKey %02X pressed %d\n", scancode, pressed);
  switch (scancode) {
    case SDLK_UP:
      d = 0x52;
      break;
    case SDLK_DOWN:
      d = 0x51;
      break;
    case SDLK_LEFT:
      d = 0x50;
      break;
    case SDLK_RIGHT:
      d = 0x4F;
      break;
    case SDLK_F12:
      d = 0x45;
      break;
    case SDLK_RETURN:
      d = 0x28;
      break;
    case SDLK_SPACE:
      d = 0x2C;
      break;
    case SDLK_a: if (pressed) usbaction('a'); break;
    case SDLK_b: if (pressed) usbaction('b'); break;
    case SDLK_c: if (pressed) usbaction('c'); break;
    case SDLK_d: if (pressed) usbaction('d'); break;
    case SDLK_e: if (pressed) usbaction('e'); break;
    case SDLK_f: if (pressed) usbaction('f'); break;
    case SDLK_r: if (pressed) usbaction('r'); break;
    case SDLK_i: if (pressed) usbaction('i'); break;
    case SDLK_1: if (pressed) usbaction('1'); break;
    case SDLK_2: if (pressed) usbaction('2'); break;
    case SDLK_3: if (pressed) usbaction('3'); break;
    case SDLK_4: if (pressed) usbaction('4'); break;
  }

    uint8_t keys[6];
    int changed = 0;

    if (d) {
      if (!pressed) {
        // debug(("release {%03X}\n", d));
        for (int i=0; i<6; i++) {
          if (kbdkeys[i] == (d & 0xff)) {
            kbdkeys[i] = 0;
            changed = 1;
          }
        }
      } else {
        // debug(("pressed {%03X}\n", d));
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

    if (changed) {
      memcpy(keys, kbdkeys, 6);
      user_io_kbd(modifier, keys, UIO_PRIORITY_KEYBOARD, 0, 0);
    }



#if 0
	for (int r = 0; r < 8; r++) {
		uint8_t mask = 0x01;
		for (int c = 0; c<5; c++) {
			if (scancode == scancodeLut[r][c]) {
				if (pressed) {
					specKeys[r] &= ~mask;
				} else {
					specKeys[r] |= mask;
				}
				printf("row %d column %d kb %02X\n", r, c, specKeys[r]);
				return;
			}
			mask <<= 1;
		}
	}
#endif
}

void pollKeyboard() {
	SDL_Event event;

/* Poll for events */
	while( SDL_PollEvent( &event ) ){
			switch( event.type ){
					/* Keyboard event */
					/* Pass the event data onto PrintKeyInfo() */
					case SDL_KEYDOWN:
					case SDL_KEYUP:
							if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
								quit = 1;
							} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB) {
								debug = !debug;
							}

							processKey(event.key.keysym.sym, event.type == SDL_KEYDOWN);
							break;

					/* SDL_QUIT event (window close) */
					case SDL_QUIT:
							quit = 1;
							break;

					default:
							break;
			}
	}
  if (quit) exit(1);
}
