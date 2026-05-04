//----------------------------------------------------------------------------
// bp_stdlib_ps2.h
// stdlib functions as they worked on PS2
//----------------------------------------------------------------------------
#ifndef __BP_STDLIB_PS2_H__
#define __BP_STDLIB_PS2_H__

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------

#include "MGS_SysCommon.h"

//----------------------------------------------------------------------------
// RAND FUNCTIONS
//----------------------------------------------------------------------------

#define BP_PS2_RAND_MAX 0x7FFFFFFF

extern unsigned int bpe_ps2_rand_seed;

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_PS2_srand(unsigned int seed)
{
   bpe_ps2_rand_seed = seed;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_PS2_rand()
{
   // I don't agree with it...
   //  return (__stdlib_rand_seed = ((((__stdlib_rand_seed * 214013) + 2531011) >> 16) & 0xffff));
   unsigned long long t = bpe_ps2_rand_seed;
   t *= 254124045ull;
   t += 76447ull;
   bpe_ps2_rand_seed = (unsigned int)t;
   // We return a number between 0 and RAND_MAX, which is 2^31-1.
   return (int)((t >> 16) & 0x7FFFFFFF);
}

#endif   //#ifndef __BP_MATH_H__
