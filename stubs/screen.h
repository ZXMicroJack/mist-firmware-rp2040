void initScreenReal();
void putpixel(int x, int y, uint32_t pixel);
void updateScreenSpectrum();
void updateScreenJupiterAce();
void updateScreenZX80();
void initScreenJupiterAce();
void initScreenZX80();
void pollKeyboard();

extern uint8_t mem[0x10000];

extern uint8_t specKeys[8];
extern int quit;
extern int debug;