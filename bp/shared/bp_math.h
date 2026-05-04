//----------------------------------------------------------------------------
// bp_math.h
//----------------------------------------------------------------------------
#ifndef __BP_MATH_H__
#define __BP_MATH_H__

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------

#include "MGS_SysCommon.h"

#include <math.h>       // For sqrt, sinf, cosf
#include <float.h>
#include <assert.h>

#if defined(BP_360)
#include <ppcintrinsics.h>
#endif
//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

// Enable/disable vu math asserts (very slow, but catches invalid floats!)
#if defined(_DEBUG) && defined(STEVEB)
#define bp_math_break               BP_BREAK
#define bp_math_assert(__exp__)     if(!(__exp__)) bp_math_break
#else
#define bp_math_break
#define bp_math_assert(__exp__)
#endif

#if BP_VITA
#  define M2NEONPTR(m) ((float32x4x4_t *) (m) )
#  define M2NEONCPTR(m) ((float32x4x4_t const *) (m) )
#  define M2NEON(m) ( *M2NEONCPTR(m) )
#endif

#define DELTA 1e-6f     // error tolerance

#define VU_FLAGS_COUNT     8
#define VU_FLT_REG_COUNT   32
#define VU_INT_REG_COUNT   16
#define VU_MEM_COUNT       1024

#define EE_FPR_REG_COUNT   16
#define EE_GPR_REG_COUNT   16

#define SQRT_FLT_MAX       (1.8446743e+019)

#ifndef PI
#define PI 3.1415926535f
#endif

//----------------------------------------------------------------------------
// TYPES
//----------------------------------------------------------------------------

struct PS2CTX_s;
typedef void(*vu_function)(struct PS2CTX_s *pPS2);

//----------------------------------------------------------------------------
// STRUCTURES
//----------------------------------------------------------------------------

// VU0 flags - an array of these is used to easily emulate overlapping flag operations
typedef struct 
{
   // status registers
   int      status;
   int      mac;
   int      clipping;
} VU0_Flags;

//----------------------------------------------------------------------------

// VU0 memory entry. Union of float/integer vector4
typedef union
{
   FVECTOR  fv;     
   IVECTOR  iv;     
} VU0_Mem;

//----------------------------------------------------------------------------

typedef struct
{
   FVECTOR siz;
   FVECTOR pos;
} SVU_Dot;
typedef struct 
{
   FVECTOR siz;
   FVECTOR pos;
   FVECTOR dir;
} SVU_Line;
typedef struct
{
   FVECTOR nrm;
   FVECTOR pos[4];
   FVECTOR dir[4];
} SVU_Poly;
typedef struct
{
   FVECTOR siz;
   FVECTOR inv_pos;
   FMATRIX mat;
   FMATRIX inv; // note the position isn't in this matrix!
} SVU_Box;
typedef struct
{
   FVECTOR siz;
   FVECTOR pos;
} SVU_Field;
typedef struct
{
   FVECTOR siz;
   FVECTOR pos;
} SVU_Ref;
typedef struct
{
   FVECTOR pad[4];
   FVECTOR tmp[6];
} SVU_Tmp;

typedef union
{
   FVECTOR pad[10];
   SVU_Dot dot;
   SVU_Line line;
   SVU_Poly poly;
   SVU_Box box;
   SVU_Field field;
   SVU_Ref reff;
   SVU_Tmp tmp;
}
UVU_Geom;

typedef struct
{
   FVECTOR pad[10];
   UVU_Geom a;
   UVU_Geom b;
}
SVU_Geom;

// VU0 register storage
typedef struct VU0_s
{
   // float registers
   float          i;
   float          q;
   FVECTOR        acc;

   union
   {
      // vector registers
      SVU_Geom    geom;
      FVECTOR     vf[VU_FLT_REG_COUNT];
   };

   // integer registers
   short          vi[VU_INT_REG_COUNT];

   // flags
   // NOTE: Array is used for overlapping operations/tests
   VU0_Flags      flags[VU_FLAGS_COUNT];

   // internal memory
   VU0_Mem        memory[VU_MEM_COUNT];  //16K of memory

   // misc
   unsigned             short random;     // current random number
} VU0;

