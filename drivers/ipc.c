#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <pico/time.h>
#ifdef IPC_SLAVE
#include <pico/i2c_slave.h>
#endif

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/i2c.h"
// #include "hardware/spi.h"
#include "hardware/irq.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hardware/pio.h"
#include "fifo.h"
#include "ipc.h"
#include "pins.h"
// #define DEBUG
#include "debug.h"

/*                                 USB          MIDI
  (SPI1 RX) (UART1 TX) (I2C0 SDA) GP24 - COM0 - GP26 (SPI1 SCK) (UART1 CTS) (I2C1 SDA)
 (SPI1 CSN) (UART1 RX) (I2C0 SCL) GP25 - COM1 - GP27 (SPI1 TX) (UART1 RTS) (I2C1 SCL)
 (SPI1 CSN) (UART0 RX) (I2C0 SCL) GP29 - COM2 - GP28 (SPI1 RX) (UART0 TX) (I2C0 SDA)
  (UART1 RX) (I2C0 SCL) (SPI1 CSN) GP9 - COM3 - GP29 (SPI1 CSn) (UART0 RX) (I2C0 SCL)
(UART1 CTS) (I2C1 SDA) (SPI1 SCK) GP10 - COM4 - GP11 (I2C1 SCL) (SPI1 TX) (UART1 RTS)
 (SPI1 CSN) (I2C0 SCL) (UART0 RX) GP13 - COM5 - GP12 (I2C0 SDA) (SPI1 RX) (UART0 TX)
(SPI0 SCK) (UART1 CTS) (I2C1 SDA) GP22 - COM6 - GP14 (SPI1 SCK) (I2C1 SDA) 
 (SPI0 TX) (UART1 RTS) (I2C1 SCL) GP23 - COM7 - GP15 (SPI1 TX)  (I2C1 SCL)
*/

/*
 GP0 -> I2C0 SDA
 GP1 -> I2C0 SCL
 GP2 -> I2C1 SDA
 GP3 -> I2C1 SCL
 GP4 -> I2C0 SDA
 GP5 -> I2C0 SCL
 GP6
 GP7
 GP8
 GP9
 */

// define base address of I2C controller hardware
// #define I2C0_BASE 0x40044000

#define SLAVE_ADDRESS     0xaa
#define IPC_CMD_TIMEOUT 1000000

// MJ: have seen it working up to 4M
#define IPC_BAUDRATE        100000
#define IPC_BAUDRATE_MAX    2000000
#define IPC_MAX_PAYLOAD 192

#ifdef IPC_SLAVE
// define the hardware registers used
volatile uint32_t * const I2C0_DATA_CMD       = (volatile uint32_t * const)(I2C0_BASE + 0x10);
volatile uint32_t * const I2C0_INTR_STAT      = (volatile uint32_t * const)(I2C0_BASE + 0x2c);
volatile uint32_t * const I2C0_INTR_MASK      = (volatile uint32_t * const)(I2C0_BASE + 0x30);
volatile uint32_t * const I2C0_CLR_RD_REQ     = (volatile uint32_t * const)(I2C0_BASE + 0x50);

// Declare the bits in the registers we use
#define I2C_DATA_CMD_FIRST_BYTE 0x00000800
#define I2C_DATA_CMD_DATA       0x000000ff
#define I2C_INTR_STAT_READ_REQ  0x00000020
#define I2C_INTR_STAT_RX_FULL   0x00000004
#define I2C_INTR_MASK_READ_REQ  0x00000020
#define I2C_INTR_MASK_RX_FULL   0x00000004

static uint8_t cmdbuff[IPC_MAX_PAYLOAD+2];
static uint8_t len = 0;
static uint8_t response = 0xff;

static uint8_t got_cmd = 0;
static uint8_t error = 0;

static fifo_t readback_fifo;
uint8_t readback_fifo_buf[4096];

void ipc_Debug() {
  debug(("len %d got_cmd %d error %d response %02X\n", len, got_cmd, error, response));
  hexdump(cmdbuff, 130);
}

fifo_t *ipc_GetFifo() {
  return &readback_fifo;
}

