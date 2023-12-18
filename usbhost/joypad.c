#include <stdint.h>
#include <stdlib.h>

#include "bsp/board.h"
#include "tusb.h"
#include "ps2.h"
// #define DEBUG
#include "debug.h"
#include "joypad.h"

#define printf uprintf

//TODO move to central position
#define RP2U_TRAINING_DATA         0x10100000
#define RP2U_TRAINING_DATA_SIZE   4096
#define RP2U_MAX_TRAINING_RECORDS 78
#define RP2U_TRAINING_RECORD_SIZE 52

#ifdef DEBUG
void dumphex(char *s, uint8_t *data, int len) {
    uprintf("%s: ", s);
    for (int i=0; i<len; i++) {
      uprintf("%02X ", data[i]);
    }
    uprintf("\n");
}
#endif

static uint8_t train_inst = 0xff;

#if 0
uint8_t sample_training_data[] = {
  0x05, 0x4c, 0x02, 0x68, // vid pid
  0x00, 14, 0xff, OP_BIT1, // up
  0x00, 16, 0xff, OP_BIT1, // down
  0x00, 17, 0xff, OP_BIT1, // left
  0x00, 15, 0xff, OP_BIT1, // right
  0x00, 0, 0x00, OP_NONE, // ne
  0x00, 0, 0x00, OP_NONE, // se
  0x00, 0, 0x00, OP_NONE, // sw
  0x00, 0, 0x00, OP_NONE, // nw
  0x00, 2, 0x80, OP_BIT1, // fire1
  0x00, 2, 0x40, OP_BIT1, // fire2
  0x00, 2, 0x20, OP_BIT1, // fire3
  0x00, 2, 0x10, OP_BIT1, // fire3

  0xff, 0xff, 0xff, 0xff // vid pid - EOF
  
};
#endif

#define MAX_JOYPAD  2
static joypad_driver_t driver[MAX_JOYPAD];

static uint32_t bitmap_lut[] = {
  JOYPAD_UP,
  JOYPAD_DOWN,
  JOYPAD_LEFT,
  JOYPAD_RIGHT,
  JOYPAD_UP|JOYPAD_RIGHT,
  JOYPAD_DOWN|JOYPAD_RIGHT,
  JOYPAD_DOWN|JOYPAD_LEFT,
  JOYPAD_UP|JOYPAD_LEFT,
  JOYPAD_FIRE1,
  JOYPAD_FIRE2,
  JOYPAD_FIRE3,
  JOYPAD_START
};  

static void set_axis(int inst, int axis, uint16_t pos, uint8_t data, uint8_t op) {
  driver[inst].axis[axis].pos = pos;
  driver[inst].axis[axis].data = data;
  driver[inst].axis[axis].op = op;
}

uint32_t joypad_DecodeTrained(uint8_t inst, uint8_t *rpt, uint32_t len) {
  joypad_driver_t *td = (joypad_driver_t *)&driver[inst];
  uint32_t data = 0;
  for (int i=0; i<JP_ENDSTOP; i++) {
    switch (td->axis[i].op) {
      case OP_GT:
        data |= (rpt[td->axis[i].pos] > td->axis[i].data) ? bitmap_lut[i] : 0;
        break;
      case OP_LT:
        data |= (rpt[td->axis[i].pos] < td->axis[i].data) ? bitmap_lut[i] : 0;
        break;
      case OP_BIT0:
        data |= (rpt[td->axis[i].pos] & td->axis[i].data) ? 0 : bitmap_lut[i];
        break;
      case OP_BIT1:
        data |= (rpt[td->axis[i].pos] & td->axis[i].data) ? bitmap_lut[i] : 0;
        break;
      case OP_EQU:
        data |= (rpt[td->axis[i].pos] == td->axis[i].data) ? bitmap_lut[i] : 0;
        break;
      case OP_NONE:
        break;
    }
  }
//   debug(("joypad_decode_trained: returns %08X\n", data));
  td->joystate = data;

  return data;
}

