#include <stdio.h>
#include <stdbool.h>

#include "host/usbh.h"
#include "xinput_host.h"
#include "host/usbh_classdriver.h"
#include "usbhost.h"

//#define DEBUG
#include "../drivers/debug.h"

#define XBOX_VID                                0x045E // Microsoft Corporation
#define XBOX_WIRED_PID                          0x028E // Microsoft 360 Wired controller
//#undef MIST_USB

void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, xinputh_interface_t const* xid_itf, uint16_t len)
{
    const xinput_gamepad_t *p = &xid_itf->pad;
    const char* type_str;

    debug(("tuh_xinput_report_received_cb\n"));

    if (xid_itf->last_xfer_result == XFER_RESULT_SUCCESS)
    {
#ifdef MIST_USB
        if (xid_itf->connected && xid_itf->new_pad_data) {
          usb_handle_data(dev_addr, instance, (uint8_t *)xid_itf, sizeof (xinputh_interface_t));
          debug(("[%02x, %02x], Buttons %04x, LT: %02x RT: %02x, LX: %d, LY: %d, RX: %d, RY: %d\n",
              dev_addr, instance, p->wButtons, p->bLeftTrigger, p->bRightTrigger, p->sThumbLX, p->sThumbLY, p->sThumbRX, p->sThumbRY));
        }
#else
        switch (xid_itf->type)
        {
            case 1: type_str = "Xbox One";          break;
            case 2: type_str = "Xbox 360 Wireless"; break;
            case 3: type_str = "Xbox 360 Wired";    break;
            case 4: type_str = "Xbox OG";           break;
            default: type_str = "Unknown";
        }
        if (xid_itf->connected && xid_itf->new_pad_data)
        {
            debug(("[%02x, %02x], Type: %s, Buttons %04x, LT: %02x RT: %02x, LX: %d, LY: %d, RX: %d, RY: %d\n",
                dev_addr, instance, type_str, p->wButtons, p->bLeftTrigger, p->bRightTrigger, p->sThumbLX, p->sThumbLY, p->sThumbRX, p->sThumbRY));

            //How to check specific buttons
            if (p->wButtons & XINPUT_GAMEPAD_A) TU_LOG1("You are pressing A\n");
        }
#endif
    }
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, const xinputh_interface_t *xinput_itf)
{
  uint16_t vid, pid;

  tuh_vid_pid_get(dev_addr, &vid, &pid);

  if (xinput_itf->type == XBOX360_WIRELESS && xinput_itf->connected == false)
  {
      tuh_xinput_receive_report(dev_addr, instance);
      return;
  }
  tuh_xinput_set_led(dev_addr, instance, 0, true);
  tuh_xinput_set_led(dev_addr, instance, 1, true);
  tuh_xinput_set_rumble(dev_addr, instance, 0, 0, true);
  tuh_xinput_receive_report(dev_addr, instance);

#ifdef MIST_USB
  usb_attached(dev_addr, instance, vid, pid, (uint8_t *)xinput_itf, sizeof (xinputh_interface_t), USB_TYPE_XBOX);
#endif

}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance)
{
#ifdef MIST_USB
  usb_detached(dev_addr);
#else
  debug(("XINPUT UNMOUNTED %02x %d\n", dev_addr, instance));
#endif
}
