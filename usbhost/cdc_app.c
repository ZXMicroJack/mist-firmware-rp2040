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

#include "tusb.h"
#include "bsp/board.h"
#include "fifo.h"

#if CFG_TUH_CDC

static uint8_t cdc_started = 0;
static fifo_t cdc_in;
static fifo_t cdc_out;
static uint8_t cdc_in_fifo[256];
static uint8_t cdc_out_fifo[256];

static void cdc_init() {
  if (!cdc_started) {
    cdc_started = 1;
    fifo_Init(&cdc_in, cdc_in_fifo, sizeof cdc_in_fifo);
    fifo_Init(&cdc_out, cdc_out_fifo, sizeof cdc_out_fifo);
  }
}

uint8_t  usb_cdc_is_configured(void) {
  return tuh_cdc_mounted(0);
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

#if 0
//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+


//------------- IMPLEMENTATION -------------//
int xboard_getchar();

size_t get_console_inputs(uint8_t* buf, size_t bufsize)
{
  size_t count = 0;
  while (count < bufsize)
  {
    int ch = xboard_getchar();
    if ( ch <= 0 ) break;

    buf[count] = (uint8_t) ch;
    count++;
  }

  return count;
}
#endif

void cdc_task(void)
{
  uint8_t buf[64]; // +1 for extra null character
  int count;

  cdc_init();

  // pass data to CDC
  while (count < sizeof buf) {
    int c = fifo_Get(&cdc_out);
    if (c < 0) {
      break;
    } else {
      buf[count++] = c;
    }
  }

  // loop over all mounted interfaces
  for(uint8_t idx=0; idx<CFG_TUH_CDC; idx++)
  {
    if ( tuh_cdc_mounted(idx) )
    {
      // console --> cdc interfaces
      if (count)
      {
        tuh_cdc_write(idx, buf, count);
        tuh_cdc_write_flush(idx);
      }
    }
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
  tuh_cdc_itf_info_t itf_info = { 0 };
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is mounted: address = %u, itf_num = %u\r\n", itf_info.daddr, itf_info.bInterfaceNumber);

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
}

void tuh_cdc_umount_cb(uint8_t idx)
{
  tuh_cdc_itf_info_t itf_info = { 0 };
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is unmounted: address = %u, itf_num = %u\r\n", itf_info.daddr, itf_info.bInterfaceNumber);
}

#else
uint8_t  usb_cdc_is_configured(void) { return 0; }
uint16_t usb_cdc_write(const char *pData, uint16_t length) { return 0; }
uint16_t usb_cdc_read(char *pData, uint16_t length) { return 0; }
#endif
