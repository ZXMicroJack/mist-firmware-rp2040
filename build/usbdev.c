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
  usb_device_class_config_t *usb_dev;
  uint8_t dev_addr;
  uint8_t inst;
} report[MAX_USB] = {
  {0,0,0,0,NULL}
};

#define MAX_TUSB_DEVS	8

uint8_t dev_tusb[MAX_TUSB_DEVS];


uint8_t tuh_descriptor_get_device_sync(uint8_t dev_addr, uint8_t *dd, uint16_t len);

static void myusb_init() {
  static uint8_t usb_inited = 0;
  if (usb_inited) return;

  for (int i=0; i<MAX_TUSB_DEVS; i++) {
    dev_tusb[i] = 0xff;
  }
  memset(device, 0, sizeof device);
  memset(report, 0, sizeof report);
  usb_inited = 1;
}


void fakeusb_poll() {
#if 0
  for (int i=0; i<MAX_USB; i++) {
    if (report[i].usb_dev != NULL) {
      report[i].usb_dev->poll(&device[i]);
    }
  }
#endif
}

// uint8_t dd[128];

void usb_attached(uint8_t dev, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen) {
  usb_device_descriptor_t dd;
  uint8_t r;

  myusb_init();

  int n;
  for (n=0; n<MAX_USB; n++) {
    if (report[n].usb_dev == NULL) {
      break;
    }
  }

  if (n >= MAX_USB) {
    printf("Error: Too many USB devices...\n");
    return;
  }

  dev_tusb[dev] = n;
  report[n].dev_addr = dev;
  report[n].inst = idx;

  tuh_descriptor_get_device_sync(dev, &dd, sizeof dd);
  report[n].desc_stored = desc;
  report[n].desc_len = desclen;
  report[n].report_size = 0;
  device[n].vid = vid;
  device[n].pid = pid;
  device[n].bAddress = 1;

  r = usb_hid_class.init(&device[n], &dd);
  if (!r) {
    report[n].usb_dev = &usb_hid_class;
//    report[n].usb_dev->poll(&device[n]);
  }
  printf("r returns %d\n", r);
  
  //TODO MJ maybe do more at this point?
}

void usb_detached(uint8_t dev) {
  uint8_t n;

  myusb_init();
  n = dev_tusb[dev];
  if (n > MAX_USB || report[n].usb_dev == NULL) return;

  if (report[n].report_size) {
    free(report[n].last_report);
    report[n].report_size = 0;
  }
  report[n].usb_dev->release(&device[n]);
  report[n].usb_dev = NULL;
}

void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen) {
  uint8_t n;
  
  myusb_init();
  n = dev_tusb[dev];
  if (n > MAX_USB || report[n].usb_dev == NULL) return;

  if (report[n].report_size != desclen) {
    if (report[n].report_size) free(report[dev].last_report);
    if (desclen) report[n].last_report = (uint8_t *)malloc(desclen);
    report[n].report_size = desclen;
  }
  memcpy(report[n].last_report, desc, desclen);
  report[n].usb_dev->poll(&device[n]);
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


// void mist_usb_init() {
// }

// void mist_usb_loop() {
// }

#if 1
// TODO MJ implement usb_ctrl_req
// NOTE: Faked to return report descriptor.
uint8_t usb_ctrl_req( usb_device_t *dev, uint8_t bmReqType,
                      uint8_t bRequest, uint8_t wValLo, uint8_t wValHi,
                      uint16_t wInd, uint16_t nbytes, uint8_t* dataptr) {
  debug(("usb_ctrl_req: dev %08x reqt %02X req %02X wValLo %02X wValHi %02X wInd %04X nbytes %04X dataptr %08x\n",
         dev, bmReqType, bRequest, wValLo, wValHi, wInd, nbytes, dataptr));
  uint8_t n = dev - device;

  if (bmReqType == HID_REQ_HIDREPORT && bRequest == USB_REQUEST_GET_DESCRIPTOR && wValLo == 0 && wValHi == HID_DESCRIPTOR_REPORT) {
    memcpy(dataptr, report[n].desc_stored, lowest(report[n].desc_len, nbytes));
  // } else if (bmReqType == HID_REQ_HIDIN && bRequest == HID_REQUEST_GET_IDLE) {
  } else if (bmReqType == HID_REQ_HIDIN && bRequest == HID_REQUEST_GET_PROTOCOL) {
    *dataptr = tuh_hid_get_protocol(report[n].dev_addr, report[n].inst);
  // } else if (bmReqType == HID_REQ_HIDOUT && bRequest == HID_REQUEST_SET_IDLE) {
  } else if (bmReqType == HID_REQ_HIDOUT && bRequest == HID_REQUEST_SET_PROTOCOL) {
    tuh_hid_set_protocol(report[n].dev_addr, report[n].inst, *dataptr);
  } else if (bmReqType == HID_REQ_HIDOUT && bRequest == HID_REQUEST_SET_REPORT) {
    tuh_hid_set_report(report[n].dev_addr, report[n].inst, wValLo, wValHi, dataptr, nbytes);
  }

  return 0;
}`

// TODO MJ implement usb_ctrl_req
// NOTE: Faked purely to pass on the last report.
uint8_t usb_in_transfer( usb_device_t *dev, ep_t *ep, uint16_t *nbytesptr, uint8_t* data) {
  uint8_t n = dev - device;
  debug(("usb_in_transfer(%04X:%04X:%02X): dev %08x, ep %08x, ptr %08X data %08X\n", 
			  device[n].vid, device[n].pid, device[n].bAddress,
			  dev, ep, nbytesptr, data));
  if (report[n].report_size) {
    debug(("usb_in_transfer(%04X:%04X:%02X): %d %d\n", 
			    device[n].vid, device[n].pid, device[n].bAddress,
			    *nbytesptr, report[n].report_size));


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
//#ifndef USBFAKE
  uint8_t dev_addr = report[dev - device].dev_addr;
  tuh_descriptor_get_configuration_sync(dev_addr, conf, dataptr, nbytes);
//#endif
  return 0;
}

uint8_t usb_set_conf( usb_device_t *dev, uint8_t conf_value ) {
  debug(("usb_set_conf: dev %08x conf %02X\n", dev, conf_value));
#ifndef USBFAKE
  uint8_t dev_addr = report[dev - device].dev_addr;
  tuh_configuration_set(dev_addr, conf_value, NULL, NULL);
#endif
  return 0;
}
#endif
#endif
