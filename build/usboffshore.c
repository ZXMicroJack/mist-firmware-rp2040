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

#define USB_ON_CORE2

#define lowest(a,b) ((a) < (b) ? (a) : (b))

static uint8_t storedDD[USB_DEVICE_DESCRIPTOR_LEN];
static uint8_t config[MAX_USB][256];

static uint32_t jammaData = 0xffff0000;

#ifdef MB2USB
void tuh_task() {
}
void hid_app_task() {}
#endif

static fifo_t kbdfifo;
static uint8_t kbdfifo_buf[64];

static fifo_t mousefifo;
static uint8_t mousefifo_buf[64];

#ifdef USB_ON_CORE2
static fifo_t kbdusbfifo;
static uint8_t kbdusbfifo_buf[64];

static fifo_t mouseusbfifo;
static uint8_t mouseusbfifo_buf[64];

static uint8_t jamma[2] = {0,0};
static uint8_t jamma_prev[2] = {0,0};
#endif

/********************************************/
/* JAMMA mb2 */
void jamma_Init() {
}

uint32_t jamma_GetData(uint8_t inst) {
  uint8_t d = ~(inst ? (jammaData >> 24) : (jammaData >> 16));
  // debug(("jamma_GetData: inst %d returns %02X\n", inst, d));
  return d;
}

#ifdef USB_ON_CORE2
void jamma_SetData(uint8_t inst, uint32_t data) {
  jamma[inst] = data;
}

void jamma_SendMessages() {
  uint8_t cmddata[2];
  for (int i=0; i<2; i++) {
    if (jamma[i] != jamma_prev[i]) {
      cmddata[0] = i;
      cmddata[1] = jamma[i];
      ipc_Command(IPC_SENDJAMMA, cmddata, sizeof cmddata);
      jamma_prev[i] = jamma[i];
    }
  }
}
#else
void jamma_SetData(uint8_t inst, uint32_t data) {
  uint8_t cmddata[2];
  cmddata[0] = inst;
  cmddata[1] = data;
  ipc_Command(IPC_SENDJAMMA, cmddata, sizeof cmddata);
}
#endif


#ifdef CORE2_IPC_TICKS
static void ipc_core() {
  ipc_InitMaster();
  for(;;) {
    ipc_MasterTick();
  }
}
#endif

// #define MAP_KEY_TO_START
#ifdef MAP_KEY_TO_START
int prev_key = 0;
int _ps2_GetChar(uint8_t ch);
int ps2_GetChar(uint8_t ch) {
  int n = _ps2_GetChar(ch);
  if (n == 0x0e) {
    user_io_digital_joystick(0, prev_key == 0xf0 ? 0 : 0x80);
    user_io_digital_joystick_ext(0, prev_key == 0xf0 ? 0 : 0x80);
    user_io_digital_joystick(1, prev_key == 0xf0 ? 0 : 0x80);
    user_io_digital_joystick_ext(1, prev_key == 0xf0 ? 0 : 0x80);
    printf("Pressing start realase status = %d\n", prev_key == 0xf0);
  }
  prev_key = n;
  return n;
}
#define ps2_GetChar _ps2_GetChar
#endif

int ps2_GetChar(uint8_t ch) {
  return ch == 0 ? fifo_Get(&kbdfifo) : fifo_Get(&mousefifo);
}

static uint8_t ipc_started = 0;
void ps2_Init() {
  fifo_Init(&kbdfifo, kbdfifo_buf, sizeof kbdfifo_buf);
  fifo_Init(&mousefifo, mousefifo_buf, sizeof mousefifo_buf);
#ifdef USB_ON_CORE2
  fifo_Init(&kbdusbfifo, kbdusbfifo_buf, sizeof kbdusbfifo_buf);
  fifo_Init(&mouseusbfifo, mouseusbfifo_buf, sizeof mouseusbfifo_buf);
#endif

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
void ps2_SwitchMode(int hostMode) {}


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
  debug(("ipc_HandleData: tag %02x len %d\n", tag, len));
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
      usb_handle_data(data[0], 0, data + 1, len - 1);
      break;
#endif

    case IPC_UPDATE_JAMMA:
      memcpy(&jammaData, data, sizeof jammaData);
      debug(("IPC_UPDATE_JAMMA: %x\n", jammaData));
      break;

    case IPC_PS2_DATA: {
      debug(("IPC_PS2_DATA\n"));
      hexdump1(data, len);
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

void mb2_SendPS2(uint8_t *data, uint8_t len) {
  debug(("mb2_SendPS2(len %d)\n", len));
#ifdef USB_ON_CORE2
  if (len > 1) {
    uint8_t ch = *data++;
    len --;
    if (!ch) while (len --) fifo_Put(&kbdusbfifo, *data++);
    else     while (len --) fifo_Put(&mouseusbfifo, *data++);
  }
#else
  ipc_Command(IPC_SENDPS2, data, len);
#endif
}

#ifdef USB_ON_CORE2
void mb2_SendPS2FromQueue(uint8_t ch, fifo_t *fifo) {
  uint16_t count = fifo_Count(fifo);
  if (count) {
    debug(("mb2_SendPS2FromQueue: ch %d count %d)\n", ch, count));
    int len = 0;
    uint8_t data[64];

    data[len++] = ch;
    while (len < sizeof data && len <= count) {
      data[len++] = fifo_Get(fifo);
    }
    ipc_Command(IPC_SENDPS2, data, len);
  }
}

void mb2_SendMessages() {
  mb2_SendPS2FromQueue(0, &kbdusbfifo);
  mb2_SendPS2FromQueue(1, &mouseusbfifo);
  jamma_SendMessages();
}
#else
void mb2_SendMessages() {
}
#endif
