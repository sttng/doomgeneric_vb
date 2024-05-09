#pragma once

#include <stdint.h>

extern const int DITHER_MATRIX[8][8];
inline int Dither(int x, int y, uint8_t val) {
  int final_val = val + DITHER_MATRIX[x & 7][y & 7];
  final_val >>= 6;
  return final_val >= 3 ? 3 : final_val;
}
