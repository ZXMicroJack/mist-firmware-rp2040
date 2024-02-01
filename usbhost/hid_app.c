/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board.h"
#include "tusb.h"
#include "ps2.h"
// #define DEBUG
#include "debug.h"
#include "joypad.h"
#ifdef RP2U
#include "fifo.h"
#include "ipc.h"
#endif

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

// If your host terminal support ansi escape code such as TeraTerm
// it can be use to simulate mouse cursor movement within terminal

#define MAX_REPORT  4

#if CFG_TUH_HID
// static uint8_t const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };

uint8_t const led2ps2[] = { 0, 4, 1, 5, 2, 6, 3, 7 };
uint8_t const mod2ps2[] = { 0x14, 0x12, 0x11, 0x1f, 0x14, 0x59, 0x11, 0x27 };
uint8_t const hid2ps2[] = {
  0x00, 0x00, 0xfc, 0x00, 0x1c, 0x32, 0x21, 0x23, 0x24, 0x2b, 0x34, 0x33, 0x43, 0x3b, 0x42, 0x4b,
  0x3a, 0x31, 0x44, 0x4d, 0x15, 0x2d, 0x1b, 0x2c, 0x3c, 0x2a, 0x1d, 0x22, 0x35, 0x1a, 0x16, 0x1e,
  0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46, 0x45, 0x5a, 0x76, 0x66, 0x0d, 0x29, 0x4e, 0x55, 0x54,
  0x5b, 0x5d, 0x5d, 0x4c, 0x52, 0x0e, 0x41, 0x49, 0x4a, 0x58, 0x05, 0x06, 0x04, 0x0c, 0x03, 0x0b,
  0x83, 0x0a, 0x01, 0x09, 0x78, 0x07, 0x7c, 0x7e, 0x7e, 0x70, 0x6c, 0x7d, 0x71, 0x69, 0x7a, 0x74,
  0x6b, 0x72, 0x75, 0x77, 0x4a, 0x7c, 0x7b, 0x79, 0x5a, 0x69, 0x72, 0x7a, 0x6b, 0x73, 0x74, 0x6c,
  0x75, 0x7d, 0x70, 0x71, 0x61, 0x2f, 0x37, 0x0f, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40,
  0x48, 0x50, 0x57, 0x5f
};
uint8_t const maparray = sizeof(hid2ps2) / sizeof(uint8_t);

bool kbd_enabled = true;
uint8_t prev_rpt[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#define MS_TYPE_STANDARD  0x00
#define MS_TYPE_WHEEL_3   0x03
#define MS_TYPE_WHEEL_5   0x04

#define MS_MODE_IDLE      0
#define MS_MODE_STREAMING 1

#define MS_INPUT_CMD      0
#define MS_INPUT_SET_RATE 1

alarm_id_t repeater;
uint8_t repeat = 0;
bool blinking = false;
bool receive_kbd = false;
bool receive_ms = false;
bool repeating = false;
uint32_t repeat_us = 35000;
uint16_t delay_ms = 250;
uint8_t ms_type = MS_TYPE_STANDARD;
uint8_t ms_mode = MS_MODE_IDLE;
uint8_t ms_input_mode = MS_INPUT_CMD;
uint8_t ms_rate = 100;
uint32_t ms_magic_seq = 0x00;
uint8_t prev_kbd = 0;
uint8_t resend_kbd = 0;
uint8_t resend_ms = 0;
uint8_t kbd_addr = 0;
uint8_t kbd_inst = 0;
uint8_t leds = 0;

#ifdef RP2U
static uint8_t mistMode = 1;

void HID_setMistMode(uint8_t on) {
  mistMode = on;
}
#endif

// Each HID instance can has multiple reports
static struct hid_info
{
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_REPORT];
  
  uint16_t vid, pid;
  bool joypad;
  uint32_t (*joypad_decode)(struct hid_info *info, uint8_t *rpt, uint32_t len);
  uint8_t joypad_x, joypad_y, joypad_butts;
  uint8_t joypad_inst;
  
#ifdef JOYPAD_INITIAL_MASK
  uint32_t joypad_mask;
  bool joypad_firstreport;
#endif
  uint32_t last_joydata;
  
}hid_info[MAX_USB];

#define printf uprintf

