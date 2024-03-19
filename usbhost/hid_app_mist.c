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
// #include "ps2.h"
// #define DEBUG
#include "debug.h"
// #include "joypad.h"


#ifdef MIST_USB
void usb_attached(uint8_t dev, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen);
void usb_detached(uint8_t dev);
void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen);
#else
#endif

#ifdef MIST_USB
#else
#endif

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

// If your host terminal support ansi escape code such as TeraTerm
// it can be use to simulate mouse cursor movement within terminal

#define MAX_REPORT  4

#if CFG_TUH_HID

#ifdef MIST_USB
#else
void dumphex(char *s, uint8_t *data, int len) {
    uprintf("%s: ", s);
    for (int i=0; i<len; i++) {
      uprintf("0x%02X,", data[i]);
    }
    uprintf("\n");
}
#endif

// Each HID instance can has multiple reports
static struct hid_info
{
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_REPORT];
  
  uint16_t vid, pid;
} hid_info[CFG_TUSB_HOST_DEVICE_MAX];

#define printf uprintf

  
void hid_app_task(void)
{
}

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



void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
#ifdef DEBUG
  printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  uprintf("tuh_hid_mount_cb(dev_addr:%d inst:%d)\n", dev_addr, instance);
  dumphex("report", desc_report, desc_len);
#endif

  tuh_vid_pid_get(dev_addr, &hid_info[dev_addr].vid, &hid_info[dev_addr].pid);

#ifdef MIST_USB
  usb_attached(dev_addr, instance, hid_info[dev_addr].vid, hid_info[dev_addr].pid, desc_report, desc_len);
#else
  uprintf("\tvid %04X pid %04X\n", hid_info[dev_addr].vid, hid_info[dev_addr].pid);

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
  // tuh_hid_report_received_cb() will be invoked when report is available
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
#ifdef DEBUG
    printf("Error: cannot request to receive report\r\n");
#endif

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
  uprintf("tuh_hid_report_received_cb(dev_addr:%d inst:%d)\n", dev_addr, instance);
  dumphex("report", report, len);
#endif
  tuh_hid_receive_report(dev_addr, instance);
}

#endif
