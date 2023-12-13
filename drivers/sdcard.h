#ifndef _SDCARD_H
#define _SDCARD_H

void hexdump(uint8_t *buf, int len);

int sd_is_sdhc();
uint8_t sd_writesector(pio_spi_inst_t *spi, uint32_t lba, uint8_t *data);
uint8_t sd_readsector(pio_spi_inst_t *spi, uint32_t lba, uint8_t *sector);
int sd_init_card(pio_spi_inst_t *spi);
void sd_hw_kill(pio_spi_inst_t *spi);
pio_spi_inst_t *sd_hw_init();

uint8_t sd_cmd9(pio_spi_inst_t *spi, uint8_t buf[]); // CSD
uint8_t sd_cmd10(pio_spi_inst_t *spi, uint8_t buf[]); // CID

#endif
