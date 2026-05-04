/*
	mgs_type.h
	各種設定定義ヘッダ

	2001/01/21 M.Kobayashi
	$Id: mgsx_type.h,v 1.1.1.3 2002/11/19 11:42:00 Yoshizawa1 Exp $

*/

#error THIS FILE SHOULD NOT BE INCLUDED


#ifndef __MGSX_TYPE__H__
#define __MGSX_TYPE__H__

#include "mgs_type.h" //BP

#ifdef KP_XBOX
#include <xtl.h>
#elif defined(_WIN32)
#include <d3dx8.h>
#endif

#if 0//BP (see platform-specific part of mgs_type.h)

// 基本型の置き換え 
#define long __int64

// u_* 系の置き換え
// NOTE: typedef でなく、define で強引に置き換えているのは、
//       windows 側で u_char 等が定義されているため 

#define u_char  unsigned __int8
#define u_short unsigned __int16
#define u_int   unsigned __int32
#define u_long64  unsigned __int64

#endif //BP

// VC++でC言語を使用する場合、inline キーワードは __inline
#ifndef __cplusplus
#define inline __inline
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif
	
#if 0 //BP (see platform-specific part of mgs_type.h)
#ifndef long32
#define long32 int
#endif

#ifndef long64
#define long64 __int64
#endif

typedef struct u_long128 {
	unsigned __int64 lo, hi;
} u_long128;

typedef struct long128 {
	__int64 lo, hi;
} long128;

#endif //BP

/* 以下は未使用 */
typedef ALIGN16_PRE int IVector[4] ALIGN16_POST;
typedef ALIGN16_PRE float FVector[4] ALIGN16_POST;
typedef ALIGN16_PRE float FMatrix[4][4] ALIGN16_POST;
typedef u_char CVector[4];
typedef u_char *CVectorP;

/// (XBOX)なぜかCからsinf 系のfloat 関数が見えない
#ifndef __cplusplus	///
#ifdef BPE_TEMP_UNUSED
#define acosf( f )	((float)acos( f ))
#define asinf( f )	((float)asin( f ))
#define cosf( f )	((float)cos( f ))
#define sqrtf( f )	((float)sqrt( fabs( (double)(f) ) ))
#define logf(_x)	((float)log((double)(_x)))
#define powf(_x,_y)	((float)pow((double)(_x),(double)(_y)))
#define sinf( f )	((float)sin( f ))
#define atanf( f )	((float)atan( f ))
#define atan2f( f0, f1 )	((float)atan2( f0, f1 ))
#define fabsf(_s)	(fabs( (double)(_s) ))
#endif

#define bzero(_s,_r)	(GV_ZeroMemory((_s),(_r)))
#endif	///__cplusplus

// near far 対策
#undef near
#define near	var_near


/*
	各種ベクトル＆マトリクス定義
*/
#ifdef KP_XBOX
#define IALIGN16 _declspec(align(16))
#else
#define IALIGN16
#endif
	
typedef struct { unsigned char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, pad ; } SVECTOR ;
typedef ALIGN16_DECL(struct) IALIGN16 { int vx, vy, vz, vw ; } IVECTOR ;
	
	
	
