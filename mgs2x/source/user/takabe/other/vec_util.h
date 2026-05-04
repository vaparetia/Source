/*
	vec_util.h

	演算補助ライブラリヘッダ

	2001/04/06 K.Takabe
	$Id: vec_util.h,v 1.1.1.3 2002/11/19 11:51:19 Yoshizawa1 Exp $

*/

#define VEC_UTIL_FORCE_NORMAL //BP_ASM

#ifdef __cplusplus
extern "C" {
#endif

/* 共通補助マクロ */
#define GTE_PI	((float)M_PI)
#define GTE_SetVector( _v0, _x, _y, _z, _w )	{ (_v0)->vx = _x ; (_v0)->vy = _y ; (_v0)->vz = _z ; (_v0)->vw = _w ; }
/* ＰＳ式角度を-2048~2047の範囲になるように正規化する */
#define GTE_REGULAR_ANGLE( _a )	( (signed short)( ((unsigned short)(_a)) << 4 ) >> 4 )
//#define GTE_PS2RAD( _a ) ({ float _r ; int _t = (_a) ; _t &= 4095 ; if ( _t & 2048 ) _t -= 4096 ; _r = _t / 2048.0f * GTE_PI ; _r ; })
#ifdef __GNUC__
#define GTE_PS2RAD( _a ) ({ float _r ; int _t = GTE_REGULAR_ANGLE(_a) ; _r = _t * ( GTE_PI / 2048.0f ) ; (_r) ; })
#define GTE_RAD2PS( _a ) ({ int _t = ( (_a) * ( 2048.0f / GTE_PI ) ); (_t); })
#else
static float inline GTE_PS2RAD( int _a ) { float _r ; int _t = GTE_REGULAR_ANGLE(_a) ; _r = _t * ( GTE_PI / 2048.0f ) ; return (_r) ; }
static int inline GTE_RAD2PS( float _a ) { int _t = ( (_a) * ( 2048.0f / GTE_PI ) ); return (_t); }
#endif

/* 共通プロトタイプ宣言 */
extern void GTE_InitGTE( void );


#if !defined(PSX2) || defined(VEC_UTIL_FORCE_NORMAL)
	/*
		移植用コード
	*/

/* グローバル変数 */
extern FVECTOR		_EE_VU0_ACC ;

/* 基本演算ライブラリ */
#define GTE_ZeroVector( _v0 )	{ (_v0)->vx = 0 ; (_v0)->vy = 0 ; (_v0)->vz = 0 ; (_v0)->vw = 1 ; }
#define GTE_UnitMatrix( _m0 )	_sceVu0UnitMatrix( _m0 )
#define GTE_AddVector( _v0, _v1, _v2 ) _sceVu0AddVector( _v0, _v1, _v2 )
#define GTE_SubVector( _v0, _v1, _v2 ) _sceVu0SubVector( _v0, _v1, _v2 )
#define GTE_MulVector( _v0, _v1, _v2 ) _sceVu0MulVector( _v0, _v1, _v2 )
#define GTE_ScaleVector( _v0, _v1, _s ) _sceVu0ScaleVector( _v0, _v1, _s )
#define GTE_InterVector( _v0, _v1, _v2, _s ) _sceVu0InterVector( _v0, _v2, _v1, _s )
#define GTE_InnerProduct( _v0, _v1 ) _sceVu0InnerProduct( _v0, _v1 )
#define GTE_OuterProduct( _v0, _v1, _v2 ) _sceVu0OuterProduct( _v0, _v1, _v2 )
#if 1
#define GTE_FTOI0Vector( _v0, _v1 ) _sceVu0FTOI0Vector( _v0, _v1 )
#else
static void __inline GTE_FTOI0Vector( IVECTOR *v0, FVECTOR *v1 ){
	v0->vx = (int)v1->vx ; v0->vy = (int)v1->vy ; v0->vz = (int)v1->vz ; v0->vw = (int)v1->vw ;
}
#endif
#define GTE_FTOI4Vector( _v0, _v1 ) _sceVu0FTOI4Vector( _v0, _v1 )
#define GTE_ITOF0Vector( _v0, _v1 ) _sceVu0ITOF0Vector( _v0, _v1 )
#define GTE_ITOF4Vector( _v0, _v1 ) _sceVu0ITOF4Vector( _v0, _v1 )
#define GTE_MaxVector( _v0, _v1, _v2 ) { (_v0)->vx = DG_MAX( (_v1)->vx, (_v2)->vx );\
										   (_v0)->vy = DG_MAX( (_v1)->vy, (_v2)->vy );\
										   (_v0)->vz = DG_MAX( (_v1)->vz, (_v2)->vz );\
										   (_v0)->vw = DG_MAX( (_v1)->vw, (_v2)->vw );}
