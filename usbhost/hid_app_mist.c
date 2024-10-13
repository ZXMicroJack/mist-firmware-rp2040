/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board.h"
#include "tusb.h"
// #define DEBUG
#ifdef MIST_USB
#include "drivers/debug.h"
#else
#include "debug.h"
#endif
#include "usbhost.h"

#ifdef PIODEBUG
#define printf dbgprintf
#define uprintf dbgprintf
#else
// #define printf debugprintf
#endif

#define MAX_USB 8

enum {
  NORMAL,
  DUALSHOCK3
};
static uint8_t hid_type[MAX_USB] = {0};
static uint8_t hid_setup[MAX_USB] = {0};


#ifndef MIST_USB
// #define SLOW_TEST_REQUESTS
#endif

#if CFG_TUH_HID

// #define uprintf dbgprintf
#ifdef MIST_USB
#define dumphex(a, d, l)
#else
void dumphex(char *s, uint8_t *data, int len) {
    debug(("%s: ", s));
    for (int i=0; i<len; i++) {
      if (len > 16 && (i & 0xf) == 0) debug(("\n"));
      debug(("0x%02X,", data[i]));
    }
    debug(("\n"));
}
#endif

#ifdef SLOW_TEST_REQUESTS
uint64_t then = 0;
uint8_t capture = 0;
void hid_app_task(void)
{
  uint64_t now = time_us_64();

  if ((now - then) > 1000000) {
    then = now;
    capture = 1;
  }
}
#else
void hid_app_task(void)
{
}
#endif
//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0

#if 0 //TODO MJ will need
void kbd_set_leds(uint8_t data) {
  if(data > 7) data = 0;
  leds = led2ps2[data];
  tuh_hid_set_report(kbd_addr, kbd_inst, 0, HID_REPORT_TYPE_OUTPUT, &leds, sizeof(leds));
}
#endif

//--------------------------------------------------------------------+
// Sony Dualshock 3 specifics to wake the bugger up
//--------------------------------------------------------------------+
#define DS3_FEATURE_START_DEVICE 0x03F4
#define DS3_FEATURE_DEVICE_ADDRESS 0x03F2
#define DS3_FEATURE_HOST_ADDRESS 0x03F5

// 0x22 - 0x21 was original
static uint8_t send_control_message(uint8_t dev_addr, uint8_t instance, 
  uint8_t reqtype,
  uint8_t req, uint16_t val, uint8_t *data, uint16_t len) {
  tusb_control_request_t setup_packet = 
  {
      // .bmRequestType = 0x21,
      .bmRequestType = reqtype,
      .bRequest = req, // SET_REPORT
      .wValue = val,
      .wIndex = 0x0000,
      .wLength = len
  };

  tuh_xfer_t transfer = 
  {
      .daddr = dev_addr,
      .ep_addr = 0x00,
      .setup = &setup_packet, 
      .buffer = data,
      .complete_cb = NULL, 
      .user_data = (unsigned int)NULL
  };

  uint8_t result = tuh_control_xfer(&transfer);
  debug(("send_control_message: returns %d\n", result));
  dumphex("data", transfer.buffer, setup_packet.wLength );
  debug(("xfer->result = %d\n", transfer.result));
  return result;
}