//typedef struct { float vx, vy, vz, vw ; } FVECTOR ALIGN16;
//	D3DXVECTOR3 との整合性をとる
typedef _declspec(align(16)) struct  FVECTOR {
#ifdef __cplusplus	
public:
#endif	
	union {
		struct { float	vx, vy, vz, vw; };
		struct { float	x, y, z, w; };	// D3DXVECTOR like
	};

#ifdef __cplusplus	
	// キャスト演算子
	operator D3DXVECTOR3&() { return *(D3DXVECTOR3*)this; }
	operator D3DXVECTOR3() { return *(D3DXVECTOR3*)this; }
	operator const D3DXVECTOR3&() { return *(const D3DXVECTOR3*)this; }
	operator const D3DXVECTOR3() { return *(const D3DXVECTOR3*)this; }
	operator D3DXVECTOR4&() { return *(D3DXVECTOR4*)this; }
	operator D3DXVECTOR4() { return *(D3DXVECTOR4*)this; }
	operator const D3DXVECTOR4&() { return *(const D3DXVECTOR4*)this; }
	operator const D3DXVECTOR4() { return *(const D3DXVECTOR4*)this; }

	FVECTOR& operator=(D3DXVECTOR3& vec){ return *(FVECTOR*)&(*(D3DXVECTOR3*)this = vec); }

	// とりあえず ==, != だけ欲しいので…
	inline BOOL operator == (CONST FVECTOR& vec) const {
		return (x == vec.x && y == vec.y && z == vec.z && w == vec.w);
	}
	inline BOOL operator != (CONST FVECTOR& vec) const {
		return (x != vec.x || y != vec.y || z != vec.z || w == vec.w);
	}

	FVECTOR& operator *= ( FLOAT f) { return *(FVECTOR*)&((*(D3DXVECTOR3*)this) *= f); }
	FVECTOR operator * ( FLOAT f ) const {
		return *(FVECTOR*)&(*(D3DXVECTOR3*)this * f);
	}
	FVECTOR& operator += ( FVECTOR& vec) { return *(FVECTOR*)&((*(D3DXVECTOR3*)this) += *reinterpret_cast<D3DXVECTOR3 const *>(&vec)); }
	FVECTOR& operator -= ( FVECTOR& vec) { return *(FVECTOR*)&((*(D3DXVECTOR3*)this) -= *reinterpret_cast<D3DXVECTOR3 const *>(&vec)); }

	FVECTOR operator + () const { return *(FVECTOR*)&(+(*(D3DXVECTOR3*)this)); }
	FVECTOR operator - () const { return *(FVECTOR*)&(-(*(D3DXVECTOR3*)this)); }

    // binary operators
	FVECTOR operator + ( FVECTOR& vec) const {
		return *(FVECTOR*)&(*(D3DXVECTOR3*)this + *reinterpret_cast<D3DXVECTOR3 const *>(&vec));
	}
	FVECTOR operator - ( FVECTOR& vec) const {
		return *(FVECTOR*)&(*(D3DXVECTOR3*)this - *reinterpret_cast<D3DXVECTOR3 const *>(&vec));
	}

	FVECTOR& operator = ( FLOAT f ) { x = y = z = f; return *this; }
	FVECTOR& operator () (float fx, float fy, float fz) { x = fx; y = fy; z = fz; return *this; }
	FVECTOR& operator () (float fx, float fy, float fz, float fw) { x = fx; y = fy; z = fz; w = fw; return *this; }
	
	float	Dist2( FVECTOR* pvecTo );
	float	Dist( FVECTOR* pvecTo ){ return sqrtf( Dist2( pvecTo ) ); }

	float	Len2( void ){ return x * x + y * y + z * z; }
	float	Len( void ){ return sqrtf( Len2() ); }
#endif	// __cplusplus
} FVECTOR;

#ifdef __cplusplus	
typedef _declspec(align(16)) struct FMATRIX {
public:
	union {
		float m[4][4] ;
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
		struct
		{
			FVECTOR	vecX;
			FVECTOR	vecY;
			FVECTOR	vecZ;
			FVECTOR	vecT;
		};
	};

#ifdef __cplusplus	
	operator D3DXMATRIX&() { return *(D3DXMATRIX*)this; }
	operator D3DXMATRIX() { return *(D3DXMATRIX*)this; }
	operator const D3DXMATRIX&() { return *(const D3DXMATRIX*)this; }
	operator const D3DXMATRIX() { return *(const D3DXMATRIX*)this; }

	FMATRIX& operator=(D3DXMATRIX& mat){ return *(FMATRIX*)&(*(D3DXMATRIX*)this = mat); }
#endif
} FMATRIX;
#else
typedef ALIGN16_DECL(struct) { float m[4][4] ; } FMATRIX ;
#endif	

