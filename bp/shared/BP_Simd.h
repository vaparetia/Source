//----------------------------------------------------------------------------
// BP_Simd.h
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#pragma once

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------
// PLATFORM SPECIFIC
//----------------------------------------------------------------------------

#if BP_PS3
#define BP_USE_SIMD  1
#include "BP_Simd_PS3.h"
#endif

#if BP_360
#define BP_USE_SIMD  1
#include "BP_Simd_X360.h"
#endif

#if BP_VITA
#define BP_USE_SIMD  1
#include "BP_Simd_VTA.h"
#endif

#if BP_WIN32
#ifndef BPE_FORCEINLINE
#define BPE_FORCEINLINE __forceinline
#endif
#endif

//----------------------------------------------------------------------------
// USEFUL SIMD FUNCTIONS
//----------------------------------------------------------------------------

#if BP_USE_ALTIVEC

// Returns 1 if bounds overlap. NOTE: w components of all vectors must be equal (0 or 1)
BPE_FORCEINLINE 
const int vec_bounds_overlap( const __vec_float4 bound_min0, const __vec_float4 bound_max0,
                              const __vec_float4 bound_min1, const __vec_float4 bound_max1 )
{
   int overlap;

   // Check min0 <= max1
   overlap = vec_all_le( bound_min0, bound_max1 );

   // Check min1 <= max0
   overlap &= vec_all_le( bound_min1, bound_max0 );

   return overlap;
}

#define vec_bounds_overlap_xyzw  vec_bounds_overlap

#endif

//----------------------------------------------------------------------------
// USEFUL NEON FUNCTIONS
//----------------------------------------------------------------------------

#if BP_USE_NEON

// Macros mappings
#define vec_div(a,b)       neon_div(a,b)
#define vec_normalize(v)   neon_normalize(v)

//-----------------------------------------------------------------------------

// Returns full precision (within a few bits) of (a/b)
BPE_FORCEINLINE
const __vec_float4 neon_div( const __vec_float4 a, const __vec_float4 b )
{
   // get an initial estimate of 1/b.
   __vec_float4 estimate = vrecpeq_f32(b);

   // Newton-Raphson refinement
   //estimate = vec_mul(vrecpsq_f32(b, estimate), estimate);
   //estimate = vec_mul(vrecpsq_f32(b, estimate), estimate);

   // and finally, compute a/b = a*(1/b)
   __vec_float4 result = vec_mul(a,estimate);
   return result;
}

//-----------------------------------------------------------------------------

// Returns full precision (within a few bits) of:  (v/sqrt(dot(v,v)))
BPE_FORCEINLINE
const __vec_float4 neon_normalize( const __vec_float4 v )
{
   // Compute length squared
   __vec_float4 lenSquared = vec_dot3( v, v );

   // Compute reciprocal square root estimate of (1/sqrt(lenSquared))
   __vec_float4 estimate = vrsqrteq_f32(lenSquared);

   // Newton-Raphson refinement
   //estimate = vec_mul(vrsqrtsq_f32(lenSquared, estimate), estimate);
   //estimate = vec_mul(vrsqrtsq_f32(lenSquared, estimate), estimate);

   // Perform normalization
   __vec_float4 result = vec_mul(v,estimate);
   return result;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------
// USEFUL ALTIVEC FUNCTIONS
//----------------------------------------------------------------------------

#if BP_USE_ALTIVEC

// Macro mappings assumes zero, half, one, select_perm_ax_ay_az_bx are defined
#define vec_div(a,b)       altivec_div(a,b,zero,one)
#define vec_normalize(v)   altivec_normalize(v,zero,half,one)
#define vec_clearw(v)      vec_perm(v, zero, select_perm_ax_ay_az_bx)

//-----------------------------------------------------------------------------

// Returns full precision (within a few bits) of (a/b)
BPE_FORCEINLINE
const __vec_float4 altivec_div( const __vec_float4 a, const __vec_float4 b, const __vec_float4 zero, const __vec_float4 one )
{
   // Get the reciprocal estimate of (1/b)
   __vec_float4 estimate = vec_re( b );

   // Newton-Raphson refinement
   estimate = vec_madd( vec_nmsub( estimate, b, one ), estimate, estimate );

   // Perform div
   return vec_madd( a, estimate, zero );
}

//-----------------------------------------------------------------------------

// Returns full precision (within a few bits) of:  (v/sqrt(dot(v,v)))
BPE_FORCEINLINE
const __vec_float4 altivec_normalize( const __vec_float4 v, const __vec_float4 zero, const __vec_float4 half, const __vec_float4 one )
{
   // Compute length squared
   __vec_float4 lenSquared = vec_dot3( v, v );

   // Get the reciprocal square root estimate of (1/sqrt(lenSquared))
   __vec_float4 estimate = vec_rsqrte( lenSquared );

   // Newton-Raphson refinement
   __vec_float4 estimateSquared = vec_madd( estimate, estimate, zero );
   __vec_float4 halfEstimate = vec_madd( estimate, half, zero );
   estimate = vec_madd( vec_nmsub( lenSquared, estimateSquared, one ), halfEstimate, estimate );

   // Perform normalization
   return vec_madd( v, estimate, zero );
}

//----------------------------------------------------------------------------

#endif   //#if BP_USE_ALTIVEC


//----------------------------------------------------------------------------
// TBP_Neon_Vec3
//----------------------------------------------------------------------------

#if defined(BP_USE_SIMD) && defined(BP_USE_NEON)

typedef float32x2x3_t TBP_Neon_Vec3;

#define bp_neon_copy_vec3( dst, src )  vst3_lane_f32( dst, vld3_f32( src ), 0 )
#define bp_neon_copy_s16_3( dst, src ) vst3_lane_s16( dst, vld3_s16( src ), 0 )
#define bp_neon_load_vec3( src )       vld3_f32(src)
#define bp_neon_store_vec3( dst, v )   vst3_lane_f32(dst,v,0)

#else

//----------------------------------------------------------------------------

typedef struct _TBP_Neon_Vec3
{
   float x, y, z;
} TBP_Neon_Vec3;

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
void bp_neon_copy_vec3( float *dst, float const *src )
{
   dst[0] = src[0];
   dst[1] = src[1];
   dst[2] = src[2];
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
void bp_neon_copy_s16_3( short *dst, short const *src )
{
   dst[0] = src[0];
   dst[1] = src[1];
   dst[2] = src[2];
}   

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
TBP_Neon_Vec3 bp_neon_load_vec3( float const *src )
{
   TBP_Neon_Vec3 v = { src[0], src[1], src[2] };
   return v;
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE 
void bp_neon_store_vec3( float *dst, TBP_Neon_Vec3 v ) 
{ 
   dst[0] = v.x;
   dst[1] = v.y;
   dst[2] = v.z;
}

//----------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------

