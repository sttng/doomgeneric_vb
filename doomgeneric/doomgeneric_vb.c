#include <stdint.h>
#include <sys/stat.h>

#include "doomgeneric.h"
#include "doomkeys.h"

volatile uint32_t vb_ms;

//#define RENDER3D_GRAYSCALE 0

static const int DITHER_MATRIX[8][8] = {
    // Premultiply by 3 since we will divide by 3 later.
    {0 * 3, 48 * 3, 12 * 3, 60 * 3, 3 * 3, 51 * 3, 15 * 3, 63 * 3},
    {32 * 3, 16 * 3, 44 * 3, 28 * 3, 35 * 3, 19 * 3, 47 * 3, 31 * 3},
    {8 * 3, 56 * 3, 4 * 3, 52 * 3, 11 * 3, 59 * 3, 7 * 3, 55 * 3},
    {40 * 3, 24 * 3, 36 * 3, 20 * 3, 43 * 3, 27 * 3, 39 * 3, 23 * 3},
    {2 * 3, 50 * 3, 14 * 3, 62 * 3, 1 * 3, 49 * 3, 13 * 3, 61 * 3},
    {34 * 3, 18 * 3, 46 * 3, 30 * 3, 33 * 3, 17 * 3, 45 * 3, 29 * 3},
    {10 * 3, 58 * 3, 6 * 3, 54 * 3, 9 * 3, 57 * 3, 5 * 3, 53 * 3},
    {42 * 3, 26 * 3, 38 * 3, 22 * 3, 41 * 3, 25 * 3, 37 * 3, 21 * 3}};

int Dither(int x, int y, uint32_t in) {
  const int sum = ((in >> 24) & 0xFF) + ((in >> 16) & 0xFF) +
                  ((in >> 8) & 0xFF) + DITHER_MATRIX[x & 7][y & 7];

  // Little trick to divide by 3 by multiplying by its fixed-point reciprocal.
  int final_val = (sum * 0x5555) >> (6 + 16);
  if (final_val > 3) {
    final_val = 3;
  }
  return final_val;
}

// VB Hardware registers.
volatile uint16_t *tcr = (volatile uint16_t *)(0x02000020);
volatile uint8_t *scr = (volatile uint8_t *)(0x02000028);
volatile uint8_t *sdlr = (volatile uint8_t *)(0x02000010);
volatile uint8_t *sdhr = (volatile uint8_t *)(0x02000014);
volatile uint8_t *brta = (volatile uint8_t *)(0x0005F824);
volatile uint8_t *brtb = (volatile uint8_t *)(0x0005F826);
volatile uint8_t *brtc = (volatile uint8_t *)(0x0005F828);
volatile uint8_t *wcr = (volatile uint8_t *)(0x02000024);

void DG_Init() {
  // Enable the timer to support DG_GetTicksMs
  *tcr = 0b1111;

  // Set display brightness.

  *brta = 32;
  *brtb = 64;
  *brtc = 32;

  // TODO: Set column table, etc.

  // 1-wait mode for both expansion and ROM
  *wcr = 0b11;

  // Initiate reading next button state.
  *scr = 0b10000100;
}

enum vb_btn_t {
  VB_BTN_PWR,
  VB_BTN_SGN,
  VB_BTN_A,
  VB_BTN_B,
  VB_BTN_RT,
  VB_BTN_LT,
  VB_BTN_RU,
  VB_BTN_RR,
  VB_BTN_LR,
  VB_BTN_LL,
  VB_BTN_LD,
  VB_BTN_LU,
  VB_BTN_STA,
  VB_BTN_SEL,
  VB_BTN_RL,
  VB_BTN_RD,
};

uint16_t buttons_down;

