#ifndef _USBHOST_H
#define _USBHOST_H

#define USB_TYPE_HID      0
#define USB_TYPE_XBOX     1
#define USB_TYPE_DS3      2
#define USB_TYPE_DS4      3
#define USB_TYPE_KEYBOARD 4
#define USB_TYPE_MOUSE    5

void usb_attached(uint8_t dev, uint8_t idx, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen, uint8_t type);
void usb_detached(uint8_t dev);
void usb_handle_data(uint8_t dev, uint8_t instance, uint8_t *desc, uint16_t desclen);

#endif