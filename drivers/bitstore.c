#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bitstore.h"
// #define DEBUG
#include "debug.h"

#define CHUNKSIZE   1024
#define N_SYMBOLS 256

//#define TRACE_CODES
//#define TRACE_CODES_D

static const uint16_t std_hcodes[N_SYMBOLS] = {
0x3000, 0x4005, 0x500C, 0x500E, 0x5018, 0x501D, 0x501F, 0x6008,
0x600A, 0x600B, 0x600C, 0x600F, 0x6011, 0x6012, 0x601A, 0x601B,
0x6022, 0x6023, 0x6024, 0x6025, 0x6026, 0x6027, 0x6029, 0x6036,
0x6037, 0x701C, 0x7026, 0x703F, 0x7042, 0x7051, 0x7055, 0x7057,
0x7058, 0x7059, 0x705D, 0x705F, 0x7069, 0x8026, 0x8027, 0x8036,
0x803B, 0x80A0, 0x80A1, 0x80B5, 0x80B6, 0x80B7, 0x80B8, 0x80BD,
0x80CA, 0x80CB, 0x80CC, 0x80CD, 0x80CF, 0x80D6, 0x80E6, 0x80F1,
0x904A, 0x904B, 0x9068, 0x9069, 0x906A, 0x906E, 0x9083, 0x9084,
0x9086, 0x909C, 0x909D, 0x909F, 0x90F0, 0x90F2, 0x90F4, 0x9100,
0x9106, 0x9107, 0x910D, 0x910E, 0x9150, 0x9151, 0x9152, 0x9158,
0x9169, 0x9173, 0x9178, 0x9190, 0x919C, 0x91A3, 0x91A9, 0x91AA,
0x91AF, 0x91C1, 0x91C3, 0x91C5, 0x91C7, 0x91C8, 0x91C9, 0x91CA,
0x91CB, 0x91E5, 0x91E6, 0x91E7, 0x91ED, 0x91EE, 0xA090, 0xA091,
0xA092, 0xA093, 0xA0D6, 0xA0D7, 0xA0DE, 0xA0DF, 0xA0E8, 0xA0E9,
0xA0EA, 0xA0EB, 0xA100, 0xA101, 0xA102, 0xA103, 0xA104, 0xA105,
0xA10A, 0xA10B, 0xA10F, 0xA13C, 0xA13D, 0xA1E2, 0xA1E3, 0xA1E6,
0xA1E7, 0xA1EB, 0xA1EC, 0xA1ED, 0xA1EE, 0xA1EF, 0xA1F0, 0xA1F1,
0xA1F2, 0xA1F4, 0xA1F5, 0xA1F6, 0xA1F7, 0xA202, 0xA203, 0xA204,
0xA205, 0xA207, 0xA208, 0xA209, 0xA20A, 0xA20B, 0xA218, 0xA21F,
0xA2A7, 0xA2B3, 0xA2B4, 0xA2B5, 0xA2B6, 0xA2B7, 0xA2D0, 0xA2E5,
0xA2F3, 0xA322, 0xA323, 0xA324, 0xA325, 0xA327, 0xA33A, 0xA33B,
0xA340, 0xA342, 0xA357, 0xA35D, 0xA380, 0xA381, 0xA385, 0xA388,
0xA389, 0xA38C, 0xA39D, 0xA39E, 0xA39F, 0xA3C2, 0xA3C3, 0xA3D1,
0xA3D4, 0xA3D7, 0xA3D8, 0xA3DF, 0xB21C, 0xB21D, 0xB3D4, 0xB3D5,
0xB3E6, 0xB3E7, 0xB40C, 0xB40D, 0xB432, 0xB433, 0xB43C, 0xB43D,
0xB54C, 0xB54D, 0xB564, 0xB565, 0xB5A2, 0xB5A3, 0xB5C8, 0xB5C9,
0xB5E4, 0xB5E5, 0xB64C, 0xB64D, 0xB682, 0xB683, 0xB686, 0xB687,
0xB688, 0xB689, 0xB68A, 0xB68B, 0xB6A0, 0xB6A1, 0xB6A2, 0xB6A3,
0xB6AC, 0xB6AD, 0xB6B8, 0xB6B9, 0xB708, 0xB709, 0xB71A, 0xB71B,
0xB738, 0xB739, 0xB780, 0xB781, 0xB782, 0xB783, 0xB790, 0xB791,
0xB792, 0xB793, 0xB7A0, 0xB7A1, 0xB7A4, 0xB7A5, 0xB7A6, 0xB7A7,
0xB7AA, 0xB7AB, 0xB7AC, 0xB7AD, 0xB7B2, 0xB7B3, 0xB7BC, 0xB7BD,
};

