/*
	mgs_type.h
	各種設定定義ヘッダ

	1999/07/07 K.Takabe
	$Id: mgs_type.h,v 1.1.1.3 2002/11/19 11:41:59 Yoshizawa1 Exp $

*/

#pragma once

#ifndef __MGS_TYPE__
#define __MGS_TYPE__

#ifdef GOLD_VERSION
#define printf( ... )
#endif

#ifdef SN_TARGET_PS3
#define BP_PS3 1
#endif

#ifdef SN_TARGET_PSP2
#define BP_VITA 1
#endif

#include <float.h>//BP_ASM
#include <math.h>//BP_ASM

// BP - Pragmas and warnings
#if defined(BP_WIN32) || defined(BP_360)
//#  pragma warning( error : 4013 ) // function undefined
//#  pragma warning( error : 4005 ) // macro redefinition
#  pragma warning( error : 4293 ) // shift negative or too big (usually because you're shifting a 32-bit type and want it to be 64)
#  pragma warning( disable : 4018 ) // signed/unsigned mismatch
#  pragma warning( disable : 4101 ) // local variable never used
#  pragma warning( disable : 4996 ) // local variable never used
#  pragma warning( disable : 4200 ) // nonstandard extension used : zero-sized array in struct/union
#  pragma warning( error : 4013 ) // 'function' undefined; assuming extern returning int
#  pragma warning( error : 4067 ) // unexpected tokens following preprocessor directive - expected a newline
#  pragma warning( error : 4716 ) // 'function' must return a value
#endif

#if defined(BP_VITA)
// expression has no effect
#  pragma diag_suppress=175
// declared but never referenced
#  pragma diag_suppress=178
// typedef name has already been declared
#  pragma diag_suppress=302
// multicharacter character literal
#  pragma diag_suppress=1437
// Incompatible macro redefinition
#  pragma diag_error=47
// Function declared implicitly
#  pragma diag_error=224
// missing return statement
#  pragma diag_error=942
// too few args in call
#  pragma diag_error=1624
#endif

//BP - BEGIN CUSTOM DEFS

#define SYS_COMMON_TYPES

#define EXTERN_INLINE static inline

#ifdef BP_WIN32

#include <xmmintrin.h>

#  ifdef __cplusplus
#    define _LANGUAGE_C_PLUS_PLUS
#  else
#    define LANGUAGE_C
#  endif

#  define BPE_IS_ENDIAN_BIG() (0)
#  define BPE_IS_ENDIAN_LITTLE() (1)

   typedef unsigned __int8    u_char;
   typedef unsigned __int16   u_short;
   typedef unsigned __int32   u_int;
   typedef unsigned __int64   u_long64;
   typedef unsigned __int32   u_long32;
   typedef unsigned __int64   u_long64;
   typedef unsigned __int64   u_int64;
   typedef          __int32   long32;
   typedef          __int64   long64;

   typedef __m128				   long128;
   typedef __m128             u_long128;

   typedef int             qword[4];

#  undef inline
#  define inline __inline

#  define BP_BREAK __debugbreak()

#  define __PRETTY_FUNCTION__     __FUNCTION__

#  define Copy128(_dst,_src)    memcpy(_dst,_src,16)
#  define CopyVector(_dst,_src) memcpy(_dst,_src,16)
#  define CopyMatrix(_dst,_src) memcpy(_dst,_src,64)

#  define PREALIGN(N) __declspec(align(N))
#  define POSTALIGN(N)

#  define STACK_ALLOC(X) _alloca(X)

#  define I64(_num)  _num##i64

#elif defined(BP_PS3)

#  include <vec_types.h>

#  ifdef __cplusplus
#    define _LANGUAGE_C_PLUS_PLUS
#  else
#    define LANGUAGE_C
#  endif

#ifndef GOLD_VERSION
#  if defined(SPU)
#     define BP_BREAK            spu_hcmpeq(0, 0);
#  else
#define BP_BREAK {\
   printf("BP_BREAK: %s(%d)...\n", __FILE__,__LINE__);\
   __asm__ volatile("trap");\
}
#  endif
#else
#  define BP_BREAK
#endif

