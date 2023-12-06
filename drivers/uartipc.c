#include <stdio.h>
#include <stdint.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "uartipc.h"
#include "debug.h"
#include "flash.h"
#include "pins.h"

static uint8_t pkt[512];
static uint16_t pos = 0;
static uint16_t pktlen = 0;

void uart_Init() {
  uart_init (uart0, 115200);
  uart_set_fifo_enabled(uart0, true);
  gpio_set_function(GPIO_RP2M_UART0_TX, GPIO_FUNC_UART);
  gpio_set_function(GPIO_RP2M_UART0_RX, GPIO_FUNC_UART);
}

////////////////////////////////////////////////////////////////////////////////
// READ UART
void uart_Loop() {
  int readable = uart_is_readable(uart0);
  int wantlen, thisread;
  
  while (readable) {
    if (pos >= 5) {
      pktlen = (pkt[3] << 8) | pkt[4];
      wantlen = pktlen - pos;
    } else if ( pos < 2 ) {
      wantlen = 1;
    } else {
      wantlen = 5 - pos;
    }
    
    thisread = readable > wantlen ? wantlen : readable;
    readable -= thisread;
    
    uart_read_blocking(uart0, &pkt[pos], thisread);
    pos += thisread;
    
    // if no sync - skip
    if (pos == 1 && pkt[0] != 0x55) {
      pos = 0;
      debug(("pkt[0] = %02X\n", pkt[0]));
    }
    if (pos == 2 && pkt[1] != 0xaa) {
      if (pkt[1] == 0x55) {
        pos = 1;
      } else {
        pos = 0;
      }
      debug(("pkt[1] = %02X\n", pkt[1]));
    }
    
    if (pos >= pktlen && pos > 5) {
      if (!crc16(pkt, pktlen)) {
        // got packet
        uint8_t ret = uart_ProcessPacket(pkt[2], &pkt[5], pktlen - 7);
        uart_putc(uart0, ret);
        pos = 0;
      } else {
        debug(("Packet failed CRC %04X vs %02X%02X\n", crc16(pkt, pktlen + 3), pkt[pktlen-2], pkt[pktlen-1]));
        uart_putc(uart0, 0xfe);
        pos = 0;
      }
    }
    readable = uart_is_readable(uart0);
  }
}
