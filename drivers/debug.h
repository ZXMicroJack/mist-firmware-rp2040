#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
#define debug(a) printf a
void hexdump(uint8_t *buf, int len);
#else
#define debug(a)
#define hexdump(a,b)
#endif


#endif
