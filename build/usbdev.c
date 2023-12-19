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
#include <stdlib.h>

#include "debug.h"
#include "utils.h"
#include "usbdev.h"
#include "hardware.h"
#include "mist_cfg.h"

#include "usb.h"

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

#if 0
// usb_data_t *usb_get_handle(uint16_t vid, uint16_t pid) {
//   usb_data_t *this = malloc(sizeof(usb_data_t));
//   memset(this, 0, sizeof(usb_data_t));
//
//   this->vid = vid;
//   this->pid = pid;
// }

// static uint8_t usb_hid_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
// static uint8_t usb_hid_release(usb_device_t *dev)
// static uint8_t usb_hid_poll(usb_device_t *dev)

extern const usb_device_class_config_t usb_hid_class;
extern const usb_device_class_config_t usb_hid_class;


void usb_attached(usb_data_t *handle, uint8_t idx, uint8_t *desc, uint16_t desclen) {
  usb_device_descriptor_t dd;

  uint8_t r;
//   r = usb_hid_class.init(&handle->device, &dd);
//   r = usb_xbox_class.init(&handle->device, &dd);
//   r = usb_pl2303_class.init(&handle->device, &dd);

}

void usb_detached(usb_data_t *handle) {
}

void usb_handle_data(usb_data_t *handle, uint8_t *desc, uint16_t desclen) {
}



// const usb_device_class_config_t usb_hid_class = {
//   usb_hid_init, usb_hid_release, usb_hid_poll };

#endif

void mist_usb_init() {
  uint8_t r;
  usb_device_descriptor_t dd;
  usb_device_t dev;
  r = usb_hid_class.init(&dev, &dd);
  r = usb_xbox_class.init(&dev, &dd);
  r = usb_pl2303_class.init(&dev, &dd);
}

void mist_usb_loop() {
}

uint8_t usb_ctrl_req( usb_device_t *dev, uint8_t bmReqType,
                      uint8_t bRequest, uint8_t wValLo, uint8_t wValHi,
                      uint16_t wInd, uint16_t nbytes, uint8_t* dataptr) {
  return 0;
}

// void usb_poll() {}

uint8_t usb_in_transfer( usb_device_t *dev, ep_t *ep, uint16_t *nbytesptr, uint8_t* data) {
  return 0;
}

uint8_t usb_out_transfer( usb_device_t *dev, ep_t *ep, uint16_t nbytes, const uint8_t* data ) {
  return 0;
}

usb_device_t *usb_get_devices() {
}

uint8_t usb_get_conf_descr( usb_device_t *dev, uint16_t nbytes, uint8_t conf, usb_configuration_descriptor_t* dataptr ) {
  return 0;
}

uint8_t usb_set_conf( usb_device_t *dev, uint8_t conf_value ) {
  return 0;
}
