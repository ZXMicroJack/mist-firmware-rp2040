//// includes ////
#include <string.h>
#include "ini_parser.h"
#include "mbconfig.h"

mb_cfg_t mb_cfg = { 
  .fpga_type = 0xff // a200t
};

const ini_section_t mb_ini_sections[] = {
  {1, "MB"},
};

// mist ini vars
const ini_var_t mb_ini_vars[] = {
  {"ZXTRES_TYPE", (void*)(&(mb_cfg.fpga_type)), UINT8, 0, 1, 1},
};

// mist ini config
const ini_cfg_t mb_ini_cfg = {
  "/MB.INI",
  mb_ini_sections,
  mb_ini_vars,
  (int)(sizeof(mb_ini_sections) / sizeof(ini_section_t)),
  (int)(sizeof(mb_ini_vars)     / sizeof(ini_var_t))
};

void mb_ini_parse()
{
  mb_cfg.fpga_type = 0xff;
  ini_parse(&mb_ini_cfg, 0, 0);
}


