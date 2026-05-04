//----------------------------------------------------------------------------
// bp_vector.h
//----------------------------------------------------------------------------
#ifndef __BP_VECTOR_H__
#define __BP_VECTOR_H__

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include "bp_math.h"


//----------------------------------------------------------------------------
// VECTOR3 FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Vec3_Check( const FVECTOR* vec )
{
   return BP_Float_Check(vec->vx) && 
      BP_Float_Check(vec->vy) && 
      BP_Float_Check(vec->vz);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Vec3_IsAnyComponentNeg( const FVECTOR* vec )
{
   return (vec->vx < 0.0f) || 
      (vec->vy < 0.0f) || 
      (vec->vz < 0.0f);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_Copy( FVECTOR* dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );

   dst->vx = src->vx;
   dst->vy = src->vy;
   dst->vz = src->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MaxFloat( FVECTOR* dst, const FVECTOR* a, const float b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Float_Check(b) );

   dst->vx = BP_Float_Max( a->vx, b );
   dst->vy = BP_Float_Max( a->vy, b );
   dst->vz = BP_Float_Max( a->vz, b );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MinFloat( FVECTOR* dst, const FVECTOR* a, const float b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Float_Check(b) );

   dst->vx = BP_Float_Min( a->vx, b );
   dst->vy = BP_Float_Min( a->vy, b );
   dst->vz = BP_Float_Min( a->vz, b );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MaxVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = BP_Float_Max( a->vx, b->vx );
   dst->vy = BP_Float_Max( a->vy, b->vy );
   dst->vz = BP_Float_Max( a->vz, b->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MinVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = BP_Float_Min( a->vx, b->vx );
   dst->vy = BP_Float_Min( a->vy, b->vy );
   dst->vz = BP_Float_Min( a->vz, b->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_NegVec( FVECTOR* dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );

   dst->vx = -src->vx;
   dst->vy = -src->vy;
   dst->vz = -src->vz;
   dst->vw = src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MulFloat( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec3_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx * f;
   dst->vy = src->vy * f;
   dst->vz = src->vz * f;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_AddFloat( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec3_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx + f;
   dst->vy = src->vy + f;
   dst->vz = src->vz + f;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_SubFloat( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec3_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx - f;
   dst->vy = src->vy - f;
   dst->vz = src->vz - f;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_AddVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = a->vx + b->vx;
   dst->vy = a->vy + b->vy;
   dst->vz = a->vz + b->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_SubVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = a->vx - b->vx;
   dst->vy = a->vy - b->vy;
   dst->vz = a->vz - b->vz;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_ShortToFloat0( FVECTOR* dst, const SVECTOR *src )
{
   dst->vx = BP_Int_ToFloat0( src->vx );
   dst->vy = BP_Int_ToFloat0( src->vy );
   dst->vz = BP_Int_ToFloat0( src->vz );

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_ShortToFloat4( FVECTOR* dst, const SVECTOR *src )
{
   dst->vx = BP_Int_ToFloat4( src->vx );
   dst->vy = BP_Int_ToFloat4( src->vy );
   dst->vz = BP_Int_ToFloat4( src->vz );

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_ShortToFloat12( FVECTOR* dst, const SVECTOR *src )
{
   dst->vx = BP_Int_ToFloat12( src->vx );
   dst->vy = BP_Int_ToFloat12( src->vy );
   dst->vz = BP_Int_ToFloat12( src->vz );

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_FloatToShort0( SVECTOR *dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );
   
   dst->vx = (short)BP_Float_ToInt0( src->vx );
   dst->vy = (short)BP_Float_ToInt0( src->vy );
   dst->vz = (short)BP_Float_ToInt0( src->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_FloatToShort4( SVECTOR *dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );

   dst->vx = (short)BP_Float_ToInt4( src->vx );
   dst->vy = (short)BP_Float_ToInt4( src->vy );
   dst->vz = (short)BP_Float_ToInt4( src->vz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_FloatToShort12( SVECTOR *dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );

   dst->vx = (short)BP_Float_ToInt12( src->vx );
   dst->vy = (short)BP_Float_ToInt12( src->vy );
   dst->vz = (short)BP_Float_ToInt12( src->vz );
}

//----------------------------------------------------------------------------

// dst = (s1 * (1-t)) + (s2 * t)
EXTERN_INLINE 
void BP_Vec3_Lerp( FVECTOR* dst, const FVECTOR* s1, const FVECTOR* s2, const float t )
{
   bp_math_assert( BP_Float_Check(t) );
   bp_math_assert( BP_Vec3_Check(s1) );
   bp_math_assert( BP_Vec3_Check(s2) );

   dst->vx = s1->vx + (t * (s2->vx - s1->vx));
   dst->vy = s1->vy + (t * (s2->vy - s1->vy));
   dst->vz = s1->vz + (t * (s2->vz - s1->vz));

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_Lerp2( FVECTOR* dst, const FVECTOR* s1, const FVECTOR* s2, const float t1, const float t2 )
{
   bp_math_assert( BP_Float_Check(t1) );
   bp_math_assert( BP_Float_Check(t2) );
   bp_math_assert( BP_Vec3_Check(s1) );
   bp_math_assert( BP_Vec3_Check(s2) );

   dst->vx = (t1 * s1->vx) + (t2 * s2->vx);
   dst->vy = (t1 * s1->vy) + (t2 * s2->vy);
   dst->vz = (t1 * s1->vz) + (t2 * s2->vz);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
float BP_Vec3_LengthSquared( const FVECTOR* src )
{
   return (src->vx * src->vx) + (src->vy * src->vy) + (src->vz * src->vz);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
float BP_Vec3_Length( const FVECTOR* src )
{
   const float length_sqr = BP_Vec3_LengthSquared(src);
   if( length_sqr > 0.000001f )
   {
      const float length = BP_Sqrt(length_sqr);
      return length;
   }
   else
   {
      return 0.0f;
   }
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
float BP_Vec3_DistanceSquared( const FVECTOR* a, const FVECTOR* b )
{
   FVECTOR delta;
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   BP_Vec3_SubVec( &delta, a, b );
   return BP_Vec3_LengthSquared( &delta );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
float BP_Vec3_Distance( const FVECTOR* a, const FVECTOR* b )
{
   FVECTOR delta;
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   BP_Vec3_SubVec( &delta, a, b );
   return BP_Vec3_Length( &delta );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec3_Normalize( FVECTOR* dst, const FVECTOR* src )
{
   const float length_sqr = BP_Vec3_LengthSquared( src );
   if( length_sqr > 0.000001f )
   {
      const float length = BP_Sqrt(length_sqr);
      const float scale  = 1.0f / length;
      dst->vx = src->vx * scale;
      dst->vy = src->vy * scale;
      dst->vz = src->vz * scale;
      dst->vw = src->vw;
   }
   else
   {
      dst->vx = 0.0f;
      dst->vy = 0.0f;
      dst->vz = 0.0f;
      dst->vw = src->vw;
   }
   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MulAddFloat( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b, const float f )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = a->vx + (b->vx * f);
   dst->vy = a->vy + (b->vy * f);
   dst->vz = a->vz + (b->vz * f);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_MulVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = a->vx * b->vx;
   dst->vy = a->vy * b->vy;
   dst->vz = a->vz * b->vz;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec3_RecipVec( FVECTOR* dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );

   dst->vx = 1.0f / src->vx;
   dst->vy = 1.0f / src->vy;
   dst->vz = 1.0f / src->vz;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Vec3_InnerProduct( const FVECTOR* a, const FVECTOR* b )
{
   float d;

   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   d = (a->vx * b->vx) + (a->vy * b->vy) + (a->vz * b->vz);
   bp_math_assert( BP_Float_Check(d) );

   return d;
}

//----------------------------------------------------------------------------

// dst = cross_product( v0, v1 )
EXTERN_INLINE
void BP_Vec3_OuterProduct( FVECTOR* dst, const FVECTOR* v0, const FVECTOR* v1 )
{
   // VU_Users_Manual.pdf, Page 117
   // vopmula.xyz ACC, vf05, vf06
   // vopmsub.xyz vf07, vf06, vf05
   // vf07x = (vf05y * vf06z) - (vf06y * vf05z);
   // vf07y = (vf05z * vf06x) - (vf06z * vf05x);
   // vf07z = (vf05x * vf06y) - (vf06x * vf05y);
   // vsub.w vf07, vf07, vf07
   FVECTOR res;
   bp_math_assert( BP_Vec3_Check(v0) );
   bp_math_assert( BP_Vec3_Check(v1) );
   res.vx = (v0->vy * v1->vz) - (v1->vy * v0->vz);
   res.vy = (v0->vz * v1->vx) - (v1->vz * v0->vx);
   res.vz = (v0->vx * v1->vy) - (v1->vx * v0->vy);
   BP_Vec3_Copy( dst, &res );
   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------
// VECTOR4 FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Vec4_Check( const FVECTOR* vec )
{
   return BP_Float_Check(vec->vx) && 
      BP_Float_Check(vec->vy) && 
      BP_Float_Check(vec->vz) && 
      BP_Float_Check(vec->vw);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_SetInvalid( FVECTOR* v )
{
   BP_Float_SetInvalid( &v->vx );
   BP_Float_SetInvalid( &v->vy );
   BP_Float_SetInvalid( &v->vz );
   BP_Float_SetInvalid( &v->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_Copy( FVECTOR* dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );

   dst->vx = src->vx;
   dst->vy = src->vy;
   dst->vz = src->vz;
   dst->vw = src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_SetZero( FVECTOR* dst )
{
   dst->vx = 0.0f;
   dst->vy = 0.0f;
   dst->vz = 0.0f;
   dst->vw = 0.0f;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_Set( FVECTOR* dst, const float x, const float y, const float z, const float w )
{
    dst->vx = x;
    dst->vy = y;
    dst->vz = z;
    dst->vw = w;

    bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MaxFloat( FVECTOR* dst, const FVECTOR* a, const float b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Float_Check(b) );

   dst->vx = BP_Float_Max( a->vx, b );
   dst->vy = BP_Float_Max( a->vy, b );
   dst->vz = BP_Float_Max( a->vz, b );
   dst->vw = BP_Float_Max( a->vw, b );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MinFloat( FVECTOR* dst, const FVECTOR* a, const float b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Float_Check(b) );

   dst->vx = BP_Float_Min( a->vx, b );
   dst->vy = BP_Float_Min( a->vy, b );
   dst->vz = BP_Float_Min( a->vz, b );
   dst->vw = BP_Float_Min( a->vw, b );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MaxVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = BP_Float_Max( a->vx, b->vx );
   dst->vy = BP_Float_Max( a->vy, b->vy );
   dst->vz = BP_Float_Max( a->vz, b->vz );
   dst->vw = BP_Float_Max( a->vw, b->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MinVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = BP_Float_Min( a->vx, b->vx );
   dst->vy = BP_Float_Min( a->vy, b->vy );
   dst->vz = BP_Float_Min( a->vz, b->vz );
   dst->vw = BP_Float_Min( a->vw, b->vw );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_FromColor( FVECTOR* dst, const u_int c )
{
   // Decode
   dst->vx = (float)((c >>  0) & 0xff);
   dst->vy = (float)((c >>  8) & 0xff);
   dst->vz = (float)((c >> 16) & 0xff);
   dst->vw = (float)((c >> 24) & 0xff);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
u_int BP_Vec4_ToColor( const FVECTOR* src )
{
   u_int c;
   FVECTOR vcol;

   // Clamp
   BP_Vec4_MaxFloat( &vcol, src,   0.0f );
   BP_Vec4_MinFloat( &vcol, &vcol, 255.0f );

   // Encode
   c = ((u_int)vcol.vx <<  0) |
       ((u_int)vcol.vy <<  8) |
       ((u_int)vcol.vz << 16) |
       ((u_int)vcol.vw << 24);
   return c;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_ShortToFloat0( FVECTOR* dst, const SVECTOR *src )
{
   dst->vx = BP_Int_ToFloat0( src->vx );
   dst->vy = BP_Int_ToFloat0( src->vy );
   dst->vz = BP_Int_ToFloat0( src->vz );
   dst->vw = BP_Int_ToFloat0( src->pad );

   bp_math_assert( BP_Vec4_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_ShortToFloat4( FVECTOR* dst, const SVECTOR *src )
{
   dst->vx = BP_Int_ToFloat4( src->vx );
   dst->vy = BP_Int_ToFloat4( src->vy );
   dst->vz = BP_Int_ToFloat4( src->vz );
   dst->vw = BP_Int_ToFloat4( src->pad );

   bp_math_assert( BP_Vec4_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_ShortToFloat12( FVECTOR* dst, const SVECTOR *src )
{
   dst->vx = BP_Int_ToFloat12( src->vx );
   dst->vy = BP_Int_ToFloat12( src->vy );
   dst->vz = BP_Int_ToFloat12( src->vz );
   dst->vw = BP_Int_ToFloat12( src->pad );

   bp_math_assert( BP_Vec4_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_FloatToShort12( SVECTOR *dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );
   dst->vx  = (short)(src->vx * 4096.0f);
   dst->vy  = (short)(src->vy * 4096.0f);
   dst->vz  = (short)(src->vz * 4096.0f);
   dst->pad = (short)(src->vw * 4096.0f);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_FloatToInt4( IVECTOR *dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );
   dst->vx = (int)(src->vx * 16.0f);
   dst->vy = (int)(src->vy * 16.0f);
   dst->vz = (int)(src->vz * 16.0f);
   dst->vw = (int)(src->vw * 16.0f);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_FloatToInt0( IVECTOR *dst, const FVECTOR* src )
{
   bp_math_assert( BP_Vec3_Check(src) );
   dst->vx = (int)src->vx;
   dst->vy = (int)src->vy;
   dst->vz = (int)src->vz;
   dst->vw = (int)src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_IntToFloat0( FVECTOR *dst, const IVECTOR* src )
{
   dst->vx = BP_Int_ToFloat0( src->vx );
   dst->vy = BP_Int_ToFloat0( src->vy );
   dst->vz = BP_Int_ToFloat0( src->vz );
   dst->vw = BP_Int_ToFloat0( src->vw );

   bp_math_assert( BP_Vec4_Check(dst) );
}

//----------------------------------------------------------------------------

// dst = (s1 * (1-t)) + (s2 * t)
EXTERN_INLINE 
void BP_Vec4_Lerp( FVECTOR* dst, const FVECTOR* s1, const FVECTOR* s2, const float t )
{
   bp_math_assert( BP_Float_Check(t) );
   bp_math_assert( BP_Vec3_Check(s1) );
   bp_math_assert( BP_Vec3_Check(s2) );

   dst->vx = s1->vx + (t * (s2->vx - s1->vx));
   dst->vy = s1->vy + (t * (s2->vy - s1->vy));
   dst->vz = s1->vz + (t * (s2->vz - s1->vz));
   dst->vw = s1->vw + (t * (s2->vw - s1->vw));

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Vec4_Lerp2( FVECTOR* dst, const FVECTOR* s1, const FVECTOR* s2, const float t1, const float t2 )
{
   bp_math_assert( BP_Float_Check(t1) );
   bp_math_assert( BP_Float_Check(t2) );
   bp_math_assert( BP_Vec3_Check(s1) );
   bp_math_assert( BP_Vec3_Check(s2) );

   dst->vx = (t1 * s1->vx) + (t2 * s2->vx);
   dst->vy = (t1 * s1->vy) + (t2 * s2->vy);
   dst->vz = (t1 * s1->vz) + (t2 * s2->vz);
   dst->vw = (t1 * s1->vw) + (t2 * s2->vw);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_AddFloatX( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec3_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx + f;
   dst->vy = src->vy;
   dst->vz = src->vz;
   dst->vw = src->vw;

   bp_math_assert( BP_Vec3_Check(src) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_AddFloatY( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec3_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx;
   dst->vy = src->vy + f;
   dst->vz = src->vz;
   dst->vw = src->vw;

   bp_math_assert( BP_Vec3_Check(src) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_AddFloatZ( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec3_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx;
   dst->vy = src->vy;
   dst->vz = src->vz + f;
   dst->vw = src->vw;

   bp_math_assert( BP_Vec3_Check(src) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_AddFloatW( FVECTOR* dst, const FVECTOR* src, const float f )
{
   bp_math_assert( BP_Vec4_Check(src) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = src->vx;
   dst->vy = src->vy;
   dst->vz = src->vz;
   dst->vw = src->vw + f;

   bp_math_assert( BP_Vec4_Check(src) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MulVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = a->vx * b->vx;
   dst->vy = a->vy * b->vy;
   dst->vz = a->vz * b->vz;
   dst->vw = a->vw * b->vw;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MulFloat( FVECTOR* dst, const FVECTOR* src, const float f )
{
    bp_math_assert( BP_Vec3_Check(src) );
    bp_math_assert( BP_Float_Check(f) );

    dst->vx = src->vx * f;
    dst->vy = src->vy * f;
    dst->vz = src->vz * f;
    dst->vw = src->vw * f;

    bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MulAddFloat( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b, const float f )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = a->vx + (b->vx * f);
   dst->vy = a->vy + (b->vy * f);
   dst->vz = a->vz + (b->vz * f);
   dst->vw = a->vw + (b->vw * f);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MulAddVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b, const FVECTOR* c )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   bp_math_assert( BP_Vec3_Check(c) );

   dst->vx = a->vx + (b->vx * c->vx);
   dst->vy = a->vy + (b->vy * c->vy);
   dst->vz = a->vz + (b->vz * c->vz);
   dst->vw = a->vw + (b->vw * c->vw);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_AddFloat( FVECTOR* dst, const FVECTOR* a, const float b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Float_Check(b) );

   dst->vx = a->vx + b;
   dst->vy = a->vy + b;
   dst->vz = a->vz + b;
   dst->vw = a->vw + b;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_SubFloat( FVECTOR* dst, const FVECTOR* a, const float b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Float_Check(b) );

   dst->vx = a->vx - b;
   dst->vy = a->vy - b;
   dst->vz = a->vz - b;
   dst->vw = a->vw - b;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_AddVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = a->vx + b->vx;
   dst->vy = a->vy + b->vy;
   dst->vz = a->vz + b->vz;
   dst->vw = a->vw + b->vw;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MulSubFloat( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b, const float f )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   bp_math_assert( BP_Float_Check(f) );

   dst->vx = a->vx - (b->vx * f);
   dst->vy = a->vy - (b->vy * f);
   dst->vz = a->vz - (b->vz * f);
   dst->vw = a->vw - (b->vw * f);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_MulSubVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b, const FVECTOR* c )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );
   bp_math_assert( BP_Vec3_Check(c) );

   dst->vx = a->vx - (b->vx * c->vx);
   dst->vy = a->vy - (b->vy * c->vy);
   dst->vz = a->vz - (b->vz * c->vz);
   dst->vw = a->vw - (b->vw * c->vw);

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Vec4_SubVec( FVECTOR* dst, const FVECTOR* a, const FVECTOR* b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

   dst->vx = a->vx - b->vx;
   dst->vy = a->vy - b->vy;
   dst->vz = a->vz - b->vz;
   dst->vw = a->vw - b->vw;

   bp_math_assert( BP_Vec3_Check(dst) );
}

//----------------------------------------------------------------------------

// dst.xyz = cross_product( v0, v1 )
// dst.w = 0
EXTERN_INLINE
void BP_Vec4_OuterProduct( FVECTOR* dst, const FVECTOR* v0, const FVECTOR* v1 )
{
   BP_Vec3_OuterProduct( dst, v0, v1 );
   dst->vw = 0.0f;
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 2

//----------------------------------------------------------------------------

EXTERN_INLINE 
void  sceVu0SubVector( sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
   BP_Vec4_SubVec( (FVECTOR*)v0, (FVECTOR*)v1, (FVECTOR*)v2 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void  sceVu0AddVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
   BP_Vec4_AddVec( (FVECTOR*)v0, (FVECTOR*)v1, (FVECTOR*)v2 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float sceVu0InnerProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
{
   return BP_Vec3_InnerProduct( (const FVECTOR*)v0, (const FVECTOR*)v1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
   BP_Vec4_OuterProduct( (FVECTOR*)v0, (const FVECTOR*)v1, (const FVECTOR*)v2 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0ScaleVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float s)
{
   BP_Vec4_MulFloat( (FVECTOR*)v0, (const FVECTOR*)v1, s );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0CopyVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
{
   BP_Vec4_Copy( (FVECTOR*)v0, (const FVECTOR*)v1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0CopyVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
{
   BP_Vec3_Copy( (FVECTOR*)v0, (const FVECTOR*)v1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0ScaleVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float s)
{
   BP_Vec3_MulFloat( (FVECTOR*)v0, (const FVECTOR*)v1, s );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0InterVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2, float r)
{
   BP_Vec4_Lerp2( (FVECTOR*)v0, (const FVECTOR*)v1, (const FVECTOR*)v2, r, 1.0f - r );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0DivVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float q)
{
   const float s = BP_SafeDivideFEx( 1.0f, q, 0.0f );
   BP_Vec4_MulFloat( (FVECTOR*)v0, (const FVECTOR*)v1, s );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void sceVu0MulVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2)
{
   BP_Vec4_MulVec( (FVECTOR*)v0, (const FVECTOR*)v1, (const FVECTOR*)v2 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0FTOI0Vector(sceVu0IVECTOR v0, sceVu0FVECTOR v1)
{
   BP_Vec4_FloatToInt0( (IVECTOR*)v0, (const FVECTOR*)v1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0ITOF0Vector(sceVu0FVECTOR v0, sceVu0IVECTOR v1)
{
   BP_Vec4_IntToFloat0( (FVECTOR*)v0, (const IVECTOR*)v1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void sceVu0ClampVector(sceVu0FVECTOR _v0, sceVu0FVECTOR _v1, float _min, float _max)
{
   BP_Vec4_MaxFloat( (FVECTOR*)_v0, (const FVECTOR*)_v1, _min );
   BP_Vec4_MinFloat( (FVECTOR*)_v0, (const FVECTOR*)_v0, _max );
}

//----------------------------------------------------------------------------

#endif //MGS_VERSION == 2

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_VECTOR_H__
