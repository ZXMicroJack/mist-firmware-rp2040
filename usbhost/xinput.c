#include <stdio.h>
#include <stdbool.h>

#include "host/usbh.h"
#include "xinput_host.h"
// #include "/pico/pico-sdk/lib/tinyusb/src/host/usbh_classdriver.h"
#include "host/usbh_classdriver.h"

#define DEBUG
#include "debug.h"

#ifdef MIST_USB
void usb_attached(uint8_t dev, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen);
void usb_detached(uint8_t dev);
void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen);
#else
#endif

#define XBOX_VID                                0x045E // Microsoft Corporation
#define XBOX_WIRED_PID                          0x028E // Microsoft 360 Wired controller

#if 0
//Since https://github.com/hathach/tinyusb/pull/2222, we can add in custom vendor drivers easily
usbh_class_driver_t const* usbh_app_driver_get_cb(uint8_t* driver_count){
    *driver_count = 1;
    return &usbh_xinput_driver;
}
#endif

#if 0
void usb_detached(uint8_t dev);
void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen);
#endif



#undef TU_LOG1
#define TU_LOG1 dbgprintf

void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, xinputh_interface_t const* xid_itf, uint16_t len)
{
    const xinput_gamepad_t *p = &xid_itf->pad;
    const char* type_str;

    if (xid_itf->last_xfer_result == XFER_RESULT_SUCCESS)
    {
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
            TU_LOG1("[%02x, %02x], Type: %s, Buttons %04x, LT: %02x RT: %02x, LX: %d, LY: %d, RX: %d, RY: %d\n",
                dev_addr, instance, type_str, p->wButtons, p->bLeftTrigger, p->bRightTrigger, p->sThumbLX, p->sThumbLY, p->sThumbRX, p->sThumbRY);

            //How to check specific buttons
            if (p->wButtons & XINPUT_GAMEPAD_A) TU_LOG1("You are pressing A\n");
        }
    }
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, const xinputh_interface_t *xinput_itf)
{
#if 0
      	usb_attached(dev_addr, instance, XBOX_VID, XBOX_WIRED_PID, xinput_itf, uint16_t desclen);

  usb_attached(dev_addr, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen);
void usb_detached(uint8_t dev);
void usb_handle_data(uint8_t dev, uint8_t *desc, uint16_t desclen);
#endif

    TU_LOG1("XINPUT MOUNTED %02x %d\n", dev_addr, instance);
    // If this is a Xbox 360 Wireless controller we need to wait for a connection packet
    // on the in pipe before setting LEDs etc. So just start getting data until a controller is connected.
    if (xinput_itf->type == XBOX360_WIRELESS && xinput_itf->connected == false)
    {
        tuh_xinput_receive_report(dev_addr, instance);
        return;
    }
    tuh_xinput_set_led(dev_addr, instance, 0, true);
    tuh_xinput_set_led(dev_addr, instance, 1, true);
    tuh_xinput_set_rumble(dev_addr, instance, 0, 0, true);
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    TU_LOG1("XINPUT UNMOUNTED %02x %d\n", dev_addr, instance);
}