#define sceVu0Sqrt sqrtf

#define scePcStart( _a, _b, _c ){}

#define FLOAT_MAX 3.40282e+38F

/*
	sce*との互換用
*/
typedef float sceVu0FMATRIX[4][4];
typedef float sceVu0FVECTOR[4];
typedef int sceVu0IVECTOR[4];


#define sceVu0CopyVector( _v0, _v1) \
	(*(D3DXVECTOR4*)(_v0) = *(D3DXVECTOR4*)(_v1))
#define sceVu0CopyVectorXYZ( _v0, _v1) \
	(*(D3DXVECTOR3*)(_v0) = *(D3DXVECTOR3*)(_v1))
#define sceVu0FTOI0Vector( _v0, _v1) \
	{ *((int*)(_v0) + 0) = (int)*((float*)(_v1) + 0); \
	*((int*)(_v0) + 1) = (int)*((float*)(_v1) + 1); \
	*((int*)(_v0) + 2) = (int)*((float*)(_v1) + 2); \
	*((int*)(_v0) + 3) = (int)*((float*)(_v1) + 3); }
#define sceVu0FTOI4Vector( _v0, _v1) \
	{ *((int*)(_v0) + 0) = (int)*(((float*)(_v1) + 0) * 16.f); \
	*((int*)(_v0) + 1) = (int)(*((float*)(_v1) + 1) * 16.f); \
	*((int*)(_v0) + 2) = (int)(*((float*)(_v1) + 2) * 16.f); \
	*((int*)(_v0) + 3) = (int)(*((float*)(_v1) + 3) * 16.f); }
#define sceVu0ITOF0Vector( _v0, _v1) \
	{ *((float*)(_v0) + 0) = (float)*((int*)(_v1) + 0); \
	*((float*)(_v0) + 1) = (float)*((int*)(_v1) + 1); \
	*((float*)(_v0) + 2) = (float)*((int*)(_v1) + 2); \
	*((float*)(_v0) + 3) = (float)*((int*)(_v1) + 3); }
#define sceVu0ITOF4Vector( _v0, _v1) \
	{ *((float*)(_v0) + 0) = ((float)*((int*)(_v1) + 0)) * ( 1.f / 16.f); \
	*((float*)(_v0) + 1) = ((float)*((int*)(_v1) + 1)) * ( 1.f / 16.f); \
	*((float*)(_v0) + 2) = ((float)*((int*)(_v1) + 2)) * ( 1.f / 16.f); \
	*((float*)(_v0) + 3) = ((float)*((int*)(_v1) + 3)) * ( 1.f / 16.f); }
#define sceVu0ScaleVector( _v0, _v1, _s) \
	D3DXVec4Scale((D3DXVECTOR4*)(_v0), (D3DXVECTOR4*)(_v1), (float)(_s))
#define sceVu0ScaleVectorXYZ( _v0, _v1, _s) \
	{ D3DXVec3Scale((D3DXVECTOR3*)(_v0), (D3DXVECTOR3*)(_v1), (float)(_s) ); \
	 ((D3DXVECTOR4*)(_v0))->w = ((D3DXVECTOR4*)(_v1))->w; }	//?
#define sceVu0AddVector(_v0,_v1,_v2) \
	D3DXVec4Add((D3DXVECTOR4*)(_v0), (D3DXVECTOR4*)(_v1), (D3DXVECTOR4*)(_v2))
#define sceVu0SubVector(_v0,_v1,_v2) \
	D3DXVec4Subtract((D3DXVECTOR4*)(_v0), (D3DXVECTOR4*)(_v1), (D3DXVECTOR4*)(_v2))
