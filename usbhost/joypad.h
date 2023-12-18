#ifndef _JOYPAD_H
#define _JOYPAD_H

#define MAX_USB     5

enum {
  JP_UP,
  JP_DOWN,
  JP_LEFT,
  JP_RIGHT,
  JP_NE,
  JP_SE,
  JP_SW,
  JP_NW,
  JP_FIRE1,
  JP_FIRE2,
  JP_FIRE3,
  JP_FIRE4,
  JP_ENDSTOP
};

enum {
  OP_GT,
  OP_LT,
  OP_BIT0,
  OP_BIT1,
  OP_EQU,
  OP_NONE
};

#define JOYPAD_UP     0x80
#define JOYPAD_DOWN   0x40
#define JOYPAD_LEFT   0x20
#define JOYPAD_RIGHT  0x10
#define JOYPAD_FIRE1  0x08
#define JOYPAD_FIRE2  0x04
#define JOYPAD_FIRE3  0x02
#define JOYPAD_START  0x01

typedef struct {
  struct {
    uint16_t pos;
    uint8_t data; // threshold or bitmask
    uint8_t op;
  } axis[JP_ENDSTOP];
  uint16_t vid, pid;
  uint8_t dev_addr;
  uint8_t joystate;
} joypad_driver_t;

void joypad_Init();
void joypad_Action(int inst, uint32_t data);
void joypad_Add(uint8_t inst, uint8_t dev_addr, uint16_t vid, uint16_t pid, uint8_t *desc_report, uint16_t desc_len);
uint32_t joypad_Decode(uint8_t dev_addr, uint8_t *rpt, uint16_t len);
uint8_t joypad_TrainingAPI(uint8_t *data, int len);
void prd_Parse(uint8_t *desc, uint32_t desclen, uint8_t *off_x, uint8_t *off_y, uint8_t *off_butts);

#endif