typedef struct chunk {
  uint8_t data[CHUNKSIZE];
  struct chunk *next;
} chunk_t;

static uint16_t lastblock = 0;
static uint16_t nr_chunks = 0;
static chunk_t *store = NULL;
static chunk_t *latest = NULL;
static int cursor = 0;

static void bitstore_add() {
  nr_chunks ++;
  chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
  chunk->next = NULL;

  if (!store) {
    store = latest = chunk;
  } else {
    latest->next = chunk;
    latest = latest->next;
  }
  debug(("chunk %d\n", nr_chunks));
}

static void bitstore_put(uint8_t data) {
  if (latest == NULL || lastblock == CHUNKSIZE) {
    bitstore_add();
    lastblock = 0;
  }
  latest->data[lastblock++] = data;
}

static int bitstore_get(uint8_t *data, int len) {
  int read = 0;
  while (len--) {
    if (cursor >= (store->next == NULL ? lastblock : CHUNKSIZE)) {
      chunk_t *this_one = store;
      store = store->next;
      free(this_one);
      cursor = 0;

      if (store == NULL) break;
    }
    *data++ = store->data[cursor++];
    read ++;
  }
  return read;
}

#if 0
static int huff_get(void) {
  if (store == NULL) return -1;
  
  if (cursor >= (store->next == NULL ? lastblock : CHUNKSIZE)) {
    chunk_t *this_one = store;
    store = store->next;
    free(this_one);
    cursor = 0;

    if (store == NULL) return -1; /* end of stream */
  }
  return store->data[cursor++];
}
#endif

////////////////////////////////////////////////////////////////////
// Huffman state variables
static uint8_t reverse_order[256];
static uint8_t huff_l = 0;
static uint8_t huff_d = 0;
static uint32_t huff_size = 0;
static uint8_t huff_byteorder[256];
static uint8_t huff_init = 0;

static uint8_t d = 0x00;
static uint8_t l = 8;
static unsigned short code;
static unsigned short mask;
static uint8_t r;
static uint32_t reallen = 0;

////////////////////////////////////////////////////////////////////
// Huffman functions

/*
  data => huff_byteorder[i] 
  reverse_order[data] => i
*/
static void promote_byte(uint8_t b) {
  uint8_t i = reverse_order[b];

  if (i > 0) {
    uint8_t s = huff_byteorder[i-1];
    huff_byteorder[i-1] = huff_byteorder[i];
    huff_byteorder[i] = s;

    reverse_order[huff_byteorder[i-1]] = i-1;
    reverse_order[huff_byteorder[i]] = i;
  }
}

static void huff_reset(void) {
  huff_l = huff_d = huff_size = huff_init = 0;
  d = 0x00;
  l = 8;
  reallen = 0;

  for (int i=0; i<N_SYMBOLS; i++)
  {
      reverse_order[i] = i;
      huff_byteorder[i] = i;
  }
}

static int huff_get(void) {
  int p1, p2, p;
  unsigned short code = 0;
  unsigned short len;
  unsigned short mask;

  for(;;) {
      // fetch new byte if needed
      if (huff_l<0x1) {
        int c = huff_get();
        if (c < 0) return -1;

        huff_d = c;
        huff_l = 0x80;
      }

      // get another bit from input
      code = (((code << 1) | ((huff_d&huff_l) ? 1 : 0))&0xfff) | ((code&0xf000) + 0x1000);
      huff_l>>=1;

      // has reached max bitsize - reached error
      if ((code & 0xf000)==0xf000) {
          debug(("Error in decompression.\n"));
          return -1;
      }

      // search huffman table by binary chop
      p1 = 0; p2 = N_SYMBOLS-1;
      p = 0;
      do {
          p = p1 + ((p2-p1)>>1);
          if ((p2-p1)<2) {
              if (std_hcodes[p1]==code) p2=p=p1;
              else p=p1=p2;
          }
          if (std_hcodes[p]>code) p2 = p;
          else if (std_hcodes[p]<code) p1 = p;
          else p1 = p2 = p;
      } while (p2!=p1);

      // did we find a code in the search?
      if (code==std_hcodes[p]) {
#ifdef TRACE_CODES_D
          debug(("D:%04X (%02X) (%02X)\n", std_hcodes[p], p, huff_byteorder[p]));
#endif
          huff_size--;
          uint8_t data = huff_byteorder[p];
          promote_byte(data);
          return data;
      }
  }
  return -1;
}

