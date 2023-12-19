typedef struct {
  uint16_t vid;
  uint16_t pid;
} usb_data_t;

usb_data_t *usb_get_handle(uint16_t vid, uint16_t pid);
void usb_attached(usb_data_t *handle, uint8_t idx, uint8_t *desc, uint16_t desclen);
void usb_detached(usb_data_t *handle);
void usb_handle_data(usb_data_t *handle, uint8_t *desc, uint16_t desclen);

