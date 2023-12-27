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
#include <stdint.h>
#include <stdlib.h>

#include "debug.h"
#include "utils.h"
#include "usbdev.h"
#include "hardware.h"
#include "mist_cfg.h"


#include "usb.h"

#define WORD(a) (a)&0xff, ((a)>>8)&0xff

#define debug(a) printf a


void usb_dev_open(void) {}
void usb_dev_reconnect(void) {}

//TODO MJ - CDC serial port implementation - straight to core - can probably implement - in place of pl2303?
uint8_t  usb_cdc_is_configured(void) { return 0; }
uint16_t usb_cdc_write(const char *pData, uint16_t length) { return 0; }
uint16_t usb_cdc_read(char *pData, uint16_t length) { return 0; }


//TODO MJ - All from storage control - part of USB stack - rp2040 uses own TinyUSB stack for this.
//TODO MJ - can probably stub out storage_control_poll and remove this
// uint8_t  usb_storage_is_configured(void) { return 0; }
// uint16_t usb_storage_write(const char *pData, uint16_t length) { return 0; }
// uint16_t usb_storage_read(char *pData, uint16_t length) { return 0; }

// #ifdef USBFAKE
// uint8_t storage_devices = 0;
// #endif

#ifdef USB

// // usb_data_t *usb_get_handle(uint16_t vid, uint16_t pid) {
// //   usb_data_t *this = malloc(sizeof(usb_data_t));
// //   memset(this, 0, sizeof(usb_data_t));
// //
// //   this->vid = vid;
// //   this->pid = pid;
// // }
//
// // static uint8_t usb_hid_init(usb_device_t *dev, usb_device_descriptor_t *dev_desc) {
// // static uint8_t usb_hid_release(usb_device_t *dev)
// // static uint8_t usb_hid_poll(usb_device_t *dev)
//
// extern const usb_device_class_config_t usb_hid_class;
// extern const usb_device_class_config_t usb_hid_class;

#ifdef TEST_BUILD
#define MAX_USB   1
#else
#define MAX_USB   4
#endif

usb_device_t device[MAX_USB];

struct {
  uint8_t *last_report;
  uint16_t report_size;
  uint8_t *desc_stored;
  uint16_t desc_len;
} report[MAX_USB] = {
  {0,0}
};

uint8_t tuh_descriptor_get_device_sync(uint8_t dev_addr, uint8_t *dd, uint16_t len);


uint8_t dd[128];

void usb_attached(uint8_t dev, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen) {
  usb_device_descriptor_t dd;
  uint8_t r;
  tuh_descriptor_get_device_sync(dev, &dd, sizeof dd);
  report[dev].desc_stored = desc;
  report[dev].desc_len = desclen;
  report[dev].report_size = 0;
  r = usb_hid_class.init(&device[dev], &dd);
  //TODO MJ maybe do more at this point?
}

void usb_detached(uint8_t dev) {
  if (report[dev].report_size) {
    free(report[dev].last_report);
    report[dev].report_size = 0;
    usb_hid_class.release(&device[dev]);
  }
}

void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen) {
  if (report[dev].report_size != desclen) {
    if (report[dev].report_size) free(report[dev].last_report);
    if (desclen) report[dev].last_report = (uint8_t *)malloc(desclen);
    report[dev].report_size = desclen;
  }
  memcpy(report[dev].last_report, desc, desclen);
  usb_hid_class.poll(&device[dev]);
}

