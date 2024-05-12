#ifndef _MBCONFIG_H
#define _MBCONFIG_H

typedef struct {
  uint8_t fpga_type;
} mb_cfg_t;

void mb_ini_parse();

extern mb_cfg_t mb_cfg;

#endif

