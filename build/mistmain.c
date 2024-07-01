 /*
Copyright 2005, 2006, 2007 Dennis van Weeren
Copyright 2008, 2009 Jakub Bednarski
Copyright 2012 Till Harbaum

This file is part of Minimig

Minimig is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Minimig is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// 2008-10-04   - porting to ARM
// 2008-10-06   - support for 4 floppy drives
// 2008-10-30   - hdd write support
// 2009-05-01   - subdirectory support
// 2009-06-26   - SDHC and FAT32 support
// 2009-08-10   - hardfile selection
// 2009-09-11   - minor changes to hardware initialization routine
// 2009-10-10   - any length fpga core file support
// 2009-11-14   - adapted floppy gap size
//              - changes to OSD labels
// 2009-12-24   - updated version number
// 2010-01-09   - changes to floppy handling
// 2010-07-28   - improved menu button handling
//              - improved FPGA configuration routines
//              - added support for OSD vsync
// 2010-08-15   - support for joystick emulation
// 2010-08-18   - clean-up

#include "stdio.h"
#include "string.h"
#include "errors.h"
#include "hardware.h"
#include "mmc.h"
#include "fat_compat.h"
#include "osd.h"
#include "fpga.h"
#include "fdd.h"
#include "hdd.h"
#include "config.h"
#include "menu.h"
#include "user_io.h"
#include "arc_file.h"
#include "font.h"
#include "tos.h"
#include "usb.h"
#include "debug.h"
#include "mist_cfg.h"
#include "usbdev.h"
#include "cdc_control.h"
#include "storage_control.h"
#include "FatFs/diskio.h"
#include "mistmain.h"
#include "settings.h"

#include "drivers/ipc.h"
#include "drivers/midi.h"
// #define DEBUG
#include "drivers/debug.h"

#ifndef _WANT_IO_LONG_LONG
#error "newlib lacks support of long long type in IO functions. Please use a toolchain that was compiled with option --enable-newlib-io-long-long."
#endif

const char version[] = {"$VER:ATH" VDATE};

unsigned char Error;
char s[FF_LFN_BUF + 1];

unsigned long storage_size = 0;

#if 0
void FatalError(unsigned long error) {
  unsigned long i;
  
  iprintf("Fatal error: %lu\r", error);
  
  while (1) {
    for (i = 0; i < error; i++) {
      DISKLED_ON;
      WaitTimer(250);
      DISKLED_OFF;
      WaitTimer(250);
    }
    WaitTimer(1000);
    MCUReset();
  }
}
#endif

void HandleFpga(void) {
  unsigned char  c1, c2;
  
  EnableFpga();
  c1 = SPI(0); // cmd request and drive number
  c2 = SPI(0); // track number
  SPI(0);
  SPI(0);
  SPI(0);
  SPI(0);
  DisableFpga();
  
  HandleFDD(c1, c2);
  HandleHDD(c1, c2, 1);
  
  UpdateDriveStatus();
}

extern void inserttestfloppy();

uint8_t legacy_mode = DEFAULT_MODE;

void set_legacy_mode(uint8_t mode) {
  if (mode != legacy_mode) {
    printf("Setting legacy mode to %d\n", mode);
#ifdef ZXUNO
#else
#ifdef MB2
    ipc_Command(IPC_SETMISTER, &mode, sizeof mode);
#else
    jamma_Kill();
    jamma_InitEx(mode == MIST_MODE);
#endif
#endif
  }
  legacy_mode = mode;
}

#ifdef USB_STORAGE
int GetUSBStorageDevices()
{
  uint32_t to = GetTimer(4000);

  // poll usb 2 seconds or until a mass storage device becomes ready
  while(!storage_devices && !CheckTimer(to)) {
    usb_poll();
    if (storage_devices) {
      usb_poll();
    }
  }

  return storage_devices;
}
#endif

// Test use of USB disk instead of MMC - when MMC is not inserted.
// #define MMC_AS_USB

#ifdef USBFAKE

#ifdef MMC_AS_USB
uint8_t storage_devices = 1;

unsigned char usb_host_storage_read(unsigned long lba, unsigned char *pReadBuffer, uint16_t len) {
  printf("usb_host_storage_read: lba %ld len %d\n", lba, len);
  return MMC_ReadMultiple(lba, pReadBuffer, len);
}

unsigned char usb_host_storage_write(unsigned long lba, const unsigned char *pWriteBuffer, uint16_t len) {
  printf("usb_host_storage_write: lba %ld len %d\n", lba, len);
  return MMC_WriteMultiple(lba, pWriteBuffer, len);
}

unsigned int usb_host_storage_capacity() {
  printf("usb_host_storage_capacity\n");
  return MMC_GetCapacity();
}
#else
uint8_t storage_devices = 0;
unsigned char usb_host_storage_read(unsigned long lba, unsigned char *pReadBuffer, uint16_t len) {
  return 0;
}

unsigned char usb_host_storage_write(unsigned long lba, const unsigned char *pWriteBuffer, uint16_t len) {
  return 0;
}

unsigned int usb_host_storage_capacity() {
  return 0;
}
#endif
#endif

int mist_init() {
    uint8_t mmc_ok = 0;

    DISKLED_ON;

    // Timer_Init();
    USART_Init(115200);

    iprintf("\rMinimig by Dennis van Weeren");
    iprintf("\rARM Controller by Jakub Bednarski\r\r");
    iprintf("Version %s\r\r", version+5);

    mist_spi_init();

#if defined(ZXUNO) && PICO_NO_FLASH
    {
      extern void ConfigureFPGAStdin();
      ConfigureFPGAStdin();
    }
#endif

#ifdef ZXUNO
  {
    void ConfigureFPGAFlash();
    ConfigureFPGAFlash();
  }
#else
#ifdef XILINX
    fpga_initialise();
    fpga_claim(true);
    fpga_reset(0);
#endif
#endif

    if(MMC_Init()) mmc_ok = 1;
    else           spi_fast();

#ifdef MMC_AS_USB
    mmc_ok = 0;
#endif

    iprintf("spiclk: %u MHz\r", GetSPICLK());

    usb_init();
    InitDB9();

    InitADC();

#ifdef USB_STORAGE
    if(UserButton()) USB_BOOT_VAR = (USB_BOOT_VAR == USB_BOOT_VALUE) ? 0 : USB_BOOT_VALUE;

    if(USB_BOOT_VAR == USB_BOOT_VALUE)
      if (!GetUSBStorageDevices()) {
        if(!mmc_ok)
          FatalError(ERROR_FILE_NOT_FOUND);
      } else
        fat_switch_to_usb();  // redirect file io to usb
    else {
#endif
      if(!mmc_ok) {
#ifdef USB_STORAGE
        if(!GetUSBStorageDevices()) {
#ifdef BOOT_FLASH_ON_ERROR
          BootFromFlash();
          return 0;
#else
          FatalError(ERROR_FILE_NOT_FOUND);
#endif
        }

        fat_switch_to_usb();  // redirect file io to usb
#else
        // no file to boot
#ifdef BOOT_FLASH_ON_ERROR
        BootFromFlash();
        return 0;
#else
        FatalError(ERROR_FILE_NOT_FOUND);
#endif
#endif
      }
#ifdef USB_STORAGE
    }
#endif

    if (!FindDrive()) {
#ifdef BOOT_FLASH_ON_ERROR
        BootFromFlash();
        return 0;
#else
        FatalError(ERROR_INVALID_DATA);
#endif
    }

    disk_ioctl(fs.pdrv, GET_SECTOR_COUNT, &storage_size);
    storage_size >>= 11;

    ChangeDirectoryName(MIST_ROOT);

    arc_reset();

    font_load();

    user_io_init();

    // tos config also contains cdc redirect settings used by minimig
    tos_config_load(-1);

    char mod = -1;

    if((USB_LOAD_VAR != USB_LOAD_VALUE) && !user_io_dip_switch1()) {
        mod = arc_open(MIST_ROOT "/CORE.ARC");
    } else {
        user_io_detect_core_type();
        if(user_io_core_type() != CORE_TYPE_UNKNOWN && !user_io_create_config_name(s, "ARC", CONFIG_ROOT)) {
            // when loaded from USB, try to load the development ARC file
            iprintf("Load development ARC: %s\n", s);
            mod = arc_open(s);
        }
    }

    if(mod < 0 || !strlen(arc_get_rbfname())) {
        fpga_init(NULL); // error opening default ARC, try with default RBF
    } else {
        user_io_set_core_mod(mod);
        strncpy(s, arc_get_rbfname(), sizeof(s)-5);
#ifdef XILINX
        strcat(s,".BIT");
#else
        strcat(s,".RBF");
#endif
        fpga_init(s);
    }

    usb_dev_open();
#ifdef ZXUNO
  //TODO joystick
#else
#ifndef MB2
    jamma_InitEx(1);
#endif
#endif

#if defined(XILINX) && !defined(USBFAKE)
    midi_init();
#endif

#ifdef PICOSYNTH
    picosynth_Init();
#endif

#ifdef ZXUNO
    settings_board_load();
    settings_load(1);
    if (!settings_boot_menu()) {
      jtag_ResetFPGA();
      user_io_detect_core_type();
    }
#endif

    set_legacy_mode(user_io_core_type() == CORE_TYPE_UNKNOWN ? LEGACY_MODE : MIST_MODE);


    // set_legacy_mode(user_io_core_type() == CORE_TYPE_UNKNOWN ? LEGACY_MODE : MIST_MODE);
    return 0;
}


/* handle sysex control */
#define MIDI_SYSEX_START 		0xF0
#define MIDI_SYSEX_END	 		0xF7

