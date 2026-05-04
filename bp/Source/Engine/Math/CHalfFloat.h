//------------------------------------------------------------------------------------------
// CHalfFloat.h
//
// Bluepoint 2009
//
// Basic support for half precision floats (IEEE 754r)
// See http://en.wikipedia.org/wiki/Half_precision
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CHalfFloat
{
public:
   // constructs uninitialized, modeled after real32
   BPE_FORCEINLINE CHalfFloat() {};

   BPE_FORCEINLINE CHalfFloat(real32 r32)       { mU16 = ConvertFromR32(r32); };
   BPE_FORCEINLINE CHalfFloat(uint16 u16)       { mU16 = u16; };

   BPE_FORCEINLINE real32  AsR32() const        { return ConvertToR32(mU16); };
   BPE_FORCEINLINE bool    operator == (CHalfFloat const rhs) const { return mU16 == rhs.mU16; }

   // get/put vector to stream
   ENGINE_API explicit CHalfFloat(CInputStream & stream);
   ENGINE_API void PutTo(COutputStream & stream) const;

   // Factory functions
   ENGINE_API static uint16 ConvertFromR32(real32 r32);
   ENGINE_API static real32 ConvertToR32(uint16 u16);
   
public:
   uint16   mU16;
};

//------------------------------------------------------------------------------------------
// Typedef for readability
typedef CHalfFloat   real16;

//------------------------------------------------------------------------------------------
