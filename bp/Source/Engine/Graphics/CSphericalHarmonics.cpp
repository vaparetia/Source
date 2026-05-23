//------------------------------------------------------------------------------------------
// CSphericalHarmonics.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CSphericalHarmonics.h"

//------------------------------------------------------------------------------------------

CSphericalHarmonics::CSphericalHarmonics()
{
   for( int i = 0; i < kNumCoeffs; ++i )
   {
      mCoeffs[i] = 0.0f;
   }
}

//------------------------------------------------------------------------------------------
   
CSphericalHarmonics::CSphericalHarmonics( real32 const c1,
                                          real32 const c2,
                                          real32 const c3,
                                          real32 const c4,
                                          real32 const c5,
                                          real32 const c6,
                                          real32 const c7,
                                          real32 const c8,
                                          real32 const c9 )
{
   mCoeffs[0] = c1;
   mCoeffs[1] = c2;
   mCoeffs[2] = c3;
   mCoeffs[3] = c4;
   mCoeffs[4] = c5;
   mCoeffs[5] = c6;
   mCoeffs[6] = c7;
   mCoeffs[7] = c8;
   mCoeffs[8] = c9;
}

//------------------------------------------------------------------------------------------
   
CSphericalHarmonics const CSphericalHarmonics::operator * ( real32 const rhs ) const
{
   return CSphericalHarmonics( *this ) *= rhs;
}
   
//------------------------------------------------------------------------------------------

CSphericalHarmonics & CSphericalHarmonics::operator *= ( real32 const rhs )
{
   for( int i = 0; i < kNumCoeffs; ++i )
   {
      mCoeffs[i] *= rhs;
   }

   return *this;
}

//------------------------------------------------------------------------------------------

CSphericalHarmonics const CSphericalHarmonics::operator + ( CSphericalHarmonics const & rhs ) const
{
   return CSphericalHarmonics( *this ) += rhs;
}

//------------------------------------------------------------------------------------------

CSphericalHarmonics & CSphericalHarmonics::operator += ( CSphericalHarmonics const & rhs )
{
   for( int i = 0; i < kNumCoeffs; ++i )
   {
      mCoeffs[i] += rhs.mCoeffs[i];
   }

   return *this;
}