uint8_t sysex_chksum;
uint8_t sysex_insysex = 0;
#define MAX_SYSEX   (254)
uint8_t sysex_buffer[MAX_SYSEX];

#define CMD_ECHO            0x01
#define CMD_STOPSYNTH       0x02
#define CMD_STARTSYNTH      0x03
#define CMD_BEEP            0x04
#define CMD_BUFFERALLOC     0x05
#define CMD_BUFFERFREE      0x06
#define CMD_WRITEDATA       0x07
#define CMD_CHECKCRC16      0x08
#define CMD_VERIFYPROGRAM   0x09
#define CMD_RESET           0x0A
#define CMD_SETBAUDRATE     0x0B
#define CMD_INITFPGA        0x0C
#define CMD_INITFPGAFN      0x0D
#define CMD_BOOTSTRAP       0x0E
#define CMD_STARTMIST       0x0F

uint8_t old_coretype = 0;

extern uint8_t stop_watchdog;

uint64_t check_core_at = 0;

#ifndef USBFAKE
uint64_t lastBeep = 0;
void beep(int n) {
  uint64_t now = time_us_64();

  if ((now - lastBeep) > 1000000) {
    printf("beep: %02X\n", n);
    lastBeep = now;
  }
}
#else
#define beep(n) while(0) ;
#endif