//----------------------------------------------------------------------------

// EE register storage
typedef struct EE_s
{
   // float registers
   float    fpr[EE_FPR_REG_COUNT];

   // integer registers
   IVECTOR  gpr[EE_GPR_REG_COUNT];

   // status registers
   struct
   {
      int c;
   } flags;

} EE;

//----------------------------------------------------------------------------
typedef struct PS2CTX_s
{
   EE ee;
   VU0 vu0;
} PS2CTX;

//----------------------------------------------------------------------------
// DATA
//----------------------------------------------------------------------------

// CPU storage
//extern VU0     gVU0;
//extern EE      gEE;

// Bezier/CatmullRom curve vector storage
extern FVECTOR gCurveVec[10];


EXTERN_INLINE 
void ps2ctx_init( PS2CTX *pPS2 )
{
   pPS2->ee.gpr[0].vx = 0;
   pPS2->ee.gpr[0].vy = 0;
   pPS2->ee.gpr[0].vz = 0;
   pPS2->ee.gpr[0].vw = 0;

   pPS2->vu0.vf[0].vx = 0.0f;
   pPS2->vu0.vf[0].vy = 0.0f;
   pPS2->vu0.vf[0].vz = 0.0f;
   pPS2->vu0.vf[0].vw = 1.0f;
   pPS2->vu0.vi[0] = 0;
}


//----------------------------------------------------------------------------
// MISC FUNCTIONS
//----------------------------------------------------------------------------
void BP_InitMath();
float BP_Float_Floor( const float a );
float BP_Float_Ceil( const float a );
void BP_Vec4_CeilVec( FVECTOR *r, const FVECTOR *a );
void BP_Vec4_CeilIVec( IVECTOR *r, const FVECTOR *a );

void BP_SinCos( FVECTOR *r, float angle );
float BP_Sin( float angle );
float BP_Cos( float angle );
float BP_ASin( float x );

float BP_ATan2Approx( float y, float x );

// these versions do not do any input clamping
// so you need to be sure that the input is between -pi and pi
void BP_SinCosFast( FVECTOR *r, float angle );
float BP_SinFast( float angle );
float BP_CosFast( float angle );

// defines to allow the code base to remain unchanged
#define MA_SinCos BP_SinCos
#define MA_Sin BP_Sin
#define MA_Cos BP_Cos

//----------------------------------------------------------------------------
// FLOAT FUNCTIONS
//----------------------------------------------------------------------------