typedef struct {
  uint8_t *mask;
  uint8_t *prevdata;
  uint8_t status;
  uint32_t n;
  uint16_t pos;
  uint16_t len;
} train_data_t;

train_data_t td;

#define TR_SETSTATE     0 // nn mm (nn = 01 train, 02 filter; mm = instance)
#define TR_NRREPORTS    1 // none - get number of reports since last check max 255
#define TR_GETPOS       2 // get first change detected at pos:16 offset nn
#define TR_GETRPT       3 // nn nn - get report offset nnn
#define TR_GETUSB       4 // nn mm - get joypad nn offset mm into record [vid:16] [pid:16]
#define TR_SETTRAINING  5 // set training data
#define TR_RESETDETECT  6 // reset detection of change
#define TR_GETSTATE     7 // get joypad state

void set_training_blob(uint8_t dev_addr, uint8_t *data) {
  set_axis(dev_addr, JP_UP,    (data[0]<<8)|data[1],   data[2],  data[3]);
  set_axis(dev_addr, JP_DOWN,  (data[4]<<8)|data[5],   data[6],  data[7]);
  set_axis(dev_addr, JP_LEFT,  (data[8]<<8)|data[9],   data[10], data[11]);
  set_axis(dev_addr, JP_RIGHT, (data[12]<<8)|data[13], data[14], data[15]);
  set_axis(dev_addr, JP_NE,    (data[16]<<8)|data[17], data[18], data[19]);
  set_axis(dev_addr, JP_SE,    (data[20]<<8)|data[21], data[22], data[23]);
  set_axis(dev_addr, JP_SW,    (data[24]<<8)|data[25], data[26], data[27]);
  set_axis(dev_addr, JP_NW,    (data[28]<<8)|data[29], data[30], data[31]);
  set_axis(dev_addr, JP_FIRE1, (data[32]<<8)|data[33], data[34], data[35]);
  set_axis(dev_addr, JP_FIRE2, (data[36]<<8)|data[37], data[38], data[39]);
  set_axis(dev_addr, JP_FIRE3, (data[40]<<8)|data[41], data[42], data[43]);
  set_axis(dev_addr, JP_FIRE4, (data[44]<<8)|data[45], data[46], data[47]);
}


static void joypad_TrainingCancel() {
  if (td.mask) free(td.mask);
  if (td.prevdata) free(td.prevdata);
  td.prevdata = td.mask = NULL;
  train_inst = 0xff;
  td.status = 0;
}

uint8_t joypad_TrainingAPI(uint8_t *data, int len) {
  uint8_t n;
  switch (data[0]) {
    case TR_SETSTATE:
      td.status = data[1];
      train_inst = data[2];
      if (!data[1]) {
        joypad_TrainingCancel();
      }
      return td.status;

    case TR_NRREPORTS:
      return td.n;

    case TR_GETPOS:
      if (data[1]) return td.pos >> 8;
      else return td.pos & 0xff;
    
    case TR_RESETDETECT:
      td.pos = 0xffff;
      td.n = 0;
      return 0;
    
    case TR_GETRPT: {
      uint16_t pos = (data[1] << 8) | data[2];
      return td.prevdata[pos > td.len ? 0 : pos];
    }
    
    case TR_GETUSB: {
      switch(data[2]) {
        case 0: return driver[data[1]].vid >> 8;
        case 1: return driver[data[1]].vid & 0xff;
        case 2: return driver[data[1]].pid >> 8;
        case 3: return driver[data[1]].pid & 0xff;
      }
      break;
    }
    
    case TR_SETTRAINING:
      set_training_blob(data[1], &data[2]);
      return 0x00;

    case TR_GETSTATE:
      return driver[data[1]].joystate;
    
  }
  return 0xef;
}

