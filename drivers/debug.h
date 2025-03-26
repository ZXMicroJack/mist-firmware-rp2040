#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
void hexdump1(uint8_t *buf, int len);
#ifdef PIODEBUG
void debuginit();
int dbgprintf(const char *fmt, ...);
#define debug(a) dbgprintf a
#elif defined(USB)
extern uint8_t usbdebug;
int usbprintf(const char *fmt, ...);
#define debug(a) usbprintf a
#else
#define debuginit()
#define debug(a) printf a
#endif
#else
#define debuginit()
#define debug(a)
#undef hexdump1
#define hexdump1(a,b)
#endif


#endif
