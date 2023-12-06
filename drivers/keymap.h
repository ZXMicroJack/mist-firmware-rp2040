#ifndef _KEYMAP_H
#define _KEYMAP_H

                        //SCANCODES SET 1
#define KS1_RELEASE 0x80 //Valor que se suma al codigo de la tecla para soltarla
#define KS1_ESCAPE  0x01
#define KS1_BACKSP  0x0E
#define KS1_SCRLCK  0x46

#define KS1_LCTRL   0x1D
#define KS1_LALT    0x38

                        //Especiales, requieren E0
#define KS1_RIGHT   0x4D
#define KS1_LEFT    0x4B
#define KS1_DOWN    0x50
#define KS1_UP      0x48

#define KS1_RCTRL   0x1D
#define KS1_RALT    0x38

#define KS1_LWIN    0x5B
#define KS1_RWIN    0x5C
#define KS1_APPS    0x5D

#define KS1_PGUP    0x49
#define KS1_PGDW    0x51
#define KS1_HOME    0x47
#define KS1_END     0x4F
#define KS1_INS     0x52
#define KS1_DELETE  0x53
                        //Fin Especiales

#define KS1_A       0x1E
#define KS1_B       0x30
#define KS1_C       0x2E
#define KS1_D       0x20
#define KS1_E       0x12
#define KS1_F       0x21
#define KS1_G       0x22
#define KS1_H       0x23
#define KS1_I       0x17
#define KS1_J       0x24
#define KS1_K       0x25
#define KS1_L       0x26
#define KS1_M       0x32
#define KS1_N       0x31
#define KS1_O       0x18
#define KS1_P       0x19
#define KS1_Q       0x10
#define KS1_R       0x13
#define KS1_S       0x1F
#define KS1_T       0x14
#define KS1_U       0x16
#define KS1_V       0x2F
#define KS1_W       0x11
#define KS1_X       0x2D
#define KS1_Y       0x15
#define KS1_Z       0x2C
#define KS1_1       0x02
#define KS1_2       0x03
#define KS1_3       0x04
#define KS1_4       0x05
#define KS1_5       0x06
#define KS1_6       0x07
#define KS1_7       0x08
#define KS1_8       0x09
#define KS1_9       0x0A
#define KS1_0       0x0B

#define KS1_ENTER   0x1C
#define KS1_SPACE   0x39

#define KS1_F1      0x3B
#define KS1_F2      0x3C
#define KS1_F3      0x3D
#define KS1_F4      0x3E
#define KS1_F5      0x3F
#define KS1_F6      0x40
#define KS1_F7      0x41
#define KS1_F8      0x42
#define KS1_F9      0x43
#define KS1_F10     0x44
#define KS1_F11     0x57
#define KS1_F12     0x58

#define KS1_LSHIFT  0x2A
#define KS1_RSHIFT  0x36

#define KS1_CAPS    0x3A

#define KS1_TAB     0x0F

#define KS1_TLD     0x29 //Izxda del 1
#define KS1_MENOS   0x0C //Drcha del 0
#define KS1_IGUAL   0x0D //Izda de Backspace
#define KS1_ACORCHE 0x1A //Drcha de la P
#define KS1_CCORCHE 0x1B //Siguiente a la de la Drcha de la P
#define KS1_BKSLASH 0x2B //Izda del Enter (Puede estar en la fila de la P o de la L
#define KS1_PTOCOMA 0x27 //La Ñ
#define KS1_COMILLA 0x28 //Derecha de la Ñ
#define KS1_COMA    0x33 //Decha de la M
#define KS1_PUNTO   0x34 //Siguiente del de la Derecha de la M
#define KS1_SLASH   0x35 //Izda del Shift Derecho
#define KS1_LESS    0x56 //Izda de la Z
#define KS1_NUMLCK 0x45 //Numlock
#define KS1_SYSRQ   0x71

// Taken from https://github.com/antoniovillena/zxunops2USB
#define KEY_ESCAPE  0x76
#define KEY_BACKSP  0x66
#define KEY_SCRLCK  0x7E
#define KEY_NUMLCK  0x77

#define KEY_LCTRL   0x14
#define KEY_LALT    0x11

                        //Especiales, requieren E0
#define KEY_RIGHT   0x74
#define KEY_LEFT    0x6B
#define KEY_DOWN    0x72
#define KEY_UP      0x75
#define KEY_RCTRL   0x14
#define KEY_RALT    0x11
#define KEY_LWIN    0x1F
#define KEY_RWIN    0x27
#define KEY_APPS    0x2F
#define KEY_PGUP    0x7D
#define KEY_PGDW    0x7A
#define KEY_HOME    0x6C
#define KEY_END     0x69
#define KEY_INS     0x70
#define KEY_DELETE  0x71
                        //Fin Especiales

