//----------------------------------------------------------------------------
// CPerlinNoise.h
// Copyright 2006
//----------------------------------------------------------------------------
// Refactored from Aqsis renderer.
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class CVector2;
class CVector3;

//----------------------------------------------------------------------------

class ENGINE_API CPerlinNoise
{
public:
	static real32  	FGNoise1( real32 x );
	static real32     FGPNoise1( real32 x, real32 px );
	static real32  	FGNoise2( real32 x, real32 y );
	static real32  	FGPNoise2( real32 x, real32 y, real32 px, real32 py );
	static real32  	FGNoise3( const CVector3& v );
	static real32  	FGPNoise3( const CVector3& v, const CVector3& pv );
	static real32  	FGNoise4( const CVector3& v, const real32 t );
	static real32  	FGPNoise4( const CVector3& v, const real32 t, const CVector3& pv, const real32 pt );
	static CVector3   PGNoise1( real32 x );
	static CVector3   PGPNoise1( real32 x, real32 px );
	static CVector3   PGNoise2( real32 x, real32 y );
	static CVector3   PGPNoise2( real32 x, real32 y, real32 px, real32 py );
	static CVector3   PGNoise3( const CVector3& v );
	static CVector3   PGPNoise3( const CVector3& v, const CVector3& pv );
	static CVector3   PGNoise4( const CVector3& v, real32 t );
	static CVector3   PGPNoise4( const CVector3& v, real32 t, const CVector3& pv, real32 pt );

private:
	/** 1D, 2D, 3D and 4D float Perlin noise, SL "noise()"
	 */
	static real32 InternalNoise( real32 x );
	static real32 InternalNoise( real32 x, real32 y );
	static real32 InternalNoise( real32 x, real32 y, real32 z );
	static real32 InternalNoise( real32 x, real32 y, real32 z, real32 w );

	/** 1D, 2D, 3D and 4D float Perlin periodic noise, SL "pnoise()"
	 */
	static real32 InternalPNoise( real32 x, int px );
	static real32 InternalPNoise( real32 x, real32 y, int px, int py );
	static real32 InternalPNoise( real32 x, real32 y, real32 z, int px, int py, int pz );
	static real32 InternalPNoise( real32 x, real32 y, real32 z, real32 w, int px, int py, int pz, int pw );

	static unsigned char perm[];
	static real32  grad( int hash, real32 x );
	static real32  grad( int hash, real32 x, real32 y );
	static real32  grad( int hash, real32 x, real32 y , real32 z );
	static real32  grad( int hash, real32 x, real32 y, real32 z, real32 t );
};
