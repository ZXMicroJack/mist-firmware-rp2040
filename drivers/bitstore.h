#ifndef _BITSTORE_H
#define _BITSTORE_H

void bitstore_free();
int bitstore_init_store();
int bitstore_init_retrieve();

void bitstore_put_block(uint8_t *blk, uint8_t last);
int bitstore_get_block(uint8_t *blk);


#endif