#define sceVu0MulVector(_v0,_v1,_v2) \
	{ ((D3DXVECTOR4*)(_v0))->x = ((D3DXVECTOR4*)(_v1))->x * ((D3DXVECTOR4*)(_v2))->x; \
	 ((D3DXVECTOR4*)(_v0))->y = ((D3DXVECTOR4*)(_v1))->y * ((D3DXVECTOR4*)(_v2))->y; \
	 ((D3DXVECTOR4*)(_v0))->z = ((D3DXVECTOR4*)(_v1))->z * ((D3DXVECTOR4*)(_v2))->z; \
	 ((D3DXVECTOR4*)(_v0))->w = ((D3DXVECTOR4*)(_v1))->w * ((D3DXVECTOR4*)(_v2))->w; }
#define sceVu0InterVector(_v0,_v1,_v2, _r) \
	D3DXVec4Lerp((D3DXVECTOR4*)(_v0), (D3DXVECTOR4*)(_v2), (D3DXVECTOR4*)(_v1), (_r))
#define sceVu0InterVectorXYZ(_v0, _v1, _v2, _r) \
	D3DXVec3Lerp((D3DXVECTOR3*)(_v0), (D3DXVECTOR3*)(_v2), (D3DXVECTOR3*)(_v1), (_r))
#define sceVu0DivVector(_v0,_v1,_q) \
	D3DXVec4Scale((D3DXVECTOR4*)(_v0), (D3DXVECTOR4*)(_v1), 1.f / (float)(_q))
#define sceVu0DivVectorXYZ(_v0, _v1, _q) \
	D3DXVec3Scale((D3DXVECTOR3*)(_v0), (D3DXVECTOR3*)(_v1), 1.f / (float)(_q))
#define sceVu0InnerProduct(_v0,_v1) \
	D3DXVec3Dot((D3DXVECTOR3*)(_v0), (D3DXVECTOR3*)(_v1))
#define sceVu0OuterProduct(_v0,_v1,_v2) \
	{D3DXVec3Cross((D3DXVECTOR3*)(_v0), (D3DXVECTOR3*)(_v1), (D3DXVECTOR3*)(_v2));((D3DXVECTOR4*)(_v0))->w=0.0f;}
#define sceVu0Normalize(_v0,_v1) \
	D3DXVec3Normalize((D3DXVECTOR3*)(_v0), (D3DXVECTOR3*)(_v1))
#define sceVu0ApplyMatrix(_v0, _m,_v1) \
	D3DXVec4Transform((D3DXVECTOR4*)(_v0), (D3DXVECTOR4*)(_v1), (D3DXMATRIX*)(_m))
#define sceVu0UnitMatrix(_m) \
	D3DXMatrixIdentity((D3DXMATRIX*)(_m))
#define sceVu0CopyMatrix(_m0, _m1) \
	{ *(D3DXMATRIX*)(_m0) = *(D3DXMATRIX*)(_m1); }
#define sceVu0TransposeMatrix( _m0, _m1) \
	D3DXMatrixTranspose((D3DXMATRIX *)(_m0), (D3DXMATRIX *)(_m1));
#define sceVu0MulMatrix( _m0, _m1, _m2) \
	D3DXMatrixMultiply((D3DXMATRIX*)(_m0), (D3DXMATRIX*)(_m2), (D3DXMATRIX*)(_m1))
#define sceVu0InversMatrix(_m0,_m1) \
	D3DXMatrixInverse((D3DXMATRIX*)(_m0), NULL, (D3DXMATRIX*)(_m1))

#if 1

#define sceVu0RotMatrixX(_m0, _m1, _rx)  DG_RotMatrixX( (FMATRIX *)(_m0), (FMATRIX *)(_m1), (float)(_rx) )
#define sceVu0RotMatrixY(_m0, _m1, _ry)  DG_RotMatrixY( (FMATRIX *)(_m0), (FMATRIX *)(_m1), (float)(_ry) )
#define sceVu0RotMatrixZ(_m0, _m1, _rz)  DG_RotMatrixZ( (FMATRIX *)(_m0), (FMATRIX *)(_m1), (float)(_rz) )

#else

