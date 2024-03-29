#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
void hexdump(uint8_t *buf, int len);
#ifdef PIODEBUG
void debuginit();
int dbgprintf(const char *fmt, ...);
#define debug(a) dbgprintf a
#else
#define debuginit()
#define debug(a) printf a
#endif
#else
#define debuginit()
#define debug(a)
#define hexdump(a,b)
#endif


#endif