#ifdef USB_ON_RP2U
#define ipc_SendData(a,b,c) while(0) {}
#define ipc_SendDataEx(a,b,c,d,e) while(0) {}

void usb_attached(uint8_t dev, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen) {
  uint8_t dd[USB_DEVICE_DESCRIPTOR_LEN];
  tuh_descriptor_get_device_sync(dev, dd, sizeof dd);
  ipc_SendData(IPC_USB_DEVICE_DESC, dd, sizeof dd);

  uint8_t cfg[256];
  tuh_descriptor_get_configuration_sync(dev, 0, cfg, sizeof cfg);
  ipc_SendDataEx(IPC_USB_CONFIG_DESC, &dev, sizeof dev, cfg, sizeof cfg);

  IPC_usb_attached_t d;
  d.dev = dev;
  d.idx = idx;
  d.vid = vid;
  d.pid = pid;
  ipc_SendDataEx(IPC_USB_ATTACHED, &d, sizeof d, desc, desclen);
}

void usb_detached(uint8_t dev) {
  ipc_SendData(IPC_USB_DETACHED, &dev, sizeof dev);
}

void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen) {
  ipc_SendDataEx(IPC_USB_HANDLE_DATA, &dev, sizeof dev, desc, desclen);
}
#endif

void process_kbd(uint8_t data);
void process_ms(uint8_t data);

void hid_app_task(void)
{
#ifdef RP2U
  // mist mode off, means ps2 reacts in client mode
  // these are normally messages to the keyboard.
  if (!mistMode) {
#endif
    int ch;
    while ((ch = ps2_GetChar(0)) >= 0) {
      process_kbd(ch);
    }
    while ((ch = ps2_GetChar(1)) >= 0) {
      process_ms(ch);
    }
#ifdef RP2U
  }
#endif
}

//--------------------------------------------------------------------+
// Parse reports
//--------------------------------------------------------------------+
#include "parserpt.c"

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
#ifdef DEBUG
  printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
#endif  