void DG_DrawFrame() {
  _Static_assert(sizeof(pixel_t) == sizeof(uint32_t));

  if (!(*scr & 0b10)) {
    // Copy current button state.
    buttons_down = *sdlr | (*sdhr << 8);
    // Initiate reading next button state.
    *scr = 0b10000100;
  }

  // TODO: Need to draw both eyes (ideally with parallax),
  // and need to select the correct FB based on VIP state.
  volatile uint16_t *fb = (volatile uint16_t *)(0x00000000);

  // TODO: Avoid multiplies, use accumulators.
  // TODO: Update I_FinishUpdate to directly output to
  // the VB FrameBuffer and skip double (triple?) buffering
  for (int x = 0; x < 384; x++) {
    for (int octcol = 0; octcol < 28; ++octcol) {
      uint16_t col = 0;
      for (int cy = 0; cy < 8; cy++) {
        const int y = (octcol << 3) + cy;
        const uint32_t px32 = DG_ScreenBuffer[y * 384 + x];
        col >>= 2;
        col |= (Dither(x, y, px32) << 14);
      }
      fb[x * 32 + octcol] = col;
    }
  }
}

void DG_SleepMs(uint32_t ms) {
  uint32_t target_ms = vb_ms + ms;
  while (vb_ms < target_ms) {
    asm volatile("halt" :::);
  }
}

uint32_t DG_GetTicksMs() { return vb_ms; }

unsigned char DoomKeyFor(enum vb_btn_t btn) {
  const char NO_OP = 'g';
  switch (btn) {
  default:
  case VB_BTN_PWR:
  case VB_BTN_SGN:
      return NO_OP;
  case VB_BTN_A:
      return KEY_USE;
  case VB_BTN_B:
      return KEY_FIRE;
  case VB_BTN_RT:
      return KEY_FIRE;
  case VB_BTN_LT:
      return KEY_USE;
  case VB_BTN_RU:
      return NO_OP;
    break;
  case VB_BTN_RR:
    return KEY_STRAFE_R;
  case VB_BTN_LR:
    return KEY_RIGHTARROW;
  case VB_BTN_LL:
    return KEY_LEFTARROW;
  case VB_BTN_LD:
    return KEY_DOWNARROW;
  case VB_BTN_LU:
    return KEY_UPARROW;
  case VB_BTN_STA:
    return KEY_ENTER;
  case VB_BTN_SEL:
    return KEY_ESCAPE;
  case VB_BTN_RL:
    return KEY_STRAFE_L;
  case VB_BTN_RD:
    return NO_OP;
  }
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
  static uint16_t last_state;
  uint16_t diffs = last_state ^ buttons_down;
  if (!diffs) {
    return 0;
  }
  const int tz = __builtin_ctz(diffs);

  if ((buttons_down >> tz) & 1) {
    *pressed = 1;
    last_state |= (1 << tz);
  } else {
    *pressed = 0;
    last_state &= ~(1 << tz);
  }
  *doomKey = DoomKeyFor((enum vb_btn_t)tz);

  // Keep going until last_state matches the current state.
  return 1;
}

void DG_SetWindowTitle(const char *title) {
  // We haved no window, so do nothing.
}

int main() {
  char *argv[4] = {
      "doom.vb", "-warp", "1", "1", // Skip title screen
  };
  doomgeneric_Create(4, argv);

  for (;;) {
    doomgeneric_Tick();
  }

  return 0;
}

int mkdir(const char *pathname, mode_t mode) {
  // Stub to make linker happy
  return 0;
}

void putc(int) {}

void puts(char *s) {}

int __call_exitprocs() { return 0; }
void __attribute__((interrupt)) VbPlus_VipInterrupt() {}

void __attribute__((interrupt)) VbPlus_TimerInterrupt() {
  ++vb_ms;
  volatile uint16_t *tlr = (volatile uint16_t *)(0x02000018);
  volatile uint16_t *thr = (volatile uint16_t *)(0x0200001c);
  volatile uint16_t *tcr = (volatile uint16_t *)(0x02000020);

  // 100 100us ticks is 1ms
  *thr = 0;
  *tlr = 100;
  *tcr = 0b1111;
}

void __attribute__((interrupt)) VbPlus_Trap1Exception() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_GamePadInterrupt() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_GamePakInterrupt() {}

void __attribute__((interrupt)) VbPlus_ComInterrupt() {}

void __attribute__((interrupt)) VbPlus_FloatException() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_DivideByZeroException() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_InvalidOpcodeException() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_Trap0Exception() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_AddrTrapException() {
  for (;;)
    ;
}

void __attribute__((interrupt)) VbPlus_DuplexedException() {
  for (;;)
    ;
}
