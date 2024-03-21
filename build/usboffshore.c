#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/multicore.h"

#include "drivers/jamma.h"
#include "drivers/fifo.h"
#include "drivers/ipc.h"

// #define DEBUG
#include "drivers/debug.h"

#include "usbdev.h"

#define MAX_USB   5

#define lowest(a,b) ((a) < (b) ? (a) : (b))

static uint8_t storedDD[USB_DEVICE_DESCRIPTOR_LEN];
static uint8_t config[MAX_USB][256];

static uint32_t jammaData = 0xffff0000;

void jamma_Init() {
}

uint32_t jamma_GetData(uint8_t inst) {
  uint8_t d = (inst ? (jammaData >> 24) : (jammaData >> 16));
  // debug(("jamma_GetData: inst %d returns %02X\n", inst, d));
  return d;
}

#ifdef MB2USB
void tuh_task() {
}
void hid_app_task() {}
#endif

static fifo_t kbdfifo;
static uint8_t kbdfifo_buf[64];

static fifo_t mousefifo;
static uint8_t mousefifo_buf[64];


#ifdef CORE2_IPC_TICKS
static void ipc_core() {
  ipc_InitMaster();
  for(;;) {
    ipc_MasterTick();
  }
}
#endif

int ps2_GetChar(uint8_t ch) {
  return ch == 0 ? fifo_Get(&kbdfifo) : fifo_Get(&mousefifo);
}

static uint8_t ipc_started = 0;
void ps2_Init() {
  fifo_Init(&kbdfifo, kbdfifo_buf, sizeof kbdfifo_buf);
  fifo_Init(&mousefifo, mousefifo_buf, sizeof mousefifo_buf);

  if (!ipc_started) {
#ifdef CORE2_IPC_TICKS
    multicore_reset_core1();
    multicore_launch_core1(ipc_core);
#else
    ipc_InitMaster();
#endif
    ipc_started = 1;
  }
}
void ps2_EnablePort(uint8_t ch, bool enabled) {}

void ps2_EnablePortEx(uint8_t ch, bool enabled, uint8_t hostMode) {}

#ifdef MB2USB
uint8_t tuh_descriptor_get_device_sync(uint8_t dev_addr, uint8_t *dd, uint16_t len) {
  memcpy(dd, storedDD, lowest(len, sizeof storedDD));
  return 0;
}

uint8_t tuh_descriptor_get_configuration_sync(uint8_t dev_addr, uint8_t inst, uint8_t *dd, uint16_t len) {
  memcpy(dd, config[dev_addr - 1], lowest(len, sizeof config[0]));
  return 0;
}

//NOTE: only time this is called, it ignores the last two parameters and the return function, so only here for
//      linkage purposes.
bool tuh_configuration_set(uint8_t daddr, uint8_t config_num, void * complete_cb, uintptr_t user_data) {
  uint8_t data[2];
  data[0] = daddr;
  data[1] = config_num;
  ipc_Command(IPC_USB_SETCONFIG, data, sizeof data);
  return true;
}

//TODO
uint8_t storage_devices = 0;
unsigned char usb_host_storage_write(unsigned long lba, const unsigned char *pWriteBuffer, uint16_t len) {
  return 0;
}

unsigned int usb_host_storage_capacity() {
  return 0;
}

unsigned char usb_host_storage_read(unsigned long lba, unsigned char *pReadBuffer, uint16_t len) {
  return 0;
}
#endif

#define IPC_USB_ATTACHED        0x40
#define IPC_USB_DETACHED        0x41
#define IPC_USB_HANDLE_DATA     0x42
#define IPC_UPDATE_JAMMA        0x43
#define IPC_PS2_DATA            0x44
#define IPC_USB_DEVICE_DESC     0x45
#define IPC_USB_CONFIG_DESC     0x46

void ipc_HandleData(uint8_t tag, uint8_t *data, uint16_t len) {
  printf("ipc_HandleData: tag %02x len %d\n", tag, len);
  switch(tag) {
#ifdef MB2USB
    case IPC_USB_DEVICE_DESC:
      memcpy(storedDD, data, lowest(len, sizeof storedDD));
      break;

    case IPC_USB_CONFIG_DESC:
      memcpy(config[data[0] & (MAX_USB - 1)], data + 1, lowest(len - 1, sizeof config[0]));
      break;

    case IPC_USB_ATTACHED: {
      IPC_usb_attached_t d;
      memcpy(&d, data, sizeof d);
      usb_attached(d.dev, d.idx, d.vid, d.pid, data + sizeof d, len - sizeof d);
      break;
    }
    case IPC_USB_DETACHED:
      usb_detached(data[0]);
      break;

    case IPC_USB_HANDLE_DATA:
      usb_handle_data(data[0], data + 1, len - 1);
      break;
#endif

    case IPC_UPDATE_JAMMA:
      memcpy(&jammaData, data, sizeof jammaData);
      debug(("IPC_UPDATE_JAMMA: %x\n", jammaData));
      break;

    case IPC_PS2_DATA: {
      debug(("IPC_PS2_DATA\n"));
      hexdump(data, len);
			if (data[0] == 0) {
      	for (int i=1; i<len; i++) {
        	fifo_Put(&kbdfifo, data[i]);
      	}
			} else {
      	for (int i=1; i<len; i++) {
        	fifo_Put(&mousefifo, data[i]);
      	}
      }

      break;
    }
  }
}

//   tuh_descriptor_get_device_sync(dev, dd, sizeof dd);
//   ipc_SendData(IPC_USB_DEVICE_DESC, dd, sizeof dd);
