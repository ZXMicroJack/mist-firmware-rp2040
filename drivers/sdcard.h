#ifndef _SDCARD_H
#define _SDCARD_H

void hexdump(uint8_t *buf, int len);

uint8_t sd_writesector(pio_spi_inst_t *spi, uint32_t lba, uint8_t *data);
uint8_t sd_readsector(pio_spi_inst_t *spi, uint32_t lba, uint8_t *sector);
int sd_init_card(pio_spi_inst_t *spi);
void sd_hw_kill(pio_spi_inst_t *spi);
pio_spi_inst_t *sd_hw_init();

#endif
