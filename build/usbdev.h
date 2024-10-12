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

#ifndef USBDEV_H
#define USBDEV_H

#include <inttypes.h>
#include "usb.h"
#include "at91sam_usb.h"

extern const usb_device_class_config_t usb_kbd_class;

void usb_ToPS2Mouse(uint8_t report[], uint16_t len);
void usb_ToPS2(uint8_t modifier, uint8_t keys[6]);

#define BULK_IN_SIZE  AT91C_EP_IN_SIZE
#define BULK_OUT_SIZE AT91C_EP_OUT_SIZE

void usb_dev_open(void);
void usb_dev_reconnect(void);

uint8_t  usb_cdc_is_configured(void);
uint16_t usb_cdc_write(const char *pData, uint16_t length);
uint16_t usb_cdc_read(char *pData, uint16_t length);

uint8_t  usb_storage_is_configured(void);
uint16_t usb_storage_write(const char *pData, uint16_t length);
uint16_t usb_storage_read(char *pData, uint16_t length);

void usb_deferred_poll();

#endif // USBDEV_H
