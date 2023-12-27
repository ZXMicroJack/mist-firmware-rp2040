#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
//#include "wtsynth.h"
//#include "version.h"

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"

#include "flash.h"
#include "crc16.h"
// #define DEBUG
#include "debug.h"

static void set_xip(int on) {
  rom_flash_exit_xip_fn flash_exit_xip = (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
  rom_flash_flush_cache_fn flash_flush_cache = (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);
  rom_flash_enter_cmd_xip_fn flash_enter_cmd_xip = (rom_flash_enter_cmd_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_ENTER_CMD_XIP);

  if (!on) {
    flash_exit_xip();            // Init SSI, prepare flash for command mode
  } else {
    flash_flush_cache();         // Flush & enable XIP cache
    flash_enter_cmd_xip();       // Configure SSI with read cmd
  }
}

int flash_EraseProgram(uint32_t addr, uint8_t *codeBuff, uint32_t rlen) {
  // if already programmed dont bother
  if (crc16((uint8_t *)addr, rlen) == crc16(codeBuff, rlen)) {
    return 1;
  }
  
  if (addr >= XIP_BASE && addr < (XIP_BASE + 0x01000000)) {
    uint32_t elen = (rlen + FLASH_SECTOR_SIZE - 1) & (~(FLASH_SECTOR_SIZE - 1));
    uint32_t plen = (rlen + FLASH_PAGE_SIZE - 1) & (~(FLASH_PAGE_SIZE - 1));
    
    debug(("addr:%08X rlen:%08X elen:%08X plen:%08X\n", addr, rlen, elen, plen));

    debug(("before erase program crc %04X\n", crc16((uint8_t *)addr, rlen)));
    set_xip(0);
    debug(("Erasing ...\n"));
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(addr & 0xffffff, elen);
    restore_interrupts(ints);
    debug(("Programming...\n"));
    ints = save_and_disable_interrupts();
    flash_range_program(addr & 0xffffff, codeBuff, plen);
    restore_interrupts(ints);

    debug(("Checking...\n"));
    set_xip(1);
    
    uint16_t crcFlash;
    
    uint16_t crc = crc16(codeBuff, rlen);

    // not sure why this is needed - but first time crc is calculated, it's wrong.
    int retries = 10;
    uint32_t nocache_addr = addr | 0x03000000;
    do {
      crcFlash = crc16((uint8_t *)nocache_addr, rlen);
      debug(("crcFlash %04X crc %04X\n", crcFlash, crc));
      retries --;
      if (!retries) break;
    } while (crcFlash != crc);
    
    
    return crcFlash == crc;
  }
  return 0;
}

void system_Reset() {
  watchdog_enable(1, 1);
}

