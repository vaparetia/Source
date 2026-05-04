//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------

#include "stdafx.h"

// - This is not currently used, but it might get used if we need to optimize
// this functionality in MGS2.
#if 0 

#if defined( _MSC_VER ) && !defined( WIN32 )
#include <xtl.h>
#include <xnamath.h>
#endif

// include altivec intrinsics.
#include "bp_altivec.h"
#include "bp_boundvu.h"
#include "bp_vu.h"

#define USE_INTEGER_FLAGS           0

#define USE_NEW_VERSION             1

#define USE_OLD_VERSION             0

#define DEBUG_OPT_VERSION           0

void MA_MulVector( FVECTOR *r, FVECTOR *a, FVECTOR *b )
{
   bp_math_assert( BP_Vec3_Check(a) );
   bp_math_assert( BP_Vec3_Check(b) );

#if 0 //BP_ASM def PSX2 
        asm volatile ( "
                lqc2           vf4,0(%0)
                lqc2           vf5,0(%1)
                vmul.wxyz  vf4,vf4,vf5
                sqc2           vf4,0(%2)
                " : : "r"(a),"r"(b),"r"(r) : "memory" ) ;
#else
        r->vx = a->vx * b->vx ;
        r->vy = a->vy * b->vy ;
        r->vz = a->vz * b->vz ;
        r->vw = a->vw * b->vw ;
#endif
}

//#define NO_INTRINSICS

#if defined( _MSC_VER ) && !defined( WIN32 ) && !defined( NO_INTRINSICS )
#pragma warning( disable : 4700 )
#define ASM_360
#elif !defined( _MSC_VER ) && defined( _PS3 ) && !defined( NO_INTRINSICS )
#define ASM_PS3
#endif

#if !defined( ASM_PS3 ) && !defined( ASM_360 )
static inline float FSel(float condition, float a, float b)
{
   return condition > 0.0f ? a : b;
}
#endif

#if !USE_INTEGER_FLAGS
#define GT( x, y, result )          FSel( x - y, result, 0.0f )
#define LT( x, y, result )          FSel( y - x, result, 0.0f )
#else
#define GT( x, y, temp )            ( temp = ( y ) - ( x ),                \
                                      ( *( unsigned int* )&temp >> 31 ) )

#define LT( x, y, temp )            ( temp = ( x ) - ( y ),                \
                                      ( *( unsigned int* )&temp >> 31 ) )
#endif

static void BP_boundvu_BoundCheckRef( void );

//BP - special matrix function to multiply a vector with a matrix.  Applies
// rotation, scale, and projection.
#if defined( ASM_PS3 )
static void BP_MatrixMul( FVECTOR* result, FMATRIX* mat, FVECTOR* corner )
{
   vector float row0;
   vector float row1;
   vector float row2;
   vector float row3;
   vector float c;
   vector float r;
   vector float zero;

   ASSERT( ( ( size_t )result & 0xF ) == 0 );
   ASSERT( ( ( size_t )mat & 0xF ) == 0 );
   ASSERT( ( ( size_t )corner & 0xF ) == 0 );

   // calculate zero.
   zero = vec_xor( zero, zero );

   // load the corner vector.
   c = vec_ld( 0, &corner->vx );

   // load each row of the provided matrix.
   row0 = vec_ld( 0, mat->m[ 0 ] );
   row1 = vec_ld( 0, mat->m[ 1 ] );
   row2 = vec_ld( 0, mat->m[ 2 ] );
   row3 = vec_ld( 0, mat->m[ 3 ] );

   // grab the X component of the corner vector and multiply.
   r = vec_madd( row0, vec_splat( c, 0 ), zero );
   r = vec_madd( row1, vec_splat( c, 1 ), r );
   r = vec_madd( row2, vec_splat( c, 2 ), r );
   r = vec_madd( row3, vec_splat( c, 3 ), r );

   // now output the result.
   vec_st( r, 0, &result->vx );
}
#elif defined( ASM_360 )

/* Not used on the 360.
static inline __vector4 BP_MatrixMul( FMATRIX* mat, __vector4 corner )
{
   // assembly version of the matrix multiply.
   __vector4 ret;
   __vector4 row0;
   __vector4 row1;
   __vector4 row2;
   __vector4 row3;

   // load the rows.
   row0 = __lvx( mat->m[ 0 ], 0 );
   row1 = __lvx( mat->m[ 1 ], 0 );
   row2 = __lvx( mat->m[ 2 ], 0 );
   row3 = __lvx( mat->m[ 3 ], 0 );

   // multiply against each row.
   ret = __vmulfp( row0, __vspltw( corner, 0 ) );
   ret = __vmaddfp( row1, __vspltw( corner, 1 ), ret );
   ret = __vmaddfp( row2, __vspltw( corner, 2 ), ret );
   ret = __vmaddfp( row3, __vspltw( corner, 3 ), ret );

   // return the result.
   return ret;
}
*/