static void huff_put(int data)
{
    if (data >= 0) {
      r = data;
	    r = reverse_order[r];
      code = std_hcodes[r];
      promote_byte(data);
 } else {
      code = std_hcodes[255] | 0xfff;
    }
    mask = 0x1 << ((code>>12)-1);
#ifdef TRACE_CODES
    debug(("C:%04X - mask = %04X\n", code, mask));
#endif
    while (mask>0x0)
    {
      d<<=1;
      d |= (code&mask) ? 1 : 0;
      l--;
      mask >>=1;
      if (!l)
      {
	      bitstore_put(d);
      	l = 8;
      	d = 0x00;
      }
    }

  if (data < 0 && l)
  {
    bitstore_put(d<<l);
  }
}

////////////////////////////////////////////////////////////////////
// RLE state variables

static uint8_t flag_byte;
static uint8_t f_got_flag_byte = 0;
static uint8_t f_cont = 0x00;
static uint8_t f_rep = 0x00;

////////////////////////////////////////////////////////////////////
// RLE functions
static int leastused(uint8_t *in, uint32_t inlen)
{
	uint8_t buffer[1024];
	int i;
	int prob[256];
	int bytesread;
	int leastused = 0;

	for (i=0; i<256; i++) prob[i] = 0;

	for (i=0; i<inlen; i++)
		prob[in[i]]++;

	for (i=0; i<256; i++)
		if (prob[i] < prob[leastused]) leastused = i;
	return leastused;
}

static int process_rle(int cont, uint8_t data, uint8_t flag_byte, uint8_t *out)
{
	uint8_t d;
	if (cont==1 && data==flag_byte)
	{
		huff_put(flag_byte);
		d = 0xff;
		huff_put(d);
		return 2;
	}
	else if (cont>3 || (cont>1 && flag_byte == data))
	{
		huff_put(flag_byte);
		huff_put(cont - 1);
		huff_put(data);
		return 3;
	}
	else
	{

		for (int i=0; i<cont; i++) huff_put(data);
		return cont;
	}
}

static void rle_reset(void) {
	f_rep = f_cont = f_got_flag_byte = 0;
}

/* decompress */
static int rle_get(void) {
	int c;
	if (!f_got_flag_byte) {
		c = huff_get();
		if (c<0) return c;

		flag_byte = c;
		f_got_flag_byte = 1;
	}

	if (!f_cont) {
		c = huff_get();
		if (c<0) return c;

		if (c == flag_byte) {
			c = huff_get();
			if (c<0) return c;

			if (c == 0xff) {
				return flag_byte;
			} else {
				f_cont = c;
				c = huff_get();
				if (c<0) return c;

				f_rep = c;
				return f_rep;
			}
		} else {
			return c;
		}
	} else {
		f_cont --;
		return f_rep;
	}
}

int bitstore_InitRetrieve() {
  cursor = 0;
  huff_reset();
  return 0;
}

int bitstore_Size() {
  return reallen;
}