#define sceVu0RotMatrixX(_m0,  _m1, _rx)  \
{	\
	D3DXMATRIX _local_mat; \
	D3DXMatrixMultiply((D3DXMATRIX*)&(_m0), (D3DXMATRIX*)&(_m1), D3DXMatrixRotationX(&_local_mat, (float)(_rx) )); \
}
#define sceVu0RotMatrixY(_m0, _m1, _ry) \
{	\
	D3DXMATRIX _local_mat; \
	D3DXMatrixMultiply((D3DXMATRIX*)&(_m0), (D3DXMATRIX*)&(_m1), D3DXMatrixRotationY(&_local_mat, (float)(_ry) )); \
} 
#define sceVu0RotMatrixZ( _m0, _m1,  _rz) \
{	\
	D3DXMATRIX _local_mat; \
	D3DXMatrixMultiply((D3DXMATRIX*)&(_m0), (D3DXMATRIX*)&(_m1), D3DXMatrixRotationZ(&_local_mat, (float)(_rz) )); \
} 
#endif

#define sceVu0RotMatrix( _m0, _m1, _rot) \
		{sceVu0RotMatrixZ(_m0, _m1, ((float *)_rot)[2]); \
		 sceVu0RotMatrixY(_m0, _m0, ((float *)_rot)[1]); \
		 sceVu0RotMatrixX(_m0, _m0, ((float *)_rot)[0]);}
#define sceVu0TransMatrix(_m0,_m1,_tv) \
	{DG_COPY_MAT( (FMATRIX *)(_m0), (FMATRIX *)(_m1) );\
	 D3DXVec3Add( (D3DXVECTOR3*)&(((D3DXMATRIX*)(_m0))->_41), (D3DXVECTOR3*)&(((D3DXMATRIX*)(_m1))->_41), \
				  (D3DXVECTOR3*)(_tv) );}

extern void sceVu0DropShadowMatrix(sceVu0FMATRIX m, sceVu0FVECTOR lp, float a, float b, float c, int mode);
	

#if 0	// まだエミュレートできてない
#define sceVu0CameraMatrix( _m, _p, _zd, _yd)	
#define sceVu0NormalLightMatrix(_m,_l0,_l1,_l2) 
#define sceVu0LightColorMatrix(_m, _c0, _c1, _c2, _a)
#define sceVu0ViewScreenMatrix(_m,  _scrz,  _ax,  _ay,  _cx,  _cy,  _zmin,  _zmax,  _nearz,  _farz)
#define sceVu0ClipAll( _minv, _maxv, _ms, _vm, _n) 
#endif



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
#define sceVu0ClampVector(_v0, _v1,  _min,  _max)  _sceVu0ClampVector(_v0, _v1,  _min,  _max) 
#define _sceVu0ClampVector(_v0, _v1,  _min,  _max) \
(\
	  (_v0)->vx = (_v1)->vx>(_min) ? (_v1)->vx : (_min),\
	  (_v0)->vx = (_v1)->vx<(_max) ? (_v1)->vx : (_max),\
	  (_v0)->vy = (_v1)->vy>(_min) ? (_v1)->vy : (_min),\
	  (_v0)->vy = (_v1)->vy<(_max) ? (_v1)->vy : (_max),\
	  (_v0)->vz = (_v1)->vz>(_min) ? (_v1)->vz : (_min),\
	  (_v0)->vz = (_v1)->vz<(_max) ? (_v1)->vz : (_max),\
	  (_v0)->vw = (_v1)->vw>(_min) ? (_v1)->vw : (_min),\
	  (_v0)->vw = (_v1)->vw<(_max) ? (_v1)->vw : (_max)\
) 


typedef struct {	// sceCdCLOCK のエミュレート
        u_char stat;            /* status */
        u_char second;          /* second */
        u_char minute;          /* minute */
        u_char hour;            /* hour   */

        u_char pad;             /* pad    */
        u_char day;             /* day    */
        u_char month;           /* month  */
        u_char year;            /* year   */
} sceCdCLOCK;