#else
static void BP_MatrixMul( FVECTOR* result, FMATRIX* mat, FVECTOR* corner )
{
   FVECTOR temp;

   temp.vx = mat->m[ 0 ][ 0 ] * corner->vx;
   temp.vy = mat->m[ 0 ][ 1 ] * corner->vx;
   temp.vz = mat->m[ 0 ][ 2 ] * corner->vx;
   temp.vw = mat->m[ 0 ][ 3 ] * corner->vx;

   temp.vx += mat->m[ 1 ][ 0 ] * corner->vy;
   temp.vy += mat->m[ 1 ][ 1 ] * corner->vy;
   temp.vz += mat->m[ 1 ][ 2 ] * corner->vy;
   temp.vw += mat->m[ 1 ][ 3 ] * corner->vy;

   temp.vx += mat->m[ 2 ][ 0 ] * corner->vz;
   temp.vy += mat->m[ 2 ][ 1 ] * corner->vz;
   temp.vz += mat->m[ 2 ][ 2 ] * corner->vz;
   temp.vw += mat->m[ 2 ][ 3 ] * corner->vz;

   temp.vx += mat->m[ 3 ][ 0 ] * corner->vw;
   temp.vy += mat->m[ 3 ][ 1 ] * corner->vw;
   temp.vz += mat->m[ 3 ][ 2 ] * corner->vw;
   temp.vw += mat->m[ 3 ][ 3 ] * corner->vw;

   *result = temp;
}
#endif

//-----------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------

//; Check the bounding
//; programmatic
//; vf01:   Bounding vertex 1
//; vf02:   Bounding vertex 2
//; vf04-07:Perspective transformation matrix
//; vf28:   Check the coordinates for the primitive scale
//; vf29:   To double check the coordinates of the offset primitives
//; vf31w:  Median Z
#if USE_NEW_VERSION
#if defined( ASM_360 ) || defined( ASM_PS3 )

#define MATRIX_MUL( ret, row0, row1, row2, row3, vec, tmp )    \
      tmp = vec_mul( row0, vec_splat( vec, 0 ) );              \
      tmp = vec_madd( row1, vec_splat( vec, 1 ), tmp );        \
      tmp = vec_madd( row2, vec_splat( vec, 2 ), tmp );        \
      ret = vec_madd( row3, vec_splat( vec, 3 ), tmp );

static ALIGN16_PRE const __vec_float4 kBits[ 4 ] ALIGN16_POST = {
   { 64.0f,  256.0f, 1024.0f,    0.0f, }, { 128.0f,  512.0f, 2048.0f,    0.0f, },
   {  1.0f,    4.0f,   16.0f,    0.0f, }, {   2.0f,    8.0f,   32.0f,    0.0f, },
};

// table that contains corner selection masks.
static ALIGN16_PRE unsigned int cornerMasks[ 32 ] ALIGN16_POST = {
   0x00000000, 0x00000000, 0x00000000, 0x00000000,
   0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
   0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,
   0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
   0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
   0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
   0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
};

static ALIGN16_PRE const unsigned int selectW[ 4 ] ALIGN16_POST = {
   0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
};

static ALIGN16_PRE float zeroOne[ 4 ] ALIGN16_POST = {
   0.0f, 0.0f, 0.0f, 1.0f,
};

#define CALC_CLIP_FLAGS( corner, cornerSB )                                   \
      /* calculate +W/-W. */                                                  \
      pw = vec_splat( corner, 3 );                                            \
      pw = vec_cast( __vec_float4, vec_and( vec_cast( __vec_uint4, pw ), abs ) );   \
      nw = vec_sub( zero, pw );                                               \
                                                                              \
      /* perform the clipping judgement against +W. */                        \
      mask = vec_cast( __vec_uint4, vec_cmpgt( corner, pw ) );                \
      clipFlags = vec_and( vec_cast( __vec_float4, mask ), pwBits0 );         \
                                                                              \
      /* perform the clipping judgement against -W. */                        \
      mask = vec_cast( __vec_uint4, vec_cmpgt( nw, corner ) );                \
                                                                              \
      /* finish computing the clipping judgement. */                          \
      temp = vec_and( vec_cast( __vec_float4, mask ), nwBits0 );              \
      clipFlags = vec_add( clipFlags, temp );                                 \
                                                                              \
      /* calculate +W/-W. */                                                  \
      pw = vec_splat( cornerSB, 3 );                                          \
      pw = vec_and( pw, vec_cast( __vec_float4, abs ) );                      \
      nw = vec_sub( zero, pw );                                               \
                                                                              \
      /* perform the clipping judgement against +W. */                        \
      mask = vec_cast( __vec_uint4, vec_cmpgt( cornerSB, pw ) );              \
      temp = vec_and( vec_cast( __vec_float4, mask ), pwBits1 );              \
      clipFlags = vec_add( clipFlags, temp );                                 \
                                                                              \
      /* perform the clipping judgement against -W. */                        \
      mask = vec_cast( __vec_uint4, vec_cmpgt( nw, cornerSB ) );              \
      temp = vec_and( vec_cast( __vec_float4, mask ), nwBits1 );              \
      clipFlags = vec_add( clipFlags, temp );                                 \
                                                                              \
      /* convert the current flags to integer values. */                      \
      temp = vec_cast( __vec_float4, vec_cts( clipFlags, 0 ) );               \
                                                                              \
      /* track the current AND and OR values. */                              \
      andFlags = vec_and( andFlags, vec_cast( __vec_uint4, temp ) );          \
      orFlags = vec_or( orFlags, vec_cast( __vec_uint4, temp ) );