#ifdef USB_ON_RP2U
  if (mistMode) {
    tuh_vid_pid_get(dev_addr, &hid_info[dev_addr].vid, &hid_info[dev_addr].pid);
    usb_attached(dev_addr, instance, hid_info[dev_addr].vid, hid_info[dev_addr].pid, desc_report, desc_len);
  } else {
#endif
    // Interface protocol (hid_interface_protocol_enum_t)
    const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    hid_info[dev_addr].joypad = false;

#ifdef RP2U
#ifdef USB_ON_RP2U
  if (mistMode) {
    tuh_vid_pid_get(dev_addr, &hid_info[dev_addr].vid, &hid_info[dev_addr].pid);
    usb_attached(dev_addr, instance, hid_info[dev_addr].vid, hid_info[dev_addr].pid, desc_report, desc_len);
  } else {
#endif
#endif
#ifdef DEBUG
  printf("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]);
#endif
    if ( itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ) {
#ifndef RP2U
      ps2_EnablePort(0, true);
#endif
      kbd_addr = dev_addr;
      kbd_inst = instance;
    } else if ( itf_protocol == HID_ITF_PROTOCOL_MOUSE ) {
#ifndef RP2U
      ps2_EnablePort(1, true);
#endif
    } else if ( itf_protocol == HID_ITF_PROTOCOL_NONE ) {
      hid_info[dev_addr].report_count = tuh_hid_parse_report_descriptor(hid_info[dev_addr].report_info, MAX_REPORT, desc_report, desc_len);
  #ifdef DEBUG
      printf("HID has %u reports \r\n", hid_info[dev_addr].report_count);
      printf("Descriptor len: \n");
  #endif
      uint8_t joystick_desc[4] = {0x05, 0x01, 0x09, 0x04};
      uint16_t vid, pid;
      if (!memcmp(desc_report, joystick_desc, 4)) {
        hid_info[dev_addr].joypad = true;

        // device is a joystick - is it one of our exceptions?
        tuh_vid_pid_get(dev_addr, &vid, &pid);
        // decide which is which
        hid_info[dev_addr].joypad_inst = 0;
        for (int i=0; i<MAX_USB; i++) {
          if (i != dev_addr && hid_info[i].joypad) {
            hid_info[dev_addr].joypad_inst = hid_info[i].joypad_inst ? 0 : 1;
          }
        }
        joypad_Add(hid_info[dev_addr].joypad_inst, dev_addr, vid, pid, desc_report, desc_len);
      }

  #ifdef DEBUG
      for (int i=0; i<desc_len; i++) {
        uprintf("%02X ", desc_report[i]);
        if ((i & 0xf) == 0xf) uprintf("\n");
      }
      uprintf("\n");
  #endif
    }
#ifdef RP2U
#ifdef USB_ON_RP2U
  }
#endif
#endif

  // request to receive report
  // tuh_hid_report_received_cb() will be invoked when report is available
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
#ifdef DEBUG
    printf("Error: cannot request to receive report\r\n");
#endif

  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
#ifdef USB_ON_RP2U
  if (mistMode) {
    usb_detached(dev_addr);
    return;
  }
#endif

#ifdef DEBUG
  printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
#endif
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if ( itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ) {
#ifndef RP2U
    ps2_EnablePort(0, false);
#endif
  } else if ( itf_protocol == HID_ITF_PROTOCOL_MOUSE ) {
#ifndef RP2U
    ps2_EnablePort(1, false);
#endif
  } else if (hid_info[dev_addr].joypad) {
    joypad_Add(hid_info[dev_addr].joypad_inst, dev_addr, 0, 0, NULL, 0);
  }
}


void kbd_set_leds(uint8_t data) {
  if(data > 7) data = 0;
  leds = led2ps2[data];
  tuh_hid_set_report(kbd_addr, kbd_inst, 0, HID_REPORT_TYPE_OUTPUT, &leds, sizeof(leds));
}

int64_t blink_callback(alarm_id_t id, void *user_data) {
  if(kbd_addr) {
    if(blinking) {
      kbd_set_leds(7);
      blinking = false;
      return 500000;
    } else {
      kbd_set_leds(0);
    }
  }
  return 0;
}

int64_t repeat_callback(alarm_id_t id, void *user_data) {
  if(repeat) {
    repeating = true;
    return repeat_us;
  }
  
  repeater = 0;
  return 0;
}

void ps2_send(uint8_t data, bool isKbd) {
#ifndef RP2U
  ps2_SendChar(isKbd ? 0 : 1, data);
#else
  if (mistMode) {
    ps2_InsertChar(isKbd ? 0 : 1, data);
  } else {
    ps2_SendChar(isKbd ? 0 : 1, data);
  }
#endif
}

void ms_send(uint8_t data) {
#ifdef DEBUG
  printf("send MS %02x\n", data);
#endif
  ps2_send(data, false);
}

void kbd_send(uint8_t data) {
#ifdef DEBUG
  printf("send KB %02x\n", data);
#endif
  ps2_send(data, true);
}

void maybe_send_e0(uint8_t data) {
  if(data == 0x46 ||
     data >= 0x48 && data <= 0x52 ||
     data == 0x54 || data == 0x58 ||
     data == 0x65 || data == 0x66 ||
     data >= 0x81) {
    ps2_send(0xe0, true);
  }
}

void process_kbd(uint8_t data) {
  switch(prev_kbd) {
/*
F0	Set scan code set - Upon receiving F0, the keyboard will reply with an ACK (FA) and wait for another byte. 
This byte can be in the range 01 to 03, and it determines the scan code set to be used. Sending 00 as the second 
byte will return the scan code set currently in use.
 */
    
    case 0xed: // CMD: Set LEDs
      prev_kbd = 0;
      kbd_set_leds(data);
    break;
    
    case 0xf3: // CMD: Set typematic rate and delay
      prev_kbd = 0;
      repeat_us = data & 0x1f;
      delay_ms = data & 0x60;
      
      repeat_us = 35000 + repeat_us * 15000;
      
      if(delay_ms == 0x00) delay_ms = 250;
      if(delay_ms == 0x20) delay_ms = 500;
      if(delay_ms == 0x40) delay_ms = 750;
      if(delay_ms == 0x60) delay_ms = 1000;
    break;
    
    default:
      switch(data) {
        case 0xff: // CMD: Reset
          kbd_send(0xfa);
          
          kbd_enabled = true;
          blinking = true;
          add_alarm_in_ms(1, blink_callback, NULL, false);
          
          sleep_ms(16);
          kbd_send(0xaa);
          
          return;
        break;
        
        case 0xfe: // CMD: Resend
          kbd_send(resend_kbd);
          return;
        break;
        
        case 0xee: // CMD: Echo
          kbd_send(0xee);
          return;
        break;
        
        case 0xf2: // CMD: Identify keyboard
          kbd_send(0xfa);
          kbd_send(0xab);
          kbd_send(0x83);
          return;
        break;
        
        case 0xf3: // CMD: Set typematic rate and delay
        case 0xed: // CMD: Set LEDs
          prev_kbd = data;
        break;
        
        case 0xf4: // CMD: Enable scanning
          kbd_enabled = true;
        break;
        
        case 0xf5: // CMD: Disable scanning, restore default parameters
        case 0xf6: // CMD: Set default parameters
          kbd_enabled = data == 0xf6;
          repeat_us = 35000;
          delay_ms = 250;
          kbd_set_leds(0);
        break;
      }
    break;
  }
  
  kbd_send(0xfa);
}

void process_ms(uint8_t data) {
  if(ms_input_mode == MS_INPUT_SET_RATE) {
    ms_rate = data;  // TODO... need to actually honor the sample rate!
    ms_input_mode = MS_INPUT_CMD;
    ms_send(0xfa);

    ms_magic_seq = (ms_magic_seq << 8) | data;
    if(ms_type == MS_TYPE_STANDARD && ms_magic_seq == 0xc86450) {
      ms_type = MS_TYPE_WHEEL_3;
    } else if (ms_type == MS_TYPE_WHEEL_3 && ms_magic_seq == 0xc8c850) {
      ms_type = MS_TYPE_WHEEL_5;
    }
#ifdef DEBUG
    printf("  MS magic seq: %06x type: %d\n", ms_magic_seq, ms_type);
#endif
    return;
  }

  if(data != 0xf3) {
    ms_magic_seq = 0x00;
  }

  switch(data) {
    case 0xff: // CMD: Reset
      ms_type = MS_TYPE_STANDARD;
      ms_mode = MS_MODE_IDLE;
      ms_rate = 100;

      ms_send(0xfa);
      ms_send(0xaa);
      ms_send(ms_type);
    return;

    case 0xf6: // CMD: Set Defaults
      ms_type = MS_TYPE_STANDARD;
      ms_rate = 100;
    case 0xf5: // CMD: Disable Data Reporting
    case 0xea: // CMD: Set Stream Mode
      ms_mode = MS_MODE_IDLE;
      ms_send(0xfa);
    return;

    case 0xf4: // CMD: Enable Data Reporting
      ms_mode = MS_MODE_STREAMING;
      ms_send(0xfa);
    return;

    case 0xf3: // CMD: Set Sample Rate
      ms_input_mode = MS_INPUT_SET_RATE;
      ms_send(0xfa);
    return;

    case 0xf2: // CMD: Get Device ID
      ms_send(0xfa);
      ms_send(ms_type);
    return;

    case 0xe9: // CMD: Status Request
      ms_send(0xfa);
      ms_send(0x00); // Bit6: Mode, Bit 5: Enable, Bit 4: Scaling, Bits[2,1,0] = Buttons[L,M,R]
      ms_send(0x02); // Resolution
      ms_send(ms_rate); // Sample Rate
    return;

// TODO: Implement (more of) these?
//    case 0xfe: // CMD: Resend
//    case 0xf0: // CMD: Set Remote Mode
//    case 0xee: // CMD: Set Wrap Mode
//    case 0xec: // CMD: Reset Wrap Mode
//    case 0xeb: // CMD: Read Data
//    case 0xe8: // CMD: Set Resolution
//    case 0xe7: // CMD: Set Scaling 2:1
//    case 0xe6: // CMD: Set Scaling 1:1
  }

  ms_send(0xfa);
}

#define MAX_JOYPAD 2
uint32_t last_joydata[MAX_JOYPAD] = {0};

void joypad_action(int inst, uint32_t data) {
  uint32_t masks[8] = {JOYPAD_UP, JOYPAD_DOWN, JOYPAD_LEFT, JOYPAD_RIGHT, 0x80, 0x40, 0x20, 0x10};
  uint8_t scancodes[MAX_JOYPAD][8] = {
  // up    down  left, right,fire1 fire2 fire3 fire4
    {0x15, 0x1c, 0x44, 0x4d, 0x29, 0x29, 0x29, 0x29 }, // joystick 1 - opqa-space
    {0x3d, 0x36, 0x2e, 0x3e, 0x45, 0x45, 0x45, 0x45 }, // joystick 2 - cursors-0
  };
  
#ifdef DEBUG
  printf("joypad_action: inst %d data %08X\n", inst, data);
#endif
  for (int i=0; i<sizeof masks / sizeof masks[0]; i++) {
    if (!(last_joydata[inst] & masks[i]) && (data & masks[i])) {
      // button down
      kbd_send(scancodes[inst][i]);
    } else if ((last_joydata[inst] & masks[i]) && !(data & masks[i])) {
      // button up
      kbd_send(0xf0);
      kbd_send(scancodes[inst][i]);
    }
  }
  
  last_joydata[inst] = data;
}


/*
typedef struct {
  enum TRAINING_STATE ts;
  uint8_t *mask;
  uint8_t *prevdata;
  uint8_t *scratch;
  uint8_t *rest;
  int len;
  int n;
  int candidate_pos;
  uint8_t candidate_mask;
  
  uint8_t candidate_rest;
  uint8_t candidate_act;
  
  dir_data_t dir_data[ACT_ENDSTOP];
  uint8_t train_dir;
  struct repeating_timer timer;
  int timeout;
  
} train_data_t;

train_data_t td;

void dumphex(char *s, uint8_t *data, int len) {
    uprintf("%s: ", s);
    for (int i=0; i<len; i++) {
      uprintf("%02X ", data[i]);
    }
    uprintf("\n");
}

static bool training_timer_callback(struct repeating_timer *t) {
  train_data_t *ptd = (train_data_t *)t->user_data;
  ptd->timeout ++;
  
  if (ptd->ts <= TS_NOISE && ptd->timeout > 8) {
    memcpy(ptd->rest, ptd->prevdata, len);
    ptd->ts = TS_FIND_KEY;
    dumphex("noise good - mask", ptd->mask, len);
  }
  
  return true;
}

  add_repeating_timer_us(500000, training_timer_callback,
                         (void *)ptd, &ptd->timer);


*/


void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
#ifdef USB_ON_RP2U
  if (mistMode) {
    usb_handle_data(dev_addr, report, len);
    tuh_hid_receive_report(dev_addr, instance);
    return;
  }
#endif  
  switch(tuh_hid_interface_protocol(dev_addr, instance)) {
    case HID_ITF_PROTOCOL_KEYBOARD:
      
      if(!kbd_enabled || report[1] != 0) {
        tuh_hid_receive_report(dev_addr, instance);
        return;
      }
      
      board_led_write(1);
      
      if(report[0] != prev_rpt[0]) {
        uint8_t rbits = report[0];
        uint8_t pbits = prev_rpt[0];
        
        for(uint8_t j = 0; j < 8; j++) {
          
          if((rbits & 0x01) != (pbits & 0x01)) {
            if(j > 2 && j != 5) kbd_send(0xe0);
            
            if(rbits & 0x01) {
              kbd_send(mod2ps2[j]);
            } else {
              kbd_send(0xf0);
              kbd_send(mod2ps2[j]);
            }
          }
          
          rbits = rbits >> 1;
          pbits = pbits >> 1;
          
        }
        
        prev_rpt[0] = report[0];
      }
      
      for(uint8_t i = 2; i < 8; i++) {
        if(prev_rpt[i]) {
          bool brk = true;
          
          for(uint8_t j = 2; j < 8; j++) {
            if(prev_rpt[i] == report[j]) {
              brk = false;
              break;
            }
          }
          
          // 0x46 - prtscr  E0 12 E0 7C
          if(brk && report[i] < maparray) {
            if(prev_rpt[i] == 0x48) continue;
            if(prev_rpt[i] == 0x46) {
              // MJ - PrtScreen
              kbd_send(0xe0); kbd_send(0xf0); kbd_send(0x7c);
              kbd_send(0xe0); kbd_send(0xf0); kbd_send(0x12);
              continue;
            }
            if(prev_rpt[i] == repeat) repeat = 0;
            
            maybe_send_e0(prev_rpt[i]);
            kbd_send(0xf0);
            kbd_send(hid2ps2[prev_rpt[i]]);
          }
        }
        
        if(report[i]) {
          bool make = true;
          
          for(uint8_t j = 2; j < 8; j++) {
            if(report[i] == prev_rpt[j]) {
              make = false;
              break;
            }
          }
          
          if(make && report[i] < maparray) {
            if(report[i] == 0x48) {
              if(report[0] & 0x1 || report[0] & 0x10) {
                kbd_send(0xe0); kbd_send(0x7e); kbd_send(0xe0); kbd_send(0xf0); kbd_send(0x7e);
              } else {
                kbd_send(0xe1); kbd_send(0x14); kbd_send(0x77); kbd_send(0xe1);
                kbd_send(0xf0); kbd_send(0x14); kbd_send(0xf0); kbd_send(0x77);
              }
              
              continue;
            } else if (report[i] == 0x46) {
              // MJ - PrtScreen
              kbd_send(0xe0); kbd_send(0x12); kbd_send(0xe0); kbd_send(0x7c);
              continue;
            }
            
            repeat = report[i];
            if(repeater) cancel_alarm(repeater);
            repeater = add_alarm_in_ms(delay_ms, repeat_callback, NULL, false);
            
            maybe_send_e0(report[i]);
            kbd_send(hid2ps2[report[i]]);
          }
        }
        
        prev_rpt[i] = report[i];
      }
      
      tuh_hid_receive_report(dev_addr, instance);
      board_led_write(0);
    break;
    
    case HID_ITF_PROTOCOL_MOUSE:
#ifdef DEBUG
      printf("%02x %02x %02x %02x\n", report[0], report[1], report[2], report[3]);
#endif      
      if(ms_mode != MS_MODE_STREAMING) {
        tuh_hid_receive_report(dev_addr, instance);
        return;
      }
      
      board_led_write(1);
      
      uint8_t s = (report[0] & 7) + 8;
      uint8_t x = report[1] & 0x7f;
      uint8_t y = report[2] & 0x7f;
      uint8_t z = report[3] & 7;
      
      if(report[1] >> 7) {
        s += 0x10;
        x += 0x80;
      }
      
      if(report[2] >> 7) {
        y = 0x80 - y;
      } else if(y) {
        s += 0x20;
        y = 0x100 - y;
      }
      
      ms_send(s);
      ms_send(x);
      ms_send(y);
      
      if (ms_type == MS_TYPE_WHEEL_3 || ms_type == MS_TYPE_WHEEL_5) {
        if(report[3] >> 7) {
          z = 0x8 - z;
        } else if(z) {
          z = 0x10 - z;
        }

        if (ms_type == MS_TYPE_WHEEL_5) {
          if (report[0] & 0x8) {
            z += 0x10;
          }

          if (report[0] & 0x10) {
            z += 0x20;
          }
        }

        ms_send(z);
      }
      
      tuh_hid_receive_report(dev_addr, instance);
      board_led_write(0);
    break;
    default: {
      if (hid_info[dev_addr].joypad) {
        uint32_t joydata = joypad_Decode(hid_info[dev_addr].joypad_inst, report, len);
      
        if (joydata != hid_info[dev_addr].last_joydata) {
#ifdef DEBUG
          uprintf("default report: ");
          for (int i=0; i<len; i++) {
            uprintf("%02X ", report[i]);
          }
          uprintf("\n");
#endif

          hid_info[dev_addr].last_joydata = joydata;
//           joypad_action(hid_info[dev_addr].joypad_inst, joydata);
          jamma_SetData(hid_info[dev_addr].joypad_inst, joydata);

#ifndef RP2U
          uint8_t data[] = {hid_info[dev_addr].joypad_inst, joydata};
          ipc_SendData(IPC_UPDATE_JAMMA, data, sizeof data);
#endif
        }
      } else {
#ifdef DEBUG
        uprintf("default report: ");
        for (int i=0; i<len; i++) {
          uprintf("%02X ", report[i]);
        }
        uprintf("\n");
#endif
      }

      tuh_hid_receive_report(dev_addr, instance);
      break;
    }
      
  }
}

#endif
