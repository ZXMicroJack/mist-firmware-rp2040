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
static uint8_t ds3_report[MAX_USB][17];
static uint8_t setup_packets[MAX_USB];


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

static void request_report(uint8_t dev_addr, uint8_t instance);

static void tuh_xfer_sony_ds_cb(tuh_xfer_t *xfer) {
  // xfer->user_data;
  dumphex("buffer", xfer->buffer, 17);
  debug(("xfer->result = %d\n", xfer->result));
  // request to receive report
  request_report(xfer->daddr, (uint8_t)xfer->user_data);
}


uint8_t tuh_hid_receive_report_sony_ds(uint8_t dev_addr, uint8_t instance) {
  tusb_control_request_t setup_packet = 
  {
      .bmRequestType = 0xA1,  
      .bRequest = 0x01, // GET_REPORT
      .wValue = (HID_REPORT_TYPE_FEATURE << 8) | 0xF2, 
      .wIndex = 0x0000,    
      .wLength = 17
  };

  tuh_xfer_t transfer = 
  {
      .daddr = dev_addr,
      .ep_addr = 0x00,
      .setup = &setup_packet, 
      .buffer = ds3_report[dev_addr],
      .complete_cb = tuh_xfer_sony_ds_cb, 
      .user_data = (void *)instance
  };

  return tuh_control_xfer(&transfer);
}
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


#if 0
NTSTATUS
SendControlRequest(
    _In_ PDEVICE_CONTEXT Context,
    _In_ WDF_USB_BMREQUEST_DIRECTION Direction,
    _In_ WDF_USB_BMREQUEST_TYPE Type,
    _In_ BYTE Request,
    _In_ USHORT Value,
    _In_ USHORT Index,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength)
{

    // "Magic packet"
    UCHAR hidCommandEnable[DS3_HID_COMMAND_ENABLE_SIZE] =
    {
        0x42, 0x0C, 0x00, 0x00
    };

    return SendControlRequest(
        Context,
        BmRequestHostToDevice,
        BmRequestClass,
        SetReport,
        Ds3FeatureStartDevice,
        0,
        hidCommandEnable,
        DS3_HID_COMMAND_ENABLE_SIZE
    );
}
#endif

// HID_REQ_CONTROL_SET_IDLE,
//req = HID_REQ_CONTROL_SET_REPORT
//val = 0x03f4
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


#if 0
        uint8_t cmd_buf[4];
        cmd_buf[0] = 0x42; // Special PS3 Controller enable commands
        cmd_buf[1] = 0x0c;
        cmd_buf[2] = 0x00;
        cmd_buf[3] = 0x00;

        // bmRequest = Host to device (0x00) | Class (0x20) | Interface (0x01) = 0x21,
        //  bRequest = Set Report (0x09), 
        // Report ID (0xF4), 
        //Report Type (Feature 0x03), interface (0x00), 
        // datalength, datalength, data)
        pUsb->ctrlReq(bAddress, 
          epInfo[PS3_CONTROL_PIPE].epAddr, 
          bmREQ_HID_OUT, 
          HID_REQUEST_SET_REPORT, 
          0xF4, 0x03, 
          0x00, 4, 4, cmd_buf, NULL);
#endif

#define CONTROL_TRANSFER_BUFFER_LENGTH      64

