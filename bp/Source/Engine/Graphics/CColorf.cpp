//------------------------------------------------------------------------------------------
// CColorf.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CColorf.h"
//------------------------------------------------------------------------------------------

#include "Engine/Math/CloseEnough.h"
#include "Engine/Graphics/CColor.h"

//------------------------------------------------------------------------------------------

const real32 CColorf::kOneOver255         = 1.0f / 255.0f;
const real32 CColorf::kAlphaTolerance     = 1e-4f;     // Within 0.01%

//------------------------------------------------------------------------------------------
// Removed unneeded output code when compiling for SPU's

#if !defined(SPU)
CColorf::CColorf( CInputStream& stream )
:  mR( stream.ReadReal32() )
,  mG( stream.ReadReal32() )
,  mB( stream.ReadReal32() )
,  mA( stream.ReadReal32() )
{
}
#endif

//------------------------------------------------------------------------------------------
// Removed unneeded output code when compiling for SPU's

#if !defined(SPU)
void CColorf::PutTo( COutputStream & stream ) const
{
   stream.WriteReal32(mR);
   stream.WriteReal32(mG);
   stream.WriteReal32(mB);
   stream.WriteReal32(mA);
}
#endif

//------------------------------------------------------------------------------------------

CColorf CColorf::FromHSV(real32 const h, real32 const s, real32 v, real32 const a)
{
   // See
   // http://en.wikipedia.org/wiki/HSV_color_space
   // for more information about these calculations.

   // HSV to RGB
   real32 const h_over60 = h / 60.0f;
   int const hi = ((int) h_over60) % 6;
   real32 const f = h_over60 - hi;
   real32 const p = v * (1 - s);
   real32 const q = v * (1 - (f * s));
   real32 const t = v * (1 - ((1 - f) * s));
   if (hi == 0) return CColorf(v, t, p, a);
   if (hi == 1) return CColorf(q, v, p, a);
   if (hi == 2) return CColorf(p, v, t, a);
   if (hi == 3) return CColorf(p, q, v, a);
   if (hi == 4) return CColorf(t, p, v, a);
   
   return CColorf(v, p, q, a); // hi == 5
}

//------------------------------------------------------------------------------------------

real32 const kLogLUV_MinFloat = 0.00001f;
real32 const kLogLUV_MaxFloat = 40.0f;
real32 const kLogLUV_LogOffset = 16.68f;

CColor const CColorf::AsLogLUV() const
{
   // add offset for black values to avoid singularities in log(lum) calculation
   CVector3 color(bpe::max_val(kLogLUV_MinFloat, mR), bpe::max_val(kLogLUV_MinFloat, mG), bpe::max_val(kLogLUV_MinFloat, mB));

   // RGB to XYZ transformation
   CVector3 xyz(CVector3::Dot(color, CVector3(0.4124f, 0.3576f, 0.1805f)),
                CVector3::Dot(color, CVector3(0.2126f, 0.7152f, 0.0722f)),
                CVector3::Dot(color, CVector3(0.0193f, 0.1192f, 0.9505f)));
                 
   // XYZ to Luv transformation

   // the float3(1,15,3) factors are from the standard XYZ->Luv transformation
   // float2(8,16) are calculated values to maximize precision available for 8bit uv

   real32 temp = CVector3::Dot(color, CVector3(1.0, 15.0, 3.0));
   real32 u = 8.0f * xyz.GetX() / temp;
   real32 v  = 16.0f * xyz.GetY() / temp;

   // store the log2 value of luminance
   // ensure that values are positive and between 0 and maxFloat
   real32 L = bpe::min_val(log(color.GetY()) + kLogLUV_LogOffset, kLogLUV_MaxFloat);

   // map maxFloat to 1.0
   L /= kLogLUV_MaxFloat;

   // multiply L by 255 to calculate lower 8bit worth of L
   real32 test = 255.0f * L;
   real32 Llow = test - static_cast<int>(test);

   // calculate the float that we would get after storing Llow into a 8bit texture
   real32 uint8Llow = floor(Llow * 255.0f) / 255.0f;

   // calculate the higher 8bit for the L value
   real32 Lhigh = L - uint8Llow / 255.0f;

   return CColor::FromFloatColor(u / 255.0f, v / 255.0f, Lhigh / 255.0f, Llow / 255.0f);
}

//------------------------------------------------------------------------------------------

bool CColorf::operator != ( CColorf const & rhs ) const
{
   return !close_enough(AsVector4(), rhs.AsVector4());
}

//------------------------------------------------------------------------------------------

bool CColorf::HasUnitAlpha() const
{
   // Compensate for potential rounding errors
   return close_enough(mA, 1.0f, kAlphaTolerance);
}

//------------------------------------------------------------------------------------------

