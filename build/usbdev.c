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

#include <string.h>
#include <stdbool.h>

#include "debug.h"
#include "utils.h"
#include "usbdev.h"
#include "hardware.h"
#include "mist_cfg.h"

#define WORD(a) (a)&0xff, ((a)>>8)&0xff


void usb_dev_open(void) {}
void usb_dev_reconnect(void) {}

//TODO MJ - CDC serial port implementation - straight to core - can probably implement - in place of pl2303?
uint8_t  usb_cdc_is_configured(void) { return 0; }
uint16_t usb_cdc_write(const char *pData, uint16_t length) { return 0; }
uint16_t usb_cdc_read(char *pData, uint16_t length) { return 0; }


//TODO MJ - All from storage control - part of USB stack - rp2040 uses own TinyUSB stack for this.
//TODO MJ - can probably stub out storage_control_poll and remove this
uint8_t  usb_storage_is_configured(void) { return 0; }
uint16_t usb_storage_write(const char *pData, uint16_t length) { return 0; }
uint16_t usb_storage_read(char *pData, uint16_t length) { return 0; }

