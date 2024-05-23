#ifndef _BITSTORE_H
#define _BITSTORE_H

int bitstore_GetBlock(uint8_t *blk);
int bitstore_Store(void *user_data, int (*get_block)(void *user_data, uint8_t *block));
int bitstore_Size();
int bitstore_InitRetrieve();
void bitstore_Free();

#endif

