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

#define DEBUG
#include "drivers/debug.h"

#include "host/usbh.h"
#include "xinput_host.h"
#include "host/usbh_classdriver.h"

static const uint16_t report_lut[] = {
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

uint8_t usb_ds3_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
	uint8_t rcode;

  // TODO probably dont need this
	usb_configuration_descriptor_t conf_desc;

	dev->xbox_info.bPollEnable = false;
	
	dev->xbox_info.interval = 4;
	dev->xbox_info.inEp.epAddr = 0x01;
	dev->xbox_info.inEp.epType = EP_TYPE_INTR;
	dev->xbox_info.inEp.maxPktSize = XBOX_EP_MAXPKTSIZE;
	dev->xbox_info.inEp.bmNakPower = USB_NAK_NOWAIT;
	dev->xbox_info.inEp.bmSndToggle = 0;
	dev->xbox_info.inEp.bmRcvToggle = 0;
	dev->xbox_info.qLastPollTime = 0;
	dev->xbox_info.jindex = joystick_add();
	dev->xbox_info.bPollEnable = true;
	return 0;
}

uint8_t usb_ds3_release(usb_device_t *dev) {
	joystick_release(dev->xbox_info.jindex);
	return 0;
}

static uint16_t analog_axis_to_digital(uint8_t val, uint16_t maskp, uint16_t maskn) {
  if (val < 0x60) return maskp;
  if (val > 0xa0) return maskn;
  return 0;
}

void usb_ds3_process(usb_device_t *dev, int inst, uint8_t *rpt, uint16_t read) {
	if(!rpt) return;

  printf("rpt = %02x%02x - %02x %02x %02x %02x\n",
    rpt[2], rpt[3], rpt[6], rpt[7], rpt[8], rpt[9]);

	uint8_t idx = dev->xbox_info.jindex;
	StateUsbIdSet(dev->vid, dev->pid, 8, idx);

	// map virtual joypad
	// if(p->wButtons != dev->xbox_info.oldButtons) {
    uint16_t buttons = 0;

    uint16_t m = 0x8000;
    for (int i=0; i<16; i++) {
      if (rpt[report_lut[i]>>8] & (report_lut[i]&0xff)) buttons |= m;
      m >>= 1;
    }


    buttons |= analog_axis_to_digital(rpt[6], JOY_LEFT, JOY_RIGHT);
    buttons |= analog_axis_to_digital(rpt[7], JOY_UP, JOY_DOWN);

		StateUsbJoySet(buttons, buttons>>8, idx);
		uint32_t vjoy = virtual_joystick_mapping(dev->vid, dev->pid, buttons);

		StateJoySet(vjoy, idx);
		StateJoySetExtra( vjoy>>8, idx);

    // Send right joy to OSD
    uint16_t jmap = 0;
    jmap |= analog_axis_to_digital(rpt[8], JOY_LEFT, JOY_RIGHT);
    jmap |= analog_axis_to_digital(rpt[9], JOY_UP, JOY_DOWN);
    StateJoySetRight( jmap, idx);

    StateJoySetAnalogue(rpt[6], rpt[7], rpt[8], rpt[9], idx);

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

		virtual_joystick_keyboard( vjoy );
    user_io_analog_joystick(idx, rpt[6], rpt[7], rpt[8], rpt[9]);
}

static uint8_t usb_ds3_poll(usb_device_t *dev) {
	// usb_hid_info_t *info = &(dev->hid_info);

  uint8_t rpt[49];
  uint16_t read = sizeof rpt;
  uint8_t rcode = usb_in_transfer(dev, NULL, &read, rpt);
  if (!rcode) {
    usb_ds3_process(dev, 0, rpt, read);
  }
	return 0;
}


const usb_device_class_config_t usb_sony_ds3_class = {
  usb_ds3_init, usb_ds3_release, usb_ds3_poll };