#if defined(XILINX) && !defined(USBFAKE)
void sysex_Process() {
  switch(sysex_buffer[0]) {
    case CMD_RESET: {
#ifdef MB2
      uint8_t data = 0x55;
      ipc_Command(IPC_REBOOT, &data, sizeof data);
      stop_watchdog = 1;
#endif
      watchdog_enable(1, 1);
      for(;;);
      break;
    }

#if 0
    case CMD_INITFPGA: {
      uint32_t lba = (sysex_buffer[1] << 28) 
        | (sysex_buffer[2] << 21)
        | (sysex_buffer[3] << 14) 
        | (sysex_buffer[4] << 7) | sysex_buffer[5];
      main_Active(1);
      spi = sd_hw_init();
      fpga_load_bitfile(spi, lba, NULL);
      sd_hw_kill(spi);
      main_Active(0);
      break;
    }
#endif

#ifdef ZXUNO
    case CMD_STARTMIST: {
      break;
    }
#endif

    case CMD_INITFPGAFN: {
      char fn[256];
      extern unsigned char ConfigureFpgaEx(const char *bitfile, uint8_t fatal, uint8_t reset);
      int i = 0;
      memset(fn, 0, sizeof fn);
      for (i = 0; i < (sysex_insysex - 4); i++) {
        fn[i] = sysex_buffer[i+1];
      }
      
      printf("fn = %s\n", fn);
      // skip initial separator, reset before load
      // ConfigureFpgaEx(fn, false, true);
      ResetFPGA();

#ifdef ZXUNO
      /* ZXUNO version has no direct access to SD card - need to reset to menu first  */
      if (legacy_mode == LEGACY_MODE) {
        void ConfigureFPGAFlash();
        ConfigureFPGAFlash();
      }
#endif

      fpga_init(fn);
      // check_core_at = time_us_64() + 100000;

      // main_Active(1);
      // spi = sd_hw_init();
      // fpga_load_bitfile(spi, 0, fn);
      // sd_hw_kill(spi);
      // main_Active(0);
      break;
    }
    
#ifdef MB2
    case CMD_BOOTSTRAP: {
      cookie_Set();
      stop_watchdog = 1;
      watchdog_enable(1, 1);
      for(;;);
    }
#endif

    default:
      printf("Unknown sysex cmd: %02X\n", sysex_buffer[0]);
  }
}