// returns TRUE if "f" is a valid float or FALSE if invalid
int BP_Float_Check( const float f );

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Float_SetInvalid( float* f )
{
   *(int*)f = 0x7f800000;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Float_Max( const float a, const float b )
{
#if defined(BP_360)
   return fpmax(a, b);
#else
   if( a > b )
   {
      return a;
   }
   else
   {
      return b;
   }
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Float_Min( const float a, const float b )
{
#if defined(BP_360)
   return fpmin(a, b);
#else
   if( a < b )
   {
      return a;
   }
   else
   {
      return b;
   }
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Float_ToInt0( const float f )
{
   bp_math_assert( BP_Float_Check(f) );

   return (int)f;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Float_ToInt4( const float f )
{
   bp_math_assert( BP_Float_Check(f) );

   return (int)(f * 16.0f);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Float_ToInt12( const float f )
{
   bp_math_assert( BP_Float_Check(f) );

   return (int)(f * 4096.0f);
}

//----------------------------------------------------------------------------
// INTEGER FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Int_ToFloat0( const int i )
{
   return (float)i;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Int_ToFloat4( const int i )
{
   return (float)i / 16.0f;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Int_ToFloat12( const int i )
{
   return (float)i / 4096.0f;
}


EXTERN_INLINE
float BP_Fabs( double f )
{
#if BP_VITA
   return __builtin_fabs(f);
#else
   return fabs(f);
#endif
}

EXTERN_INLINE
float BP_Fabsf( float f )
{
#if BP_VITA
   return __builtin_fabsf(f);
#else
   return fabsf(f);
#endif
}

//----------------------------------------------------------------------------
// SQUARE ROOT FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_Sqrt( float const f )
{
#if BP_VITA
   return __builtin_sqrtf(f);
#else
   return sqrtf(f);
#endif
}

// return square_root(f)
EXTERN_INLINE
float BP_EE_Sqrt( const float f )
{
   bp_math_assert( BP_Float_Check(f) );
   //EE_Core_Instruction_Set_Manual.pdf Page 376
   //If input is less than 0, SQRT(ABS(input)) is stored instead.
   return BP_Sqrt(BP_Fabsf(f));
}

//----------------------------------------------------------------------------

// return n/square_root(f)
EXTERN_INLINE
float BP_EE_RSqrt( const float n, const float f )
{
   float s;

   bp_math_assert( BP_Float_Check(n) );
   bp_math_assert( BP_Float_Check(f) );

   //EE_Core_Users_Manual.pdf Page 375
   s = BP_EE_Sqrt(f);
   if( s > FLT_EPSILON )
   {
      return n/s;
   }
   else if( s == 0.0f )
   {
      return (n >= 0.0f) ? FLT_MAX : -FLT_MAX;
   }
   else
   {
      return 0.0f;
   }
}

//----------------------------------------------------------------------------

// return square_root(f)
EXTERN_INLINE
float BP_VU0_Sqrt( const float f )
{
   // VU0 unit test verified that sqrt(abs(f)) is always computed
   return BP_EE_Sqrt(f);
}

//----------------------------------------------------------------------------

// return 1.0/square_root(f)
EXTERN_INLINE
float BP_VU0_InvSqrt( const float f )
{
   float const squareRoot = BP_Sqrt(f);

   bp_math_assert( BP_Float_Check(f) );

   if( squareRoot > FLT_EPSILON )
   {
      return 1.0f / squareRoot;
   }
   else
   {
      // This function is only called on VU0 for normalizing vectors.
      // Unit test shows that normalizing zero length vector on VU0 always
      // returns a zero vector. Returning 0.0f emulates this in the calling functions
      // without the need for any "if" statement.
      return 0.0f;
   }
}

//----------------------------------------------------------------------------
// DIVIDE FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_SafeDivideFEx(const float lhs, const float rhs, const float fmaxIfZeroDivide )
{
   bp_math_assert( BP_Float_Check(lhs) );
   bp_math_assert( BP_Float_Check(rhs) );

   if( rhs == 0.0f )
   {
      if( lhs >= 0.0f )
         return fmaxIfZeroDivide;
      else
         return -fmaxIfZeroDivide;
   }

   return lhs / rhs;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_SafeDivideF(const float lhs, const float rhs)
{
   bp_math_assert( BP_Float_Check(lhs) );
   bp_math_assert( BP_Float_Check(rhs) );

   return BP_SafeDivideFEx( lhs, rhs, FLT_MAX );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
float BP_ACos( float x )
{
   return 0.5f*PI - BP_ASin(x);
}

//----------------------------------------------------------------------------

#if BP_VITA
EXTERN_INLINE
void BP_Convert_Float32x4x4_to_Float16x4x4( float16x4x4_t *pDst, void const *pVoidSrc )
{
   float32x4x4_t const *pSrc = (float32x4x4_t const *)( pVoidSrc );

   pDst->val[0] = vcvt_f16_f32( pSrc->val[0] );
   pDst->val[1] = vcvt_f16_f32( pSrc->val[1] );
   pDst->val[2] = vcvt_f16_f32( pSrc->val[2] );
   pDst->val[3] = vcvt_f16_f32( pSrc->val[3] );
}
#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------


#endif   //#ifndef __BP_MATH_H__