//=============================================================================
// XBox 360 version
//=============================================================================
#ifdef _MSC_VER
#pragma runtime_checks( "u", off )
#endif
void BP_boundvu_BoundCheck( void )
{
   //BP - converted assembly to basic C implementation.
   // generate 8 points of an AABB by merging components of the two extrema.
   unsigned int i;
   register __vec_float4 corner0;
   register __vec_float4 corner1;
   register __vec_float4 corner2;
   register __vec_float4 corner3;
   register __vec_float4 corner4;
   register __vec_float4 corner5;
   register __vec_float4 corner6;
   register __vec_float4 corner7;
   register __vec_float4 cornerSB0;
   register __vec_float4 cornerSB1;
   register __vec_float4 cornerSB2;
   register __vec_float4 cornerSB3;
   register __vec_float4 cornerSB4;
   register __vec_float4 cornerSB5;
   register __vec_float4 cornerSB6;
   register __vec_float4 cornerSB7;
   register __vec_float4 temp;
   register __vec_float4 row0;
   register __vec_float4 row1;
   register __vec_float4 row2;
   register __vec_float4 row3;
   register __vec_uint4 mask;

   // we're using the multiply instruction, so generate a zero term if
   // necessary.
   vec_using_mul();

   // load the corner array.
   {
      // load the AABB extents.
      corner0 = vec_ld( 0, ( float* )vf01 );
      corner7 = vec_ld( 0, ( float* )vf02 );

      // make sure the W components of the corner vectors are valid.
      temp = vec_ld( 0, zeroOne );
      mask = vec_cast( __vec_uint4, vec_ld( 0, selectW ) );
      corner0 = vec_sel( corner0, temp, vec_cast( __vec_uint4, mask ) );
      corner7 = vec_sel( corner7, temp, vec_cast( __vec_uint4, mask ) );

      // calculate the second vector.
      mask = vec_ld( 16, cornerMasks );
      corner1 = vec_sel( corner0, corner7, mask );
      
      // calculate the third vector.
      mask = vec_ld( 32, cornerMasks );
      corner2 = vec_sel( corner0, corner7, mask );

      // calculate the fourth vector.
      mask = vec_ld( 48, cornerMasks );
      corner3 = vec_sel( corner0, corner7, mask );

      // calculate the fifth vector.
      mask = vec_ld( 64, cornerMasks );
      corner4 = vec_sel( corner0, corner7, mask );

      // calculate the sixth vector.
      mask = vec_ld( 80, cornerMasks );
      corner5 = vec_sel( corner0, corner7, mask );

      // calculate the seventh vector.
      mask = vec_ld( 96, cornerMasks );
      corner6 = vec_sel( corner0, corner7, mask );
   }

   // load in our transform.
   row0 = vec_ld( 0, ( float* )vf04 );
   row1 = vec_ld( 0, ( float* )vf05 );
   row2 = vec_ld( 0, ( float* )vf06 );
   row3 = vec_ld( 0, ( float* )vf07 );

   // transform the vectors into post-perspective space.
   MATRIX_MUL( corner0, row0, row1, row2, row3, corner0, temp );
   MATRIX_MUL( corner1, row0, row1, row2, row3, corner1, temp );
   MATRIX_MUL( corner2, row0, row1, row2, row3, corner2, temp );
   MATRIX_MUL( corner3, row0, row1, row2, row3, corner3, temp );
   MATRIX_MUL( corner4, row0, row1, row2, row3, corner4, temp );
   MATRIX_MUL( corner5, row0, row1, row2, row3, corner5, temp );
   MATRIX_MUL( corner6, row0, row1, row2, row3, corner6, temp );
   MATRIX_MUL( corner7, row0, row1, row2, row3, corner7, temp );

   // apply vector scales.
   {
      // load the scale and bias.
      __vec_float4 scale = vec_ld( 0, ( float* )vf28 );
      __vec_float4 bias = vec_ld( 0, ( float* )vf29 );
      __vec_float4 corner;

      // scale and bias each corner.
      cornerSB0 = vec_madd( scale, corner0, bias );
      cornerSB1 = vec_madd( scale, corner1, bias );
      cornerSB2 = vec_madd( scale, corner2, bias );
      cornerSB3 = vec_madd( scale, corner3, bias );
      cornerSB4 = vec_madd( scale, corner4, bias );
      cornerSB5 = vec_madd( scale, corner5, bias );
      cornerSB6 = vec_madd( scale, corner6, bias );
      cornerSB7 = vec_madd( scale, corner7, bias );
   }

   // perform clipping judgements.
   {
      // constants for bit-flag values.
      register __vec_uint4 abs;
      register __vec_float4 zero;
      register __vec_uint4 andFlags;
      register __vec_uint4 orFlags;
      register __vec_float4 temp;
      register __vec_uint4 mask;
      register __vec_float4 clipFlags;
      register __vec_float4 pwBits0;
      register __vec_float4 nwBits0;
      register __vec_float4 pwBits1;
      register __vec_float4 nwBits1;
      register __vec_float4 pw;
      register __vec_float4 nw;

      // calculate zero.
      zero = vec_zero( zero );
      orFlags = vec_cast( __vec_uint4, zero );
      andFlags = vec_cast( __vec_uint4, vec_cmpeq( zero, zero ) );

      // build the absolute value mask.
      abs = andFlags;
      temp = vec_cast( __vec_float4, vec_splat_imm( 1 ) );
      temp = vec_cast( __vec_float4, vec_splatb( temp, 3 ) );
      abs = vec_sr( abs, vec_cast( __vec_uint4, temp ) );

      // load bit flags into the proper variable.
      pwBits0 = kBits[ 0 ];
      nwBits0 = kBits[ 1 ];
      pwBits1 = kBits[ 2 ];
      nwBits1 = kBits[ 3 ];

      // calculate clipping flags.
      CALC_CLIP_FLAGS( corner0, cornerSB0 );
      CALC_CLIP_FLAGS( corner1, cornerSB1 );
      CALC_CLIP_FLAGS( corner2, cornerSB2 );
      CALC_CLIP_FLAGS( corner3, cornerSB3 );
      CALC_CLIP_FLAGS( corner4, cornerSB4 );
      CALC_CLIP_FLAGS( corner5, cornerSB5 );
      CALC_CLIP_FLAGS( corner6, cornerSB6 );
      CALC_CLIP_FLAGS( corner7, cornerSB7 );

      // perform the horizontal sum for the 'AND' flags.
      temp = vec_cast( __vec_float4, andFlags );
      andFlags = vec_or( andFlags, vec_cast( __vec_uint4, vec_splat( temp, 1 ) ) );
      andFlags = vec_or( andFlags, vec_cast( __vec_uint4, vec_splat( temp, 2 ) ) );

      // perform the horizontal sum for the 'OR' flags.
      temp = vec_cast( __vec_float4, orFlags );
      orFlags = vec_or( orFlags, vec_cast( __vec_uint4, vec_splat( temp, 1 ) ) );
      orFlags = vec_or( orFlags, vec_cast( __vec_uint4, vec_splat( temp, 2 ) ) );

      // splat the and-flags and the or-flags across their respective
      // registers.
      andFlags = vec_cast( __vec_uint4, vec_splath( vec_cast( __vec_ushort8, andFlags ), 1 ) );
      orFlags = vec_cast( __vec_uint4, vec_splath( vec_cast( __vec_ushort8, orFlags ), 1 ) );

      // return each value.
      vec_steh( andFlags, 0, vi02 );
      vec_steh( orFlags, 0, vi03 );

#if DEBUG_OPT_VERSION
      {
         int orFlagsNew = *vi03;
         int andFlagsNew = *vi02;
         int orFlagsRef;
         int andFlagsRef;

         // check our results.
         BP_boundvu_BoundCheckRef();
         orFlagsRef = *vi03;
         andFlagsRef = *vi02;
         ASSERT( orFlagsNew == orFlagsRef );
         ASSERT( andFlagsNew == andFlagsRef );
      }
#endif
   }
}