#  define BPE_IS_ENDIAN_BIG() (1)
#  define BPE_IS_ENDIAN_LITTLE() (0)

   typedef unsigned char         u_char;
   typedef unsigned short        u_short;
   typedef unsigned int          u_int;
   typedef unsigned long long    u_long64;
   typedef unsigned long         u_long32;
   typedef unsigned long long    u_int64;
   typedef long                  long32;
   typedef long long             long64;

   typedef vector signed int     long128;
   typedef vector unsigned int   u_long128;
// qword defined in vec_types.h
#  define bzero(b,len) (memset((b), 0, (len)), (void) 0)

#  define Copy128(_dst,_src)    memcpy(_dst,_src,16)
#  define CopyVector(_dst,_src) memcpy(_dst,_src,16)
#  define CopyMatrix(_dst,_src) memcpy(_dst,_src,64)

#  define PREALIGN(N)
#  define POSTALIGN(N) __attribute__((__aligned__((N))))

#  define STACK_ALLOC(size) __builtin_alloca(size)

#  define I64(_num)  _num##LL

#elif defined( BP_360 )

#  ifdef __cplusplus
#    define _LANGUAGE_C_PLUS_PLUS
#  else
#    define LANGUAGE_C
#  endif

#  define BPE_IS_ENDIAN_BIG() (1)
#  define BPE_IS_ENDIAN_LITTLE() (0)

typedef unsigned __int8    u_char;
typedef unsigned __int16   u_short;
typedef unsigned __int32   u_int;
typedef unsigned __int64   u_long64;
typedef unsigned __int32   u_long32;
typedef unsigned __int64   u_long64;
typedef unsigned __int64   u_int64;
typedef          __int32   long32;
typedef          __int64   long64;

typedef struct long128 {
   __int64 lo, hi;
} long128;
typedef struct u_long128 {
   unsigned __int64 lo, hi;
} u_long128;

typedef int             qword[4];

#  undef inline
#  define inline __inline

#ifndef GOLD_VERSION

#define BP_BREAK {\
   printf("BP_BREAK: %s(%d)...\n", __FILE__,__LINE__);\
   __debugbreak();\
}
#else
#  define BP_BREAK
#endif

#  define __PRETTY_FUNCTION__     __FUNCTION__

#  define Copy128(_dst,_src)    memcpy(_dst,_src,16)
#  define CopyVector(_dst,_src) memcpy(_dst,_src,16)
#  define CopyMatrix(_dst,_src) memcpy(_dst,_src,64)

#  define PREALIGN(N) __declspec(align(N))
#  define POSTALIGN(N)

#  define STACK_ALLOC(X) _alloca(X)

#  define I64(_num)  _num##i64

#elif defined(BP_VITA)

//#  include <vec_types.h>
#  include <alloca.h>

#  ifdef __cplusplus
#    define _LANGUAGE_C_PLUS_PLUS
#  else
#    define LANGUAGE_C
#  endif
#  ifndef GOLD_VERSION
#     define BP_BREAK            (*((int *) 0) = 0)
#  else
#     define BP_BREAK (0)
#  endif
#  define BPE_IS_ENDIAN_BIG() (0)
#  define BPE_IS_ENDIAN_LITTLE() (1)

typedef unsigned char         u_char;
typedef unsigned short        u_short;
typedef unsigned int          u_int;
typedef unsigned long long    u_long64;
typedef unsigned long         u_long32;
typedef unsigned long long    u_int64;
typedef long                  long32;
typedef long long             long64;

typedef vector signed int     long128;
typedef vector unsigned int   u_long128;
typedef int             qword[4];

// qword defined in vec_types.h
#  define bzero(b,len) (memset((b), 0, (len)), (void) 0)

#  define Copy128(_dst,_src)    memcpy(_dst,_src,16)
#  define CopyVector(_dst,_src) memcpy(_dst,_src,16)
#  define CopyMatrix(_dst,_src) memcpy(_dst,_src,64)

