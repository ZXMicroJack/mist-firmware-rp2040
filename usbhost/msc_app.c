/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 */

#include "tusb.h"

#if CFG_TUH_MSC

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+
static scsi_inquiry_resp_t inquiry_resp;

/*
TinyUSB Host CDC MSC HID Example
A MassStorage device is mounted
F8 T5 rev 1.10
Disk Size: 31 MB
Block Count = 63488, Block Size: 512
*/

// bool inquiry_complete_cb(uint8_t dev_addr, msc_cbw_t const* cbw, msc_csw_t const* csw)
bool inquiry_complete_cb(uint8_t dev_addr, tuh_msc_complete_data_t const* cb_data)
{
  if (cb_data->csw->status != 0)
  {
    printf("Inquiry failed\r\n");
    return false;
  }

  // Print out Vendor ID, Product ID and Rev
  printf("%.8s %.16s rev %.4s\r\n", inquiry_resp.vendor_id, inquiry_resp.product_id, inquiry_resp.product_rev);

  // Get capacity of device
  uint32_t const block_count = tuh_msc_get_block_count(dev_addr, cb_data->cbw->lun);
  uint32_t const block_size = tuh_msc_get_block_size(dev_addr, cb_data->cbw->lun);

  printf("Disk Size: %lu MB\r\n", block_count / ((1024*1024)/block_size));
  printf("Block Count = %lu, Block Size: %lu\r\n", block_count, block_size);

  return true;
}

// typedef struct {
//   msc_cbw_t const* cbw; // SCSI command
//   msc_csw_t const* csw; // SCSI status
//   void* scsi_data;      // SCSI Data
//   uintptr_t user_arg;   // user argument
// }tuh_msc_complete_data_t;
// 
// typedef bool (*tuh_msc_complete_cb_t)(uint8_t dev_addr, tuh_msc_complete_data_t const* cb_data);

// bool msc_fat_complete_cb(uint8_t dev_addr, msc_cbw_t const* cbw, msc_csw_t const* csw, uintptr_t intptr)
bool msc_fat_complete_cb(uint8_t dev_addr, tuh_msc_complete_data_t const *d)
{
    (void)dev_addr;
    // success = csw->status == MSC_CSW_STATUS_PASSED;
    printf("msc_fat_complete_cb: status = %d\n", d->csw->status == MSC_CSW_STATUS_PASSED);
    printf("msc_fat_complete_cb: csw->status = %d\n", d->csw->status);
    
    return d-> csw->status == MSC_CSW_STATUS_PASSED;
}

int write_sector(int pdrv, uint8_t *buff, uint32_t sector) {
  if (!tuh_msc_write10(pdrv+1, 0, buff, sector, /*count*/1, msc_fat_complete_cb, (uintptr_t)NULL)) {
    return 1;
  }
  return 0;
}

int read_sector(int pdrv, uint8_t *buff, uint32_t sector) {
  if (!tuh_msc_read10(pdrv+1, 0, buff, sector, /*count*/1, msc_fat_complete_cb, (uintptr_t)NULL)) {
    return 1;
  }
  return 0;
}

//------------- IMPLEMENTATION -------------//
void tuh_msc_mount_cb(uint8_t dev_addr)
{
  printf("A MassStorage device is mounted\r\n");

  uint8_t const lun = 0;
  tuh_msc_inquiry(dev_addr, lun, &inquiry_resp, inquiry_complete_cb, (uintptr_t)NULL);
//
//  //------------- file system (only 1 LUN support) -------------//
//  uint8_t phy_disk = dev_addr-1;
//  disk_initialize(phy_disk);
//
//  if ( disk_is_ready(phy_disk) )
//  {
//    if ( f_mount(phy_disk, &fatfs[phy_disk]) != FR_OK )
//    {
//      puts("mount failed");
//      return;
//    }
//
//    f_chdrive(phy_disk); // change to newly mounted drive
//    f_chdir("/"); // root as current dir
//
//    cli_init();
//  }
}

void tuh_msc_umount_cb(uint8_t dev_addr)
{
  (void) dev_addr;
  printf("A MassStorage device is unmounted\r\n");

//  uint8_t phy_disk = dev_addr-1;
//
//  f_mount(phy_disk, NULL); // unmount disk
//  disk_deinitialize(phy_disk);
//
//  if ( phy_disk == f_get_current_drive() )
//  { // active drive is unplugged --> change to other drive
//    for(uint8_t i=0; i<CFG_TUH_DEVICE_MAX; i++)
//    {
//      if ( disk_is_ready(i) )
//      {
//        f_chdrive(i);
//        cli_init(); // refractor, rename
//      }
//    }
//  }
}

#endif
