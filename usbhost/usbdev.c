#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "spi.h"

#include "mmc.h"

#include "drivers/pio_spi.h"
#include "drivers/sdcard.h"

usb_data_t *usb_get_handle(uint16_t vid, uint16_t pid) {
  usb_data_t *this = malloc(sizeof(usb_data_t));
  memset(this, 0, sizeof(usb_data_t));

  this->vid = vid;
  this->pid = pid;
}

void usb_attached(usb_data_t *handle, uint8_t idx, uint8_t *desc, uint16_t desclen) {
}

void usb_detached(usb_data_t *handle) {
}

void usb_handle_data(usb_data_t *handle, uint8_t *desc, uint16_t desclen) {
}
