#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "hardware.h"

#include "menu.h"
#include "osd.h"
#include "misc_cfg.h"
#include "tos.h"
#include "fat_compat.h"
#include "fpga.h"
#include "cdc_control.h"
#include "debug.h"
#include "user_io.h"
#include "data_io.h"
#include "ikbd.h"
#include "idxfile.h"
#include "font.h"
#include "mmc.h"
#include "utils.h"
#include "FatFs/diskio.h"

extern bool eth_present;
extern char s[FF_LFN_BUF + 1];

unsigned long hdd_direct = 0;
// 0-1 floppy, 2-3 hdd
char disk_inserted[4];

void tos_poll() {}

char tos_get_cdc_control_redirect(void) {
  return 0;
}

void tos_set_cdc_control_redirect(char mode) {
}

void tos_set_video_adjust(char axis, char value) {
}

char tos_get_video_adjust(char axis) {
  return 0;
}

// enable direct sd card access on acsi0
void tos_set_direct_hdd(char on) {
}

char tos_get_direct_hdd() {
  return 1;
}

void tos_load_cartridge(const char *name) {
}

char tos_cartridge_is_inserted() {
  return 0;
}


void tos_upload(const char *name) {
}


void tos_update_sysctrl(unsigned long n) {
}

char *tos_get_disk_name(char index) {
  return "";
}

char *tos_get_image_name() {
  return "";
}

char *tos_get_cartridge_name() {
  return "";
}

char tos_disk_is_inserted(char index) {
  return 1;
}

void tos_select_hdd_image(char i, const unsigned char *name) {
}

void tos_insert_disk(char i, const unsigned char *name) {
}

// force ejection of all disks (SD card has been removed)
void tos_eject_all() {
}

void tos_reset(char cold) {
}

unsigned long tos_system_ctrl(void) {
  return 0;
}

// load/init configuration
void tos_config_load(char slot) {
}

// save configuration
void tos_config_save(char slot) {
}

// configuration file check
char tos_config_exists(char slot) {
  return 0;
}

void tos_setup_menu() {
}