#  define PREALIGN(N)
#  define POSTALIGN(N) __attribute__((__aligned__((N))))

#  define STACK_ALLOC(size) alloca(size)

#  define I64(_num)  _num##LL

#else
#  error Unknown target!
#endif // BP_WIN32

#ifdef _DEBUG
    #define DEBUG
#endif

#ifndef FALSE
   #define FALSE 0
   #define TRUE 1
#endif

#define BP_TODO_BREAK               BP_BREAK

#define BP_VITA_TODO_BREAK          BP_BREAK

#ifdef STEVEB
#define BP_MATH_TODO_BREAK          BP_BREAK
#define BP_ASM_TODO_BREAK           BP_BREAK
#else
#define BP_MATH_TODO_BREAK
#define BP_ASM_TODO_BREAK
#endif

#if defined(MARCO) || defined(DAK)
#define BP_RENDER_TODO_BREAK        BP_BREAK
#else
#define BP_RENDER_TODO_BREAK
#endif

#define BP_CODEC_RENDER_TODO_BREAK
#define BP_MOVIE_TODO_BREAK
#define BP_SOUND_TODO_BREAK   
#define BP_TRIVIAL_BREAK      

#ifdef ANDY
   #define BP_ANDY_BREAK      BP_BREAK
#else
   #define BP_ANDY_BREAK
#endif

#ifdef DAK
   #define BP_DAK_BREAK       BP_BREAK
#else
   #define BP_DAK_BREAK
#endif

#ifdef GREG
   #define BP_GREG_BREAK      BP_BREAK
#else
   #define BP_GREG_BREAK
#endif

#ifdef JEFF
   #define BP_JEFF_BREAK      BP_BREAK
#else
   #define BP_JEFF_BREAK
#endif

#ifdef JACK
   #define BP_JACK_BREAK      BP_BREAK
#else
   #define BP_JACK_BREAK
#endif

#ifdef MARCO
   #define BP_MARCO_BREAK     BP_BREAK
#else
   #define BP_MARCO_BREAK
#endif

#ifdef STEVEB
   #define BP_STEVEB_BREAK    BP_BREAK
#else
   #define BP_STEVEB_BREAK
#endif

#ifdef STEVEM
   #define BP_STEVEM_BREAK    BP_BREAK
#else
   #define BP_STEVEM_BREAK
#endif

#ifdef TED
   #define BP_TED_BREAK       BP_BREAK
#else
   #define BP_TED_BREAK
#endif

extern char* BP_ScratchPad;

//BP - END CUSTOM DEFS


#ifdef KP_XBOX
#include "mgsx_type.h"
#else

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libvu0.h>

#if defined(__GNUC__) || defined( __SNC__ )

#define ALIGN8_PRE
#define ALIGN16_PRE
#define ALIGN64_PRE

#define ALIGN8_DECL(a) a __attribute__ ((aligned (8)))
#define ALIGN16_DECL(a) a __attribute__ ((aligned (16)))
#define ALIGN64_DECL(a) a __attribute__ ((aligned (64s)))

#define ALIGN8_POST  __attribute__((aligned (8 )))
#define ALIGN16_POST __attribute__((aligned (16)))
#define ALIGN64_POST __attribute__((aligned (64)))

#else

#define ALIGN8_DECL(a) __declspec(align(8)) a
#define ALIGN16_DECL(a) __declspec(align(16)) a
#define ALIGN64_DECL(a) __declspec(align(64)) a

#define ALIGN8_PRE __declspec( align(8) )
#define ALIGN16_PRE __declspec( align(16) )
#define ALIGN64_PRE __declspec( align(64) )

#define ALIGN8_POST
#define ALIGN16_POST
#define ALIGN64_POST

#endif

/* 以下は未使用 */
typedef ALIGN16_PRE int IVector[4] ALIGN16_POST;
typedef ALIGN16_PRE float FVector[4] ALIGN16_POST;
typedef ALIGN16_PRE float FMatrix[4][4] ALIGN16_POST;
typedef u_char CVector[4];
typedef u_char *CVectorP;