#define KEY_A       0x1C
#define KEY_B       0x32
#define KEY_C       0x21
#define KEY_D       0x23
#define KEY_E       0x24
#define KEY_F       0x2B
#define KEY_G       0x34
#define KEY_H       0x33
#define KEY_I       0x43
#define KEY_J       0x3B
#define KEY_K       0x42
#define KEY_L       0x4B
#define KEY_M       0x3A
#define KEY_N       0x31
#define KEY_O       0x44
#define KEY_P       0x4D
#define KEY_Q       0x15
#define KEY_R       0x2D
#define KEY_S       0x1B
#define KEY_T       0x2C
#define KEY_U       0x3C
#define KEY_V       0x2A
#define KEY_W       0x1D
#define KEY_X       0x22
#define KEY_Y       0x35
#define KEY_Z       0x1A
#define KEY_1       0x16
#define KEY_2       0x1E
#define KEY_3       0x26
#define KEY_4       0x25
#define KEY_5       0x2E
#define KEY_6       0x36
#define KEY_7       0x3D
#define KEY_8       0x3E
#define KEY_9       0x46
#define KEY_0       0x45

#define KEY_ENTER   0x5A
#define KEY_SPACE   0x29

#define KEY_F1      0x05
#define KEY_F2      0x06
#define KEY_F3      0x04
#define KEY_F4      0x0c
#define KEY_F5      0x03
#define KEY_F6      0x0B
#define KEY_F7      0x83
#define KEY_F8      0x0A
#define KEY_F9      0x01
#define KEY_F10     0x09
#define KEY_F11     0x78
#define KEY_F12     0x07

#define KEY_LSHIFT  0x12
#define KEY_RSHIFT  0x59

#define KEY_CAPS    0x58

#define KEY_TAB     0x0D

#define KEY_TLD     0x0E //Izxda del 1
#define KEY_MENOS   0x4E //Drcha del 0
#define KEY_IGUAL   0x55 //Izda de Backspace
#define KEY_ACORCHE 0x54 //Drcha de la P
#define KEY_CCORCHE 0x5B //Siguiente a la de la Drcha de la P
#define KEY_BKSLASH 0x5D //Izda del Enter (Puede estar en la fila de la P o de la L
#define KEY_PTOCOMA 0x4C //La Ñ
#define KEY_COMILLA 0x52 //Derecha de la Ñ
#define KEY_COMA    0x41 //Decha de la M
#define KEY_PUNTO   0x49 //Siguiente del de la Derecha de la M
#define KEY_SLASH   0x4A //Izda del Shift Derecho
#define KEY_LESS    0x61 //Izda de la Z

#define KEY_X_FN    0xF1 //Tecla extra de funcion
#define KEY_X_NMI   0xF2 //Tecla extra de NMI
#define KEY_X_RST   0xF3 //Tecla extra de reset

#define KC_OFFSET   K|M
#define KC_BRK      (KC_OFFSET+0)
#define KC_EDIT     (KC_OFFSET+1)
#define KC_GRAPH    (KC_OFFSET+2)
#define KC_CTALDEL  (KC_OFFSET+3) // ctrl alt del
#define KC_CTALBS   (KC_OFFSET+4) // ctrl alt del
#define KC_CTALDEL1 (KC_OFFSET+5) // ctrl alt del
#define KC_CTALBS1  (KC_OFFSET+6) // ctrl alt del


#define MAX_KEY_COMB 4

#define S 0x100 // shifted
#define E 0x800 // extended
#define K 0x200 // key combination
#define M 0x400 // make-break
#define NADA  0xFF

const uint16_t kc_lut[][MAX_KEY_COMB] = {
  {KEY_LSHIFT, KEY_SPACE, 0, 0}, //KC_BRK
  {KEY_LSHIFT, KEY_1, 0, 0}, //KC_EDIT
  {KEY_LSHIFT, KEY_9, 0, 0}, //KC_GRAPH
  {E|KEY_LCTRL, KEY_LALT, E|KEY_DELETE, 0}, //KC_CTALDEL
  {KEY_LALT, E|KEY_LCTRL, KEY_BACKSP, 0}, //KC_CTALBS
  {E|KS1_LCTRL, KS1_LALT, E|KS1_SYSRQ, 0}, //KC_CTALDEL
  {KS1_LALT, E|KS1_LCTRL, KS1_BACKSP, 0}, //KC_CTALBS
};

#define ROWS  9
#define COLS  6
// #define ROWS_T  9
// #define COLS_T  6

#define SHIFT_ROW   5
#define SHIFT_COL   0

