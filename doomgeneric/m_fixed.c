//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Fixed point implementation.
//



#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"




// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
#ifdef __v810
  int shift = FRACBITS;
  asm(
      // NOTE: This will fail if shift <= 0 but we do not check for that.
      "mul %[a], %[b];"
      "shr %[shift], %[b];"
      "not %[shift], %[shift];"
      "add 1, %[shift];"
      "shl %[shift], r30;"
      "or r30, %[b];"
      : [b] "=r"(b), [shift] "=r"(shift) // Shift is clobbered.
      // b and shift are used above, so declare them as inputs like this:
      : "0"(b), "1"(shift), [a] "r"(a)
      : "r30", "psw");
  return b;
#else  // __v8102
    return ((int64_t) a * (int64_t) b) >> FRACBITS;
#endif
}



//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
	return (a^b) < 0 ? INT_MIN : INT_MAX;
    }
    else
    {
//#if defined(__v810) && 0
      return ((a << 5) / (b >> 7)) << 4; // Looks OK, but crashes eventually.
//#else
	/*int64_t result;

	result = ((int64_t) a << 16) / b;

	return (fixed_t) result;*/
//#endif
    }
}