void joypad_Init() {
  memset(&td, 0, sizeof td);
  memset(&driver, 0, sizeof driver);
}


void joypad_Train(uint8_t *rpt, int len) {
  if (!td.status) return;
  
  if (!td.mask) {
    td.mask = (uint8_t *)malloc(len);
    td.prevdata = (uint8_t *)malloc(len);
    memset(td.mask, 0xff, len);
    td.len = len;
  }

  // train mask
  if (td.status == 2) {
      for (int i=0; i<td.len; i++) {
        td.mask[i] &= ~((rpt[i] & td.mask[i]) ^ (td.prevdata[i] & td.mask[i]));
      }
  }

  int diffs = 0;
  for (int i=0; i<td.len; i++) {
    if ((rpt[i] & td.mask[i]) != (td.prevdata[i] & td.mask[i])) {
      if (td.pos == 0xffff) td.pos = i;
      diffs ++;
    }
    td.prevdata[i] = rpt[i];
  }

  if (diffs) td.n++;
}

void joypad_Add(uint8_t inst, uint8_t dev_addr, uint16_t vid, uint16_t pid, uint8_t *desc_report, uint16_t desc_len) {
    debug(("joypad_decode: dev_addr %02X VID %04X PID %04X\r\n", dev_addr, vid, pid));

    // cancel training if joypad is removed
    if (train_inst == inst) {
      joypad_TrainingCancel();
    }

    // update basics
    driver[inst].vid = vid;
    driver[inst].pid = pid;
    driver[inst].dev_addr = dev_addr;
    if (vid == 0 && pid == 0) return;
      
    // Find stored trained data
    debug(("Going through stored training data\n"));

    uint8_t *trec = (uint8_t *) RP2U_TRAINING_DATA;
    for (int i=0; i<RP2U_MAX_TRAINING_RECORDS; i++) {
      uint16_t trvid, trpid;
      trvid = (trec[0] << 8) | trec[1];
      trpid = (trec[2] << 8) | trec[3];
      
      if (trvid == vid && trpid == pid) {
        debug(("found trained model\n"));
        set_training_blob(inst, trec + 4);
        return;
      }
      if ((trvid == 0xffff && trpid == 0xffff) || (trvid == 0x0000 && trpid == 0x0000)) {
        break; // no record found
      }
      trec += RP2U_TRAINING_RECORD_SIZE;
    }
      
    // otherwise try to use descriptor
    uint8_t joypad_x, joypad_y, joypad_butts;
    prd_Parse(desc_report, desc_len, &joypad_x, &joypad_y, &joypad_butts);
    set_axis(inst, JP_RIGHT, joypad_x, 0x88, OP_GT);
    set_axis(inst, JP_LEFT, joypad_x, 0x77, OP_LT);
    set_axis(inst, JP_DOWN, joypad_y, 0x88, OP_GT);
    set_axis(inst, JP_UP, joypad_y, 0x77, OP_LT);
    set_axis(inst, JP_NE, 0, 0, OP_NONE);
    set_axis(inst, JP_SE, 0, 0, OP_NONE);
    set_axis(inst, JP_SW, 0, 0, OP_NONE);
    set_axis(inst, JP_NW, 0, 0, OP_NONE);
    set_axis(inst, JP_FIRE1, joypad_butts, 0x80, OP_BIT1);
    set_axis(inst, JP_FIRE2, joypad_butts, 0x40, OP_BIT1);
    set_axis(inst, JP_FIRE3, joypad_butts, 0x20, OP_BIT1);
    set_axis(inst, JP_FIRE4, joypad_butts, 0x10, OP_BIT1);
}

uint32_t joypad_Decode(uint8_t inst, uint8_t *rpt, uint16_t len) {
//   printf("joypad_decode: inst %02X rpt .. len %d\n", inst, len);
  if (train_inst == inst) {
    joypad_Train(rpt, len);
  }
  return joypad_DecodeTrained(inst, rpt, len);
}