extern void sceScfGetLocalTimefromRTC( sceCdCLOCK* pRtc );
extern int sceCdReadClock( sceCdCLOCK* pRtc );

#define cdbios_get_time	sceCdReadClock


#ifdef __cplusplus
} 
#endif


#ifdef __cplusplus
//-----------------------------------------------------------------------------
// D3DX系の関数置き換え(FMATRIX, FVECTOR を使用するように)
//-----------------------------------------------------------------------------

// VECTOR3関係
inline FVECTOR *D3DXVec3Add(FVECTOR *pOut, CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return (FVECTOR *)D3DXVec3Add((D3DXVECTOR3 *)pOut, (CONST D3DXVECTOR3 *)pV1, (CONST D3DXVECTOR3 *)pV2);
}
inline FVECTOR *D3DXVec3Cross(FVECTOR *pOut, CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return (FVECTOR *)D3DXVec3Cross((D3DXVECTOR3 *)pOut, (CONST D3DXVECTOR3 *)pV1, (CONST D3DXVECTOR3 *)pV2);
}
inline FLOAT D3DXVec3Dot(CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return D3DXVec3Dot((CONST D3DXVECTOR3 *)pV1, (CONST D3DXVECTOR3 *)pV2);
}
inline FLOAT D3DXVec3Length(FVECTOR *pV) {
	return D3DXVec3Length((CONST D3DXVECTOR3 *)pV);
}
inline FVECTOR *D3DXVec3Normalize(FVECTOR *pv1, CONST FVECTOR *pv2) {
	return (FVECTOR *)D3DXVec3Normalize((D3DXVECTOR3 *)pv1, (D3DXVECTOR3 *)pv2);
}
inline FVECTOR *D3DXVec3Scale(FVECTOR *pOut, CONST FVECTOR *pV, FLOAT s) {
	return (FVECTOR *)D3DXVec3Scale((D3DXVECTOR3 *)pOut, (CONST D3DXVECTOR3 *)pV, s);
}
inline FVECTOR *D3DXVec3Subtract(FVECTOR *pOut, CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return (FVECTOR *)D3DXVec3Subtract((D3DXVECTOR3 *)pOut, (CONST D3DXVECTOR3*)pV1, (CONST D3DXVECTOR3*)pV2);
}
inline FVECTOR *D3DXVec3Transform(FVECTOR *pOut, CONST FVECTOR *pV, CONST FMATRIX *pM) {
	return (FVECTOR *)D3DXVec3Transform((D3DXVECTOR4 *)pOut, (CONST D3DXVECTOR3 *)pV, (CONST D3DXMATRIX *)pM);
}
inline FVECTOR *D3DXVec3TransformNormal(FVECTOR *pOut, CONST FVECTOR *pV, CONST FMATRIX *pM) {
	return (FVECTOR *)D3DXVec3TransformNormal((D3DXVECTOR3 *)pOut, (CONST D3DXVECTOR3 *)pV, (CONST D3DXMATRIX *)pM);
}