// PL2303
// ------
//
// usb_ctrl_req( dev, PL2303_REQ_CDCOUT, CDC_SET_CONTROL_LINE_STATE, state, 0, 0, 0, NULL);
// usb_ctrl_req( dev, PL2303_REQ_CDCOUT, CDC_SET_LINE_CODING, 0, 0, 0, sizeof(line_coding_t), (uint8_t*)dataptr);
// usb_ctrl_req( dev, USB_VENDOR_REQ_IN, 1, val&0xff, val>>8, 0, 1, buf);
// usb_ctrl_req( dev, USB_VENDOR_REQ_OUT, 1, val&0xff, val>>8, index, 0, NULL);
//
// HID
// ---
//
// usb_ctrl_req( dev, HID_REQ_HIDREPORT, USB_REQUEST_GET_DESCRIPTOR, 0x00, HID_DESCRIPTOR_REPORT, info->iface[i].iface_idx, size, buf);
// usb_ctrl_req( dev, HID_REQ_HIDIN, HID_REQUEST_GET_IDLE, reportID,0, iface, 0x0001, duration);
// usb_ctrl_req( dev, HID_REQ_HIDOUT, HID_REQUEST_SET_IDLE, reportID, duration, iface, 0x0000, NULL);
// usb_ctrl_req( dev, HID_REQ_HIDIN, HID_REQUEST_GET_PROTOCOL, 0, 0x00, iface, 0x0001, protocol);
// usb_ctrl_req( dev, HID_REQ_HIDOUT, HID_REQUEST_SET_PROTOCOL, protocol, 0x00, iface, 0x0000, NULL);
// usb_ctrl_req( dev, HID_REQ_HIDOUT, HID_REQUEST_SET_REPORT, report_id, report_type, iface, nbytes, dataptr);


void mist_usb_init() {
}

void mist_usb_loop() {
}

#if 1
// TODO MJ implement usb_ctrl_req
// NOTE: Faked to return report descriptor.
uint8_t usb_ctrl_req( usb_device_t *dev, uint8_t bmReqType,
                      uint8_t bRequest, uint8_t wValLo, uint8_t wValHi,
                      uint16_t wInd, uint16_t nbytes, uint8_t* dataptr) {
  debug(("usb_ctrl_req: dev %08x reqt %02X req %02X wValLo %02X wValHi %02X wInd %04X nbytes %04X dataptr %08x\n",
         dev, bmReqType, bRequest, wValLo, wValHi, wInd, nbytes, dataptr));

  if (bmReqType == HID_REQ_HIDREPORT && bRequest == USB_REQUEST_GET_DESCRIPTOR && wValLo == 0 && wValHi == HID_DESCRIPTOR_REPORT) {
    uint8_t n = dev - device;
    memcpy(dataptr, report[n].desc_stored, lowest(report[n].desc_len, nbytes));
  }

  return 0;
}

// void usb_poll() {}

// TODO MJ implement usb_ctrl_req
// NOTE: Faked purely to pass on the last report.
uint8_t usb_in_transfer( usb_device_t *dev, ep_t *ep, uint16_t *nbytesptr, uint8_t* data) {
  debug(("usb_in_transfer: dev %08x, ep %08x, ptr %08X data %08X\n", dev, ep, nbytesptr, data));
  uint8_t n = dev - device;
  if (report[n].report_size) {
    uint16_t this_copy = lowest(report[n].report_size, *nbytesptr);
    memcpy(data, report[n].last_report, this_copy);
    *nbytesptr = this_copy;
  } else {
    *nbytesptr = 0;
  }
  return 0;
}

// TODO MJ implement usb_ctrl_req
uint8_t usb_out_transfer( usb_device_t *dev, ep_t *ep, uint16_t nbytes, const uint8_t* data ) {
  debug(("usb_out_transfer: dev %08x, ep %08x, nbytes %04X data %08X\n", dev, ep, nbytes, data));
  return 0;
}

usb_device_t *usb_get_devices() {
  debug(("usb_get_devices\n"));
  return device;
}

uint8_t usb_get_conf_descr( usb_device_t *dev, uint16_t nbytes, uint8_t conf, usb_configuration_descriptor_t* dataptr ) {
  debug(("usb_get_conf_descr: dev %08x, nbytes %04X conf %02X data %08X\n", dev, nbytes, conf, dataptr));
  tuh_descriptor_get_configuration_sync(dev - device, conf, dataptr, nbytes);
  return 0;
}

uint8_t usb_set_conf( usb_device_t *dev, uint8_t conf_value ) {
  debug(("usb_set_conf: dev %08x conf %02X\n", dev, conf_value));
#ifndef USBFAKE
  uint8_t dev_addr = dev - device;
  tuh_configuration_set(dev_addr, conf_value, NULL, NULL);
#endif
  return 0;
}
#endif
#endif
