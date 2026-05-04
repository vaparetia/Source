//----------------------------------------------------------------------------
// bp_color.h
//----------------------------------------------------------------------------
#ifndef __BP_COLOR_H__
#define __BP_COLOR_H__

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
// COLOR FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
u_long64 BP_Color_32_to_64( const u_int col )
{
   const u_long64 r = (col >>  0) & 0xff;
   const u_long64 g = (col >>  8) & 0xff;
   const u_long64 b = (col >> 16) & 0xff;
   const u_long64 a = (col >> 24) & 0xff;

   const u_long64 rgba = (r << 0) | (g << 16) | (b << 32) | (a << 48);
   return rgba;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
u_int BP_Color_Mul( const u_int rgba0, const u_int rgba1 )
{
   const u_int r0 = (rgba0 >>  0) & 0xff;
   const u_int g0 = (rgba0 >>  8) & 0xff;
   const u_int b0 = (rgba0 >> 16) & 0xff;
   const u_int a0 = (rgba0 >> 24) & 0xff;

   const u_int r1 = (rgba1 >>  0) & 0xff;
   const u_int g1 = (rgba1 >>  8) & 0xff;
   const u_int b1 = (rgba1 >> 16) & 0xff;
   const u_int a1 = (rgba1 >> 24) & 0xff;

   const u_int r = (r0 * r1) >> 7;
   const u_int g = (g0 * g1) >> 7;
   const u_int b = (b0 * b1) >> 7;
   const u_int a = (a0 * a1) >> 7;

   const u_int rgba = (r<<0) | (g<<8) | (b<<16) | (a<<24);
   return rgba;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
u_int BP_Color_BlendRGB( const u_int col0, const u_int col1, const int alpha )
{
   u_int    res;
   int		r, g, b, inv_alpha;
   inv_alpha = 128 - alpha;
   r = ( ( ( col0 >>  0 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >>  0 ) & 0xff ) * alpha );
   g = ( ( ( col0 >>  8 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >>  8 ) & 0xff ) * alpha );
   b = ( ( ( col0 >> 16 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >> 16 ) & 0xff ) * alpha );
   r >>= 7;
   g >>= 7;
   b >>= 7;
   res = col0 & 0xff000000;
   res |= ( r ) | ( g << 8 ) | ( b << 16 );
   return ( res );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
u_int BP_Color_BlendRGB0( const u_int col0, const u_int col1, const int alpha )
{
   u_int    res;
   int		r, g, b, inv_alpha;
   inv_alpha = 128 - alpha;
   r = ( ( ( col0 >>  0 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >>  0 ) & 0xff ) * alpha );
   g = ( ( ( col0 >>  8 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >>  8 ) & 0xff ) * alpha );
   b = ( ( ( col0 >> 16 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >> 16 ) & 0xff ) * alpha );
   r >>= 7;
   g >>= 7;
   b >>= 7;
   res = ( r ) | ( g << 8 ) | ( b << 16 );
   return ( res );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
u_int BP_Color_BlendRGBA( const u_int col0, const u_int col1, const int alpha )
{
   u_int    res;
   int		r, g, b, a, inv_alpha;
   inv_alpha = 128 - alpha;
   r = ( ( ( col0 >>  0 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >>  0 ) & 0xff ) * alpha );
   g = ( ( ( col0 >>  8 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >>  8 ) & 0xff ) * alpha );
   b = ( ( ( col0 >> 16 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >> 16 ) & 0xff ) * alpha );
   a = ( ( ( col0 >> 24 ) & 0xff ) * inv_alpha ) + ( ( ( col1 >> 24 ) & 0xff ) * alpha );
   r >>= 7;
   g >>= 7;
   b >>= 7;
   a >>= 7;
   res = ( r ) | ( g << 8 ) | ( b << 16 ) | ( a << 24 );
   return ( res );
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_COLOR_H__
