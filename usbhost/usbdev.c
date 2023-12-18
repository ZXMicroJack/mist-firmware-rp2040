

void *usb_attached(uint16_t vid, uint16_t pid, uint8_t idx, uint8_t *desc, uint16_t desclen);
void usb_detached(void *handle);
void usb_handle_data(void *handle, uint16_t vid, uint16_t pid, uint8_t *desc, uint16_t desclen);