#ifdef _MSC_VER
#pragma runtime_checks( "u", restore )
#endif

#else

//=============================================================================
// PC version
//=============================================================================

unsigned int BP_boundvu_BoundCheck( FMATRIX* matrix, FVECTOR* aabbMin,
                                    FVECTOR* aabbMax, FVECTOR* scale )
{
   //BP - converted assembly to basic C implementation.
   // generate 8 points of an AABB by merging components of the two extrema.

   //
   unsigned int i;
   ALIGN16_PRE FVECTOR corners[ 8 ] ALIGN16_POST;
   ALIGN16_PRE FVECTOR cornersSB[ 8 ] ALIGN16_POST;
   ALIGN16_PRE float clipFlags[ 8 ] ALIGN16_POST;
   ALIGN16_PRE int intClipFlags[ 8 ] ALIGN16_POST;
   unsigned int andFlags = 0x00000FFF;

   // compose extrema vectors...
   corners[ 0 ] = *aabbMin;
   corners[ 7 ] = *aabbMax;
   corners[ 0 ].vw = 1.0f;
   corners[ 7 ].vw = 1.0f;
   for ( i = 1; i < 7; ++i )
   {
      corners[ i ].vx = ( i & 1 ) ? corners[ 7 ].vx : corners[ 0 ].vx;
      corners[ i ].vy = ( i & 2 ) ? corners[ 7 ].vy : corners[ 0 ].vy;
      corners[ i ].vz = ( i & 4 ) ? corners[ 7 ].vz : corners[ 0 ].vz;
      corners[ i ].vw = 1.0f;
   }

   // transform the vectors into post-perspective space.
   BP_MatrixMul( corners + 0, matrix, corners + 0 );
   BP_MatrixMul( corners + 1, matrix, corners + 1 );
   BP_MatrixMul( corners + 2, matrix, corners + 2 );
   BP_MatrixMul( corners + 3, matrix, corners + 3 );
   BP_MatrixMul( corners + 4, matrix, corners + 4 );
   BP_MatrixMul( corners + 5, matrix, corners + 5 );
   BP_MatrixMul( corners + 6, matrix, corners + 6 );
   BP_MatrixMul( corners + 7, matrix, corners + 7 );

   // scale the vectors as needed.
   MA_MulVector( cornersSB + 0, corners + 0, scale );
   MA_MulVector( cornersSB + 1, corners + 1, scale );
   MA_MulVector( cornersSB + 2, corners + 2, scale );
   MA_MulVector( cornersSB + 3, corners + 3, scale );
   MA_MulVector( cornersSB + 4, corners + 4, scale );
   MA_MulVector( cornersSB + 5, corners + 5, scale );
   MA_MulVector( cornersSB + 6, corners + 6, scale );
   MA_MulVector( cornersSB + 7, corners + 7, scale );

   for ( i = 0; i < 8; ++i )
   {
      // get the current w component.
      float pw = fabsf( corners[ i ].vw );
      float nw = -pw;

      // perform the clipping judgement on the corner points.
      clipFlags[ i ]  = GT( corners[ i ].vx, pw,   64.0f );
      clipFlags[ i ] += GT( corners[ i ].vy, pw,  256.0f );
      clipFlags[ i ] += GT( corners[ i ].vz, pw, 1024.0f );
      clipFlags[ i ] += LT( corners[ i ].vx, nw,  128.0f );
      clipFlags[ i ] += LT( corners[ i ].vy, nw,  512.0f );
      clipFlags[ i ] += LT( corners[ i ].vz, nw, 2048.0f );

      // get the current w component.
      pw = fabsf( cornersSB[ i ].vw );
      nw = -pw;

      // perform the clipping judgement on the scaled/biased points.
      clipFlags[ i ] += GT( cornersSB[ i ].vx, pw,  1.0f );
      clipFlags[ i ] += GT( cornersSB[ i ].vy, pw,  4.0f );
      clipFlags[ i ] += GT( cornersSB[ i ].vz, pw, 16.0f );
      clipFlags[ i ] += LT( cornersSB[ i ].vx, nw,  2.0f );
      clipFlags[ i ] += LT( cornersSB[ i ].vy, nw,  8.0f );
      clipFlags[ i ] += LT( cornersSB[ i ].vz, nw, 32.0f );
   }

   // convert the clipping flags to integers.
   for ( i = 0; i < 8; ++i )
      intClipFlags[ i ] = ( int )clipFlags[ i ];

   // generate the final visibility flags.
   for ( i = 0; i < 8; ++i )
      andFlags &= intClipFlags[ i ];

   // return the flags.
   return andFlags;

#if DEBUG_OPT_VERSION
   {
      int orFlagsNew = *vi03;
      int andFlagsNew = *vi02;
      int orFlagsRef;
      int andFlagsRef;

      // check our results.
      *vi03 = 0x00000000;
      BP_boundvu_BoundCheckRef();
      orFlagsRef = *vi03;
      andFlagsRef = *vi02;
      ASSERT( orFlagsNew == orFlagsRef );
      ASSERT( andFlagsNew == andFlagsRef );
   }
#endif
}
#endif

