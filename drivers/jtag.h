#ifndef _JTAG_H
#define _JTAG_H

#define XILINX_SPARTAN6_XL9		0x4001093
#define XILINX_SPARTAN6_XL25		0x4004093
#define XILINX_SPARTAN3E_500    0x1C22093

void delay(int i);

int jtag_init(int ch, int preinslen);
void jtag_kill(void);

void jtag_tms(int value);
int jtag_tdi(int value);
uint8_t jtag_tdin(int n, uint8_t bits);
void jtag_tck(void);
void jtag_reset();
void jtag_idle();
uint32_t jtag_ins(uint8_t ins, uint8_t *data, int n);
uint32_t jtag_ins_crc(uint8_t ins, uint8_t *data, int n, uint32_t *crcCalc);

void jtag_enumdevs(int ch);
void jtag_start(int ch, uint8_t *image, uint32_t imageSize, uint32_t device, uint32_t devicemask, uint32_t crc);

void jtag_detect(int ch);

#endif