#if 0
        // 
        // Initial output state (rumble & LEDs off)
        // 
        UCHAR DefaultOutputReport[DS4_HID_OUTPUT_REPORT_SIZE] =
        {
            0x05, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
#endif


#if 0

https://github.com/search?q=Ds3FeatureStartDevice&type=code

#define SIXAXIS_REPORT_0xF2_SIZE 17
#define SIXAXIS_REPORT_0xF5_SIZE 8


static int sixaxis_set_operational_usb(struct hid_device *hdev)
{
	struct sony_sc *sc = hid_get_drvdata(hdev);
	const int buf_size =
		max(SIXAXIS_REPORT_0xF2_SIZE, SIXAXIS_REPORT_0xF5_SIZE);
	u8 *buf;
	int ret;

	buf = kmalloc(buf_size, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = hid_hw_raw_request(hdev, 0xf2, buf, SIXAXIS_REPORT_0xF2_SIZE, 
				 HID_FEATURE_REPORT, HID_REQ_GET_REPORT);
	if (ret < 0) {
		hid_err(hdev, "can't set operational mode: step 1\n");
		goto out;
	}

	/*
	 * Some compatible controllers like the Speedlink Strike FX and
	 * Gasia need another query plus an USB interrupt to get operational.
	 */
	ret = hid_hw_raw_request(hdev, 0xf5, buf, SIXAXIS_REPORT_0xF5_SIZE,
				 HID_FEATURE_REPORT, HID_REQ_GET_REPORT);
	if (ret < 0) {
		hid_err(hdev, "can't set operational mode: step 2\n");
		goto out;
	}

	/*
	 * But the USB interrupt would cause SHANWAN controllers to
	 * start rumbling non-stop, so skip step 3 for these controllers.
	 */
	if (sc->quirks & SHANWAN_GAMEPAD)
		goto out;

	ret = hid_hw_output_report(hdev, buf, 1);
	if (ret < 0) {
		hid_info(hdev, "can't set operational mode: step 3, ignoring\n");
		ret = 0;
	}

out:
	kfree(buf);

	return ret;
}











typedef enum {
  TUSB_REQ_GET_STATUS        = 0  ,
  TUSB_REQ_CLEAR_FEATURE     = 1  ,
  TUSB_REQ_RESERVED          = 2  ,
  TUSB_REQ_SET_FEATURE       = 3  ,
  TUSB_REQ_RESERVED2         = 4  ,
  TUSB_REQ_SET_ADDRESS       = 5  ,
  TUSB_REQ_GET_DESCRIPTOR    = 6  ,
  TUSB_REQ_SET_DESCRIPTOR    = 7  ,
  TUSB_REQ_GET_CONFIGURATION = 8  ,
  TUSB_REQ_SET_CONFIGURATION = 9  ,
  TUSB_REQ_GET_INTERFACE     = 10 ,
  TUSB_REQ_SET_INTERFACE     = 11 ,
  TUSB_REQ_SYNCH_FRAME       = 12
} tusb_request_code_t;





typedef enum {
  TUSB_DIR_OUT = 0,
  TUSB_DIR_IN  = 1,

  TUSB_DIR_IN_MASK = 0x80
} tusb_dir_t;


typedef enum {
  TUSB_REQ_RCPT_DEVICE =0,
  TUSB_REQ_RCPT_INTERFACE,
  TUSB_REQ_RCPT_ENDPOINT,
  TUSB_REQ_RCPT_OTHER
} tusb_request_recipient_t;

typedef enum {
  TUSB_REQ_TYPE_STANDARD = 0,
  TUSB_REQ_TYPE_CLASS,
  TUSB_REQ_TYPE_VENDOR,
  TUSB_REQ_TYPE_INVALID
} tusb_request_type_t;


            uint8_t recipient :  5; ///< Recipient type tusb_request_recipient_t.
            uint8_t type      :  2; ///< Request type tusb_request_type_t.
            uint8_t direction :  1; ///< Direction type. tusb_dir_t
        } bmRequestType_bit;



 tried it with TinyUSB:
const tusb_control_request_t xfer_ctrl_req = {
                .bmRequestType_bit.recipient = TUSB_REQ_RCPT_INTERFACE,
                .bmRequestType_bit.type = TUSB_REQ_TYPE_CLASS,
                .bmRequestType_bit.direction = TUSB_DIR_IN,
                .bRequest = HID_REQ_CONTROL_GET_REPORT,
                .wValue = (HID_REPORT_TYPE_FEATURE << 8) + cR.usbRequest.reportId,
                .wIndex = 0,
                .wLength = cR.usbRequest.reportLen
};

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




  send_control_message(dev_addr, instance, 0x21,
    HID_REQ_CONTROL_SET_REPORT, 
    // HID_REPORT_TYPE_FEATURE
    DS3_FEATURE_START_DEVICE, 
    magic_packet, sizeof magic_packet);








#endif



#if 0
    // Dual Shock 3 Sixasis requires a magic packet to be sent in order to enable reports. Taken from:
    // https://github.com/torvalds/linux/blob/1d1df41c5a33359a00e919d54eaebfb789711fdc/drivers/hid/hid-sony.c#L1684
    static uint8_t sixaxisEnableReports[] = {(HID_MESSAGE_TYPE_SET_REPORT << 4) | HID_REPORT_TYPE_FEATURE,
                                             0xf4,  // Report ID
                                             0x42,
                                             0x03,
                                             0x00,
                                             0x00};
    uni_hid_device_send_ctrl_report(d, (uint8_t*)&sixaxisEnableReports, sizeof(sixaxisEnableReports));






void uni_hid_device_send_ctrl_report(uni_hid_device_t* d, const uint8_t* report, uint16_t len) {
    uni_hid_device_send_report(d, d->conn.control_cid, report, len);
#endif

void sony_ds3_magic_package(uint8_t dev_addr, uint8_t instance) {
  // uint8_t magic_packet[] = {0x42, 0x0c, 0x00, 0x00};
  uint8_t magic_packet[] = {0x42, 0x03, 0x00, 0x00};
  send_control_message(dev_addr, instance, 0x21,
    HID_REQ_CONTROL_SET_REPORT, 
    // HID_REPORT_TYPE_FEATURE
    DS3_FEATURE_START_DEVICE, 
    magic_packet, sizeof magic_packet);

  uint8_t control_xfer_buff[CONTROL_TRANSFER_BUFFER_LENGTH];
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
    // HID_REPORT_TYPE_FEATURE
    DS3_FEATURE_START_DEVICE, 
    magic_packet, sizeof magic_packet);

#if 0
  send_control_message(dev_addr, instance, 0x21,
    HID_REQ_CONTROL_SET_REPORT,
    0x0201,
    sony_ds3_default_report, sizeof sony_ds3_default_report);
#endif

  // bool result = tuh_hid_set_report(dev_addr, instance, 0xf4, 0x03, magic_packet, 4);
  // debug(("magic packet: returns %d\n", result));
#if 0
  tusb_control_request_t setup_packet = 
  {
      .bmRequestType = 0x21,
      .bRequest = 0x09, // SET_REPORT
      .wValue = 0x03f4,
      .wIndex = 0x0000,
      .wLength = 4
  };

  tuh_xfer_t transfer = 
  {
      .daddr = dev_addr,
      .ep_addr = 0x00,
      .setup = &setup_packet, 
      .buffer = magic_packet,
      .complete_cb = NULL, 
      .user_data = NULL
  };

  debug(("magic packet: returns %d\n", tuh_control_xfer(&transfer)));
  dumphex("data", transfer.buffer, setup_packet.wLength );
  debug(("xfer->result = %d\n", transfer.result));
#endif
}

void sony_ds3_setup(uint8_t dev_addr, uint8_t instance) {
  tusb_control_request_t setup_packet = 
  {
      .bmRequestType = 0xA1,  
      .bRequest = 0x01, // GET_REPORT
      .wValue = (HID_REPORT_TYPE_FEATURE << 8) | 0xF2, 
      .wIndex = 0x0000,
      .wLength = 17
  };

  tuh_xfer_t transfer = 
  {
      .daddr = dev_addr,
      .ep_addr = 0x00,
      .setup = &setup_packet, 
      .buffer = ds3_report[dev_addr],
      .complete_cb = NULL, 
      .user_data = NULL
      // .complete_cb = tuh_xfer_sony_ds_cb, 
      // .user_data = (void *)instance
  };

  for (int i=0; i<4; i++) {
    // switch(setup_packets[dev_addr]) {
    switch(i) {
      case 0: // leave as is
      case 1: // leave as is
        break;
      case 2: // leave as is
        setup_packet.wLength = 8;
        break;
      case 3: // set report
          setup_packet.bmRequestType = 0x21;
          setup_packet.bRequest = 0x09; // SET_REPORT
          setup_packet.wValue = 0x0201;
          setup_packet.wIndex = 0x0000; // same
          setup_packet.wLength = sizeof(sony_ds3_default_report);
          transfer.buffer = sony_ds3_default_report;
          sony_ds3_default_report[9] = 0x02; // led 1
          sony_ds3_default_report[10] = 0xff; // led 1
          break;
    }

    debug(("packet %d: returns %d\n", i, tuh_control_xfer(&transfer)));
    dumphex("data", transfer.buffer, setup_packet.wLength );
    debug(("xfer->result = %d\n", transfer.result));
  }
}


// static void request_report(uint8_t dev_addr, uint8_t instance) {
//   if (hid_type[dev_addr] == DUALSHOCK3 && setup_packets[dev_addr] < 3) {
//     if (!tuh_hid_receive_report_sony_ds(dev_addr, instance)) {
//       debug(("Error: cannot request DS3 to receive report\n"));
//     }

//   } else {
//     // tuh_hid_report_received_cb() will be invoked when report is available
//     if ( !tuh_hid_receive_report(dev_addr, instance)) {
//       debug(("Error: cannot request to receive report\n"));
//     }
//   }
// }

int magic_count = 0;

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
#ifdef DEBUG
  printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  printf("tuh_hid_mount_cb(dev_addr:%d inst:%d)\n", dev_addr, instance);
  dumphex("report", desc_report, desc_len);
#endif
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  if (vid == 0x054C && pid == 0x0268) {
    // dualshock 3
    debug(("FOUND DUALSHOCK3\n"));
    sony_ds3_magic_package(dev_addr, instance);
    magic_count = 3;
    if ( !tuh_hid_receive_report(dev_addr, instance)) {
      debug(("Error: cannot request to receive report\n"));
    }
    return;
    // sony_ds3_setup(dev_addr, instance);
    // hid_type[dev_addr] = NORMAL;
  //   hid_type[dev_addr] = DUALSHOCK3;
  //   setup_packets[dev_addr] = 0;
  // } else {
  //   debug(("FOUND USB HID\n"));
  //   hid_type[dev_addr] = NORMAL;
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

  if (vid == 0x054C && pid == 0x0268) {
    // dualshock 3
    debug(("FOUND DUALSHOCK3\n"));
    sony_ds3_magic_package(dev_addr, instance);
    // sony_ds3_setup(dev_addr, instance);
    // hid_type[dev_addr] = NORMAL;
  //   hid_type[dev_addr] = DUALSHOCK3;
  //   setup_packets[dev_addr] = 0;
  // } else {
  //   debug(("FOUND USB HID\n"));
  //   hid_type[dev_addr] = NORMAL;
  }

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

  if (magic_count) {
    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);
    
    if (vid == 0x054C && pid == 0x0268) {
      // dualshock 3
      debug(("FOUND DUALSHOCK3\n"));
      sony_ds3_magic_package(dev_addr, instance);
      magic_count --;
    }
  }
  


  if ( !tuh_hid_receive_report(dev_addr, instance)) {
    debug(("Error: cannot request to receive report\n"));
  }
}

#endif