/*
	各種ベクトル＆マトリクス定義
*/
typedef struct { unsigned char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, pad ; } SVECTOR ;
typedef ALIGN16_DECL(struct) { int vx, vy, vz, vw ; } IVECTOR;

#if 1 //BP
typedef ALIGN16_DECL(struct) FVECTOR {
   union {
      struct { float	vx, vy, vz, vw; };
      struct { float	x, y, z, w; };	// D3DXVECTOR like
   };

#ifdef __cplusplus

   FVECTOR operator + ( FVECTOR& vec) const
   {
      FVECTOR temp( *this );
      temp.x += vec.x;
      temp.y += vec.y;
      temp.z += vec.z;
      return temp;
   }

   FVECTOR& operator += ( FVECTOR& vec)
   {
      x += vec.x;
      y += vec.y;
      z += vec.z;
      return *this;
   }

   FVECTOR& operator -= ( FVECTOR& vec)
   {
      x -= vec.x;
      y -= vec.y;
      z -= vec.z;
      return *this;
   }

   FVECTOR operator * ( float f ) const
   {
      FVECTOR temp( *this );
      temp.x *= f;
      temp.y *= f;
      temp.z *= f;
      return temp;
   }

   float	Dist2(FVECTOR* pvecTo)
   {
      float dx,dy,dz;

      dx = pvecTo->x - x;
      dy = pvecTo->y - y;
      dz = pvecTo->z - z;

      return dx * dx + dy * dy + dz * dz;
   }

   FVECTOR& operator () (float fx, float fy, float fz) { x = fx; y = fy; z = fz; return *this; }
   FVECTOR& operator () (float fx, float fy, float fz, float fw) { x = fx; y = fy; z = fz; w = fw; return *this; }

#endif	// __cplusplus

} FVECTOR;
#else
typedef ALIGN16_DECL(struct) { float vx, vy, vz, vw ; } FVECTOR;
#endif

typedef ALIGN16_DECL(struct) { float m[4][4] ; } FMATRIX ;

#define MATRIX FMATRIX
#define VECTOR FVECTOR
typedef int BOOL;

//BP #define sqrtf(m) ({float _f=(m);asm("sqrt.s %0,%0":"+f"(_f));_f;})

//BP_MATH - use PS2 emulation square root functions so all platforms act the same
#if 0
#define sceVu0Sqrt   sqrtf
#else
static inline float bp_sqrtf( const float f ) { return sqrtf(fabs(f)); }
static inline float sceVu0Sqrt( const float f ) { return sqrtf(fabs(f)); }
#endif

#define scePcStart( _a, _b, _c ){\
								   DI();\
								   scePcStart( (_a), (_b), (_c) );\
								   EI();\
							 }

#define FLOAT_MAX 3.40282e+38F

/*
	sce*との互換用
*/

#define _sceVu0CopyVector( _v0, _v1) \
	sceVu0CopyVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0CopyVectorXYZ( _v0, _v1) \
		sceVu0CopyVectorXYZ(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0FTOI0Vector( _v0, _v1) \
		sceVu0FTOI0Vector(*(sceVu0IVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0FTOI4Vector( _v0, _v1) \
		sceVu0FTOI4Vector(*(sceVu0IVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0ITOF0Vector( _v0, _v1) \
		sceVu0ITOF0Vector(*(sceVu0FVECTOR *) (_v0), *(sceVu0IVECTOR *) (_v1))
#define _sceVu0ITOF4Vector( _v0, _v1) \
		sceVu0ITOF4Vector(*(sceVu0FVECTOR *) (_v0), *(sceVu0IVECTOR *) (_v1))
#define _sceVu0ScaleVector( _v0, _v1, _s) \
		sceVu0ScaleVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), _s)
#define _sceVu0ScaleVectorXYZ( _v0, _v1, _s) \
		sceVu0ScaleVectorXYZ(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), _s)
#define _sceVu0AddVector(_v0,_v1,_v2) \
		sceVu0AddVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), *(sceVu0FVECTOR *)(_v2))
