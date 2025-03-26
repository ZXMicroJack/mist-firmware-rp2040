/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022, Ha Thach (tinyusb.org)
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
 * This file is part of the TinyUSB stack.
 */

#include <stdarg.h>

#include "tusb.h"
#include "drivers/fifo.h"

#if CFG_TUH_CDC

#ifdef USBDEV
uint16_t usb_cdc_write(const char *pData, uint16_t length);
static char str[256];
void qprintf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);

  // console --> cdc interfaces
  usb_cdc_write(str, strlen(str));
}

#undef debug
#define debug(a) { qprintf a; qprintf("\r"); }
#endif

static uint8_t cdc_started = 0;
static fifo_t cdc_in;
static fifo_t cdc_out;
static uint8_t cdc_in_fifo[1024];
static uint8_t cdc_out_fifo[1024];

static void cdc_init() {
  if (!cdc_started) {
    cdc_started = 1;
    fifo_Init(&cdc_in, cdc_in_fifo, sizeof cdc_in_fifo);
    fifo_Init(&cdc_out, cdc_out_fifo, sizeof cdc_out_fifo);
  }
}

uint8_t usb_cdc_is_configured(void) {
  return tuh_cdc_mounted(0);
}

void usb_cdc_putc(uint8_t ch) {
  fifo_Put(&cdc_out, ch);
}

int usb_cdc_getc(void) {
  return fifo_Get(&cdc_in);
}

uint16_t usb_cdc_write(const char *pData, uint16_t length) { 
  while (length --) {
    fifo_Put(&cdc_out, *pData++);
  }
  return 0;
}

uint16_t usb_cdc_read(char *pData, uint16_t length) {
  int i;

  while (i < length) {
    int c = fifo_Get(&cdc_in);
    if (c < 0) {
      break;
    } else {
      *pData++ = c;
    }
  }
  return i;
}

void cdc_task(void)
{
  uint8_t buf[64]; // +1 for extra null character
  int count = 0;

  cdc_init();

  // pass data to CDC - just one
  if (tuh_cdc_mounted(0)) {
    uint32_t available = tuh_cdc_write_available(0);
    available = available > sizeof buf ? sizeof buf : available;

    while (count < available) {
      int c = fifo_Get(&cdc_out);
      if (c < 0) {
        break;
      } else {
        buf[count++] = c;
      }
    }

    tuh_cdc_write(0, buf, count);
    tuh_cdc_write_flush(0);
  }
}

// Invoked when received new data
void tuh_cdc_rx_cb(uint8_t idx)
{
  uint8_t buf[64]; // +1 for extra null character

  // forward cdc interfaces -> console
  uint32_t count = tuh_cdc_read(idx, buf, sizeof buf);

  for (int i=0; i<count; i++) {
    fifo_Put(&cdc_in, buf[i]);
  }
}

void tuh_cdc_mount_cb(uint8_t idx)
{
  tuh_itf_info_t itf_info = { 0 };
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is mounted: address = %u, itf_num = %u\r\n", itf_info.daddr);

#ifdef CFG_TUH_CDC_LINE_CODING_ON_ENUM
  // CFG_TUH_CDC_LINE_CODING_ON_ENUM must be defined for line coding is set by tinyusb in enumeration
  // otherwise you need to call tuh_cdc_set_line_coding() first
  cdc_line_coding_t line_coding = { 0 };
  if ( tuh_cdc_get_local_line_coding(idx, &line_coding) )
  {
    printf("  Baudrate: %lu, Stop Bits : %u\r\n", line_coding.bit_rate, line_coding.stop_bits);
    printf("  Parity  : %u, Data Width: %u\r\n", line_coding.parity  , line_coding.data_bits);
  }
#endif


  // Set Line Coding upon mounted
  cdc_line_coding_t new_line_coding = { 115200, CDC_LINE_CODING_STOP_BITS_1, CDC_LINE_CODING_PARITY_NONE, 8 };
  tuh_cdc_set_line_coding(idx, &new_line_coding, NULL, 0);

}

void tuh_cdc_umount_cb(uint8_t idx)
{
  tuh_itf_info_t itf_info = { 0 };
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is unmounted: address = %u, itf_num = %u\r\n", itf_info.daddr);
}

#else
uint8_t  usb_cdc_is_configured(void) { return 0; }
uint16_t usb_cdc_write(const char *pData, uint16_t length) { return 0; }
uint16_t usb_cdc_read(char *pData, uint16_t length) { return 0; }
#endif
