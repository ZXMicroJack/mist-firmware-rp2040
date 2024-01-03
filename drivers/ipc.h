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
#endif

#ifdef IPC_MASTER
void ipc_InitMaster();
int ipc_Command(uint8_t cmd, uint8_t *data, uint8_t len);
int ipc_ReadBack(uint8_t *data, uint8_t len);
#endif

#endif