#else

//=============================================================================
// Microcode converted version.
//=============================================================================

void BP_boundvu_BoundCheck( void )
{
   BP_boundvu_BoundCheckRef();
}

#endif

#if DEBUG_OPT_VERSION
void BP_boundvu_BoundCheckRef( void )
{
   // generate 8 points of an AABB by merging components of the two extrema.

	//; Synthesized consisting of eight vertices bounding
	vu_nop();                                    vu_move_xyz    ( vf08,vf01 );          //BP_ASM_BOUND -  nop                              move.xyz    vf08,vf01			; 	
	vu_add_xyz			( vf15,vf00,vf02 );        vu_move_yz		( vf09,vf01 );          //BP_ASM_BOUND -  add.xyz			vf15,vf00,vf02	   move.yz		vf09,vf01			; 	
	vu_add_x			   ( vf09,vf00,vf02 );	      vu_move_xz		( vf10,vf01 );          //BP_ASM_BOUND -  add.x			   vf09,vf00,vf02	   move.xz		vf10,vf01			; 	
	vu_add_y			   ( vf10,vf00,vf02 );	      vu_move_z		( vf11,vf01 );          //BP_ASM_BOUND -  add.y			   vf10,vf00,vf02	   move.z		vf11,vf01			; 	
	vu_add_xy			( vf11,vf00,vf02 );	      vu_move_xy		( vf12,vf01 );          //BP_ASM_BOUND -  add.xy			vf11,vf00,vf02	   move.xy		vf12,vf01			; 	
	vu_add_z			   ( vf12,vf00,vf02 );	      vu_move_y		( vf13,vf01 );          //BP_ASM_BOUND -  add.z			   vf12,vf00,vf02	   move.y		vf13,vf01			; 	
	vu_add_xz			( vf13,vf00,vf02 );	      vu_move_x		( vf14,vf01 );          //BP_ASM_BOUND -  add.xz			vf13,vf00,vf02	   move.x		vf14,vf01			; 	
	vu_add_yz			( vf14,vf00,vf02 );	      vu_loi			( 0.5f );  	            //BP_ASM_BOUND -  add.yz			vf14,vf00,vf02	   loi			0.5					; 
	vu_mulai_xyz		( ACC ,vf01,I );		      vu_nop();						            //BP_ASM_BOUND -  mulai.xyz		ACC ,vf01,I		   nop								   ; 
	vu_maddi_xyz		( vf31,vf02,I );		      vu_nop();						            //BP_ASM_BOUND -  maddi.xyz		vf31,vf02,I		   nop								   ; 
	
	//; To convert the top four vertices perspective

	vu_mulax     		( ACC ,vf04,vf08 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf08	   nop								   ; The first conversion of vertex coordinates
	vu_madday     		( ACC ,vf05,vf08 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf08	   nop								   ; The first conversion of vertex coordinates
	vu_maddaz     		( ACC ,vf06,vf08 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf08	   nop								   ; The first conversion of vertex coordinates
	vu_maddw     		( vf08,vf07,vf00 );        vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf8 ,vf07,vf00	   nop								   ; The first conversion of vertex coordinates

	vu_mulax     		( ACC ,vf04,vf09 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf09	   nop								   ; The second transformation of vertex coordinates
	vu_madday     		( ACC ,vf05,vf09 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf09	   nop								   ; The second transformation of vertex coordinates
	vu_maddaz     		( ACC ,vf06,vf09 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf09	   nop								   ; The second transformation of vertex coordinates
	vu_maddw     		( vf09,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf09,vf07,vf00	   nop								   ; The second transformation of vertex coordinates

   vu_mula     		( ACC ,vf08,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf08,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf16,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf16,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_mulax     		( ACC ,vf04,vf10 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf10	   nop								   ; The third vertex coordinate transformation
	vu_madday     		( ACC ,vf05,vf10 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf10	   nop								   ; The third vertex coordinate transformation
	vu_maddaz     		( ACC ,vf06,vf10 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf10	   nop								   ; The third vertex coordinate transformation
	vu_maddw     		( vf10,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf10,vf07,vf00	   nop								   ; The third vertex coordinate transformation

   vu_mula     		( ACC ,vf09,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf09,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf17,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf17,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_mulax     		( ACC ,vf04,vf11 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf11	   nop								   ; The fourth vertex coordinate transformation
	vu_madday     		( ACC ,vf05,vf11 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf11	   nop								   ; The fourth vertex coordinate transformation
	vu_maddaz     		( ACC ,vf06,vf11 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf11	   nop								   ; The fourth vertex coordinate transformation
	vu_maddw     		( vf11,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf11,vf07,vf00	   nop								   ; The fourth vertex coordinate transformation

   vu_mula     		( ACC ,vf10,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf10,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf18,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf18,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_mulax     		( ACC ,vf04,vf12 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf12	   nop								   ; Coordinate transformation of vertex 5
	vu_madday     		( ACC ,vf05,vf12 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf12	   nop								   ; Coordinate transformation of vertex 5
	vu_maddaz     		( ACC, vf06,vf12 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC, vf06,vf12	   nop								   ; Coordinate transformation of vertex 5
	vu_maddw     		( vf12,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf12,vf07,vf00	   nop								   ; Coordinate transformation of vertex 5

   vu_mula     		( ACC ,vf11,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf11,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf19,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf19,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_mulax     		( ACC ,vf04,vf13 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf13	   nop								   ; 6 vertex coordinate transformation
	vu_madday     		( ACC ,vf05,vf13 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf13	   nop								   ; 6 vertex coordinate transformation
	vu_maddaz     		( ACC ,vf06,vf13 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf13	   nop								   ; 6 vertex coordinate transformation
	vu_maddw     		( vf13,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf13,vf07,vf00	   nop								   ; 6 vertex coordinate transformation

   vu_mula     		( ACC ,vf12,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf12,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf20,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf20,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_mulax     		( ACC ,vf04,vf14 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf14	   nop								   ; 7 vertex coordinate transformation
	vu_madday     		( ACC ,vf05,vf14 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf14	   nop								   ; 7 vertex coordinate transformation
	vu_maddaz     		( ACC ,vf06,vf14 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf14	   nop								   ; 7 vertex coordinate transformation
	vu_maddw     		( vf14,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf14,vf07,vf00	   nop								   ; 7 vertex coordinate transformation

   vu_mula     		( ACC ,vf13,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf13,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf21,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf21,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_mulax     		( ACC ,vf04,vf15 );	      vu_nop();								      //BP_ASM_BOUND -  mulax.xyzw		ACC ,vf04,vf15	   nop								   ; 8 vertex coordinate transformation
	vu_madday     		( ACC ,vf05,vf15 );	      vu_nop();								      //BP_ASM_BOUND -  madday.xyzw		ACC ,vf05,vf15	   nop								   ; 8 vertex coordinate transformation
	vu_maddaz     		( ACC ,vf06,vf15 );	      vu_nop();								      //BP_ASM_BOUND -  maddaz.xyzw		ACC ,vf06,vf15	   nop								   ; 8 vertex coordinate transformation
	vu_maddw     		( vf15,vf07,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf15,vf07,vf00	   nop								   ; 8 vertex coordinate transformation

   vu_mula     		( ACC ,vf14,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf14,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf22,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf22,vf29,vf00	   nop								   ; Check the clip for peak generation

   vu_nop();                                    vu_nop();								      //BP_ASM_BOUND -  nop								      nop								   ; 

   vu_mula     		( ACC ,vf15,vf28 );	      vu_nop();								      //BP_ASM_BOUND -  mula.xyzw		ACC ,vf15,vf28	   nop								   ; Check the clip for peak generation
	vu_maddw     		( vf23,vf29,vf00 );	      vu_nop();								      //BP_ASM_BOUND -  maddw.xyzw		vf23,vf29,vf00	   nop								   ; Check the clip for peak generation

   // vf08-vf15 = corner points, multiplied by the matrix contained in vf04-vf07.
   // vf16-vf23 = corner points which are scaled by the value contained in vf28, then translated by the value contained in vf29.

   vu_clipw_xyz_f		( vf08,vf08, vu_flags0 );  vu_nop();								      //BP_ASM_BOUND -  clipw.xyz		vf08,vf08		   nop								   ; 
	vu_clipw_xyz_f		( vf16,vf16, vu_flags0 );	vu_nop();								      //BP_ASM_BOUND -  clipw.xyz		vf16,vf16		   nop								   ; 
	vu_clipw_xyz_f		( vf09,vf09, vu_flags1 );	vu_nop();								      //BP_ASM_BOUND -  clipw.xyz		vf09,vf09		   nop								   ; 

	vu_clipw_xyz_f		( vf17,vf17, vu_flags1 );	vu_iaddiu		( vi02,vi00,0xfff );	   //BP_ASM_BOUND -  clipw.xyz		vf17,vf17		   iaddiu		vi02,vi00,0xfff   ; 
	vu_clipw_xyz_f		( vf10,vf10, vu_flags2 );	vu_iaddiu		( vi03,vi00,0 );			//BP_ASM_BOUND -  clipw.xyz		vf10,vf10		   iaddiu		vi03,vi00,0			; 
	vu_clipw_xyz_f		( vf18,vf18, vu_flags2 );	vu_fcget		   ( vi08, vu_flags0 );	   //BP_ASM_BOUND -  clipw.xyz		vf18,vf18		   fcget		   vi08				   ; 
	vu_clipw_xyz_f		( vf11,vf11, vu_flags3 );	vu_iand		   ( vi02,vi02,vi08 );		//BP_ASM_BOUND -  clipw.xyz		vf11,vf11		   iand		   vi02,vi02,vi08		; 
	vu_clipw_xyz_f		( vf19,vf19, vu_flags3 );	vu_fcget		   ( vi09, vu_flags1 );		//BP_ASM_BOUND -  clipw.xyz		vf19,vf19		   fcget		   vi09				   ; 
	vu_clipw_xyz_f		( vf12,vf12, vu_flags4 );	vu_iand		   ( vi02,vi02,vi09 );		//BP_ASM_BOUND -  clipw.xyz		vf12,vf12		   iand		   vi02,vi02,vi09		; 
	vu_clipw_xyz_f		( vf20,vf20, vu_flags4 );	vu_fcget		   ( vi10, vu_flags2 );		//BP_ASM_BOUND -  clipw.xyz		vf20,vf20		   fcget		   vi10				   ; 
	vu_clipw_xyz_f		( vf13,vf13, vu_flags5 );	vu_iand		   ( vi02,vi02,vi10 );		//BP_ASM_BOUND -  clipw.xyz		vf13,vf13		   iand		   vi02,vi02,vi10		; 
	vu_clipw_xyz_f		( vf21,vf21, vu_flags5 );	vu_fcget		   ( vi11, vu_flags3 );		//BP_ASM_BOUND -  clipw.xyz		vf21,vf21		   fcget		   vi11				   ; 
	vu_clipw_xyz_f		( vf14,vf14, vu_flags6 );	vu_iand		   ( vi02,vi02,vi11 );	   //BP_ASM_BOUND -  clipw.xyz		vf14,vf14		   iand		   vi02,vi02,vi11		; 
	vu_clipw_xyz_f		( vf22,vf22, vu_flags6 );	vu_fcget		   ( vi12, vu_flags4 );	   //BP_ASM_BOUND -  clipw.xyz		vf22,vf22		   fcget		   vi12				   ; 
	vu_clipw_xyz_f		( vf15,vf15, vu_flags7 );	vu_iand		   ( vi02,vi02,vi12 );		//BP_ASM_BOUND -  clipw.xyz		vf15,vf15		   iand		   vi02,vi02,vi12		; 
	vu_clipw_xyz_f		( vf23,vf23, vu_flags7 );	vu_fcget		   ( vi13, vu_flags5 );	   //BP_ASM_BOUND -  clipw.xyz		vf23,vf23		   fcget		   vi13				   ; 

   // bitwise-AND together clipping judgements to determine if the AABB is fully-in.
   // bitwise-OR together clipping judgements to determine if the AABB is partially-in or fully-out.

	vu_nop();								            vu_iand		   ( vi02,vi02,vi13 );		//BP_ASM_BOUND -  nop								      iand		   vi02,vi02,vi13		; 
	vu_nop();								            vu_fcget		   ( vi14, vu_flags6 );	   //BP_ASM_BOUND -  nop								      fcget		   vi14				   ; 
	vu_nop();								            vu_iand		   ( vi02,vi02,vi14 );		//BP_ASM_BOUND -  nop								      iand		   vi02,vi02,vi14		; 
	vu_nop();								            vu_fcget		   ( vi15, vu_flags7 );	   //BP_ASM_BOUND -  nop								      fcget		   vi15				   ; 
	vu_nop();								            vu_iand		   ( vi02,vi02,vi15 );     //BP_ASM_BOUND -  nop								      iand		   vi02,vi02,vi15		; 
	vu_nop();								            vu_ior			( vi08,vi08,vi09 );		//BP_ASM_BOUND -  nop								      ior			vi08,vi08,vi09		; 
	vu_nop();								            vu_ior			( vi10,vi10,vi11 );		//BP_ASM_BOUND -  nop								      ior			vi10,vi10,vi11		; 
	vu_nop();								            vu_ior			( vi12,vi12,vi13 );		//BP_ASM_BOUND -  nop								      ior			vi12,vi12,vi13		; 
	vu_nop();								            vu_ior			( vi14,vi14,vi15 );		//BP_ASM_BOUND -  nop								      ior			vi14,vi14,vi15		; 
	vu_nop();								            vu_ior			( vi08,vi08,vi10 );		//BP_ASM_BOUND -  nop								      ior			vi08,vi08,vi10		; 
	vu_nop();								            vu_ior			( vi12,vi12,vi14 );     //BP_ASM_BOUND -  nop								      ior			vi12,vi12,vi14		; 
	vu_nop();								            vu_ior			( vi03,vi08,vi12 );     //BP_ASM_BOUND -  nop								      ior			vi03,vi08,vi12		; 
   
	vu_nop();   							            vu_nop();								      //BP_ASM_BOUND -  nop[e]							      nop								   ; 
	vu_nop();								            vu_nop();								      //BP_ASM_BOUND -  nop								      nop								   ; 
}
#endif

#endif