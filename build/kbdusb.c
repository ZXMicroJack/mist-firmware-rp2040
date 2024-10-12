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


// #include "host/usbh.h"

#include "host/usbh.h"
#include "xinput_host.h"
// #include "/pico/pico-sdk/lib/tinyusb/src/host/usbh_classdriver.h"
#include "host/usbh_classdriver.h"



uint8_t usb_kbd_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
	return 0;
}


static uint8_t usb_kbd_poll(usb_device_t *dev) {
  uint8_t rpt[9];
  uint16_t read = sizeof rpt;
  uint8_t rcode = usb_in_transfer(dev, NULL, &read, rpt);
  if (!rcode) {
    /* handle keyboard message */
    if(read == 8) {
      user_io_kbd(rpt[0], &rpt[2], UIO_PRIORITY_KEYBOARD, dev->vid, dev->pid);
    } else {
      user_io_kbd(rpt[1], &rpt[3], UIO_PRIORITY_KEYBOARD, dev->vid, dev->pid);
    }
  }
	return 0;
}

uint8_t usb_kbd_release(usb_device_t *dev) {
	return 0;
}

const usb_device_class_config_t usb_kbd_class = {
  usb_kbd_init, usb_kbd_release, usb_kbd_poll };