// Interrupt handler implements the RAM
static void i2c0_irq_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
  switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
      cmdbuff[len++] = i2c_read_byte_raw(i2c);
      if (len > 1) {
        if (len >= (cmdbuff[1] + 2)) {
          if (cmdbuff[0] != IPC_READBACKSIZE && cmdbuff[0] != IPC_READBACKDATA) {
            got_cmd = 1;
          }
          response = 0xff;
        }
      }
      break;

    case I2C_SLAVE_REQUEST: // master is requesting data
      // load from memory
      if (cmdbuff[0] == IPC_READBACKSIZE) {
        uint16_t cnt = fifo_Count(&readback_fifo);
        i2c_write_byte_raw(i2c, cnt > 255 ? 255 : cnt);
      } else if (cmdbuff[0] == IPC_READBACKDATA) {
        i2c_write_byte_raw(i2c, fifo_Get(&readback_fifo));
        if (fifo_Count(&readback_fifo) == 0) gpio_put(GPIO_RP2U_BCDATAREADY, 0); // no more data
      } else {
        i2c_write_byte_raw(i2c, response);
      }
      break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
      len = 0;
      break;
    default:
      break;
    }
}

void ipc_InitSlave() {
  gpio_init(GPIO_IPCS_I2C_CLK);
  gpio_init(GPIO_IPCS_I2C_DAT);

  gpio_set_function(GPIO_IPCS_I2C_CLK, GPIO_FUNC_I2C);
  gpio_set_function(GPIO_IPCS_I2C_DAT, GPIO_FUNC_I2C);
  gpio_pull_up(GPIO_IPCS_I2C_CLK);
  gpio_pull_up(GPIO_IPCS_I2C_DAT);

  i2c_init(i2c0, IPC_BAUDRATE);
  i2c_slave_init(i2c0, SLAVE_ADDRESS, &i2c0_irq_handler);

  gpio_init(GPIO_RP2U_BCDATAREADY);
  gpio_put(GPIO_RP2U_BCDATAREADY, 0); // no data

#if 0
  i2c_set_slave_mode(i2c0, true, SLAVE_ADDRESS);
  // Enable the interrupts we want
  *I2C0_INTR_MASK = (I2C_INTR_MASK_READ_REQ | I2C_INTR_MASK_RX_FULL);

  // Set up the interrupt handlers
  irq_set_exclusive_handler(I2C0_IRQ, i2c0_irq_handler);
  // Enable I2C interrupts
  irq_set_enabled(I2C0_IRQ, true);
#endif
  fifo_Init(&readback_fifo, readback_fifo_buf, sizeof readback_fifo_buf);

}

int ipc_SlaveTick() {
  if (got_cmd) {
    response = ipc_GotCommand(cmdbuff[0], &cmdbuff[2], cmdbuff[1]);
    got_cmd = 0;
  }
  return 0;
}

void ipc_SendDataEx(uint8_t tag, uint8_t *data, uint16_t len, uint8_t *data2, uint16_t len2) {
  uint16_t crc = 0xffff;
  uint8_t d;
  uint16_t pktlen;

//   debug(("ipc_SendDataEx: tag %02X len %d len2 %d\n", tag, len, len2));

  pktlen = len + len2 + 7;

  // don't send if not enough space in fifo
  if (fifo_Space(&readback_fifo) < pktlen) return;

  d = 0x55;
  crc = crc16iv(&d, 1, crc);
  d = 0xaa;
  crc = crc16iv(&d, 1, crc);
  crc = crc16iv(&tag, 1, crc);
  d = pktlen >> 8;
  crc = crc16iv(&d, 1, crc);
  d = pktlen & 0xff;
  crc = crc16iv(&d, 1, crc);

  crc = crc16iv(data, len, crc);
  if (data2) crc = crc16iv(data2, len2, crc);

  fifo_Put(&readback_fifo, 0x55);
  fifo_Put(&readback_fifo, 0xaa);
  fifo_Put(&readback_fifo, tag);
  fifo_Put(&readback_fifo, pktlen >> 8);
  fifo_Put(&readback_fifo, pktlen & 0xff);
  for (int i=0; i<len; i++) {
    fifo_Put(&readback_fifo, data[i]);
  }
  for (int i=0; i<len2; i++) {
    fifo_Put(&readback_fifo, data2[i]);
  }
  fifo_Put(&readback_fifo, crc >> 8);
  fifo_Put(&readback_fifo, crc & 0xff);
  
  gpio_put(GPIO_RP2U_BCDATAREADY, 1); // got data

  debug(("ipc_SendDataEx: tag %02X len %d len2 %d fifo %d\n", tag, len, len2, fifo_Count(&readback_fifo)));
}

void ipc_SendData(uint8_t tag, uint8_t *data, uint16_t len) {
  ipc_SendDataEx(tag, data, len, NULL, 0);
}

#endif

int ipc_SetFastMode(uint8_t on) {
#ifdef IPC_SLAVE
  i2c_set_baudrate(i2c0, IPC_BAUDRATE_MAX);
#endif
#ifdef IPC_MASTER
  i2c_set_baudrate(i2c1, IPC_BAUDRATE_MAX);
#endif
}