#define _sceVu0SubVector(_v0,_v1,_v2) \
		sceVu0SubVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), *(sceVu0FVECTOR *)(_v2))
#define _sceVu0MulVector(_v0,_v1,_v2) \
		sceVu0MulVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), *(sceVu0FVECTOR *)(_v2))
#define _sceVu0InterVector(_v0,_v1,_v2, _r) \
		sceVu0InterVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), *(sceVu0FVECTOR *)(_v2), _r)
#define _sceVu0InterVectorXYZ(_v0, _v1, _v2, _r) \
		sceVu0InterVectorXYZ(*(sceVu0FVECTOR *)(_v0), *(sceVu0FVECTOR *)(_v1), *(sceVu0FVECTOR *)(_v2), _r)
#define _sceVu0DivVector(_v0,_v1,_q) \
		sceVu0DivVector(*(sceVu0FVECTOR *)(_v0), *(sceVu0FVECTOR *) (_v1), _q)
#define _sceVu0DivVectorXYZ(_v0, _v1, _q) \
		sceVu0DivVectorXYZ(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), (float)(_q) )
#define _sceVu0InnerProduct(_v0,_v1) \
		sceVu0InnerProduct(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0OuterProduct(_v0,_v1,_v2) \
		sceVu0OuterProduct(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1), *(sceVu0FVECTOR *)(_v2))
#define _sceVu0Normalize(_v0,_v1) \
		sceVu0Normalize(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0ApplyMatrix(_v0, _m,_v1) \
		sceVu0ApplyMatrix(*(sceVu0FVECTOR *) (_v0), *(sceVu0FMATRIX *) (_m), *(sceVu0FVECTOR *) (_v1))
#define _sceVu0UnitMatrix(_m) \
		sceVu0UnitMatrix(*(sceVu0FMATRIX *) (_m))
#define _sceVu0CopyMatrix(_m0, _m1) \
		sceVu0CopyMatrix(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1))
#define _sceVu0TransposeMatrix( _m0, _m1) \
		sceVu0TransposeMatrix(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1))
#define _sceVu0MulMatrix( _m0, _m1, _m2) \
		sceVu0MulMatrix(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1), *(sceVu0FMATRIX *)(_m2))
#define _sceVu0InversMatrix(_m0,_m1) \
		sceVu0InversMatrix(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1))
#define _sceVu0RotMatrixX(_m0,  _m1, _rx) \
		sceVu0RotMatrixX(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1), _rx)
#define _sceVu0RotMatrixY(_m0, _m1, _ry) \
		sceVu0RotMatrixY(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1), _ry)
#define _sceVu0RotMatrixZ( _m0, _m1,  _rz) \
		sceVu0RotMatrixZ(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1),  _rz)
#define _sceVu0RotMatrix( _m0, _m1, _rot) \
		sceVu0RotMatrix(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1), *(sceVu0FVECTOR *) (_rot))
#define _sceVu0TransMatrix(_m0,_m1,_tv) \
		sceVu0TransMatrix(*(sceVu0FMATRIX *) (_m0), *(sceVu0FMATRIX *) (_m1), *(sceVu0FVECTOR *) (_tv))
#define _sceVu0CameraMatrix( _m, _p, _zd, _yd) \
		sceVu0CameraMatrix(*(sceVu0FMATRIX *)(_m), *(sceVu0FVECTOR *)(_p), *(sceVu0FVECTOR *)(_zd), *(sceVu0FVECTOR *)(_yd))
#define _sceVu0NormalLightMatrix(_m,_l0,_l1,_l2) \
		sceVu0NormalLightMatrix(*(sceVu0FMATRIX *)(_m), *(sceVu0FVECTOR *)(_l0), *(sceVu0FVECTOR *)(_l1), *(sceVu0FVECTOR *)(_l2))
#define _sceVu0LightColorMatrix(_m, _c0, _c1, _c2, _a) \
		sceVu0LightColorMatrix(*(sceVu0FMATRIX *)(_m), *(sceVu0FVECTOR *)(_c0), *(sceVu0FVECTOR *)(_c1), *(sceVu0FVECTOR *)(_c2), *(sceVu0FVECTOR *)(_a))
