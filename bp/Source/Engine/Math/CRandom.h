//------------------------------------------------------------------------------------------
// CRandom.h
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------
class CVector3;
//------------------------------------------------------------------------------------------

class CRandom
{
public:
      CRandom(uint32 const seed)
      : mSeed(seed)
   {
   };

   // Returns 0-1.0f including 1.0f
   real32 const RandomFloat()
   {
      mSeed = mSeed * 1103515245 + 12345;
      return (mSeed & 0x0FFFFFFF) * (1.0f / 0x0FFFFFFF);
   }

   // Range 0-32767
   uint32 const Random16()
   {
      mSeed = mSeed * 1103515245 + 12345;
      return (mSeed >> 16) & 0xFFFF;
   }

   // Range 0-(2^32 - 1)
   uint32 const Random32()
   {
      mSeed = mSeed * 1103515245 + 12345;
      return mSeed;
   }

   real32 const RangeFloat( real32 const min, real32 const max )
   {
      return min + RandomFloat() * (max - min);
   }

   // Max can be returned, with equal probability of min and max.
   // Min == max is also fine.
   int const RangeInt( int const min, int const max )
   {
      int32 val = (int32) ((Random32() * ((max - (int64) min) + 1)) >> 32);
      val += min;
      return (int) val;
   }

   // Gaussian (bell curve) random (4 sigma)
   ENGINE_API real32   RandomFloatRangeGD(real32 const mean, real32 const standardDeviation);

   // generates points evenly distributed on a sphere
   ENGINE_API CVector3 RandomVector3(real32 const magnitude);
   /// Makes a Y axis vector with random pitch and yaw
   ENGINE_API CVector3 RandomVector3(real32 const yawRand, real32 const pitchRand);
   /// Makes a world vector with random pitch and yaw
   ENGINE_API CVector3 RandomVector3(real32 const magnitude, real32 const yawRand, real32 const pitchRand);

private:
   uint32      mSeed;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CGlobalRandom
{
public:
   CGlobalRandom(CRandom &random);
   CGlobalRandom(uint32 const seed);
   ~CGlobalRandom();

   CRandom &         Random();

private:
   void              FixupPointers();

   CGlobalRandom *   mpPrev;
   CGlobalRandom *   mpNext;
   CRandom *         mpSuppliedRandom;
   CRandom           mInternalRandom;
};

//------------------------------------------------------------------------------------------
