#include <stdint.h>
#include "screen.h"

void usb_poll() {
  pollKeyboard();
  // fakeusb_poll();
}


