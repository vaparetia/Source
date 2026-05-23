//------------------------------------------------------------------------------------------
// CSphericalHarmonics3.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector3.h"

//------------------------------------------------------------------------------------------

/**
 * CSphericalHarmonics3 class.
 * 
 * CSphericalHarmonics3 is a container for spherical harmonics coefficients.
 * Simple operations can be done on them, eg. Multiply by scalar, add two spherical harmonics together.
 * Spherical harmonics are used to evaluate a spherical function.
 * 
 */
 
class CSphericalHarmonics3
{
private:
   enum { kNumCoeffs = 9 };

public:
   /// construct empty spherical harmonics, all coeffs have value of zero.
   ENGINE_API explicit CSphericalHarmonics3();
   
   /// construct directly from coeffs
   ENGINE_API explicit CSphericalHarmonics3( CVector3 const & c1, CVector3 const & c2, CVector3 const & c3, CVector3 const & c4, CVector3 const & c5, CVector3 const & c6, CVector3 const & c7, CVector3 const & c8, CVector3 const & c9 );

   /// multiply spherical harmonics by rhs
   ENGINE_API CSphericalHarmonics3 const operator * ( real32 const rhs ) const;
   /// multiply spherical harmonics by rhs inplace
   ENGINE_API CSphericalHarmonics3 & operator *= ( real32 const rhs );

   /// add rhs to current spherical harmonics
   ENGINE_API CSphericalHarmonics3 const operator + ( CSphericalHarmonics3 const & rhs ) const;
   /// add rhs to current spherical harmonics inplace
   ENGINE_API CSphericalHarmonics3 & operator += ( CSphericalHarmonics3 const & rhs );

   /// const array accessor
   CVector3 const & operator [] ( int const index ) const { BPE_ASSERT( index < kNumCoeffs, "Bad index" ); return mCoeffs[index]; }
   /// non-const array accessor
   CVector3 & operator [] ( int const index ) { BPE_ASSERT( index < kNumCoeffs, "Bad index" ); return mCoeffs[index]; }

   /// returns number of coeffs, currently always 9.
   int const GetNumCoeffs() const { return kNumCoeffs; }

private:
   bpe::reserved_vector<CVector3, kNumCoeffs> mCoeffs;
};