// VECTOR4関係
inline FVECTOR *D3DXVec4Add(FVECTOR *pOut, CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return (FVECTOR *)D3DXVec4Add((D3DXVECTOR4 *)pOut, (CONST D3DXVECTOR4 *)pV1, (CONST D3DXVECTOR4 *)pV2);
}
inline FVECTOR *D3DXVec4Cross(FVECTOR *pOut, CONST FVECTOR *pV1, CONST FVECTOR *pV2, CONST FVECTOR *pV3) {
	return (FVECTOR *)D3DXVec4Cross((D3DXVECTOR4 *)pOut, (CONST D3DXVECTOR4 *)pV1, (CONST D3DXVECTOR4 *)pV2, (CONST D3DXVECTOR4 *)pV3);
}
inline FLOAT D3DXVec4Dot(CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return D3DXVec4Dot((CONST D3DXVECTOR4 *)pV1, (CONST D3DXVECTOR4 *)pV2);
}
inline FLOAT D3DXVec4Length(FVECTOR *pV) {
	return D3DXVec4Length((CONST D3DXVECTOR4 *)pV);
}
inline FVECTOR *D3DXVec4Normalize(FVECTOR *pv1, CONST FVECTOR *pv2) {
	return (FVECTOR *)D3DXVec4Normalize((D3DXVECTOR4 *)pv1, (D3DXVECTOR4 *)pv2);
}
inline FVECTOR *D3DXVec4Scale(FVECTOR *pOut, CONST FVECTOR *pV, FLOAT s) {
	return (FVECTOR *)D3DXVec4Scale((D3DXVECTOR4 *)pOut, (CONST D3DXVECTOR4 *)pV, s);
}
inline FVECTOR *D3DXVec4Subtract(FVECTOR *pOut, CONST FVECTOR *pV1, CONST FVECTOR *pV2) {
	return (FVECTOR *)D3DXVec4Subtract((D3DXVECTOR4 *)pOut, (CONST D3DXVECTOR4 *)pV1, (CONST D3DXVECTOR4 *)pV2);
}
inline FVECTOR *D3DXVec4Transform(FVECTOR *pOut, CONST FVECTOR *pV, CONST FMATRIX *pM) {
	return (FVECTOR *)D3DXVec4Transform((D3DXVECTOR4 *)pOut, (CONST D3DXVECTOR4 *)pV, (CONST D3DXMATRIX *)pM);
}


// MATRIX関係
inline FMATRIX *D3DXMatrixIdentity(FMATRIX *pOut) {
	return (FMATRIX *)D3DXMatrixIdentity((D3DXMATRIX *)pOut);
}
inline FMATRIX *D3DXMatrixInverse(FMATRIX *pOut, FLOAT *pDeterminant, CONST FMATRIX *pM) {
	return (FMATRIX *)D3DXMatrixInverse((D3DXMATRIX *)pOut, pDeterminant, (CONST D3DXMATRIX *)pM);
}
inline FMATRIX *D3DXMatrixMultiply(FMATRIX *pOut, CONST FMATRIX *pM1, CONST FMATRIX *pM2) {
	return (FMATRIX *)D3DXMatrixMultiply((D3DXMATRIX *)pOut, (CONST D3DXMATRIX *)pM1, (CONST D3DXMATRIX *)pM2);
}
inline FMATRIX *D3DXMatrixRotationX(FMATRIX* pOut, FLOAT Angle) {
	return (FMATRIX *)D3DXMatrixRotationX((D3DXMATRIX *)pOut, Angle);
}
inline FMATRIX *D3DXMatrixRotationY(FMATRIX* pOut, FLOAT Angle) {
	return (FMATRIX *)D3DXMatrixRotationY((D3DXMATRIX *)pOut, Angle);
}
inline FMATRIX *D3DXMatrixRotationZ(FMATRIX* pOut, FLOAT Angle) {
	return (FMATRIX *)D3DXMatrixRotationZ((D3DXMATRIX *)pOut, Angle);
}
inline FMATRIX *D3DXMatrixScaling(FMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz) {
	return (FMATRIX *)D3DXMatrixScaling((D3DXMATRIX *)pOut, sx, sy, sz);
}
inline FMATRIX *D3DXMatrixTranslation(FMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z) {
	return (FMATRIX *)D3DXMatrixTranslation((D3DXMATRIX *)pOut, x, y, z);
}
inline FMATRIX *D3DXMatrixTranspose(FMATRIX *pOut, CONST FMATRIX *pM) {
	return (FMATRIX *)D3DXMatrixTranspose((D3DXMATRIX *)pOut, (CONST D3DXMATRIX *)pM);
}

FVECTOR operator*(FMATRIX& mat, FVECTOR &vec);

#endif // __cplusplus


// とりあえずこの定義で。
#define VECTOR	FVECTOR
#define MATRIX	FMATRIX

#endif /* __MGSX_TYPE__H__ */


#include "xbox_ps2emu.h"
	
