#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ini_parser.h"
#include "picosynth.h"
#include "menu.h"
#include "osd.h"
#include "errors.h"

extern unsigned char Error;

// #define ZXUNO

/* for now there is only one entry in BOARD.INI for boot_menu, this is used
   only for ZXUNO.  For now only ZXUNO writes and reads BOARD.INI. */
#ifdef ZXUNO
static uint8_t boot_menu = 1;

// core ini vars
const ini_var_t board_ini_vars[] = {
  {"BOOT_MENU", (void*)(&(boot_menu)), UINT8, 0, 1, 1}
};

const ini_section_t board_ini_sections[] = {
  {1, "BOARD"},
};

// mist ini config
const ini_cfg_t board_ini_cfg = {
  MIST_ROOT"/BOARD.INI",
  board_ini_sections,
  board_ini_vars,
  (int)(sizeof(board_ini_sections) / sizeof(ini_section_t)),
  (int)(sizeof(board_ini_vars)     / sizeof(ini_var_t))
};


void settings_board_load() {
  boot_menu = 1;
  ini_parse(&board_ini_cfg, 0, 0);
}

void settings_board_save() {
  ini_save(&board_ini_cfg, 0);
}

uint8_t settings_boot_menu() {
  return boot_menu;
}

void settings_set_boot(uint8_t state) {
  boot_menu = state;
}
                                         
static char StartupSequenceSet(uint8_t idx) {
  if ((settings_boot_menu() && idx != 0) || (!settings_boot_menu() && idx == 0)) {
    settings_set_boot(idx == 0);
    settings_board_save();
  }
	return 0;
}
#endif

static char GetMenuPage_Platform(uint8_t idx, char action, menu_page_t *page) {
  if (action == MENU_PAGE_EXIT) return 0;

  page->timer = 0;
  page->stdexit = MENU_STD_EXIT;
  page->flags = 0;

  switch(idx) {
    case 0:
      page->title = "Platform";
      page->flags = OSD_ARROW_LEFT;
      break;
  }

  return 0;
}

static char FirmwareUpdateError() {
	switch (Error) {
		case ERROR_FILE_NOT_FOUND :
			DialogBox("\n       Update file\n        not found!\n", MENU_DIALOG_OK, 0);
			break;
		case ERROR_INVALID_DATA :
			DialogBox("\n       Invalid\n     update file!\n", MENU_DIALOG_OK, 0);
			break;
		case ERROR_UPDATE_FAILED :
			DialogBox("\n\n    Update failed!\n", MENU_DIALOG_OK, 0);
			break;
	}
	return 0;
}

static char FirmwareUpdatingDialog(uint8_t idx) {
	WriteFirmware("/FIRMWARE.UPG");
#ifndef RP2040
	Error = ERROR_UPDATE_FAILED;
#endif
	FirmwareUpdateError();
	return 0;
}

static char FirmwareUpdateDialog(uint8_t idx) {
	if (idx == 0) { // yes
		DialogBox("\n      Updating firmware\n\n         Please wait\n", 0, FirmwareUpdatingDialog);
	}
	return 0;
}

static char FirmwareUpdatingUSBDialog(uint8_t idx) {
  extern int UpdateFirmwareUSB();
  Error = UpdateFirmwareUSB();
  DialogBox("\n\n    Update failed!\n", MENU_DIALOG_OK, 0);
  FirmwareUpdateError();
	return 0;
}

static char FirmwareUpdateUSBDialog(uint8_t idx) {
	if (idx == 0) { // yes
		DialogBox("\n   Updating USB firmware\n\n         Please wait\n", 0, FirmwareUpdatingUSBDialog);
	}
	return 0;
}

static char KeyEvent_Platform(uint8_t key) {
  return false;
}

const static char *GetMidiStatus(char *status) {
#ifdef PICOSYNTH
  int state = picosynth_GetStatus();
  sprintf(status, " MIDI status: %d (%s)", state,
    (state == 0) ? "ok" :
    (state < 0 && state <= -3) ? "bad-luts" :
    (state > -3) ? "bad-soundfont" :
    "bad-unknown");
  return status;
#else
  return " MIDI status: disabled";
#endif
}

#ifdef ZXUNO
const static char *GetBootSequence(char *status) {
  int state = settings_boot_menu();
  sprintf(status, " Boot sequence: (%s)", state ? "menu" : "zxuno");
  return status;
}
#endif

const static char *GetFirmwareVersion(char *status) {
  extern const char firmwareVersion[];
  siprintf(status, " Update FIRMWARE (%s)", firmwareVersion);
  return status;
}

const static char *GetUSBFWVersion(char *status) {
  extern const char *GetUSBVersion();
  char *v = GetUSBVersion();

  if (v) {
    siprintf(status, " Update USB (%s)", v);
  } else {
    status[0] = '\0';
  }
  return status;
}

static char GetMenuItem_Platform(uint8_t idx, char action, menu_item_t *item) {
  static char status[40];
	item->stipple = 0;
	item->active = 1;
	item->page = 0;
	item->newpage = 0;
	item->newsub = 0;
	item->item = "";

  // printf("GetMenuItem_Platform: idx %d action %d item %p\n", idx, action, item);
	switch (action) {
		case MENU_ACT_GET:
			switch(idx) 
      {
				case 0:
					item->item = GetFirmwareVersion(status);
					item->active = 1;
					break;

				case 1:
					item->item = GetUSBFWVersion(status);
					item->active = item->item[0] ? 1 : 0;
					break;

        case 2:
#ifdef ZXUNO
          item->item = GetBootSequence(status);
          item->active = 1;
#else
          item->item = "";
          item->active = 0;
#endif
          break;

        case 3:
          item->item = GetMidiStatus(status);
          item->active = 0;
          break;

        default:
          return 0;
			}
			break;
		case MENU_ACT_SEL:
			switch(idx) {
				case 0:
          DialogBox("\n     Update the firmware\n        Are you sure?", MENU_DIALOG_YESNO, FirmwareUpdateDialog);
					break;

				case 1:
          DialogBox("\n   Update the USB firmware\n        Are you sure?", MENU_DIALOG_YESNO, FirmwareUpdateUSBDialog);
					break;

#ifdef ZXUNO
				case 2:
          DialogBox("\n     Display MiSTLita menu\n         on startup?", MENU_DIALOG_YESNO, StartupSequenceSet);
					break;
#endif
			}
			break;
		case MENU_ACT_LEFT:
		case MENU_ACT_RIGHT:
		case MENU_ACT_PLUS:
		case MENU_ACT_MINUS:
		default:
			return 0;
	}
	return 1;

}

void SetupPlatformMenu() {
  SetupMenu(GetMenuPage_Platform, GetMenuItem_Platform, KeyEvent_Platform);
}
