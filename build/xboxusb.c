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

//#define DEBUG
#include "drivers/debug.h"

#include "host/usbh.h"
#include "xinput_host.h"
#include "host/usbh_classdriver.h"

static const uint16_t xinput_lut[] = {
  XINPUT_GAMEPAD_RIGHT_THUMB,
  XINPUT_GAMEPAD_LEFT_THUMB,
  XINPUT_GAMEPAD_RIGHT_SHOULDER,
  XINPUT_GAMEPAD_LEFT_SHOULDER,

  0,
  0,
  XINPUT_GAMEPAD_Y,
  XINPUT_GAMEPAD_X,

  XINPUT_GAMEPAD_START,
  XINPUT_GAMEPAD_BACK,
  XINPUT_GAMEPAD_B,
  XINPUT_GAMEPAD_A,

  XINPUT_GAMEPAD_DPAD_UP,
  XINPUT_GAMEPAD_DPAD_DOWN,
  XINPUT_GAMEPAD_DPAD_LEFT,
  XINPUT_GAMEPAD_DPAD_RIGHT
};

// JOY_R3, JOY_L3, JOY_R2, JOY_L2, 
// JOY_R, JOY_L, JOY_Y, JOY_X,
// JOY_START, JOY_SELECT, JOY_B, JOY_A,
// JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT

uint8_t usb_xbox_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
	uint8_t rcode;

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

uint8_t usb_xbox_release(usb_device_t *dev) {
	joystick_release(dev->xbox_info.jindex);
	return 0;
}

static uint16_t analog_axis_to_digital(int16_t val, int16_t min, uint16_t maskp, uint16_t maskn) {
  if (val > min) return maskp;
  if (val < (-min)) return maskn;
  return 0;
}

static uint16_t analog_to_digital(uint8_t val, uint8_t min, uint16_t mask) {
  if (val > min) return mask;
  return 0;
}

// #define swp2(a, msk) (((a & msk)<<1 | (a & msk)>>1) & msk)
void usb_xbox_process(usb_device_t *dev, int inst, uint8_t *buf, uint16_t read) {
	if(!buf) return;

  xinputh_interface_t * xid_itf = (xinputh_interface_t *)buf;
  const xinput_gamepad_t *p = &xid_itf->pad;

  debug(("[%04x:%04x], Buttons %04x, LT: %02x RT: %02x, LX: %d, LY: %d, RX: %d, RY: %d\n",
      dev->vid, dev->pid, p->wButtons, p->bLeftTrigger, p->bRightTrigger, p->sThumbLX, p->sThumbLY, p->sThumbRX, p->sThumbRY));

	uint8_t idx = dev->xbox_info.jindex;
	StateUsbIdSet(dev->vid, dev->pid, 8, idx);

	// map virtual joypad
	// if(p->wButtons != dev->xbox_info.oldButtons) {
    uint16_t buttons = 0;

    uint16_t m = 0x8000;
    for (int i=0; i<16; i++) {
      if (p->wButtons & xinput_lut[i]) buttons |= m;
      m >>= 1;
    }

    buttons |= analog_axis_to_digital(p->sThumbLX, 4096, JOY_RIGHT, JOY_LEFT);
    buttons |= analog_axis_to_digital(p->sThumbLY, 4096, JOY_UP, JOY_DOWN);
    buttons |= analog_to_digital(p->bRightTrigger, 32, JOY_R);
    buttons |= analog_to_digital(p->bLeftTrigger, 32, JOY_L);

		StateUsbJoySet(buttons, buttons>>8, idx);
		uint32_t vjoy = virtual_joystick_mapping(dev->vid, dev->pid, buttons);

		StateJoySet(vjoy, idx);
		StateJoySetExtra( vjoy>>8, idx);

    // Send right joy to OSD
    uint16_t jmap = 0;
    jmap |= analog_axis_to_digital(p->sThumbRX, 4096, JOY_RIGHT, JOY_LEFT);
    jmap |= analog_axis_to_digital(p->sThumbRY, 4096, JOY_UP, JOY_DOWN);
    StateJoySetRight( jmap, idx);

    StateJoySetAnalogue(
      (p->sThumbLX >> 8) + 128,
      (p->sThumbLY >> 8) + 128,
      (p->sThumbRX >> 8) + 128,
      (p->sThumbRY >> 8) + 128, idx );

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

    user_io_analog_joystick(idx,
      p->sThumbLX >> 8, 
      p->sThumbLY >> 8, 
      p->sThumbRX >> 8, 
      p->sThumbRY >> 8);


		dev->xbox_info.oldButtons = p->wButtons;
	// }
}

static uint8_t usb_xbox_poll(usb_device_t *dev) {
	// usb_hid_info_t *info = &(dev->hid_info);

  xinputh_interface_t input;
  uint16_t read = sizeof input;
  uint8_t rcode = usb_in_transfer(dev, NULL, &read, (uint8_t *)&input);
  if (!rcode) {
    usb_xbox_process(dev, 0, (uint8_t *)&input, read);
  }
	return 0;
}


const usb_device_class_config_t usb_xbox_class = {
  usb_xbox_init, usb_xbox_release, usb_xbox_poll };
