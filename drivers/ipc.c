#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <pico/time.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/irq.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hardware/pio.h"
#include "fifo.h"
#include "ipc.h"
#include "pins.h"
#define DEBUG
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
#define I2C0_BASE 0x40044000

#define SLAVE_ADDRESS     0xaa
#define IPC_CMD_TIMEOUT 1000000
// #define IPC_BAUDRATE    100000
#define IPC_BAUDRATE    1000000
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
uint8_t readback_fifo_buf[1024];

// void fifo_InitEx(fifo_t *f, uint8_t mask);
// uint8_t fifo_Count(fifo_t *f);


void ipc_Debug() {
  debug(("len %d got_cmd %d error %d response %02X\n", len, got_cmd, error, response));
  hexdump(cmdbuff, 130);
}

fifo_t *ipc_GetFifo() {
  return &readback_fifo;
}

// Interrupt handler implements the RAM
static void i2c0_irq_handler() {
    // Get interrupt status
    uint32_t status = *I2C0_INTR_STAT;
    // Check to see if we have received data from the I2C master
    if (status & I2C_INTR_STAT_RX_FULL) {
        // Read the data (this will clear the interrupt)
        uint32_t value = *I2C0_DATA_CMD;
        // Check if this is the 1st byte we have received
//         if (got_cmd || error) { error = 1; response = 0xfe; }
        if (value & I2C_DATA_CMD_FIRST_BYTE) {
            // If so treat it as the address to use
            len = 0;
            cmdbuff[len++] = (uint8_t)(value & I2C_DATA_CMD_DATA);
            if (cmdbuff[0] != IPC_GETRESPONSE) {
              response = 0xff;
            }
        } else if (len == 1) {
          cmdbuff[len++] = (uint8_t)(value & I2C_DATA_CMD_DATA);
          if (len == (cmdbuff[1] + 2)) {
            if (cmdbuff[0] != IPC_READBACKSIZE && cmdbuff[0] != IPC_READBACKDATA) {
              got_cmd = 1;
            }
          }
        } else if (len < (cmdbuff[1] + 2)) {
            // If not 1st byte then store the data in the RAM
            // and increment the address to point to next byte
            cmdbuff[len++] = (uint8_t)(value & I2C_DATA_CMD_DATA);

          if (len == (cmdbuff[1] + 2)) got_cmd = 1;
        }
    }
    // Check to see if the I2C master is requesting data from us
    if (status & I2C_INTR_STAT_READ_REQ) {
        // Write the data from the current address in RAM
        error = 0;
        if (cmdbuff[0] == IPC_READBACKSIZE) {
          uint16_t cnt = fifo_Count(&readback_fifo);
          *I2C0_DATA_CMD = (uint32_t) (cnt > 255 ? 255 : cnt);
        } else if (cmdbuff[0] == IPC_READBACKDATA) {
          *I2C0_DATA_CMD = (uint32_t)fifo_Get(&readback_fifo);
        } else {
          *I2C0_DATA_CMD = (uint32_t)response;
        }
        // Clear the interrupt
        *I2C0_CLR_RD_REQ;
    }
}

void ipc_InitSlave() {
  gpio_init(GPIO_IPCS_I2C_CLK);
  gpio_init(GPIO_IPCS_I2C_DAT);

  i2c_init(i2c0, IPC_BAUDRATE);
  gpio_set_function(GPIO_IPCS_I2C_CLK, GPIO_FUNC_I2C);
  gpio_set_function(GPIO_IPCS_I2C_DAT, GPIO_FUNC_I2C);
  gpio_pull_up(GPIO_IPCS_I2C_CLK);
  gpio_pull_up(GPIO_IPCS_I2C_DAT);
  i2c_set_slave_mode(i2c0, true, SLAVE_ADDRESS);
  // Enable the interrupts we want
  *I2C0_INTR_MASK = (I2C_INTR_MASK_READ_REQ | I2C_INTR_MASK_RX_FULL);

  // Set up the interrupt handlers
  irq_set_exclusive_handler(I2C0_IRQ, i2c0_irq_handler);
  // Enable I2C interrupts
  irq_set_enabled(I2C0_IRQ, true);
  fifo_Init(&readback_fifo, readback_fifo_buf, sizeof readback_fifo_buf);

}

