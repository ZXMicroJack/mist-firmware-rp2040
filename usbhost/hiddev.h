#ifndef _HIDDEV_H
#define _HIDDEV_H

void hid_app_task();
void HID_init();
fifo_t *HID_getPS2Fifo(uint8_t chan, uint8_t host);

#endif

