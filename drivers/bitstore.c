#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"

#include "bitstore.h"

#define lowest(a,b) ((a) < (b) ? (a) : (b))
#define CHUNKSIZE   1024

typedef struct chunk {
  uint8_t data[CHUNKSIZE];
  struct chunk *next;
} chunk_t;


static uint16_t lastblock = 0;
static uint16_t nr_chunks = 0;
static chunk_t *store = NULL;
static chunk_t *latest = NULL;
static z_stream strm;
static int level = 1;

static void bitstore_add() {
  nr_chunks ++;
  chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
  chunk->next = NULL;
  // chunkremaining = CHUNKSIZE;

  if (!store) {
    store = latest = chunk;
  } else {
    latest->next = chunk;
    latest = latest->next;
  }
}

void bitstore_free() {
  while (store) {
    chunk_t *chunk = store;
    store = store->next;
    free(chunk);
  }
}

int bitstore_init_store() {
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  int ret = deflateInit(&strm, level);
  
  return ret != Z_OK;
}

void bitstore_put_block(uint8_t *blk, uint8_t last) {
  strm.avail_in = 512;
  strm.next_in = blk;

  do {
    if (latest == NULL || strm.avail_out == 0) {
      bitstore_add();
      strm.next_out = latest->data;
      strm.avail_out = CHUNKSIZE;
    }

    int ret = deflate(&strm, last ? Z_FINISH : Z_NO_FLUSH);

    assert(ret != Z_STREAM_ERROR);
  } while (strm.avail_out == 0);

  if (last) {
    lastblock = CHUNKSIZE - strm.avail_out;
    (void)deflateEnd(&strm);    
  }
}

int bitstore_init_retrieve() {
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  int ret = inflateInit(&strm);

  strm.avail_in = store->next ? CHUNKSIZE : lastblock;
  strm.next_in = store->data;

  return ret != Z_OK;
}
  
int bitstore_get_block(uint8_t *blk) {
  strm.next_out = blk;
  strm.avail_out = 512;

  do {
    if (!strm.avail_in && store->next) {
      /* move to next chunk*/
      chunk_t *this_one = store;
      store = store->next;
      free(this_one);

      strm.avail_in = store->next ? CHUNKSIZE : lastblock;
      strm.next_in = store->data;
    }

    int ret = inflate(&strm, Z_NO_FLUSH); //store->next == NULL ? Z_FINISH : Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR && ret != Z_BUF_ERROR);
  } while (strm.avail_out && (store->next || strm.avail_in) );

  if (!store->next && strm.avail_in == 0) {
    (void)inflateEnd(&strm);
    bitstore_free();
    return 1;
  }
  return 0;
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

int main(int argc, char **argv) {
  FILE *fin = fopen(argv[1], "rb");
  if (fin) {
    inlen = fread(in, 1, sizeof in, fin);

    memset(&in[inlen], 0xff, 512-(inlen & 511));
    // inlen = (inlen + 511) & ~511;

    printf("in len %d crc %04X\n", inlen, crc16iv(in, inlen, 0xffff));
    fclose(fin);

    bitstore_init_store();
    for (int i=0; i<inlen; i+=512) {
      bitstore_put_block(&in[i], (i + 512) >= inlen);
    }
    printf("n_chunks %d lastblock %d\n", nr_chunks, lastblock);
    printf("size %d\n", nr_chunks * CHUNKSIZE + lastblock);

    bitstore_init_retrieve();
    while (!bitstore_get_block(&out[outlen])) {
      outlen += 512;
    }
    outlen += lastblock;

    printf("out len %d crc %04X crcinlen %04X\n", outlen, crc16iv(out, outlen, 0xffff), crc16iv(out, inlen, 0xffff));
  }

  return 0;
}
#endif

