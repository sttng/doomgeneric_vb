#include <stdint.h>
#include <sys/stat.h>

#include "doomgeneric.h"
#include "doomkeys.h"
#include "i_video.h"

#ifdef VB_OVERDRIVE
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>

int vb_fd;
struct timeval launch_tv;

void DG_Init() {
  vb_fd = open("/dev/virtualboy", O_SYNC | O_RDWR);
  if (vb_fd < 0) {
    fputs("Unable to open /dev/virtualboy\n", stderr);
    abort();
  }
  gettimeofday(&launch_tv, NULL);
}

#else // !VB_OVERDRIVE
volatile uint32_t vb_ms;

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

#endif // !VB_OVERDRIVE

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
#ifdef VB_OVERDRIVE
  read(vb_fd, &buttons_down, sizeof(buttons_down));
#else
  if (!(*scr & 0b10)) {
    // Copy current button state.
    buttons_down = *sdlr | (*sdhr << 8);
    // Initiate reading next button state.
    *scr = 0b10000100;
  }
#endif
}

#ifdef VB_OVERDRIVE
void DG_SleepMs(uint32_t ms) {
  usleep(ms*1000);
}

static int64_t ms_of(const struct timeval* tv) {
  return (((int64_t)tv->tv_sec) * 1000) + (tv->tv_usec / 1000);
}

uint32_t DG_GetTicksMs() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return ms_of(&now) - ms_of(&launch_tv);
}

#else  // !VB_OD

void DG_SleepMs(uint32_t ms) {
  uint32_t target_ms = vb_ms + ms;
  while (vb_ms < target_ms) {
    asm volatile("halt" :::);
  }
}

uint32_t DG_GetTicksMs() { return vb_ms; }
#endif // !VB_OD

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

#ifdef VB_OVERDRIVE
  uint16_t both[(384 * 224 / 4)];
  if (sizeof(both) != 43008) {
    fprintf(stderr, "Unexpected size of frame buffer. Wanted %d, got %d", 43008, sizeof(both));
    abort();
  }
  for (;;) {
    doomgeneric_Tick(both, &both[384*224/8]);
    write(vb_fd, both, sizeof(both));
  }
#else // !VB_OVERDRIVE
#ifdef VB_DOUBLE_BUFFER
  uint16_t left[384 * 256 / 4];
  uint16_t right[384 * 256 / 4];
  for (;;) {
    doomgeneric_Tick(left, right);
    memcpy(0, left, sizeof(left));
    memcpy(0x10000, right, sizeof(right));
  }
#else // !VB_DOUBLE_BUFFER
  for (;;) {
    doomgeneric_Tick((uint16_t *)0, (uint16_t *)0x10000);
  }
#endif // !VB_DOUBLE_BUFFER
#endif // !VB_OVERDRIVE

  return 0;
}

#ifndef VB_OVERDRIVE

int mkdir(const char *pathname, mode_t mode) {
  // Stub to make linker happy
  return 0;
}

void putc(int c) {}

void puts(char *s) {}

#ifdef __GNUC__
// Hopefully never called...
int __muldi3(int a, int b) {
  for (;;)
    ;
}
#endif

int __call_exitprocs() { return 0; }
void __attribute__((interrupt)) VbPlus_VipInterrupt() {}

void __attribute__((interrupt)) VbPlus_TimerInterrupt() {
  ++vb_ms;
  volatile uint16_t *tlr = (volatile uint16_t *)(0x02000018);
  volatile uint16_t *thr = (volatile uint16_t *)(0x0200001c);
  volatile uint16_t *tcr = (volatile uint16_t *)(0x02000020);

  // 10 100us ticks is 1ms
  *thr = 0;
  *tlr = 10;
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
#endif // !VB_OVERDRIVE