int ipc_SlaveTick() {
  if (got_cmd) {
    response = ipc_GotCommand(cmdbuff[0], &cmdbuff[2], cmdbuff[1]);
//     debug(("Got Command %02X data\n", cmdbuff[0]));
//     hexdump(&cmdbuff[2], cmdbuff[1]);
    got_cmd = 0;
//     response = 0x01;
  }
  return 0;
}
#endif

#ifdef IPC_MASTER
void ipc_InitMaster() {
  i2c_init(i2c1, IPC_BAUDRATE);
  gpio_set_function(GPIO_IPCM_I2C_CLK, GPIO_FUNC_I2C);
  gpio_set_function(GPIO_IPCM_I2C_DAT, GPIO_FUNC_I2C);
  gpio_pull_up(GPIO_IPCM_I2C_CLK);
  gpio_pull_up(GPIO_IPCM_I2C_DAT);
  i2c_set_slave_mode(i2c1, false, 0x00);
}

int ipc_ReadBackLen() {
  uint8_t cmd[2] = {IPC_READBACKSIZE, 0x00};
  uint8_t len = 0;
  i2c_write_blocking (i2c1, SLAVE_ADDRESS, cmd, sizeof cmd, false);
  i2c_read_blocking(i2c1, SLAVE_ADDRESS, &len, 1, true);
  return len;
}

int ipc_ReadBack(uint8_t *data, uint8_t len) {
  uint8_t cmd[2] = {IPC_READBACKDATA, 0x00};
  i2c_write_blocking (i2c1, SLAVE_ADDRESS, cmd, sizeof cmd, false);
  i2c_read_blocking(i2c1, SLAVE_ADDRESS, data, len, true);
  return len;
}


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
//   i2c_write_blocking (i2c1, SLAVE_ADDRESS, blob+2, len, false);
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
#endif

#if 0
#define

#define GPIO_IPCM_DL_CLK   14 // SPI1 SCK
#define GPIO_IPCM_DL_DAT   15 // SPI1 TX
#define GPIO_IPCM_DL_SEL   13 // SPI1 CSN

#define GPIO_IPCS_DL_CLK   16
#define GPIO_IPCS_DL_DAT   17
#define GPIO_IPCS_DL_SEL   18

// (SPI0 SCK) (UART1 CTS) (I2C1 SDA) GP22 - COM6 - GP14 (SPI1 SCK) (I2C1 SDA)
//  (SPI0 TX) (UART1 RTS) (I2C1 SCL) GP23 - COM7 - GP15 (SPI1 TX)  (I2C1 SCL)


void ipc_InitMasterFast() {
  i2c_init(i2c1, IPC_BAUDRATE);
  gpio_init(GPIO_IPCM_DL_CLK);
  gpio_init(GPIO_IPCM_DL_DAT);
  gpio_init(GPIO_IPCM_DL_SEL);
  gpio_put(GPIO_IPCM_DL_SEL, 1);
  gpio_set_dir(GPIO_IPCM_DL_SEL, GPIO_OUT);
  gpio_set_function(GPIO_IPCM_DL_CLK, GPIO_FUNC_SPI);
  gpio_set_function(GPIO_IPCM_DL_DAT, GPIO_FUNC_SPI);
  gpio_pull_up(GPIO_IPCM_DL_CLK);
  gpio_pull_up(GPIO_IPCM_DL_DAT);
}

void ipc_InitSlaveFast() {
}

void ipc_SendPacketFast(uint8_t *data, uint16_t len) {
  gpio_put(GPIO_IPCM_DL_SEL, 0);

}

void ipc_ProcessPacketFast(uint8_t *data, uint16_t len) {
}

void ipc_TickFast() {
}

#endif