static void sony_ds3_magic_package(uint8_t dev_addr, uint8_t instance) {
  // uint8_t magic_packet[] = {0x42, 0x0c, 0x00, 0x00};
  uint8_t magic_packet[] = {0x42, 0x03, 0x00, 0x00};
  send_control_message(dev_addr, instance, 0x21,
    HID_REQ_CONTROL_SET_REPORT, 
    DS3_FEATURE_START_DEVICE, 
    magic_packet, sizeof magic_packet);

  uint8_t control_xfer_buff[17];
  send_control_message(dev_addr, instance, 0xA1, // TBD
    HID_REQ_CONTROL_GET_REPORT, 
    DS3_FEATURE_DEVICE_ADDRESS, 
    control_xfer_buff, 17);

  send_control_message(dev_addr, instance, 0xA1, // TBD
    HID_REQ_CONTROL_GET_REPORT, 
    DS3_FEATURE_HOST_ADDRESS,
    control_xfer_buff, 8);

  send_control_message(dev_addr, instance, 0x21,
    HID_REQ_CONTROL_SET_REPORT,
    0x0201,
    control_xfer_buff, 1);

  send_control_message(dev_addr, instance, 0x21,
    HID_REQ_CONTROL_SET_REPORT, 
    DS3_FEATURE_START_DEVICE, 
    magic_packet, sizeof magic_packet);
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
#ifdef DEBUG
  debug(("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance));
  debug(("tuh_hid_mount_cb(dev_addr:%d inst:%d)\n", dev_addr, instance));
  dumphex("report", desc_report, desc_len);
#endif
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  hid_type[dev_addr] = NORMAL;

  /* DETECT SONY DUALSHOCK 3 */
  if (vid == 0x054C && pid == 0x0268) {
    debug(("FOUND DUALSHOCK3\n"));
    hid_type[dev_addr] = DUALSHOCK3;
    hid_setup[dev_addr] = 2;
    sony_ds3_magic_package(dev_addr, instance);
  }


#ifdef MIST_USB
  uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance); 
  uint8_t type = itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ? USB_TYPE_KEYBOARD :
    itf_protocol == HID_ITF_PROTOCOL_MOUSE ? USB_TYPE_MOUSE : USB_TYPE_HID;
  usb_attached(dev_addr, instance, vid, pid, (uint8_t *)desc_report, desc_len, type);
#else
  uprintf("\tvid %04X pid %04X\n", vid, pid);

  uint8_t dd[128];
  memset(dd, 0xff, sizeof dd);
  tuh_descriptor_get_device_sync(dev_addr, dd, sizeof dd);
  dumphex("dd", dd, sizeof dd);

  memset(dd, 0xff, sizeof dd);
  tuh_descriptor_get_configuration_sync(dev_addr, instance, dd, sizeof dd);
  dumphex("cfg", dd, sizeof dd);

    tuh_configuration_set(dev_addr, 1, NULL, NULL);

  for (int i=0; i<1; i++) {
    memset(dd, 0xff, sizeof dd);
    tuh_descriptor_get_string_sync(dev_addr, i, 0x0409, dd, sizeof dd);
    printf("%d: ", i);
    dumphex("str", dd, sizeof dd);
  }

  memset(dd, 0xff, sizeof dd);
  tuh_descriptor_get_manufacturer_string_sync(dev_addr, 0x0409, dd, sizeof dd);
  dumphex("mfr", dd, sizeof dd);

  memset(dd, 0xff, sizeof dd);
  tuh_descriptor_get_product_string_sync(dev_addr, 0x0409, dd, sizeof dd);
  dumphex("prod", dd, sizeof dd);

  memset(dd, 0xff, sizeof dd);
  tuh_descriptor_get_serial_string_sync(dev_addr, 0x0409, dd, sizeof dd);
  dumphex("serial", dd, sizeof dd);

  // Interface protocol (hid_interface_protocol_enum_t)
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  debug(("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]));
#endif

  // request to receive report
  // request_report(dev_addr, instance);
  if ( !tuh_hid_receive_report(dev_addr, instance)) {
    debug(("Error: cannot request to receive report\n"));
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
#ifdef MIST_USB
  usb_detached(dev_addr);
#endif
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
#ifdef MIST_USB
  usb_handle_data(dev_addr, instance, (uint8_t *)report, len);
#else
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  debug(("itf_protocol = %d\n", itf_protocol));
#ifdef SLOW_TEST_REQUESTS
  if (capture) {
    capture = 0;
#endif
    debug(("tuh_hid_report_received_cb(dev_addr:%d inst:%d)\n", dev_addr, instance));
    dumphex("report", report, len);
#ifdef SLOW_TEST_REQUESTS
  }
#endif
#endif
  /* reissue the wakeup commands for dualshock3 - it takes a bit of waking up */
  if (hid_type[dev_addr] == DUALSHOCK3 && hid_setup[dev_addr]) {
    // dualshock 3
    debug(("FOUND DUALSHOCK3\n"));
    sony_ds3_magic_package(dev_addr, instance);
    hid_setup[dev_addr] --;
  }

  if ( !tuh_hid_receive_report(dev_addr, instance)) {
    debug(("Error: cannot request to receive report\n"));
  }
}

#endif