#ifdef IPC_MASTER
static fifo_t read_fifo;
static uint8_t read_fifo_buf[256];

void ipc_InitMaster() {
  i2c_init(i2c1, IPC_BAUDRATE);
  gpio_set_function(GPIO_IPCM_I2C_CLK, GPIO_FUNC_I2C);
  gpio_set_function(GPIO_IPCM_I2C_DAT, GPIO_FUNC_I2C);
  gpio_pull_up(GPIO_IPCM_I2C_CLK);
  gpio_pull_up(GPIO_IPCM_I2C_DAT);
  i2c_set_slave_mode(i2c1, false, 0x00);
  fifo_Init(&read_fifo, read_fifo_buf, sizeof read_fifo_buf);
}


int ipc_ReadBackLen() {
  uint8_t cmd[2] = {IPC_READBACKSIZE, 0x00};
  uint8_t len = 0;
  i2c_write_blocking (i2c1, SLAVE_ADDRESS, cmd, sizeof cmd, false);
  i2c_read_blocking(i2c1, SLAVE_ADDRESS, &len, 1, true);
  return len;
}

uint8_t ipc_ReadKeyboard() {
  uint8_t cmd[2] = {IPC_READKEYBOARD, 0x00};
  uint8_t data = 0;
  i2c_write_blocking (i2c1, SLAVE_ADDRESS, cmd, sizeof cmd, false);
  i2c_read_blocking(i2c1, SLAVE_ADDRESS, &data, 1, true);
  return data;
}

int ipc_ReadBack(uint8_t *data, uint8_t len) {
  uint8_t cmd[2] = {IPC_READBACKDATA, 0x00};
  i2c_write_blocking (i2c1, SLAVE_ADDRESS, cmd, sizeof cmd, false);
  i2c_read_blocking(i2c1, SLAVE_ADDRESS, data, len, true);
  return len;
}

#if 0
int ipc_SetFastMode(uint8_t on) {
#ifdef IPC_SLAVE
  i2c_set_baudrate(i2c0, IPC_BAUDRATE_MAX);
#endif
#ifdef IPC_MASTER
  i2c_set_baudrate(i2c1, IPC_BAUDRATE_MAX);
#endif
}
#endif

int ipc_Command(uint8_t cmd, uint8_t *data, uint8_t len) {
  uint8_t response;
  uint8_t blob[IPC_MAX_PAYLOAD+2];
  
  if (len > IPC_MAX_PAYLOAD) {
    debug(("ipc_Command: payload size > IPC_MAX_PAYLOAD\n"));
    return 1;
  }
  
  blob[0] = cmd;
  blob[1] = len;
  memcpy(&blob[2], data, len);

  if (PICO_ERROR_TIMEOUT == i2c_write_timeout_us(i2c1, SLAVE_ADDRESS, blob, len+2, false, 1000000)) {
    return 0xee;
  }
#ifdef IPC_SLAVE
  ipc_SlaveTick();
#endif

  i2c_read_blocking(i2c1, SLAVE_ADDRESS, &response, 1, true);

  // if cmd takes longer...
  if (response == 0xff) {
    uint8_t get_response_msg[] = {IPC_GETRESPONSE};
    uint64_t timeout = time_us_64() + IPC_CMD_TIMEOUT;
    do {
      sleep_ms(100);
      i2c_write_blocking (i2c1, SLAVE_ADDRESS, get_response_msg, sizeof get_response_msg, false);
      i2c_read_blocking(i2c1, SLAVE_ADDRESS, &response, 1, true);
#ifdef IPC_SLAVE
      ipc_SlaveTick();
#endif
    } while (response == 0xff && time_us_64() < timeout);
  }

  return response;
}

static uint8_t pkt[512];
static uint16_t pos = 0;
static uint16_t pktlen = 0;

void ipc_MasterTick() {
  int readable;
  int wantlen, thisread;
  
  readable = ipc_ReadBackLen();
  // printf("ipc_ReadBackLen returns %d\n", readable);

  while (readable) {
    // printf("ipc_ReadBackLen returns %d\n", readable);
    // if (readable == 0xff) return;
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

    ipc_ReadBack(&pkt[pos], thisread);
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
      // got packet
      if (!crc16(pkt, pktlen)) ipc_HandleData(pkt[2], &pkt[5], pktlen - 7);
      else debug(("Packet failed CRC %04X vs %02X%02X\n", crc16(pkt, pktlen + 3), pkt[pktlen-2], pkt[pktlen-1]));

      pos = 0;
    }
    readable = ipc_ReadBackLen();
  }
}

#endif

