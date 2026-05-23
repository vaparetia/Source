//------------------------------------------------------------------------------------------
// CSphericalHarmonics3.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CSphericalHarmonics3.h"

//------------------------------------------------------------------------------------------

CSphericalHarmonics3::CSphericalHarmonics3()
:  mCoeffs( kNumCoeffs, CVector3::Zero() )
{
}

//------------------------------------------------------------------------------------------
   
CSphericalHarmonics3::CSphericalHarmonics3( CVector3 const & c1,
                                            CVector3 const & c2,
                                            CVector3 const & c3,
                                            CVector3 const & c4,
                                            CVector3 const & c5,
                                            CVector3 const & c6,
                                            CVector3 const & c7,
                                            CVector3 const & c8,
                                            CVector3 const & c9 )
{
   mCoeffs.push_back( c1 );
   mCoeffs.push_back( c2 );
   mCoeffs.push_back( c3 );
   mCoeffs.push_back( c4 );
   mCoeffs.push_back( c5 );
   mCoeffs.push_back( c6 );
   mCoeffs.push_back( c7 );
   mCoeffs.push_back( c8 );
   mCoeffs.push_back( c9 );
}

//------------------------------------------------------------------------------------------
   
CSphericalHarmonics3 const CSphericalHarmonics3::operator * ( real32 const rhs ) const
{
   return CSphericalHarmonics3( *this ) *= rhs;
}
   
//------------------------------------------------------------------------------------------

CSphericalHarmonics3 & CSphericalHarmonics3::operator *= ( real32 const rhs )
{
   for( int i = 0; i < kNumCoeffs; ++i )
   {
      mCoeffs[i] *= rhs;
   }

   return *this;
}

//------------------------------------------------------------------------------------------

CSphericalHarmonics3 const CSphericalHarmonics3::operator + ( CSphericalHarmonics3 const & rhs ) const
{
   return CSphericalHarmonics3( *this ) += rhs;
}

//------------------------------------------------------------------------------------------

CSphericalHarmonics3 & CSphericalHarmonics3::operator += ( CSphericalHarmonics3 const & rhs )
{
   for( int i = 0; i < kNumCoeffs; ++i )
   {
      mCoeffs[i] += rhs.mCoeffs[i];
   }

   return *this;
}


