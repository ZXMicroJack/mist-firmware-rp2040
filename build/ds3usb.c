/*
  This file is part of MiST-firmware

  MiST-firmware is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  MiST-firmware is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 Based on the work of

 Copyright (C) 2012 Kristian Lauszus, TKJ Electronics. All rights reserved.
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "max3421e.h"
#include "usb.h"
#include "timer.h"
#include "joystick.h"
#include "joymapping.h"
#include "mist_cfg.h"
#include "state.h"
#include "user_io.h"

// #define DEBUG
#include "drivers/debug.h"

#include "host/usbh.h"
#include "xinput_host.h"
#include "host/usbh_classdriver.h"

static const uint16_t ds3_report_lut[] = {
  0x0204, //r3
  0x0202, //l3

  0x0308, //r2
  0x0304, //l2

  0x0302, //r
  0x0301, //l
  0x0310, //y
  0x0380, //x

  0x0208, //start
  0x0201, //sel
  0x0320, //b
  0x0340, //a

  0x0210, //dup
  0x0240, //ddown
  0x0280, //dleft
  0x0220 // dright
};

static const uint16_t ds4_report_lut[] = {
  0x0680, //r3
  0x0640, //l3

  0x0602, //r2
  0x0601, //l2

  0x0608, //r
  0x0604, //l
  0x0580, //y
  0x0510, //x

  0x0610, //start
  0x0620, //sel
  0x0540, //b
  0x0520, //a

  0x0000, //dup
  0x0000, //ddown
  0x0000, //dleft
  0x0000 // dright
};


// at least 13 bytes can be used here
typedef struct {
	uint16_t *lut; // uint32_t oldButtons;
  uint8_t laxis_lr, laxis_ud;
  uint8_t raxis_lr, raxis_ud;
  uint8_t dpad8;
	uint16_t jindex;
} usb_sonyds_info_t;

uint8_t usb_ds3_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
  if (sizeof (usb_xbox_info_t) < sizeof(usb_sonyds_info_t)) {
    return 1;
  }

  usb_sonyds_info_t *info = (usb_sonyds_info_t *)&dev->xbox_info;
  info->jindex = joystick_add();
  info->lut = (uint16_t *)ds3_report_lut;
  info->laxis_lr = 6;
  info->laxis_ud = 7;
  info->raxis_lr = 8;
  info->raxis_ud = 9;
  info->dpad8 = 0xff;

	return 0;
}

uint8_t usb_ds4_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
  if (sizeof (usb_xbox_info_t) < sizeof(usb_sonyds_info_t)) {
    return 1;
  }

  usb_sonyds_info_t *info = (usb_sonyds_info_t *)&dev->xbox_info;
  info->jindex = joystick_add();
  info->lut = (uint16_t *)ds4_report_lut;
  info->laxis_lr = 1;
  info->laxis_ud = 2;
  info->raxis_lr = 3;
  info->raxis_ud = 4;
  info->dpad8 = 5;

	return 0;
}

uint8_t usb_dsX_release(usb_device_t *dev) {
  usb_sonyds_info_t *info = (usb_sonyds_info_t *)&dev->xbox_info;
	joystick_release(info->jindex);
	return 0;
}

static uint16_t analog_axis_to_digital(uint8_t val, uint16_t maskp, uint16_t maskn) {
  if (val < 0x60) return maskp;
  if (val > 0xa0) return maskn;
  return 0;
}

static const uint16_t dpad8_lut[] = {
  JOY_UP,
  JOY_UP|JOY_RIGHT,
  JOY_RIGHT,
  JOY_RIGHT|JOY_DOWN,
  JOY_DOWN,
  JOY_DOWN|JOY_LEFT,
  JOY_LEFT,
  JOY_UP|JOY_LEFT,
  0
};

void usb_dsX_process(usb_device_t *dev, int inst, uint8_t *rpt, uint16_t read) {
	if(!rpt) return;

  usb_sonyds_info_t *info = (usb_sonyds_info_t *)&dev->xbox_info;

  // printf("rpt = %02x%02x - %02x %02x %02x %02x\n",
  //   rpt[2], rpt[3], rpt[6], rpt[7], rpt[8], rpt[9]);

	uint8_t idx = info->jindex;
	StateUsbIdSet(dev->vid, dev->pid, 8, idx);

	// map virtual joypad
	// if(p->wButtons != dev->xbox_info.oldButtons) {
    uint16_t buttons = 0;

    uint16_t m = 0x8000;
    for (int i=0; i<16; i++) {
      if (rpt[info->lut[i]>>8] & (info->lut[i]&0xff)) buttons |= m;
      m >>= 1;
    }

    if (info->dpad8 != 0xff && (rpt[info->dpad8] & 0x0f) <= 8) {
      buttons |= dpad8_lut[rpt[info->dpad8] & 0x0f];
    }


    buttons |= analog_axis_to_digital(rpt[info->laxis_lr], JOY_LEFT, JOY_RIGHT);
    buttons |= analog_axis_to_digital(rpt[info->laxis_ud], JOY_UP, JOY_DOWN);

		StateUsbJoySet(buttons, buttons>>8, idx);
		uint32_t vjoy = virtual_joystick_mapping(dev->vid, dev->pid, buttons);

		StateJoySet(vjoy, idx);
		StateJoySetExtra( vjoy>>8, idx);

    // Send right joy to OSD
    uint16_t jmap = 0;
    jmap |= analog_axis_to_digital(rpt[info->raxis_lr], JOY_LEFT, JOY_RIGHT);
    jmap |= analog_axis_to_digital(rpt[info->raxis_ud], JOY_UP, JOY_DOWN);
    StateJoySetRight( jmap, idx);

    StateJoySetAnalogue(rpt[info->laxis_lr], rpt[info->laxis_ud], rpt[info->raxis_lr], 
      rpt[info->raxis_ud], idx);

    // add it to vjoy (no remapping)
    vjoy |= jmap<<16;

		// swap joystick 0 and 1 since 1 is the one.
		// used primarily on most systems (most = Amiga and ST...need to get rid of this)
		if(idx == 0)      idx = 1;
		else if(idx == 1) idx = 0;
		// if real DB9 mouse is preffered, switch the id back to 1
		idx = (idx == 0) && mist_cfg.joystick0_prefer_db9 ? 1 : idx;

		user_io_digital_joystick(idx, vjoy & 0xFF);
		// new API with all extra buttons
		user_io_digital_joystick_ext(idx, vjoy);

		virtual_joystick_keyboard_idx( idx, vjoy );
    user_io_analog_joystick(idx, rpt[info->laxis_lr], rpt[info->laxis_ud], rpt[info->raxis_lr], 
      rpt[info->raxis_ud]);
}

static uint8_t usb_dsX_poll(usb_device_t *dev) {
	// usb_hid_info_t *info = &(dev->hid_info);

  uint8_t rpt[49];
  uint16_t read = sizeof rpt;
  uint8_t rcode = usb_in_transfer(dev, NULL, &read, rpt);
  if (!rcode) {
    usb_dsX_process(dev, 0, rpt, read);
  }
	return 0;
}


const usb_device_class_config_t usb_sony_ds3_class = {
  usb_ds3_init, usb_dsX_release, usb_dsX_poll };

const usb_device_class_config_t usb_sony_ds4_class = {
  usb_ds4_init, usb_dsX_release, usb_dsX_poll };