const uint16_t mapZX[ROWS][COLS] = {
    {       KEY_1,      KEY_2,      KEY_3,      KEY_4,      KEY_5,      KEY_RSHIFT},
    {       KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_X_NMI },
    {       KEY_A,      KEY_S,      KEY_D,      KEY_F,      KEY_G,      KEY_X_RST },
    {       KEY_0,      KEY_9,      KEY_8,      KEY_7,      KEY_6,      KEY_BACKSP},
    {       KEY_P,      KEY_O,      KEY_I,      KEY_U,      KEY_Y,      E|KEY_LEFT  },
    {       KEY_LSHIFT, KEY_Z,      KEY_X,      KEY_C,      KEY_V,      E|KEY_RIGHT },
    {       KEY_ENTER,  KEY_L,      KEY_K,      KEY_J,      KEY_H,      E|KEY_DOWN  },
    {       KEY_SPACE,  KEY_RCTRL,  KEY_M,      KEY_N,      KEY_B,      E|KEY_UP    },
    {       KC_BRK,     KEY_CAPS,       0,    KEY_TAB,    KC_EDIT,     KC_GRAPH   }
};

const uint8_t mapCPC[ROWS][COLS] = {
    {       KEY_1,      KEY_2,      KEY_3,      KEY_4,      KEY_5,      KEY_RSHIFT},
    {       KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_X_NMI },
    {       KEY_A,      KEY_S,      KEY_D,      KEY_F,      KEY_G,      KEY_X_RST },
    {       KEY_0,      KEY_9,      KEY_8,      KEY_7,      KEY_6,      KEY_BACKSP},
    {       KEY_P,      KEY_O,      KEY_I,      KEY_U,      KEY_Y,      KEY_LEFT  },
    {       0,          KEY_Z,      KEY_X,      KEY_C,      KEY_V,      KEY_RIGHT },
    {       KEY_ENTER,  KEY_L,      KEY_K,      KEY_J,      KEY_H,      KEY_DOWN  },
    {       KEY_SPACE,  0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       KEY_ESCAPE, KEY_CAPS,   0,          KEY_TAB,    KEY_F2,     KEY_F10   }
};

const uint16_t mapCPC_shifted[ROWS][COLS] = {
    {       KEY_F2,      KEY_CAPS,S|KEY_3,    S|KEY_4,    E|KEY_LEFT, S|KEY_RSHIFT},
    {     S|KEY_Q,    S|KEY_W,    S|KEY_E,    S|KEY_R,    S|KEY_T,    S|KEY_X_NMI },
    {     S|KEY_A,    S|KEY_S,    S|KEY_D,    S|KEY_F,    S|KEY_G,    S|KEY_X_RST },
    {     KEY_BACKSP, S|KEY_9,E|KEY_RIGHT,    E|KEY_UP, E|KEY_DOWN,    S|KEY_BACKSP},
    {     S|KEY_P,    S|KEY_O,    S|KEY_I,    S|KEY_U,    S|KEY_Y,    S|KEY_LEFT  },
    {       0,        S|KEY_Z,    S|KEY_X,    S|KEY_C,    S|KEY_V,    S|KEY_RIGHT },
    {       KEY_TAB,  S|KEY_L,    S|KEY_K,    S|KEY_J,    S|KEY_H,    S|KEY_DOWN  },
    {       KEY_ESCAPE, 0,        S|KEY_M,    S|KEY_N,    S|KEY_B,    S|KEY_UP    },
    {     S|KEY_ESCAPE, KEY_CAPS|S, 0,          KEY_TAB|S,  KEY_F2,     KEY_F10   }
};