void wtsynth_Sysex(uint8_t data) {
//   printf("sysex: %02X\n", data);
  if (sysex_insysex && data == MIDI_SYSEX_END) {
    debug(("sysex: completed len = %d\n", sysex_insysex - 2));
    if (sysex_chksum == 0x00) sysex_Process();
#ifdef SDEBUG
    else printf("Sysex message failed checksum\n");
#endif
    sysex_insysex = 0;
  } else if (!sysex_insysex && data == MIDI_SYSEX_START) {
    sysex_insysex = 1;
    sysex_chksum = 0x47;
  } else if (sysex_insysex == 1) {
    // fairly sure a fairlight will never be connected to a zxuno
    if (data == 0x14) {
      sysex_insysex ++;
    } else {
      debug(("Sysex not for us!\n"));
      sysex_insysex = 0xff;
    }
  } else if (sysex_insysex > 0 && sysex_insysex < (MAX_SYSEX+2)) {
    sysex_buffer[sysex_insysex-2] = data;
    sysex_chksum ^= data;
    sysex_insysex ++;
  }
}



void midi_loop() {
  unsigned char uartbuff[16];
  int thisread;

  // read and process midi data
  int readable = midi_isdata();
  while (readable) {
    thisread = readable > sizeof uartbuff ? sizeof uartbuff : readable;
    readable -= thisread;
    
    thisread = midi_get(uartbuff, thisread);

#if 1 // disabled for debug
    printf("MidiIn: ");
    for (int i=0; i<thisread; i++) {
      printf("%02X %c", uartbuff[i], (uartbuff[i] >= ' ' && uartbuff[i] < 128) ? uartbuff[i] : '?');
    }
    printf("\n");
#endif
#ifndef PICOSYNTH
    for (int i=0; i<thisread; i++) {
      wtsynth_Sysex(uartbuff[i]);
    }
#else
    wtsynth_HandleMidiBlock(uartbuff, thisread);
#endif
  }
}
#endif

int mist_loop() {
  ps2_Poll();
#if defined(XILINX) && !defined(USBFAKE)
  midi_loop();
#endif

#ifndef ZXUNO
#ifndef USBFAKE
  if (fpga_ResetButtonState()) {
#ifdef MB2
    stop_watchdog = 1;
#endif
    watchdog_enable(1, 1);
  }
#endif
#endif

    cdc_control_poll();
    storage_control_poll();

    if (check_core_at && time_us_64() > check_core_at) {
      user_io_detect_core_type();
      user_io_detect_core_type();
      user_io_detect_core_type();
      check_core_at = 0;
    }

    if (legacy_mode == LEGACY_MODE) {
      if (user_io_core_type() != CORE_TYPE_UNKNOWN) {
        set_legacy_mode(MIST_MODE);
      }
  // beep(1);
    } else {
  // beep(2);
      user_io_poll();

#if 0
      if (old_coretype != user_io_core_type()) {
        old_coretype = user_io_core_type();
        printf("user_io_core_type() %02X\n", old_coretype);
      }
#endif

      // MJ: check for legacy core and switch support on
#if 1
      if (user_io_core_type() == CORE_TYPE_UNKNOWN) {
        set_legacy_mode(LEGACY_MODE);
      }
#endif
      // printf("[%d]\n", __LINE__);


      // MIST (atari) core supports the same UI as Minimig
      if((user_io_core_type() == CORE_TYPE_MIST) || (user_io_core_type() == CORE_TYPE_MIST2)) {
        if(!fat_medium_present())
          tos_eject_all();

        HandleUI();
      }
      // printf("[%d]\n", __LINE__);

      // call original minimig handlers if minimig core is found
      if((user_io_core_type() == CORE_TYPE_MINIMIG) || (user_io_core_type() == CORE_TYPE_MINIMIG2)) {
        if(!fat_medium_present())
          EjectAllFloppies();

        HandleFpga();
        HandleUI();
      }
      // printf("[%d]\n", __LINE__);

      // 8 bit cores can also have a ui if a valid config string can be read from it
      if((user_io_core_type() == CORE_TYPE_8BIT) && user_io_is_8bit_with_config_string()) HandleUI();
      // printf("[%d]\n", __LINE__);

      // Archie core will get its own treatment one day ...
      if(user_io_core_type() == CORE_TYPE_ARCHIE) HandleUI();
      // printf("[%d]\n", __LINE__);
    }
    return 0;
}
