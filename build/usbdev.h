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

typedef struct {
  uint16_t vid;
  uint16_t pid;
  usb_device_t device;
} usb_data_t;

usb_data_t *usb_get_handle(uint16_t vid, uint16_t pid);
void usb_attached(usb_data_t *handle, uint8_t idx, uint8_t *desc, uint16_t desclen);
void usb_detached(usb_data_t *handle);
void usb_handle_data(usb_data_t *handle, uint8_t *desc, uint16_t desclen);


#endif // USBDEV_H
