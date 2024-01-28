#ifndef _IPC_H
#define _IPC_H

// #define IPC_MAX_PAYLOAD 128

#define IPC_GETRESPONSE     0x00
#define IPC_FLASHDATA       0x01
#define IPC_FLASHCOMMIT     0x02

#define IPC_ECHO            0x03
#define IPC_VERSIONMAJOR    0x04
#define IPC_VERSIONMINOR    0x05

#define IPC_REBOOT          0x06
#define IPC_CHECKINTEGRITY  0x07

#define IPC_PASSTHROUGH     0x08
#define IPC_UPGRADEFRAGMENT 0x09
#define IPC_UPGRADELBA      0x0A

#define IPC_FLASHDATA       0x01
#define IPC_FLASHCOMMIT     0x02

#define IPC_ECHO            0x03
#define IPC_VERSIONMAJOR    0x04
#define IPC_VERSIONMINOR    0x05

#define IPC_REBOOT          0x06
#define IPC_CHECKINTEGRITY  0x07

#define IPC_PASSTHROUGH     0x08
#define IPC_UPGRADEFRAGMENT 0x09
#define IPC_UPGRADELBA      0x0A
#define IPC_BOOTSTRAP       0x0B

#define IPC_UPGRADEPROGRESS 0x0C
#define IPC_UPGRADESTATUS   0x0D
#define IPC_KEEPFPGA        0x0E
#define IPC_DANGEROUSMODE   0x0F
#define IPC_RUNTIME         0x10
#define IPC_BLOCKCHECK      0x11
#define IPC_APPDATA         0x12
#define IPC_UPGRADEFN       0x13
#define IPC_TRAINJOYPAD     0x14

#define IPC_READBACKSIZE    0x80
#define IPC_READBACKDATA    0x81
#define IPC_READKEYBOARD    0x82

#define IPC_SETMISTER       0x15
#define IPC_SETFASTMODE     0x16
#define IPC_USB_SETCONFIG   0x17

// return codes
#define IPC_USB_ATTACHED        0x40
#define IPC_USB_DETACHED        0x41
#define IPC_USB_HANDLE_DATA     0x42
#define IPC_UPDATE_JAMMA        0x43
#define IPC_PS2_DATA            0x44
#define IPC_USB_DEVICE_DESC     0x45
#define IPC_USB_CONFIG_DESC     0x46

#define LEGACY_MODE     2
#define MIST_MODE       1
#define DEFAULT_MODE    0

typedef struct {
  uint8_t dev;
  uint8_t idx;
  uint16_t vid;
  uint16_t pid;
} IPC_usb_attached_t;

#define USB_DEVICE_DESCRIPTOR_LEN   18


enum {
  UPST_IDLE,
  UPST_WORKING,
  UPST_F_CARD,
  UPST_F_FAILIPC,
  UPST_F_NOAPP,
  UPST_F_READ,
  UPST_ENDSTOP
};

#ifdef IPC_SLAVE
void ipc_InitSlave();
int ipc_SlaveTick();
uint8_t ipc_GotCommand(uint8_t cmd, uint8_t *data, uint8_t len);
void ipc_Debug();
fifo_t *ipc_GetFifo();
void ipc_SendData(uint8_t tag, uint8_t *data, uint16_t len);
#endif

int ipc_SetFastMode(uint8_t on);

#ifdef IPC_MASTER
void ipc_InitMaster();
void ipc_MasterTick();
int ipc_Command(uint8_t cmd, uint8_t *data, uint8_t len);
int ipc_ReadBack(uint8_t *data, uint8_t len);
int ipc_ReadBackLen();
uint8_t ipc_ReadKeyboard();
#endif

void ipc_HandleData(uint8_t tag, uint8_t *data, uint16_t len);


#if 0
void ipc_HandleData(uint8_t tag, uint8_t *data, uint16_t len) {
  printf("ipc_HandleData: tag %02X len %d\n", tag, len);
  hexdump(data, len);
}
#endif

#endif

