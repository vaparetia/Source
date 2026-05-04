//------------------------------------------------------------------------------------------
// CRandom.cpp
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/Math/CRandom.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/MathUtils.h"

//------------------------------------------------------------------------------------------
// Code from: http://www.cast.uark.edu/~kkvamme/ACN37.htm
// See http://en.wikipedia.org/wiki/Normal_distribution for more info.

// Some notable qualities of the normal distribution:

// The density function is symmetric about its mean value. 
// The mean is also its mode and median. 
// 68.268949% of the area under the curve is within one standard deviation of the mean. 
// 95.449974% of the area is within two standard deviations. 
// 99.730020% of the area is within three standard deviations. 
// 99.993666% of the area is within four standard deviations. 

real32 CRandom::RandomFloatRangeGD(real32 const mean, real32 const standardDeviation)
{
   // 4 sigma should be enough if Random32() is random enough.
   int64 x = 0;
   x += Random32();
   x += Random32();
   x += Random32();
   x += Random32();

   x += Random32();
   x += Random32();
   x += Random32();
   x += Random32();
   
   // Divide by 16 (retain fp range), convert to float
   real32 val = (x >> 4) * ((standardDeviation) / 0x0FFFFFFF);

   // Center
   val -= 4.0f * standardDeviation;

   // Set mean
   val += mean;

   return val;
}

//------------------------------------------------------------------------------------------

CVector3 CRandom::RandomVector3(real32 const magnitude)
{
   // http://mathworld.wolfram.com/SpherePointPicking.html

#if 0
   // trig method to get uniform distribution on a sphere
   real32 const phi = 2.0f * gkPi32 * RandomFloat();
   real32 const cosTheta = 2.0f * RandomFloat() - 1.0f;
   real32 const sinTheta = magnitude * sqrtf(1.0f - cosTheta * cosTheta);
   return CVector3(magnitude * cosTheta, sinTheta * cosf(phi), sinTheta * sinf(phi));
#else
   // Marsaglia's method (1972)
   // faster than the trig method on the PC
   // possible branch, so may be poor on console
   real32 x1, x2, lenSq;
   do 
   {
      x1 = 2.0f * RandomFloat() - 1.0f;
      x2 = 2.0f * RandomFloat() - 1.0f;
      lenSq = x1*x1 + x2*x2;
   }
   while (lenSq >= 1.0f); // reject fails 21% (100*(1-pi/4)) of the time
   real32 const sqrtOneMinusLenSq = magnitude * sqrtf(1.0f - lenSq);
   return CVector3(2.0f * x1 * sqrtOneMinusLenSq, 2.0f * x2 * sqrtOneMinusLenSq, magnitude * (2.0f * lenSq - 1.0f));
#endif
}

//------------------------------------------------------------------------------------------

CVector3 CRandom::RandomVector3(real32 const yawRand, real32 const pitchRand)
{
   real32 const pitch = RangeFloat(-pitchRand * 0.5f, pitchRand * 0.5f);
   real32 const yaw = RangeFloat(-yawRand * 0.5f, yawRand * 0.5f);

   real32 const cp = cosf( pitch );
   real32 const sp = sinf( pitch );
   real32 const cy = cosf( yaw );
   real32 const sy = sinf( yaw );

   CVector3 upAxis(-sp * cy, cy * cp, sy);
   return upAxis;
}

//------------------------------------------------------------------------------------------

CVector3 CRandom::RandomVector3(real32 const magnitude, real32 const yawRand, real32 const pitchRand)
{
   real32 const pitch = RangeFloat(-pitchRand, pitchRand);
   real32 const yaw = RangeFloat(-yawRand, yawRand);

   real32 const cp = cosf( pitch );
   real32 const sp = sinf( pitch );
   real32 const cy = cosf( yaw );
   real32 const sy = sinf( yaw );

   return CVector3(-sy * cp * magnitude, sp * magnitude, cy * cp * magnitude);
}

//------------------------------------------------------------------------------------------

static CGlobalRandom * gpRandom = NULL;

//------------------------------------------------------------------------------------------

CGlobalRandom::CGlobalRandom(uint32 const seed)
: mInternalRandom(seed)
, mpSuppliedRandom(NULL)
{
   FixupPointers();
}

//------------------------------------------------------------------------------------------

CGlobalRandom::CGlobalRandom(CRandom &random)
: mInternalRandom(0)
, mpSuppliedRandom(&random)
{
   FixupPointers();
}

//------------------------------------------------------------------------------------------

CGlobalRandom::~CGlobalRandom()
{
   // Need to disable interrupts/threading switching here
   if (mpPrev)
   {
      mpPrev->mpNext = mpNext;
   }
   if (mpNext == NULL)
   {
      gpRandom = mpPrev;
   }
}

//------------------------------------------------------------------------------------------

void CGlobalRandom::FixupPointers()
{
   // Need to disable interrupts/threading switching here
   mpNext = NULL;
   mpPrev = gpRandom;
   gpRandom = this;
}

//------------------------------------------------------------------------------------------

CRandom & CGlobalRandom::Random()
{
   BPE_ASSERT(gpRandom != NULL, "No global random available.");
   return gpRandom->mpSuppliedRandom ? *gpRandom->mpSuppliedRandom : gpRandom->mInternalRandom;
};

//------------------------------------------------------------------------------------------