#define _sceVu0ViewScreenMatrix(_m,  _scrz,  _ax,  _ay,  _cx,  _cy,  _zmin,  _zmax,  _nearz,  _farz) \
		sceVu0ViewScreenMatrix(*(sceVu0FMATRIX *)(_m),  _scrz,  _ax,  _ay,  _cx,  _cy,  _zmin,  _zmax,  _nearz,  _farz)
#define _sceVu0DropShadowMatrix(_m, _lp,  _a,  _b,  _c, _mode) \
		sceVu0DropShadowMatrix( *(sceVu0FMATRIX *)(_m), *(sceVu0FVECTOR *)(_lp),  _a,  _b,  _c, _mode)
#define _sceVu0ClipAll( _minv, _maxv, _ms, _vm, _n) \
		sceVu0ClipAll(*(sceVu0FVECTOR *)(_minv), *(sceVu0FVECTOR *)(_maxv), *(sceVu0FMATRIX *)(_ms), (sceVu0FVECTOR*)(_vm), _n)
#define _sceVu0ClampVector(_v0, _v1,  _min,  _max) \
		sceVu0ClampVector(*(sceVu0FVECTOR *) (_v0), *(sceVu0FVECTOR *) (_v1),  _min,  _max)



/* ＥＥバグによるsceVu0Normalize不具合回避 */
#if 1
/* ＦＰＵ直たたきＳＱＲＴ逆数 */

#if 0//BP_ASM
#define __RSQRT( _m ) ({float _f=(_m);asm("rsqrt.s %0,%1,%0":"+f"(_f):"f"(1.0f));_f;})
#else
EXTERN_INLINE
float __RSQRT( const float f )
{
   float const squareRoot = bp_sqrtf(f);
   if( squareRoot > FLT_EPSILON )
   {
      return 1.0f / squareRoot;
   }
   else
   {
      // Create valid normalized vectors of zero length
      return 0.0f;
   }
}
#endif

static inline float __InnerProduct( FVECTOR *v0, FVECTOR *v1 )
{
	float	ret ;
#if 0 //BP_ASM
	asm volatile ("
		mula.s		%1,%4
		madda.s		%2,%5
		madd.s		%0,%3,%6
	":"=f"(ret):"f"(v0->vx),"f"(v0->vy),"f"(v0->vz),"f"(v1->vx),"f"(v1->vy),"f"(v1->vz) );
#else
   ret = (v0->vx * v1->vx) + (v0->vy * v1->vy) + (v0->vz * v1->vz);
#endif
	return ( ret );
}
static inline void __ScaleVector( FVECTOR *v0, FVECTOR *v1, float s )
{
#if 0 //BP_ASM
	asm volatile("
		mfc1			$4,%2
		lqc2			vf1,0x00(%1)
		qmtc2			$4,vf2
		vmulx.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"f"(s):"$4","memory" );
#else
   v0->vx = v1->vx * s;
   v0->vy = v1->vy * s;
   v0->vz = v1->vz * s;
   v0->vw = v1->vw * s;
#endif
}
static inline void __Normalize( FVECTOR *v0, FVECTOR *v1 )
{
	float	len ;
	len = __InnerProduct( v1, v1 );
	len = __RSQRT( len );
	__ScaleVector( v0, v1, len );
}
#undef _sceVu0Normalize
#define _sceVu0Normalize(_v0,_v1) __Normalize( _v0, _v1 )
#define sceVu0Normalize(_v0,_v1) __Normalize( (FVECTOR*)_v0, (FVECTOR*)_v1 )
#endif

//typedef char byte;

#ifdef __cplusplus
};
#endif

#endif //KP_XBOX

#ifdef __SNC__
// SNC compilers don't seem to let you do extern inlines.
#  define SNC_EXTERN_INLINE 
#else
#  define SNC_EXTERN_INLINE inline
#endif

#endif
