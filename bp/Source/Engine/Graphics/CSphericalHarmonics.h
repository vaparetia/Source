//------------------------------------------------------------------------------------------
// CSphericalHarmonics.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

/**
 * CSphericalHarmonics class.
 * 
 * CSphericalHarmonics is a container for spherical harmonics coefficients.
 * Simple operations can be done on them, eg. Multiply by scalar, add two spherical harmonics together.
 * Spherical harmonics are used to evaluate a spherical function.
 * 
 */
 
class CSphericalHarmonics
{
private:
   enum { kNumCoeffs = 9 };

public:
   /// construct empty spherical harmonics, all coeffs have value of zero.
   ENGINE_API explicit CSphericalHarmonics();
   
   /// construct directly from coeffs
   ENGINE_API explicit CSphericalHarmonics( real32 const c1, real32 const c2, real32 const c3, real32 const c4, real32 const c5, real32 const c6, real32 const c7, real32 const c8, real32 const c9 );

   /// multiply spherical harmonics by rhs
   ENGINE_API CSphericalHarmonics const operator * ( real32 const rhs ) const;
   /// multiply spherical harmonics by rhs inplace
   ENGINE_API CSphericalHarmonics & operator *= ( real32 const rhs );

   /// add rhs to current spherical harmonics
   ENGINE_API CSphericalHarmonics const operator + ( CSphericalHarmonics const & rhs ) const;
   /// add rhs to current spherical harmonics inplace
   ENGINE_API CSphericalHarmonics & operator += ( CSphericalHarmonics const & rhs );

   /// const array accessor
   real32 const & operator [] ( int const index ) const { BPE_ASSERT( index < kNumCoeffs, "Bad index" ); return mCoeffs[index]; }
   /// non-const array accessor
   real32 & operator [] ( int const index ) { BPE_ASSERT( index < kNumCoeffs, "Bad index" ); return mCoeffs[index]; }

   /// returns number of coeffs, currently always 9.
   int const GetNumCoeffs() const { return kNumCoeffs; }

private:
   real32   mCoeffs[kNumCoeffs];
};