const uint16_t mapCPC_sym[ROWS][COLS] = {
    {     S|KEY_1,  KEY_ACORCHE,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA,  S|KEY_PUNTO,      0 },
    {  S|KEY_LESS,S|KEY_ACORCHE, KEY_LESS,S|KEY_CCORCHE,S|KEY_BKSLASH,  0 },
    {     S|KEY_0,    S|KEY_9,    S|KEY_8,    S|KEY_7,    S|KEY_6,      0 },
    {     S|KEY_2,KEY_COMILLA,          0,KEY_BKSLASH,KEY_CCORCHE,      0  },
    {       0,    KEY_PTOCOMA,S|KEY_IGUAL,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    S|KEY_MENOS,S|KEY_COMILLA,KEY_MENOS,  KEY_IGUAL,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapMSX_sym[ROWS][COLS] = {
    {     S|KEY_1,  S|KEY_COMILLA,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA,  S|KEY_PUNTO,      0 },
    {  S|KEY_CCORCHE,S|KEY_BKSLASH, KEY_BKSLASH,KEY_TLD,S|KEY_ACORCHE,  0 },
    {     KEY_IGUAL,    0,    S|KEY_0,    S|KEY_PTOCOMA,    KEY_COMILLA,      0 },
    {     S|KEY_8,KEY_PTOCOMA,          0,KEY_ACORCHE,S|KEY_2,      0  },
    {       0,    S|KEY_IGUAL,          0,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    S|KEY_6,  S|KEY_TLD, KEY_MENOS,  S|KEY_7,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapC64_sym[ROWS][COLS] = {
    {     S|KEY_1,  KEY_ACORCHE,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_LESS,  S|KEY_COMA,      0 },
    {  0,S|KEY_MENOS, 0,0,0,  0 },
    {     S|KEY_0, S|KEY_9, S|KEY_8, S|KEY_7, S|KEY_6,      0 },
    {     S|KEY_2,KEY_COMILLA,          0,S|KEY_COMILLA,S|KEY_PTOCOMA,      0  },
    {       0,    KEY_PTOCOMA,          0,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    KEY_IGUAL,  KEY_F10, KEY_MENOS,  KEY_BKSLASH,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapAT8_sym[ROWS][COLS] = {
    {     S|KEY_1,  S|KEY_8,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      KEY_MENOS,  KEY_IGUAL,      0 },
    {  0,S|KEY_CCORCHE, S|KEY_COMILLA,0,0,  0 },
    {     S|KEY_0, S|KEY_9, S|KEY_8, S|KEY_7, S|KEY_6,      0 },
    {     S|KEY_2,KEY_PTOCOMA,          0,S|KEY_PUNTO,S|KEY_COMA,      0  },
    {       0,    S|KEY_PTOCOMA,          0,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    KEY_CCORCHE,  KEY_COMILLA, KEY_ACORCHE,  S|KEY_BKSLASH,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapBBC_sym[ROWS][COLS] = {
    {     S|KEY_1,  KEY_TLD,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA, S|KEY_PUNTO,      0 },
    {  KEY_MENOS,S|KEY_LESS, 0,0,0,  0 },
    {     S|KEY_0, S|KEY_9, S|KEY_8, S|KEY_7, S|KEY_6,      0 },
    {     S|KEY_2,KEY_PTOCOMA,          0,0,0,      0  },
    {       0,    KEY_COMILLA, S|KEY_BKSLASH,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    S|KEY_MENOS,  S|KEY_PTOCOMA, KEY_BKSLASH,  KEY_IGUAL,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapACO_sym[ROWS][COLS] = {
    {     S|KEY_1,  S|KEY_0,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA, S|KEY_PUNTO,      0 },
    {  0,0, KEY_BKSLASH,0,0,  0 },
    {     S|KEY_0, S|KEY_9, S|KEY_8, S|KEY_7, S|KEY_6,      0 },
    {     S|KEY_2,KEY_PTOCOMA,          0,KEY_CCORCHE,S|KEY_ACORCHE,      0  },
    {       0,    KEY_COMILLA, S|KEY_ACORCHE,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    S|KEY_MENOS,  S|KEY_PTOCOMA, KEY_MENOS,  0,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapAP2_sym[ROWS][COLS] = {
    {     S|KEY_1,  S|KEY_2,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA, S|KEY_PUNTO,      0 },
    {  0,0, 0,0,0,  0 },
    {     S|KEY_MENOS, S|KEY_0, S|KEY_9, KEY_COMILLA, S|KEY_7,      0 },
    {     S|KEY_COMILLA,KEY_PTOCOMA,          0,KEY_CCORCHE,KEY_ACORCHE,      0  },
    {       0,    S|KEY_PTOCOMA, 0,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    KEY_IGUAL,  S|KEY_IGUAL, KEY_MENOS,  S|KEY_6,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapVIC_sym[ROWS][COLS] = {
    {     S|KEY_1,  KEY_ACORCHE,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_LESS, S|KEY_COMA,      0 },
    {  0,S|KEY_IGUAL, 0,0,0,  0 },
    {     S|KEY_0, S|KEY_9, S|KEY_8, S|KEY_7, S|KEY_6,      0 },
    {     S|KEY_2,KEY_COMILLA,          0,S|KEY_COMILLA,S|KEY_PTOCOMA,      0  },
    {       0,    KEY_PTOCOMA, 0,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    KEY_BKSLASH,  KEY_MENOS, KEY_IGUAL,  0,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapORI_sym[ROWS][COLS] = {
    {     S|KEY_1,  0,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA, S|KEY_PUNTO,      0 },
    {  0,S|KEY_BKSLASH, KEY_BKSLASH,S|KEY_ACORCHE,S|KEY_CCORCHE,  0 },
    {     S|KEY_0, S|KEY_0, S|KEY_9, KEY_COMILLA, S|KEY_7,      0 },
    {     S|KEY_COMILLA,KEY_PTOCOMA,          0,KEY_CCORCHE,KEY_ACORCHE,      0  },
    {       0,    S|KEY_PTOCOMA, S|KEY_MENOS,S|KEY_SLASH,  KEY_SLASH,      0 },
    {       0,    KEY_IGUAL,  S|KEY_IGUAL, KEY_MENOS,  S|KEY_6,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapSAM_sym[ROWS][COLS] = {
    {     S|KEY_1,  S|KEY_BKSLASH,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      KEY_LESS, S|KEY_LESS,      0 },
    {  KEY_PTOCOMA,0, 0,KEY_COMILLA,KEY_BKSLASH,  0 },
    {     S|KEY_SLASH, S|KEY_9, S|KEY_8, KEY_MENOS, S|KEY_6,      0 },
    {     S|KEY_2,S|KEY_COMA,  0,0,0,      0  },
    {       0,    S|KEY_PUNTO, KEY_ACORCHE,S|KEY_MENOS,  S|KEY_7,      0 },
    {       0,    S|KEY_0,  KEY_CCORCHE, KEY_SLASH,  S|KEY_ACORCHE,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapJUP_sym[ROWS][COLS] = {
    {     S|KEY_1,  KEY_BKSLASH,    S|KEY_3,    S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      KEY_LESS, S|KEY_LESS,      0 },
    {  0,0, KEY_TLD,S|KEY_COMILLA,0,  0 },
    {     S|KEY_SLASH, S|KEY_9, S|KEY_8, KEY_MENOS, S|KEY_6,      0 },
    {     S|KEY_2,S|KEY_COMA,  0,0,S|KEY_ACORCHE,      0  },
    {       0,    S|KEY_PUNTO, KEY_COMILLA,S|KEY_MENOS,  S|KEY_7,      0 },
    {       0,    S|KEY_0,  KEY_CCORCHE, KEY_SLASH,  KEY_ACORCHE,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,      KEY_UP    },
    {       0,          0,          0,          0,          0,          0         }
};

const uint16_t mapFUS[ROWS][COLS] = {
    {       KEY_1,      KEY_2,      KEY_3,      KEY_4,      KEY_5,      KEY_RSHIFT},
    {       KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_X_NMI },
    {       KEY_A,      KEY_S,      KEY_D,      KEY_F,      KEY_G,      KEY_X_RST },
    {       KEY_0,      KEY_9,      KEY_8,      KEY_7,      KEY_6,      KEY_BACKSP},
    {       KEY_P,      KEY_O,      KEY_I,      KEY_U,      KEY_Y,      E|KEY_LEFT  },
    {       KEY_LSHIFT, KEY_Z,      KEY_X,      KEY_C,      KEY_V,      E|KEY_RIGHT },
    {       KEY_ENTER,  KEY_L,      KEY_K,      KEY_J,      KEY_H,      E|KEY_DOWN  },
    {       KEY_SPACE,  KEY_LCTRL,  KEY_M,      KEY_N,      KEY_B,      E|KEY_UP    } ,
    {       KEY_ESCAPE, KEY_CAPS,   0,          KEY_TAB,    KEY_F2,     KEY_F10   }
};

const uint16_t mapFUS_shifted[ROWS][COLS] = {
    {       KEY_F2,      KEY_CAPS,S|KEY_3,    S|KEY_4,    E|KEY_LEFT, S|KEY_RSHIFT},
    {     S|KEY_Q,    S|KEY_W,    S|KEY_E,    S|KEY_R,    S|KEY_T,    S|KEY_X_NMI },
    {     S|KEY_A,    S|KEY_S,    S|KEY_D,    S|KEY_F,    S|KEY_G,    S|KEY_X_RST },
    {     KEY_BACKSP, S|KEY_9,E|KEY_RIGHT,    E|KEY_UP, E|KEY_DOWN,    S|KEY_BACKSP},
    {     S|KEY_P,    S|KEY_O,    S|KEY_I,    S|KEY_U,    S|KEY_Y,    S|KEY_LEFT  },
    {       0,        S|KEY_Z,    S|KEY_X,    S|KEY_C,    S|KEY_V,    S|KEY_RIGHT },
    {       KEY_TAB,  S|KEY_L,    S|KEY_K,    S|KEY_J,    S|KEY_H,    S|KEY_DOWN  },
    {       KEY_ESCAPE, 0,        S|KEY_M,    S|KEY_N,    S|KEY_B,    S|KEY_UP    },
    {     KEY_ESCAPE, KEY_CAPS|S, 0,          KEY_TAB|S,  KEY_F2,     KEY_F10   }
};

const uint16_t mapFUS_sym[ROWS][COLS] = {
    {       KEY_1,      KEY_2,      KEY_3,      KEY_4,      KEY_5,      KEY_RSHIFT},
    {       KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_X_NMI },
    {       KEY_A,      KEY_S,      KEY_D,      KEY_F,      KEY_G,      KEY_X_RST },
    {       KEY_0,      KEY_9,      KEY_8,      KEY_7,      KEY_6,      KEY_BACKSP},
    {       KEY_P,      KEY_O,      KEY_I,      KEY_U,      KEY_Y,      E|KEY_LEFT  },
    {       KEY_LSHIFT, KEY_Z,      KEY_X,      KEY_C,      KEY_V,      E|KEY_RIGHT },
    {       KEY_ENTER,    KEY_L,  KEY_K, KEY_J,  KEY_H,      E|KEY_DOWN  },
    {       KEY_SPACE,          0,          KEY_M,      KEY_N,      KEY_B,     KEY_UP    },
    {       KEY_ESCAPE,     KEY_CAPS,          0,         KEY_TAB,      KEY_F2,          KEY_F10         }
};

const uint16_t mapPC[ROWS][COLS] = {
    {       KEY_1,      KEY_2,      KEY_3,      KEY_4,      KEY_5,      KEY_RSHIFT},
    {       KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_X_NMI },
    {       KEY_A,      KEY_S,      KEY_D,      KEY_F,      KEY_G,      KEY_X_RST },
    {       KEY_0,      KEY_9,      KEY_8,      KEY_7,      KEY_6,      KEY_BACKSP},
    {       KEY_P,      KEY_O,      KEY_I,      KEY_U,      KEY_Y,      KEY_LEFT  },
    {       0,          KEY_Z,      KEY_X,      KEY_C,      KEY_V,      KEY_RIGHT },
    {       KEY_ENTER,  KEY_L,      KEY_K,      KEY_J,      KEY_H,      KEY_DOWN  },
    {       KEY_SPACE,  0,  KEY_M,      KEY_N,      KEY_B,      KEY_UP    } ,
    {       KEY_ESCAPE, KEY_CAPS,   0,          KEY_TAB,    KEY_F2,     KEY_F10   }
};

const uint16_t mapPCopqa[ROWS][COLS] = {
    {       KEY_1,      KEY_2,      KEY_3,      KEY_4,      KEY_5,      KEY_RSHIFT},
    {    E|KEY_UP,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_X_NMI },
    {    E|KEY_DOWN,    KEY_S,      KEY_D,      KEY_F,      KEY_G,      KEY_X_RST },
    {       KEY_0,      KEY_9,      KEY_8,      KEY_7,      KEY_6,      KEY_BACKSP},
    {    E|KEY_RIGHT,  E|KEY_LEFT,  KEY_I,      KEY_U,      KEY_Y,    E|KEY_LEFT  },
    {       0,          KEY_Z,      KEY_X,      KEY_C,      KEY_V,    E|KEY_RIGHT },
    {       KEY_ENTER,  KEY_L,      KEY_K,      KEY_J,      KEY_H,    E|KEY_DOWN  },
    {       KEY_SPACE,  0,          KEY_M,      KEY_N,      KEY_B,    E|KEY_UP    } ,
    {       KEY_ESCAPE, KEY_CAPS,   0,          KEY_TAB,    KEY_F2,     KEY_F10   }
};

const uint16_t mapPC_shifted[ROWS][COLS] = {
    {       KEY_F2,      KEY_CAPS,S|KEY_3,    S|KEY_4,    E|KEY_LEFT, S|KEY_RSHIFT},
    {     S|KEY_Q,    S|KEY_W,    S|KEY_E,    S|KEY_R,    S|KEY_T,    S|KEY_X_NMI },
    {     S|KEY_A,    S|KEY_S,    S|KEY_D,    S|KEY_F,    S|KEY_G,    S|KEY_X_RST },
    {     KEY_BACKSP, S|KEY_9,E|KEY_RIGHT,    E|KEY_UP, E|KEY_DOWN,    S|KEY_BACKSP},
    {     S|KEY_P,    S|KEY_O,    S|KEY_I,    S|KEY_U,    S|KEY_Y,    S|KEY_LEFT  },
    {       0,        S|KEY_Z,    S|KEY_X,    S|KEY_C,    S|KEY_V,    S|KEY_RIGHT },
    {       KEY_TAB,  S|KEY_L,    S|KEY_K,    S|KEY_J,    S|KEY_H,    S|KEY_DOWN  },
    {       KEY_ESCAPE, 0,        S|KEY_M,    S|KEY_N,    S|KEY_B,    S|KEY_UP    },
    {     S|KEY_ESCAPE, KEY_CAPS|S, KEY_LSHIFT,          KEY_TAB|S,  KEY_F2,     KEY_F10   }
};

const uint16_t mapPC_sym[ROWS][COLS] = {
    {       S|KEY_1,      S|KEY_2,      S|KEY_3,      S|KEY_4,      S|KEY_5,      0},
    {       0,      0,      0,      S|KEY_COMA,      S|KEY_PUNTO,      0},
    {       S|KEY_TLD,      S|KEY_BKSLASH,      KEY_LESS,      S|KEY_ACORCHE,      S|KEY_CCORCHE,      0 },
    {       S|KEY_MENOS,      S|KEY_0,      S|KEY_9,      KEY_COMILLA,      S|KEY_7,      0},
    {       S|KEY_COMILLA,      KEY_PTOCOMA,      0,      KEY_CCORCHE,      KEY_ACORCHE,      0  },
    {       0, S|KEY_PTOCOMA,      0,      S|KEY_SLASH,      KEY_SLASH,      0 },
    {       0,    KEY_IGUAL,  S|KEY_IGUAL, KEY_MENOS,  S|KEY_6,      0  },
    {       0,          0,          KEY_M,      KEY_N,      KEY_B,     KEY_UP    },
    {       0,     0,          0,         0,      0,          0}
};

const uint8_t mapSET1[ROWS][COLS] = { //MAPA Codeset 1
    {       KS1_1,      KS1_2,      KS1_3,      KS1_4,      KS1_5,      0       },
    {       KS1_Q,      KS1_W,      KS1_E,      KS1_R,      KS1_T,      0       },
    {       KS1_A,      KS1_S,      KS1_D,      KS1_F,      KS1_G,      0       },
    {       KS1_0,      KS1_9,      KS1_8,      KS1_7,      KS1_6,      0       },
    {       KS1_P,      KS1_O,      KS1_I,      KS1_U,      KS1_Y,      0       },
    {       0,          KS1_Z,      KS1_X,      KS1_C,      KS1_V,      0       },
    {       KS1_ENTER,  KS1_L,      KS1_K,      KS1_J,      KS1_H,      0       },
    {       KS1_SPACE,  0,          KS1_M,      KS1_N,      KS1_B,      0       },
    {       0,          0,          0,          0,          0,          0       }
};

const uint16_t mapSET1opqa[ROWS][COLS] = { //MAPA Codeset 1
    {       KS1_1,      KS1_2,      KS1_3,      KS1_4,      KS1_5,      0       },
    {     E|KS1_UP,     KS1_W,      KS1_E,      KS1_R,      KS1_T,      0       },
    {     E|KS1_DOWN,   KS1_S,      KS1_D,      KS1_F,      KS1_G,      0       },
    {       KS1_0,      KS1_9,      KS1_8,      KS1_7,      KS1_6,      0       },
    {     E|KS1_RIGHT,  E|KS1_LEFT, KS1_I,      KS1_U,      KS1_Y,      0       },
    {       0,          KS1_Z,      KS1_X,      KS1_C,      KS1_V,      0       },
    {       KS1_ENTER,  KS1_L,      KS1_K,      KS1_J,      KS1_H,      0       },
    {       KS1_SPACE,  0,          KS1_M,      KS1_N,      KS1_B,      0       },
    {       0,          0,          0,          0,          0,          0       }
};

const uint16_t mapEXT1[ROWS][COLS] = { //Mapa especial con caps shift para Codeset1(Igual en todos los Keymaps)
    {       KS1_F2,     KS1_CAPS, S|KS1_3,    S|KS1_4,    E|KS1_LEFT,   0       },
    {     S|KS1_Q,    S|KS1_W,    S|KS1_E,    S|KS1_R,    S|KS1_T,      0       },
    {     S|KS1_A,    S|KS1_S,    S|KS1_D,    S|KS1_F,    S|KS1_G,      0       },
    {     KS1_BACKSP, S|KS1_9,E|KS1_RIGHT,    S|KS1_7, E|KS1_DOWN,      0       },
    {     S|KS1_P,    S|KS1_O,    S|KS1_I,    S|KS1_U,    S|KS1_Y,      0       },
    {       0,        S|KS1_Z,    S|KS1_X,    S|KS1_C,    S|KS1_V,      0       },
    {       KS1_TAB,  S|KS1_L,    S|KS1_K,    S|KS1_J,    S|KS1_H,      0       },
    {       KS1_ESCAPE, 0,        S|KS1_M,    S|KS1_N,    S|KS1_B,      0       },
    {       0,          0,          0,          0,          0,          0       }
};
const uint16_t mapXT1[ROWS][COLS] = { //Mapa de PC-XT CodeSet1 pulsando Control (symbol shift)
    {     S|KS1_1,    S|KS1_2,      S|KS1_3,    S|KS1_4,      S|KS1_5,      0       },
    {     NADA,       NADA,         NADA,       S|KS1_COMA,   S|KS1_PUNTO,  0       },
    {     S|KS1_TLD,  S|KS1_BKSLASH,KS1_BKSLASH,S|KS1_ACORCHE,S|KS1_CCORCHE,0       },
    {     S|KS1_MENOS,S|KS1_0,      S|KS1_9,    KS1_COMILLA,  S|KS1_7,      0       },
    {   S|KS1_COMILLA,KS1_CCORCHE,  NADA,       KS1_CCORCHE,  KS1_ACORCHE,  0       },
    {       0,        S|KS1_PTOCOMA,NADA,       S|KS1_SLASH,  KS1_SLASH,    0       },
    {       NADA,     KS1_IGUAL,    S|KS1_IGUAL,KS1_MENOS,    S|KS1_6,      0       },
    {       NADA,     0,            KS1_PUNTO,  KS1_COMA,     S|KS1_8,      0       },
    {       0,        0,            0,          0,            0,            0       }
};
const uint8_t modXT1[ROWS][COLS] = { //Mod de PC-XT CodeSet1 1 hay q usar Shift, 0 no hay que usar
    {       1,          1,          1,          1,          1,          0       },
    {       0,          0,          0,          1,          1,          0       },
    {       1,          1,          0,          1,          1,          0       },
    {       1,          1,          1,          0,          1,          0       },
    {       1,          0,          0,          0,          0,          0       },
    {       0,          1,          0,          1,          0,          0       },
    {       0,          0,          1,          0,          1,          0       },
    {       0,          0,          0,          0,          1,          0       },
    {       0,          0,          0,          0,          0,          0       }
};

const uint16_t mapFN1[ROWS][COLS] = {
    { M|KS1_F1,    M|KS1_F2,      M|KS1_F3,   M|KS1_F4,     M|KS1_F5,      0},
    {M|KS1_F11,    M|KS1_F12,     0,          0,            0,             0 },
    {        0,    0,             0,          0,            M|KS1_SCRLCK,  0/*KS1_X_RST*/ },
    {M|KS1_F10,    M|KS1_F9,      M|KS1_F8,   M|KS1_F7,     M|KS1_F6,      E|M|KS1_DELETE},
    {        0,    0,             0,          0,            0,             0  },
    {        0,    0,             0,          0,            0,             E|M|KS1_END   },
    {        0,    0,             0,          0,            0,             E|M|KS1_PGDW  },
    {        0,    0,             0,          M|KC_CTALDEL1,KS1_B,         0  },
    {        0,    M|KEY_NUMLCK,  0,          0,            0,             0   }
};

const uint16_t mapFN[ROWS][COLS] = {
    { M|KEY_F1,    M|KEY_F2,      M|KEY_F3,   M|KEY_F4,     M|KEY_F5,      0},
    {M|KEY_F11,    M|KEY_F12,     0,          0,            0,             0 },
    {        0,    0,             0,          0,            M|KEY_SCRLCK,  0/*KEY_X_RST*/ },
    {M|KEY_F10,    M|KEY_F9,      M|KEY_F8,   M|KEY_F7,     M|KEY_F6,      E|M|KEY_DELETE},
    {        0,    0,             0,          0,            0,             E|KEY_LEFT  },
    {        0,    0,             0,          0,            0,             E|M|KEY_END   },
    {        0,    0,             0,          0,            0,             E|M|KEY_PGDW  },
    {        0,    M|KEY_RSHIFT,    0,          M|KC_CTALDEL, M|KC_CTALBS,   E|M|KEY_PGUP  },
    {M|KEY_ESCAPE, M|KEY_NUMLCK,  0,          0,            0,             0   }
};

const uint16_t mapEXT[ROWS][COLS] = { //Mapa especial con caps shift (Igual en todos los Keymaps)
    {       KEY_F2,     KEY_CAPS,     0,            0,          KEY_LEFT,     0           },
    {       0,          0,            0,            0,          0,            0           },
    {       0,          0,            0,            0,          0,            0           },
    {       KEY_BACKSP, 0,            KEY_RIGHT,    KEY_UP,     KEY_DOWN,     0           },
    {       0,          0,            0,            0,          0,            0           },
    {       0,          0,            0,            0,          0,            0           },
    {       KEY_TAB,    0,            0,            0,          0,            0           },
    {       KEY_ESCAPE, 0,            0,            0,          0,            0           },
    {       0,          0,            0,            0,          KEY_F2,       KEY_F10     }
};

#endif
