#ifndef _FPGA_H
#define _FPGA_H

int fpga_initialise();
int fpga_claim(uint8_t claim);
int fpga_reset();
int fpga_configure(void *user_data, uint8_t (*next_block)(void *, uint8_t *), uint32_t assumelength);

void fpga_configure_start();
void fpga_configure_data(uint8_t *data, uint32_t len);
void fpga_configure_end();

void fpga_load_bitfile(void *_spi, int lba, char *fn);
void fpga_boot_from_flash();
int fpga_detect_error();
void fpga_detect_init();
void fpga_holdreset();
int fpga_ResetButtonState();

uint8_t fpga_GetType();
void fpga_SetType(uint8_t type);
void fpga_ConfirmType();

#endif