int bitstore_Store(void *user_data, uint8_t (*get_block)(void *user_data, uint8_t *block)) {
	uint8_t buffer[512];
	int j, k;
	int bytesread;
	uint8_t flag_byte;
	int cont = -1;
	uint8_t data;
	uint32_t outlen = 0;
	uint32_t ptr = 0;
	uint8_t d;
	int buf;

  nr_chunks = 0;
  reallen = 512;

  bitstore_Free();
  huff_reset();

	if (!get_block(user_data, buffer)) {
    // error
    return 0;
	}

  buf = 0;
  flag_byte = leastused(buffer, 512);

	huff_put(flag_byte);

	while (buf < 512)
	{
		if (cont<0) { data = buffer[buf]; cont = 1; }
		else if (buffer[buf] == data)
		{
			cont ++;
			if (cont >= 255)
			{
				huff_put(flag_byte);
				d = 0xfe;
				huff_put(d);
				huff_put(data);
				outlen += 3;
				cont -=255;
			}
		}
		else
		{
			int l = process_rle(cont, data, flag_byte, NULL);
			outlen += l;

			cont = 1;
			data = buffer[buf];
		}
		buf++;

		if (buf >= 512) {
			if (!get_block(user_data, buffer)) {
				break;
			}
			buf = 0;
      reallen += 512;
		}
	}
	int l = process_rle(cont, data, flag_byte, NULL);
	outlen += l;
  return nr_chunks;
}
int bitstore_GetBlock(uint8_t *blk) {
  int i;
  for (i = 0; i<512; i++) {
    int c = rle_get();
    if (c < 0) break;
    blk[i] = c;
  }
  return i == 0;
}

void bitstore_Free() {
  while (store) {
    chunk_t *chunk = store;
    store = store->next;
    free(chunk);
  }
  store = latest = NULL;
  lastblock = 0;
  nr_chunks = 0;
  cursor = 0;
  rle_reset();
  huff_reset();
}


  
#ifdef TEST
uint16_t crc16iv(const uint8_t* data_p, uint32_t length, uint16_t iv) {
    uint8_t x;
    uint16_t crc = iv;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

uint8_t in[512*1024];
uint32_t inlen = 0;

uint8_t comp[512*1024];
uint32_t complen = 0;

uint8_t out[512*1024];
uint32_t outlen = 0;

uint8_t get_block(void *user_data, uint8_t *block) {
	uint32_t *cursor = (uint32_t *)user_data;

	if ((*cursor) < inlen) {
		memcpy(block, &in[*cursor], 512);
		(*cursor) += 512;
		return 1;
	}
	return 0;
}

int main(int argc, char **argv) {
  FILE *fin = fopen(argv[1], "rb");
  uint16_t incrc, outcrc;
  if (fin) {
    inlen = fread(in, 1, sizeof in, fin);

    memset(&in[inlen], 0xff, 512-(inlen & 511));
    // inlen = (inlen + 511) & ~511;

    printf("in len %d crc %04X\n", inlen, incrc=crc16iv(in, inlen, 0xffff));
    fclose(fin);

    uint32_t cursor = 0;
    bitstore_Store(&cursor, get_block);
#if 0
    bitstore_init_store();
    for (int i=0; i<inlen; i+=512) {
      bitstore_put_block(&in[i], (i + 512) >= inlen);
    }
#endif
    printf("n_chunks %d lastblock %d\n", nr_chunks, lastblock);
    printf("size %d\n", nr_chunks * CHUNKSIZE + lastblock);

    printf("in   "); for (int i=0; i<16; i++) printf("%02X ", in[i]); printf("\n");
    printf("comp "); for (int i=0; i<16; i++) printf("%02X ", store->data[i]); printf("\n");

#ifdef HUFF
    huff_reset();
#endif
    bitstore_InitRetrieve();
    while (!bitstore_GetBlock(&out[outlen])) {
      outlen += 512;
      if (outlen >= sizeof out) { fprintf(stderr, "OVERFLOW!"); break; }
    }
    outlen += lastblock;

    printf("out len %d crc %04X crcinlen %04X\n", outlen, crc16iv(out, outlen, 0xffff), outcrc=crc16iv(out, inlen, 0xffff));


    printf("out  "); for (int i=0; i<16; i++) printf("%02X ", out[i]); printf("\n");

    int c = 16;
    for (int i=0; i<inlen; i++) {
	    if (in[i] != out[i]) {
		    printf("i %d in %02X != out %02X\n", i, in[i], out[i]);
		    if (!(c--)) break;
	    }

    }

    // fprintf(stderr, "%s: blks:%d incrc:%04X outcrc:%04X\n", argv[1], nr_chunks, incrc, outcrc);

    fprintf(stderr, ",%d,%s\n", nr_chunks, incrc == outcrc ? "ok": "!BAD!");
  }

  return 0;
}
#endif

