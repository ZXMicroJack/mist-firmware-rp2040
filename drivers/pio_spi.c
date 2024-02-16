/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pio_spi.h"
#include "debug.h"

// Just 8 bit functions provided here. The PIO program supports any frame size
// 1...32, but the software to do the necessary FIFO shuffling is left as an
// exercise for the reader :)
//
// Likewise we only provide MSB-first here. To do LSB-first, you need to
// - Do shifts when reading from the FIFO, for general case n != 8, 16, 32
// - Do a narrow read at a one halfword or 3 byte offset for n == 16, 8
// in order to get the read data correctly justified. 

void __time_critical_func(pio_spi_write8_blocking)(const pio_spi_inst_t *spi, const uint8_t *src, size_t len) {
    size_t tx_remain = len, rx_remain = len;
    
    // restart the state machine
//     pio_sm_restart(spi->pio, spi->sm);
    
    // Do 8 bit accesses on FIFO, so that write data is byte-replicated. This
    // gets us the left-justification for free (for MSB-first shift-out)
    io_rw_8 *txfifo = (io_rw_8 *) &spi->pio->txf[spi->sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &spi->pio->rxf[spi->sm];
    while (tx_remain || rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(spi->pio, spi->sm)) {
            *txfifo = *src++;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(spi->pio, spi->sm)) {
            uint8_t rx = *rxfifo;
            debug(("[rx:%02X]\n", rx));
            --rx_remain;
        }
    }
}

void __time_critical_func(pio_spi_read8_blocking)(const pio_spi_inst_t *spi, uint8_t *dst, size_t len) {
    size_t tx_remain = len, rx_remain = len;

    // restart the state machine
//     pio_sm_restart(spi->pio, spi->sm);
    
    io_rw_8 *txfifo = (io_rw_8 *) &spi->pio->txf[spi->sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &spi->pio->rxf[spi->sm];
    while (tx_remain || rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(spi->pio, spi->sm)) {
//             *txfifo = 0;
            *txfifo = 0xff;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(spi->pio, spi->sm)) {
            uint8_t rx = *rxfifo;
            debug(("[rx:%02X]\n", rx));
            
            if (dst) *dst++ = rx;
            --rx_remain;
        }
    }
}

void __time_critical_func(pio_spi_read8_blocking_ex)(const pio_spi_inst_t *spi, uint8_t *dst, size_t len) {
    size_t tx_remain = len, rx_remain = len;
    int timeout = 512;

    io_rw_8 *txfifo = (io_rw_8 *) &spi->pio->txf[spi->sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &spi->pio->rxf[spi->sm];
    while (rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(spi->pio, spi->sm)) {
//             *txfifo = 0;
            *txfifo = 0xff;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(spi->pio, spi->sm)) {
            uint8_t rx = *rxfifo;
            debug(("[rx:%02X]\n", rx));
            
            if (rx_remain < len || (rx_remain == len && rx != 0xff)) {
              if (dst) *dst++ = rx;
              --rx_remain;
            } else {
              ++ tx_remain;
              -- timeout;
              if (!timeout) {
                debug(("spi_timeout\n"));
                break;
              }
            }
        }
    }
}

void __time_critical_func(pio_spi_write8_read8_blocking)(const pio_spi_inst_t *spi, uint8_t *src, uint8_t *dst,
                                                         size_t len) {
    size_t tx_remain = len, rx_remain = len;

    // restart the state machine
//     pio_sm_restart(spi->pio, spi->sm);

    io_rw_8 *txfifo = (io_rw_8 *) &spi->pio->txf[spi->sm];
    io_rw_8 *rxfifo = (io_rw_8 *) &spi->pio->rxf[spi->sm];
    while (tx_remain || rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(spi->pio, spi->sm)) {
            *txfifo = *src++;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(spi->pio, spi->sm)) {
            *dst++ = *rxfifo;
            --rx_remain;
        }
    }
}

#if 0
static inline void pio_spi_unkill(PIO pio, uint sm, uint pin_sck, uint pin_mosi, uint pin_miso) {
static inline void pio_spi_kill(PIO pio, uint sm, uint pin_sck, uint pin_mosi, uint pin_miso) {

				
typedef struct pio_spi_inst {
    PIO pio;
    uint sm;
    uint cs_pin;
} pio_spi_inst_t;
#endif

void pio_spi_select(const pio_spi_inst_t *spi, uint8_t state) {
  // printf("pio_spi_select: %d\n", state);
	if (state) {
		// active
    gpio_init(spi->cs_pin);
    gpio_put(spi->cs_pin, 1);
    gpio_set_dir(spi->cs_pin, GPIO_OUT);

		pio_spi_unkill(spi->pio, spi->sm, spi->sck_pin, spi->mosi_pin, spi->miso_pin);
	} else {
		// inactive
		pio_spi_kill(spi->pio, spi->sm, spi->sck_pin, spi->mosi_pin, spi->miso_pin);
    gpio_init(spi->cs_pin);
    gpio_put(spi->cs_pin, 1);
    gpio_set_dir(spi->cs_pin, GPIO_IN);
	}
}