#define GTE_MinVector( _v0, _v1, _v2 ) { (_v0)->vx = DG_MIN( (_v1)->vx, (_v2)->vx );\
										   (_v0)->vy = DG_MIN( (_v1)->vy, (_v2)->vy );\
										   (_v0)->vz = DG_MIN( (_v1)->vz, (_v2)->vz );\
										   (_v0)->vw = DG_MIN( (_v1)->vw, (_v2)->vw );}
#define GTE_Normalize( _v0, _v1 ) _sceVu0Normalize( _v0, _v1 )
#define GTE_ApplyMatrix( _v0, _m0, _v1 ) _sceVu0ApplyMatrix( _v0, _m0, _v1 )
#define GTE_MulMatrix( _m0, _m1, _m2 ) _sceVu0MulMatrix( _m0, _m1, _m2 )
#define GTE_InversMatrix( _m0, _m1 ) _sceVu0InversMatrix( _m0, _m1 )
extern void GTE_SinCos( FVECTOR *res, float angle );
extern void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle );
extern void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle );
/* マトリクススタック方式ライブラリ */
#define GTE_LoadMatrix( _m0 ) DG_SetPos( _m0 )
#define GTE_StoreMatrix( _m0 ) DG_GetPos( _m0 )
#define GTE_PutVector1( _v0, _v1 ) DG_PutVector( _v1, _v0, 1 )
#define GTE_RotVector1( _v0, _v1 ) DG_RotVector( _v1, _v0, 1 )
#define GTE_PutVector( _v0, _v1, _n ) DG_PutVector( _v1, _v0, _n )
#define GTE_RotVector( _v0, _v1, _n ) DG_RotVector( _v1, _v0, _n )
extern void GTE_PushMatrix( void );
extern void GTE_PopMatrix( void );
extern void GTE_LoadIdentity( void );
#define GTE_Translate( _v0 ) DG_MovePos( _v0 )
extern void GTE_MultMatrix( FMATRIX *m0 );
extern void GTE_RotateX( float a );
extern void GTE_RotateY( float a );
extern void GTE_RotateZ( float a );


#else
	/*
		ＰＳ２ネイティブコード
	*/

/*
	vf1~3:temp vec
	vf4~7:main matrix
	vf8~11:temp matrix
	vf12~15:temp matrix2
*/

/* 基本演算 */
static inline void GTE_ZeroVector( FVECTOR *v0 )
{
	asm volatile ("
		sqc2			vf0,0x00(%0)
	"::"r"(v0):"memory" );
}

static inline void GTE_UnitMatrix( FMATRIX *m0 )
{
	asm volatile ("
		sq				$0,0x00(%0)
		sq				$0,0x10(%0)
		sq				$0,0x20(%0)
		sq				$0,0x30(%0)
		swc1			%1,0x00(%0)	# キャッシュにヒットすることを予想して上書き
		swc1			%1,0x14(%0)
		swc1			%1,0x28(%0)
		swc1			%1,0x3c(%0)
	"::"r"(m0),"f"(1.0f):"memory" );
}

static inline void GTE_AddVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		vadd.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2):"memory" );
}

static inline void GTE_SubVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		vsub.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2):"memory" );
}

static inline void GTE_MulVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		vmul.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2):"memory" );
}

static inline void GTE_ScaleVector( FVECTOR *v0, FVECTOR *v1, float s )
{
	asm volatile("
		mfc1			$4,%2
		lqc2			vf1,0x00(%1)
		qmtc2			$4,vf2
		vmulx.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"f"(s):"$4","memory" );
}

static inline void GTE_InterVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2, float s )
{
	asm volatile("
		mfc1			$4,%3
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		qmtc2			$4,vf3
		vmulaw.xyzw		ACC,vf1,vf0
		vmsubax.xyzw	ACC,vf1,vf3
		vmaddx.xyzw		vf1,vf2,vf3
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2),"f"(s):"$4","memory" );
}

static inline float GTE_InnerProduct( FVECTOR *v0, FVECTOR *v1 )
{
	float	ret ;
	asm volatile ("
		mula.s		%1,%4
		madda.s		%2,%5
		madd.s		%0,%3,%6
	":"=f"(ret):"f"(v0->vx),"f"(v0->vy),"f"(v0->vz),"f"(v1->vx),"f"(v1->vy),"f"(v1->vz) );
	return ( ret );
}

static inline void GTE_OuterProduct( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		vopmula.xyz		ACC,vf1,vf2
		vopmsub.xyz		vf1,vf2,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2):"memory" );
}

static inline void GTE_FTOI0Vector( IVECTOR *v0, FVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		vftoi0.xyzw		vf1,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1):"memory" );
}

