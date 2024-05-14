#ifndef _JTAG_H
#define _JTAG_H

#define XILINX_SPARTAN6_XL9		0x4001093
#define XILINX_SPARTAN6_XL25		0x4004093
#define XILINX_SPARTAN3E_500    0x1C22093

void delay(int i);

int jtag_init(void);
void jtag_kill(void);

void jtag_tms(int value);
int jtag_tdi(int value);
uint8_t jtag_tdin(int n, uint8_t bits);
void jtag_tck(void);
void jtag_reset(void);
void jtag_idle(void);
uint32_t jtag_ins(uint8_t ins, uint8_t *data, int n);
uint32_t jtag_ins_ex(uint8_t ins, uint8_t *data, int n, uint8_t *data1, int n1, int rev);
uint32_t jtag_ins_ex_cb(uint8_t ins, uint16_t (*next_block)(void *, uint8_t *), int len, void *user_data);

void jtag_start(uint8_t *image, uint32_t imageSize, uint32_t device, uint32_t devicemask, uint32_t crc);
void jtag_detect(void);

int jtag_configure(void *user_data, uint8_t (*next_block)(void *, uint8_t *), uint32_t assumelength);

#endif
