//------------------------------------------------------------------------------------------
// CHalfFloat.h
//
// Bluepoint 2009
//
// Basic support for half precision floats (IEEE 754r)
// See http://en.wikipedia.org/wiki/Half_precision
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "CHalfFloat.h"

//------------------------------------------------------------------------------------------

namespace
{

union UR32I32
{
   int32    mI32;
   real32   mR32;
};

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE int32 _uint16_to_real32(uint16 u16)
{
   int32 s = (u16 >> 15) & 0x00000001;
   int32 e = (u16 >> 10) & 0x0000001f;
   int32 m =  u16        & 0x000003ff;

   if (e == 0)
   {
	   if (m == 0)
	   {
	      // +/- zero
	      return s << 31;
	   }
	   else
	   {
	      // Denormalized, needs renormalize
	      while (!(m & 0x00000400))
	      {
		      m <<= 1;
		      e -=  1;
	      }

	      e += 1;
	      m &= ~0x00000400;
	   }
   }
   else if (e == 31)
   {
	   if (m == 0)
	   {
	      // +/- inf
	      return (s << 31) | 0x7f800000;
	   }
	   else
	   {
	      // Nan, keep sign and significand bits
	      return (s << 31) | 0x7f800000 | (m << 13);
	   }
   }

   // Number is normalized
   e = e + (127 - 15);
   m = m << 13;

   return (s << 31) | (e << 23) | m;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE uint16 _real32_to_uint16(real32 r32)
{
   UR32I32  r32i32;
   r32i32.mR32 = r32;
   int32 i32 = r32i32.mI32;

   int32 s =  (i32 >> 16) & 0x00008000;
   int32 e = ((i32 >> 23) & 0x000000ff) - (127 - 15);
   int32 m =   i32        & 0x007fffff;

   if (e <= 0)
   {
      if (e < -10)
	   {
	      // Less than HALF_MIN, convert to a half zero with the same sign as r32.
	      return (uint16) s;
	   }

	   // Convert to a denormalized half.
	   m = m | 0x00800000;

	   int32 t = 14 - e;
	   int32 a = (1 << (t - 1)) - 1;
	   int32 b = (m >> t) & 1;

	   m = (m + a + b) >> t;

	   return (uint16) (s | m);
   }
   else if (e == 0xff - (127 - 15))
   {
	   if (m == 0)
	   {
	      // Infinity, convert to half infinity with the same sign.
	      return (uint16) (s | 0x7c00);
	   }
	   else
	   {
	      // Nan
	      m >>= 13;
	      return (uint16) (s | 0x7c00 | m | (m == 0));
	   }
   }

	// Normalized float.
	m = m + 0x00000fff + ((m >> 13) & 1);

	if (m & 0x00800000)
	{
      // overflow in significand, adjust exponent
	   m =  0;
	   e += 1;
	}

	// Check for exponent overflow
	if (e > 30)
	{
      // Infinite half, return with matching sign.
	   return (uint16) (s | 0x7c00);
   }

	return (uint16) (s | (e << 10) | (m >> 13));
}


} // anon namespace

//------------------------------------------------------------------------------------------

// Factory functions
uint16 CHalfFloat::ConvertFromR32(real32 r32)
{
   return _real32_to_uint16(r32);
}

//------------------------------------------------------------------------------------------

real32 CHalfFloat::ConvertToR32(uint16 u16)
{
      // Convert to real32
   UR32I32 r32i32;
   r32i32.mI32 = _uint16_to_real32(u16);
   return r32i32.mR32;
}

//------------------------------------------------------------------------------------------
// Removed unneeded code when compiling for SPU's
#if !defined(SPU)

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CHalfFloat::CHalfFloat(CInputStream & stream)
{
   mU16 = stream.ReadUint16();
}

//------------------------------------------------------------------------------------------

void CHalfFloat::PutTo(COutputStream & stream) const
{
   stream.WriteUint16(mU16);
}

#endif

//------------------------------------------------------------------------------------------