static inline void GTE_FTOI4Vector( IVECTOR *v0, FVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		vftoi4.xyzw		vf1,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1):"memory" );
}

static inline void GTE_ITOF0Vector( FVECTOR *v0, IVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		vitof0.xyzw		vf1,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1):"memory" );
}

static inline void GTE_ITOF4Vector( FVECTOR *v0, IVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		vitof4.xyzw		vf1,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1):"memory" );
}

static inline void GTE_MaxVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		vmax.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2):"memory" );
}

static inline void GTE_MinVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		lqc2			vf2,0x00(%2)
		vmini.xyzw		vf1,vf1,vf2
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1),"r"(v2):"memory" );
}

static inline void GTE_Normalize( FVECTOR *v0, FVECTOR *v1 )
{
	float	len ;
	len = GTE_InnerProduct( v1, v1 );
	len = DG_RSQRT( len );
	GTE_ScaleVector( v0, v1, len );
}

static inline void GTE_ApplyMatrix( FVECTOR *v0, FMATRIX *m0, FVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%2)
		lqc2			vf8,0x00(%1)
		lqc2			vf9,0x10(%1)
		lqc2			vf10,0x20(%1)
		lqc2			vf11,0x30(%1)
		vmulax.xyzw		ACC,vf8,vf1
		vmadday.xyzw	ACC,vf9,vf1
		vmaddaz.xyzw	ACC,vf10,vf1
		vmaddw.xyzw		vf1,vf11,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(m0),"r"(v1):"memory" );
}

