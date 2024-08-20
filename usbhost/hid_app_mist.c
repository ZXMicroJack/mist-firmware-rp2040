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
#define DEBUG
#include "debug.h"
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

#if CFG_TUH_HID

#ifdef MIST_USB
#define dumphex(a, d, l)
#else
void dumphex(char *s, uint8_t *data, int len) {
    printf("%s: ", s);
    for (int i=0; i<len; i++) {
      if (len > 16 && (i & 0xf) == 0) printf("\n");
      printf("0x%02X,", data[i]);
    }
    printf("\n");
}
#endif

#define SLOW_TEST_REQUESTS

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

#if 0
uint8_t sony_ds3_default_report[] = 
{
    0x01, 0xff, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x27, 0x10, 0x00, 0x32,
    0xff, 0x27, 0x10, 0x00, 0x32,
    0xff, 0x27, 0x10, 0x00, 0x32,
    0xff, 0x27, 0x10, 0x00, 0x32,
    0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

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
      .user_data = NULL
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
  printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  printf("tuh_hid_mount_cb(dev_addr:%d inst:%d)\n", dev_addr, instance);
  dumphex("report", desc_report, desc_len);
#endif
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  hid_type[dev_addr] = NORMAL;
  if (vid == 0x054C && pid == 0x0268) {
    // dualshock 3
    debug(("FOUND DUALSHOCK3\n"));
    hid_type[dev_addr] = DUALSHOCK3;
    hid_setup[dev_addr] = 2;
    sony_ds3_magic_package(dev_addr, instance);

    if ( !tuh_hid_receive_report(dev_addr, instance)) {
      debug(("Error: cannot request to receive report\n"));
    }
    return;
  }


#ifdef MIST_USB
  usb_attached(dev_addr, instance, vid, pid, desc_report, desc_len, USB_TYPE_HID);
#else
  uprintf("\tvid %04X pid %04X\n", vid, pid);

  uint8_t dd[128];
  memset(dd, 0xff, sizeof dd);
  tuh_descriptor_get_device_sync(dev_addr, dd, sizeof dd);
  dumphex("dd", dd, sizeof dd);

//   XFER_RESULT_SUCCESS

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

#ifdef DEBUG
  printf("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]);
#endif
  if ( itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ) {
//     ps2_EnablePort(0, true);
//     kbd_addr = dev_addr;
//     kbd_inst = instance;
  } else if ( itf_protocol == HID_ITF_PROTOCOL_MOUSE ) {
//     ps2_EnablePort(1, true);
  } else if ( itf_protocol == HID_ITF_PROTOCOL_NONE ) {
  }
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
#else
  uprintf("tuh_hid_umount_cb(dev_addr:%d inst:%d)\n", dev_addr, instance);
#ifdef DEBUG
  printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
#endif
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if ( itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ) {
//     ps2_EnablePort(0, false);
  } else if ( itf_protocol == HID_ITF_PROTOCOL_MOUSE ) {
//     ps2_EnablePort(1, false);
//   } else if (hid_info[dev_addr].joypad) {
//     joypad_Add(hid_info[dev_addr].joypad_inst, dev_addr, 0, 0, NULL, 0);
  }
#endif
}

void usb_ToPS2(uint8_t modifier, uint8_t keys[6]);
void usb_ToPS2Mouse(uint8_t report[], uint16_t len);

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
#ifdef MIST_USB

  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if ( itf_protocol == HID_ITF_PROTOCOL_KEYBOARD ) {
    usb_ToPS2(report[0], &report[2]);
  } else if (itf_protocol == HID_ITF_PROTOCOL_MOUSE ) {
    usb_ToPS2Mouse(report, len);
  }
  
  usb_handle_data(dev_addr, report, len);
#else
#ifdef SLOW_TEST_REQUESTS
  if (capture) {
    capture = 0;
#endif
    uprintf("tuh_hid_report_received_cb(dev_addr:%d inst:%d)\n", dev_addr, instance);
    dumphex("report", report, len);
#ifdef SLOW_TEST_REQUESTS
  }
#endif
#endif
  // request_report(dev_addr, instance);

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