static inline void GTE_MulMatrix( FMATRIX *m0, FMATRIX *m1, FMATRIX *m2 )
{
	asm volatile("
		lqc2			vf8,0x00(%1)
		lqc2			vf9,0x10(%1)
		lqc2			vf10,0x20(%1)
		lqc2			vf11,0x30(%1)
		lqc2			vf12,0x00(%2)
		lqc2			vf13,0x10(%2)
		lqc2			vf14,0x20(%2)
		lqc2			vf15,0x30(%2)
		vmulax.xyzw		ACC,vf8,vf12
		vmadday.xyzw	ACC,vf9,vf12
		vmaddaz.xyzw	ACC,vf10,vf12
		vmaddw.xyzw		vf1,vf11,vf12
		vmulax.xyzw		ACC,vf8,vf13
		vmadday.xyzw	ACC,vf9,vf13
		vmaddaz.xyzw	ACC,vf10,vf13
		vmaddw.xyzw		vf2,vf11,vf13
		sqc2			vf1,0x00(%0)
		vmulax.xyzw		ACC,vf8,vf14
		vmadday.xyzw	ACC,vf9,vf14
		vmaddaz.xyzw	ACC,vf10,vf14
		vmaddw.xyzw		vf1,vf11,vf14
		sqc2			vf2,0x10(%0)
		vmulax.xyzw		ACC,vf8,vf15
		vmadday.xyzw	ACC,vf9,vf15
		vmaddaz.xyzw	ACC,vf10,vf15
		vmaddw.xyzw		vf2,vf11,vf15
		sqc2			vf1,0x20(%0)
		sqc2			vf2,0x30(%0)
	"::"r"(m0),"r"(m1),"r"(m2):"memory" );
}

//static inline void GTE_InversMatrix( FMATRIX *m0, FMATRIX *m1 );


static inline void GTE_LoadMatrix( FMATRIX *m0 )
{
	asm volatile ("
		lqc2			vf4,0x00(%0)
		lqc2			vf5,0x10(%0)
		lqc2			vf6,0x20(%0)
		lqc2			vf7,0x30(%0)
	"::"r"(m0) );
}

static inline void GTE_StoreMatrix( FMATRIX *m0 )
{
	asm volatile ("
		sqc2			vf4,0x00(%0)
		sqc2			vf5,0x10(%0)
		sqc2			vf6,0x20(%0)
		sqc2			vf7,0x30(%0)
	"::"r"(m0):"memory" );
}

static inline void GTE_PutVector1( FVECTOR *v0, FVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		vmulax.xyzw		ACC,vf4,vf1
		vmadday.xyzw	ACC,vf5,vf1
		vmaddaz.xyzw	ACC,vf6,vf1
		vmaddw.xyzw		vf1,vf7,vf0
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1):"memory" );
}

static inline void GTE_RotVector1( FVECTOR *v0, FVECTOR *v1 )
{
	asm volatile("
		lqc2			vf1,0x00(%1)
		vmulax.xyzw		ACC,vf4,vf1
		vmadday.xyzw	ACC,vf5,vf1
		vmaddz.xyzw		vf1,vf6,vf1
		sqc2			vf1,0x00(%0)
	"::"r"(v0),"r"(v1):"memory" );
}

static inline void GTE_PutVector( FVECTOR *v0, FVECTOR *v1, int n )
{
	asm volatile("
		lqc2			vf1,0(%1)
0:
		vmulax.xyzw		ACC,vf4,vf1
		vmadday.xyzw	ACC,vf5,vf1
		vmaddaz.xyzw	ACC,vf6,vf1
		vmaddw.xyzw		vf2,vf7,vf0
		addi			%2,%2,-1
		addi			%0,%0,16
		addi			%1,%1,16
		lqc2			vf1,0(%1)
		sqc2			vf2,-16(%0)
		bgtz			%2,0b
	":"+r"(v0),"+r"(v1),"+r"(n)::"memory" );
}

static inline void GTE_RotVector( FVECTOR *v0, FVECTOR *v1, int n )
{
	asm volatile("
		lqc2			vf1,0(%1)
0:
		vmulax.xyzw		ACC,vf4,vf1
		vmadday.xyzw	ACC,vf5,vf1
		vmaddz.xyzw		vf2,vf6,vf1
		addi			%2,%2,-1
		addi			%0,%0,16
		addi			%1,%1,16
		lqc2			vf1,0(%1)
		sqc2			vf2,-16(%0)
		bgtz			%2,0b
	":"+r"(v0),"+r"(v1),"+r"(n)::"memory" );
}

static inline void GTE_PushMatrix( void )
{
	asm volatile ("
		vsqd.xzyw		vf07,(--vi15)
		vsqd.xzyw		vf06,(--vi15)
		vsqd.xzyw		vf05,(--vi15)
		vsqd.xzyw		vf04,(--vi15)
	");
}

static inline void GTE_PopMatrix( void )
{
	asm volatile ("
		vlqi.xzyw		vf04,(vi15++)
		vlqi.xzyw		vf05,(vi15++)
		vlqi.xzyw		vf06,(vi15++)
		vlqi.xzyw		vf07,(vi15++)
	");
}

static inline void GTE_LoadIdentity( void )
{
	asm volatile("
		vmulx.xyzw		vf4,vf0,vf0
		vmulx.xyzw		vf5,vf0,vf0
		vmulx.xyzw		vf6,vf0,vf0
		vmove.xyzw		vf7,vf0
		vaddw.x			vf4,vf0,vf0
		vaddw.y			vf5,vf0,vf0
		vaddw.z			vf6,vf0,vf0
	" );
}

static inline void GTE_Translate( FVECTOR *v0 )
{
	asm volatile("
		lqc2			vf1,0x00(%0)
		vmulax.xyzw		ACC,vf4,vf1
		vmadday.xyzw	ACC,vf5,vf1
		vmaddaz.xyzw	ACC,vf6,vf1
		vmaddw.xyzw		vf7,vf7,vf0
	"::"r"(v0) );
}

static inline void GTE_MultMatrix( FMATRIX *m0 )
{
	asm volatile("
		lqc2			vf12,0x00(%0)
		lqc2			vf13,0x10(%0)
		lqc2			vf14,0x20(%0)
		lqc2			vf15,0x30(%0)
		vmove.xyzw		vf8,vf4
		vmove.xyzw		vf9,vf5
		vmove.xyzw		vf10,vf6
		vmove.xyzw		vf11,vf7
		vmulax.xyzw		ACC,vf8,vf12
		vmadday.xyzw	ACC,vf9,vf12
		vmaddaz.xyzw	ACC,vf10,vf12
		vmaddw.xyzw		vf4,vf11,vf12
		vmulax.xyzw		ACC,vf8,vf13
		vmadday.xyzw	ACC,vf9,vf13
		vmaddaz.xyzw	ACC,vf10,vf13
		vmaddw.xyzw		vf5,vf11,vf13
		vmulax.xyzw		ACC,vf8,vf14
		vmadday.xyzw	ACC,vf9,vf14
		vmaddaz.xyzw	ACC,vf10,vf14
		vmaddw.xyzw		vf6,vf11,vf14
		vmulax.xyzw		ACC,vf8,vf15
		vmadday.xyzw	ACC,vf9,vf15
		vmaddaz.xyzw	ACC,vf10,vf15
		vmaddw.xyzw		vf7,vf11,vf15
	"::"r"(m0) );
}

extern void GTE_RotateX( float a );
extern void GTE_RotateY( float a );
extern void GTE_RotateZ( float a );
static inline void GTE_InversMatrix( FMATRIX *m0, FMATRIX *m1 )
{
	GTE_PushMatrix();
	_sceVu0InversMatrix( m0, m1 );
	GTE_PopMatrix();
}
extern void GTE_SinCos( FVECTOR *res, float angle );
extern void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle );
extern void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle );


#endif




#ifdef __cplusplus
}
#endif

