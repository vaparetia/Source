//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    test_slater.c
	舟りんテスト
	2000/02/07 T.Shibata
	
	$Id: demo_slater.c,v 1.1.1.3 2002/11/19 11:48:49 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
//#include	"sea_slater.h"

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )


#define		N_SSLTR_INTEX	(8)
#define		N_MOVE_SSLTR	(64)
#define		MOVE_TIME		(4096/64)
#define		INDEX_ADD_TIME	(1)
//[0-7]index
//[8-19]timer
//[20-31]rot
#define		GET_DATA_ROT(_data)		( ((*(u_int*)&(_data))>>20) )
#define		GET_DATA_TIMER(_data)	( ((*(u_int*)&(_data))>>8)&0x0fff )
#define		GET_DATA_INDEX(_data)	( (*(u_int*)&(_data))&0xff )


//[0-11] timer
//[12]	move flag
#define		CHECK_FLAGMOVE(_flags)		((_flags)&0x0001)
#define		GET_ESCAPE_FLAGS(_data)		((*(u_int*)&(_data))>>12)
#define		GET_ESCAPE_TIMER(_data)		((*(u_int*)&(_data))&0x0fff)

#define		SET_DATA(_rot,_time,_index)	(((_rot)<<20)|((_time)<<8)|(_index))
#define		SET_ESCAPE_DATA(_flags,_time)	(((_flags)<<12)|(_time))

#define		MEM_SCR_TOP		((void*)(SCRPAD_ADDR))

#define		N_SSLTS			(512)
#define		N_SSLT_GROUP	(32)
#define		N_SSLT_IN_GROUP	(N_SSLTS/N_SSLT_GROUP)

#define		N_MAX_REFUGE		(8)

typedef struct {
	u_short		index;
	u_short		rot;
	u_short		timer;
	u_short		pad;
} SSLTR_DATA;

typedef struct _work {
	GV_ACT_EX		actor;

	int				name;
	int				map;
	int				hzx_group;
	int				n_ssltr;		//テクスチャー枚数(コモデルは×N_SSLT_INTEX)
	void			*next;
	
	DG_COMDL		*low_cm;
	int				flags;
	int				n_refuge;
	ALIGN16_PRE SSLTR_DATA		ssltr_data[N_SSLTS] ALIGN16_POST;

	short			group_timer[N_SSLT_GROUP];
	short			group_index[N_SSLT_GROUP];
	short			group_n_act[N_SSLT_GROUP];
	short			group_max_act[N_SSLT_GROUP];
	short			group_wait[N_SSLT_GROUP];
	u_short			group_flags[N_SSLT_GROUP];

	FVECTOR			alltrap_center;
	FVECTOR			alltrap_wide;

	FVECTOR			refuge_center[N_MAX_REFUGE][2];
	FVECTOR			refuge_wide[N_MAX_REFUGE][2];

	FVECTOR			trap_scale[N_SSLT_GROUP];
	FVECTOR			trap_center[N_SSLT_GROUP];
	
	FVECTOR			trap_now_pos[N_SSLT_GROUP];
	FVECTOR			trap_to_pos[N_SSLT_GROUP];
	
	FVECTOR			trap_wide[N_SSLT_GROUP];
	
//	DG_COMDL		*high_cm;
} Work;

#define		GROUP_FLAG_HASTY		(0x0100)
#define		GROUP_FLAG_REFUGE		(0x0200)
#define		GROUP_FLAG_ESCAPE		(0x0400)

extern float GetCheckNearPosLen( FVECTOR *out_pos, FVECTOR *out_vec, FVECTOR *in );
extern FVECTOR *GetCheckSltrTramplePos();
extern int GetCheckSltrTrampleNum();
extern FVECTOR *GetCheckSltrBulletPos();
extern int GetCheckSltrBulletNum();
extern void CheckSltrBlast( FVECTOR *sltr, FVECTOR *wide );

extern void SetTrmplSlater( FVECTOR *center, FMATRIX*, float size );

extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
//extern void *NewSquareView( FVECTOR*, int, u_char, u_char, u_char );
extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define N_SSLTRMOVE_INDEX	(sizeof(SsltrMovePos)/sizeof(SsltrMovePos[0]) - 2)
//static float RndTable[8] = { 1.2f, 1.0f, 0.9f, 0.8f, 1.1f, 1.05f, 0.95f, 1.075f };
static float RndTable[8] = { 1.2f, 1.0f, 0.9f, 0.8f, 1.1f, 1.05f, 1.01f, 0.5f };

#define		SIZE_MUL	(1.0f)


static FVECTOR SsltrMovePos[] = {
	{-147.0f,0.0f,16.0f,1.0f},
	{-110.0f,0.0f,-145.0f,1.0f},
	{-38.0f,0.0f,-138.0f,1.0f},
	{46.0f,0.0f,58.0f,1.0f},
	{-19.0f,0.0f,100.0f,1.0f},
	{-143.0f,0.0f,-104.0f,1.0f},
	{-43.0f,0.0f,-59.0f,1.0f},
	{135.0f,0.0f,-144.0f,1.0f},
	{149.0f,0.0f,40.0f,1.0f},
	{-72.0f,0.0f,-30.0f,1.0f},
	{-154.0f,0.0f,152.0f,1.0f},
	{-79.0f,0.0f,93.0f,1.0f},
	{-152.0f,0.0f,-73.0f,1.0f},
	{17.0f,0.0f,-125.0f,1.0f},
	{95.0f,0.0f,-73.0f,1.0f},
	{31.0f,0.0f,142.0f,1.0f},
	{93.0f,0.0f,154.0f,1.0f},
	{151.0f,0.0f,69.0f,1.0f},
	{-15.0f,0.0f,-127.0f,1.0f},
	{-47.0f,0.0f,10.0f,1.0f},
	{82.0f,0.0f,95.0f,1.0f},
	{-83.0f,0.0f,153.0f,1.0f},
	{72.0f,0.0f,-149.0f,1.0f},
	{152.0f,0.0f,-104.0f,1.0f},
	{125.0f,0.0f,-62.0f,1.0f},
	{-151.0f,0.0f,69.0f,1.0f},
	{149.0f,0.0f,141.0f,1.0f},
	{113.0f,0.0f,46.0f,1.0f},
	{-60.0f,0.0f,68.0f,1.0f},
	{-85.0f,0.0f,-122.0f,1.0f},
	{41.0f,0.0f,-150.0f,1.0f},
	{90.0f,0.0f,-108.0f,1.0f},
	{-145.0f,0.0f,95.0f,1.0f},
	{14.0f,0.0f,150.0f,1.0f},
	{21.0f,0.0f,32.0f,1.0f},
	{149.0f,0.0f,-52.0f,1.0f},
	{-157.0f,0.0f,-21.0f,1.0f},
	{-36.0f,0.0f,44.0f,1.0f},
	{93.0f,0.0f,34.0f,1.0f},
	{109.0f,0.0f,-147.0f,1.0f},
	{-154.0f,0.0f,-149.0f,1.0f},
	{-12.0f,0.0f,75.0f,1.0f},
	{106.0f,0.0f,73.0f,1.0f},
	{128.0f,0.0f,-7.0f,1.0f},
	{-53.0f,0.0f,-86.0f,1.0f},
	{-81.0f,0.0f,67.0f,1.0f},
	{-34.0f,0.0f,116.0f,1.0f},
	{136.0f,0.0f,155.0f,1.0f},
	{66.0f,0.0f,-69.0f,1.0f},
	{117.0f,0.0f,-101.0f,1.0f},
	{154.0f,0.0f,-24.0f,1.0f},
	{-105.0f,0.0f,141.0f,1.0f},
	{-97.0f,0.0f,-59.0f,1.0f},
	{149.0f,0.0f,-156.0f,1.0f},
	{-14.0f,0.0f,-46.0f,1.0f},
	{-20.0f,0.0f,154.0f,1.0f},
	{-152.0f,0.0f,45.0f,1.0f},
	{31.0f,0.0f,-33.0f,1.0f},
	{54.0f,0.0f,24.0f,1.0f},
	{151.0f,0.0f,101.0f,1.0f},
	{-145.0f,0.0f,107.0f,1.0f},
	{-120.0f,0.0f,-42.0f,1.0f},
	{63.0f,0.0f,-79.0f,1.0f},
	{36.0f,0.0f,90.0f,1.0f},
	{-147.0f,0.0f,16.0f,1.0f},
	{-110.0f,0.0f,-145.0f,1.0f},
};

static inline void _RotTrans( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) :"memory" );
#else
	_sceVu0ApplyMatrix( out, world, in );
#endif
}

//32匹中4匹動く
static Work *EmmSsltrList = NULL;

static void AddEmmSsltr( Work *work )
{
	if( !work ) return;

	work->next = EmmSsltrList;
	EmmSsltrList = work;
}

static void SubEmmSsltr( Work *work )
{
	Work *tmp = EmmSsltrList;

	if( !work ) return;
	
	if( EmmSsltrList == work ){
		EmmSsltrList = work->next;
		return;
	}

	for(;tmp->next;tmp = tmp->next ){
		if( tmp->next == work ){
			tmp->next = work;
			return;
		}
	}
}

static inline float _Vu0VecLenXYZ( FVECTOR *vec1, FVECTOR *vec2 )
{
	float	ans;

#ifdef BP_PSX2_ASM
	asm volatile ("
 	 	  lqc2			vf12, 0x00(%1)
 	 	  lqc2			vf13, 0x00(%2)
		  
		  vsub.xyzw		vf14, vf12, vf13
		  
		  vmul.xyz		vf15, vf14, vf14
		  vmulax.w		ACC, vf0, vf15x
		  vmadday.w		ACC, vf0, vf15y
		  vmaddz.w		vf12, vf0, vf15z

		  vaddw.x		vf13, vf0, vf12

		  qmfc2.i		$8,vf13
		  sw			$8,0(%0)
	": : "r"(&ans), "r"(vec1), "r"(vec2) : "$8", "memory" );
#else
	FVECTOR tmp ;

	_sceVu0SubVector( &tmp, vec1, vec2 );
	_sceVu0MulVector( &tmp, &tmp, &tmp );
	ans = tmp.vx + tmp.vy + tmp.vz ;
#endif
	return (ans);
}

static inline float _Vu0VecLenXZ( FVECTOR *vec1, FVECTOR *vec2 )
{
	float	ans;

#ifdef BP_PSX2_ASM
	asm volatile ("
 	 	  lqc2			vf12, 0x00(%1)
 	 	  lqc2			vf13, 0x00(%2)
		  
		  vsub.xyzw		vf14, vf12, vf13
		  
		  vmul.xyz		vf15, vf14, vf14
		  vmulax.w		ACC, vf0, vf15x
		  #vmadday.w		ACC, vf0, vf15y
		  vmaddz.w		vf12, vf0, vf15z

		  vaddw.x		vf13, vf0, vf12

		  qmfc2.i		$8,vf13
		  sw			$8,0(%0)
	": : "r"(&ans), "r"(vec1), "r"(vec2) : "$8", "memory" );
#else
	FVECTOR tmp ;

	_sceVu0SubVector( &tmp, vec1, vec2 );
	_sceVu0MulVector( &tmp, &tmp, &tmp );
	ans = tmp.vx + tmp.vz ;
#endif

	return (ans);
}

#if 0 //BP_UNUSED

#ifdef PSX2 //yano /*関数名がかちあうため切りました。柴田了解済み*/
float GetEmmSsltrNearLen2( FVECTOR *pos )
{
	Work	*work = EmmSsltrList;
	int		i;
	float	min = FLOAT_MAX;

	for( ; work; work = work->next ){
		FVECTOR *center = work->trap_center;
		u_short *group_flags = work->group_flags;
		
		for( i = 0; i < N_SSLT_GROUP; i++ ){
			if( !(group_flags[0] & GROUP_FLAG_REFUGE) ){
				float len = _Vu0VecLenXYZ( pos, center );
				if( len < min ) min = len;
			}
			center++;
			group_flags++;
		}
	}
	return min;
}
#endif

#endif

#ifndef BP_PS2
static FMATRIX *Mat2, *Mat3 ;
#endif

static void LoadMatHermiteLerp()
{
	static FMATRIX c_mat2 = {{
		{ 2.0f,-2.0f, 1.0f, 1.0f},
		{-3.0f, 3.0f,-2.0f,-1.0f},
		{ 0.0f, 0.0f, 1.0f, 0.0f},
		{ 1.0f, 0.0f, 0.0f, 0.0f}
	}};
	static FMATRIX c_mat3 = {{
		{ 0.0f, 0.0f, 0.0f, 0.0f},
		{ 6.0f,-6.0f, 3.0f, 3.0f},
		{-6.0f, 6.0f,-4.0f,-2.0f},
		{ 0.0f, 0.0f, 1.0f, 0.0f}
	}};

#ifdef BP_PSX2_ASM
	asm volatile("
		lqc2			vf04,0x00(%0)	# 定数マトリクス読み込み
		lqc2			vf05,0x10(%0)	# 定数マトリクス読み込み
		lqc2			vf06,0x20(%0)	# 定数マトリクス読み込み
		lqc2			vf07,0x30(%0)	# 定数マトリクス読み込み

		lqc2			vf08,0x00(%1)	# 定数マトリクス読み込み
		lqc2			vf09,0x10(%1)	# 定数マトリクス読み込み
		lqc2			vf10,0x20(%1)	# 定数マトリクス読み込み
		lqc2			vf11,0x30(%1)	# 定数マトリクス読み込み
	"::"r"(&c_mat2),"r"(&c_mat3));
#else
	Mat2 = &c_mat2;
	Mat3 = &c_mat3;
#endif

}

#ifndef BP_PS2
static void MakeHermiteMat( FMATRIX *out_mat, FMATRIX *rot_mat, FVECTOR *pos )
{
	FVECTOR tmp ;
	
	DG_SetPos( rot_mat ) ;
	DG_RotVector(pos, (FVECTOR *)out_mat->m[0], 2 ) ;
	out_mat->m[0][3] = 1.0f;
	out_mat->m[1][3] = 1.0f;
	DG_RotVector((FVECTOR *)(pos+2), &tmp, 1 ) ;

	_sceVu0SubVector( (FVECTOR *)out_mat->m[2], (FVECTOR *)out_mat->m[1], (FVECTOR *)out_mat->m[0] );
	out_mat->m[2][3] = 0.0f ;
	_sceVu0SubVector( (FVECTOR *)out_mat->m[3], &tmp,                     (FVECTOR *)out_mat->m[0] );
	out_mat->m[3][3] = 0.0f ;
}

static void HermiteLerpVec( FVECTOR *out_vec1, FVECTOR *out_vec2, FMATRIX *mat, float her_t )
{
	FMATRIX	m1, m2 ;
	FVECTOR her_vec ;

	_sceVu0MulMatrix( &m1, mat, Mat2 );
	_sceVu0MulMatrix( &m2, mat, Mat3 );
	
	her_vec.vx = her_t*her_t*her_t ;
	her_vec.vy = her_t*her_t ;
	her_vec.vz = her_t ;
	her_vec.vw = 1.0f ;

	DG_SetPos( &m1 ) ;
	DG_PutVector( &her_vec, out_vec1, 1 ) ;

	DG_SetPos( &m2 ) ;
	DG_PutVector( &her_vec, out_vec2, 1 ) ;
}

#endif

static void SetSsMatrix_Normal( FMATRIX *out_mat,
								FMATRIX *rot_mat, FVECTOR *pos,
								float her_t,
								FVECTOR *outer, FVECTOR *data, float scale )
{
#ifdef BP_PSX2_ASM
	asm volatile("
		lqc2			vf16,0x00(%1)	# RotMatrix ..x
		lqc2			vf20,0x00(%2)	# pos0
		lqc2			vf17,0x10(%1)	# RotMatrix .. y
		lqc2			vf18,0x20(%1)	# RotMatrix .. z
		lqc2			vf21,0x10(%2)	# pos1
		lqc2			vf22,0x20(%2)	# pos2

		vmulax.xyzw		ACC, vf16,vf20	#pos0
		vmadday.xyzw	ACC, vf17,vf20
		vmaddz.xyzw		vf12, vf18,vf20
		vmulw.w			vf12, vf0,vf0	#pos0.vw = 1.0f
		vmulax.xyzw		ACC, vf16,vf21	#pos1
		vmadday.xyzw	ACC, vf17,vf21
		vmaddz.xyzw		vf13, vf18,vf21
		vmulw.w			vf13, vf0,vf0	#pos1.vw = 1.0f
		vmulax.xyzw		ACC, vf16,vf22	#pos2
		vmadday.xyzw	ACC, vf17,vf22
		vmaddz.xyzw		vf24, vf18,vf22
		vsub.xyz		vf14, vf13,vf12	#pos1 - pos0
		vmulx.w			vf14, vf0,vf0	#vec0.vw = 0.0f
		vsub.xyz		vf15, vf24,vf12	#pos2 - pos1
		vmulx.w			vf15, vf0,vf0	#vec1.vw = 0.0f

		qmtc2.ni		%5,vf01			#HermiteLerpVecここから

		vmulax.xyzw		ACC ,vf12,vf04	#
		vmadday.xyzw	ACC ,vf13,vf04
		vmaddaz.xyzw	ACC ,vf14,vf04
		vmaddw.xyzw		vf16,vf15,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf05
		vmadday.xyzw	ACC ,vf13,vf05
		vmaddaz.xyzw	ACC ,vf14,vf05
		vmaddw.xyzw		vf17,vf15,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf06
		vmadday.xyzw	ACC ,vf13,vf06
		vmaddaz.xyzw	ACC ,vf14,vf06
		vmaddw.xyzw		vf18,vf15,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf12,vf07
		vmadday.xyzw	ACC ,vf13,vf07
		vmaddaz.xyzw	ACC ,vf14,vf07
		vmaddw.xyzw		vf19,vf15,vf07

		vmulax.xyzw		ACC ,vf12,vf08
		vmadday.xyzw	ACC ,vf13,vf08
		vmaddaz.xyzw	ACC ,vf14,vf08
		vmaddw.xyzw		vf20,vf15,vf08
		vmulax.xyzw		ACC ,vf12,vf09
		vmadday.xyzw	ACC ,vf13,vf09
		vmaddaz.xyzw	ACC ,vf14,vf09
		vmaddw.xyzw		vf21,vf15,vf09
		vmulax.xyzw		ACC ,vf12,vf10
		vmadday.xyzw	ACC ,vf13,vf10
		vmaddaz.xyzw	ACC ,vf14,vf10
		vmaddw.xyzw		vf22,vf15,vf10
		vmulax.xyzw		ACC ,vf12,vf11
		vmadday.xyzw	ACC ,vf13,vf11
		vmaddaz.xyzw	ACC ,vf14,vf11
		vmaddw.xyzw		vf23,vf15,vf11

		vmulax.xyzw		ACC ,vf20,vf02	#vec
		vmadday.xyzw	ACC ,vf21,vf02
		vmaddaz.xyzw	ACC ,vf22,vf02
		vmaddw.xyzw		vf25,vf23,vf00
		vmulax.xyzw		ACC ,vf16,vf02	#pos
		vmadday.xyzw	ACC ,vf17,vf02
		vmaddaz.xyzw	ACC ,vf18,vf02
		vmaddw.xyzw		vf24,vf19,vf00

		lqc2		vf16 ,0x00(%4)	#data
		lqc2		vf13 ,0x00(%3)	#outer
		qmtc2.ni	%6,vf17

		vmul.xyz	vf1,vf25,vf25	#vec normalize
		vmulax.w	ACC,vf0,vf1
		vmadday.w	ACC,vf0,vf1
		vmaddz.w	vf1,vf0,vf1
		vrsqrt		Q,vf0w,vf1w
		vwaitq
		vmulq.xyz	vf14,vf25,Q

		vopmula.xyz	ACC,vf14,vf13
		vopmsub.xyz	vf12,vf13,vf14

		vadd.xyz	vf15,vf24,vf16
		vmulx.w		vf12,vf0,vf0	#out_mat x.vw = 0.0f

		vmulx.xyz		vf14,vf14,vf17	#mul matrix
		vmulx.xyz		vf13,vf13,vf17	#mul matrix
		vmulx.xyz		vf12,vf12,vf17	#mul matrix

		vmulw.w		vf15,vf0,vf0	#out_mat w.vw = 1.0f

		sqc2		vf12 ,0x00(%0)
		sqc2		vf13 ,0x10(%0)
		sqc2		vf14 ,0x20(%0)
		sqc2		vf15 ,0x30(%0)
	": : "r"(out_mat), "r"(rot_mat), "r"(pos), "r"(outer), "r"(data), "r"(her_t), "r"(scale) );
#else
	FMATRIX	m1 ;
	FVECTOR vec, vec2, opvec, her_vec1, her_vec2 ;
	float q;

	MakeHermiteMat( &m1, rot_mat, pos ) ;
	HermiteLerpVec( &her_vec1, &her_vec2, &m1, her_t ) ;

	_sceVu0Normalize( &vec, &her_vec2 );
	vec.vw = 0.0f;
	_sceVu0OuterProduct( &opvec, &vec, outer );	
	_sceVu0ScaleVector( (FVECTOR *)out_mat->m[0], &opvec, scale ) ;
	out_mat->m[0][3] = 0.0f ;
	_sceVu0ScaleVector( (FVECTOR *)out_mat->m[1], outer, scale ) ;
	_sceVu0ScaleVector( (FVECTOR *)out_mat->m[2], &vec, scale ) ;
	_sceVu0AddVector( (FVECTOR *)out_mat->m[3], &her_vec1, data );
	out_mat->m[3][3] = 1.0f ;
#endif
}


static void SetSsMatrix_Refuge( FMATRIX *out_mat,
								FMATRIX *rot_mat, FVECTOR *pos, float her_t,
								FVECTOR *outer, FVECTOR *center, FVECTOR *dvec, float scale, FVECTOR *debug )
{

#ifdef BP_PSX2_ASM
	asm volatile("
		lqc2			vf16,0x00(%1)	# RotMatrix ..x
		lqc2			vf20,0x00(%2)	# pos0
		lqc2			vf17,0x10(%1)	# RotMatrix .. y
		lqc2			vf18,0x20(%1)	# RotMatrix .. z
		lqc2			vf21,0x10(%2)	# pos1
		lqc2			vf22,0x20(%2)	# pos2

		vmulax.xyzw		ACC, vf16,vf20	#pos0
		vmadday.xyzw	ACC, vf17,vf20
		vmaddz.xyzw		vf12, vf18,vf20
		vmulw.w			vf12, vf0,vf0	#pos0.vw = 1.0f
		vmulax.xyzw		ACC, vf16,vf21	#pos1
		vmadday.xyzw	ACC, vf17,vf21
		vmaddz.xyzw		vf13, vf18,vf21
		vmulw.w			vf13, vf0,vf0	#pos1.vw = 1.0f
		vmulax.xyzw		ACC, vf16,vf22	#pos2
		vmadday.xyzw	ACC, vf17,vf22
		vmaddz.xyzw		vf24, vf18,vf22
		vsub.xyz		vf14, vf13,vf12	#pos1 - pos0
		vmulx.w			vf14, vf0,vf0	#vec0.vw = 0.0f
		vsub.xyz		vf15, vf24,vf12	#pos2 - pos1
		vmulx.w			vf15, vf0,vf0	#vec1.vw = 0.0f

		qmtc2.ni		%6,vf01			#HermiteLerpVecここから

		vmulax.xyzw		ACC ,vf12,vf04	#
		vmadday.xyzw	ACC ,vf13,vf04
		vmaddaz.xyzw	ACC ,vf14,vf04
		vmaddw.xyzw		vf16,vf15,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf05
		vmadday.xyzw	ACC ,vf13,vf05
		vmaddaz.xyzw	ACC ,vf14,vf05
		vmaddw.xyzw		vf17,vf15,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf06
		vmadday.xyzw	ACC ,vf13,vf06
		vmaddaz.xyzw	ACC ,vf14,vf06
		vmaddw.xyzw		vf18,vf15,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf12,vf07
		vmadday.xyzw	ACC ,vf13,vf07
		vmaddaz.xyzw	ACC ,vf14,vf07
		vmaddw.xyzw		vf19,vf15,vf07

		vmulax.xyzw		ACC ,vf12,vf08
		vmadday.xyzw	ACC ,vf13,vf08
		vmaddaz.xyzw	ACC ,vf14,vf08
		vmaddw.xyzw		vf20,vf15,vf08
		vmulax.xyzw		ACC ,vf12,vf09
		vmadday.xyzw	ACC ,vf13,vf09
		vmaddaz.xyzw	ACC ,vf14,vf09
		vmaddw.xyzw		vf21,vf15,vf09
		vmulax.xyzw		ACC ,vf12,vf10
		vmadday.xyzw	ACC ,vf13,vf10
		vmaddaz.xyzw	ACC ,vf14,vf10
		vmaddw.xyzw		vf22,vf15,vf10
		vmulax.xyzw		ACC ,vf12,vf11
		vmadday.xyzw	ACC ,vf13,vf11
		vmaddaz.xyzw	ACC ,vf14,vf11
		vmaddw.xyzw		vf23,vf15,vf11

		vmulax.xyzw		ACC ,vf16,vf02	#pos
		vmadday.xyzw	ACC ,vf17,vf02
		vmaddaz.xyzw	ACC ,vf18,vf02
		vmaddw.xyzw		vf24,vf19,vf00	#pos
		vmulax.xyzw		ACC ,vf20,vf02	#vec
		vmadday.xyzw	ACC ,vf21,vf02
		vmaddaz.xyzw	ACC ,vf22,vf02
		vmaddw.xyzw		vf25,vf23,vf00	#vec

		lqc2		vf17 ,0x00(%5)	#dvec
		lqc2		vf16 ,0x00(%4)	#center
		lqc2		vf13 ,0x00(%3)	#outer
		qmtc2.ni	%7,vf18			#scale

		vadd.xyzw	vf26, vf25,  vf17	#グループの移動量を考慮

		sqc2		vf25 ,0x00(%8)
		sqc2		vf17 ,0x10(%8)
		sqc2		vf26 ,0x20(%8)

		vmul.xyz	vf01, vf26,  vf26	#vec normalize
		vmulax.w	ACC,  vf00,  vf01
		vmadday.w	ACC,  vf00,  vf01
		vmaddz.w	vf1,  vf00,  vf01
		vrsqrt		Q,    vf00w, vf01w
		vwaitq
		vmulq.xyz	vf14, vf26, Q

		vopmula.xyz	ACC,vf14,vf13
		vopmsub.xyz	vf12,vf13,vf14

		vadd.xyz	vf15,vf24,vf16
		vmulx.w		vf12,vf0,vf0	#out_mat x.vw = 0.0f

		vmulx.xyz		vf14,vf14,vf18	#mul matrix
		vmulx.xyz		vf13,vf13,vf18	#mul matrix
		vmulx.xyz		vf12,vf12,vf18	#mul matrix

		vmulw.w		vf15,vf0,vf0	#out_mat w.vw = 1.0f

		sqc2		vf12 ,0x00(%0)
		sqc2		vf13 ,0x10(%0)
		sqc2		vf14 ,0x20(%0)
		sqc2		vf15 ,0x30(%0)

	": : "r"(out_mat), "r"(rot_mat), "r"(pos), "r"(outer), "r"(center), "r"(dvec), "r"(her_t), "r"(scale), "r"(debug) );
#else
	FMATRIX	m1 ;
	FVECTOR vec, vec2, opvec, her_vec1, her_vec2 ;
	float q;

	MakeHermiteMat( &m1, rot_mat, pos ) ;
	HermiteLerpVec( &her_vec1, &her_vec2, &m1, her_t ) ;

	_sceVu0AddVector( &her_vec2, &her_vec2, dvec );

	_sceVu0Normalize( &vec, &her_vec2 );
	vec.vw = 0.0f;
	
	_sceVu0OuterProduct( &opvec, &vec, outer );
	opvec.vw = 0.0f ;
	
	_sceVu0AddVector( (FVECTOR *)out_mat->m[3], &her_vec1, center );
	out_mat->m[0][3] = 0.0f ;
	
	_sceVu0ScaleVectorXYZ( (FVECTOR *)out_mat->m[2], &vec, scale ) ;
	out_mat->m[2][3] = 0.0f;
	_sceVu0ScaleVectorXYZ( (FVECTOR *)out_mat->m[1], outer, scale ) ;
	_sceVu0ScaleVectorXYZ( (FVECTOR *)out_mat->m[0], &opvec, scale ) ;
	out_mat->m[3][3] = 1.0f ;	
#endif
}


#ifndef BP_PS2
static FVECTOR _tmpvf[8] ;
// clipw エミュレート
static int Vclip4( FVECTOR *verts )
{
	int i, clip_flag ;
	float  w;

	clip_flag = 0 ;
	for ( i=0; i<4; i++ ) {
		if ( i ) clip_flag <<= 6 ;
		w = DG_FABS(verts->vw);
		if (verts->vx >   w) clip_flag |= CLIP_X0_FLAG;
		if (verts->vx < - w) clip_flag |= CLIP_X1_FLAG;
		if (verts->vy >   w) clip_flag |= CLIP_Y0_FLAG;
		if (verts->vy < - w) clip_flag |= CLIP_Y1_FLAG;
		if (verts->vz >   w) clip_flag |= CLIP_Z0_FLAG;
		if (verts->vz < - w) clip_flag |= CLIP_Z1_FLAG;
	}
	return clip_flag ;
}
#endif
/* 指定したカメラの視界内に入っているかをチェック（カメラフラグ付き） */
static int BoundCheckFlag( FVECTOR *bound_max, FVECTOR *bound_min, int flag )
{
	int		clip_flag = 0, clip_flag1, clip_flag2, check ;

	clip_flag1 = clip_flag2 = 0 ;

#ifdef BP_PSX2_ASM
	// バウンディングを８頂点に分解する
	asm volatile("
		lqc2			vf20,0x00(%0)
		lqc2			vf21,0x00(%1)

		vadd.xyzw		vf12, vf20, vf21
		vsub.xyzw		vf19, vf20, vf21

		vmove.yz		vf13,vf12
		vmove.xz		vf14,vf12
		vmove.z			vf15,vf12
		vmove.xy		vf16,vf12
		vmove.y			vf17,vf12
		vmove.x			vf18,vf12
		vmove.x			vf13,vf19
		vmove.y			vf14,vf19
		vmove.xy		vf15,vf19
		vmove.z			vf16,vf19
		vmove.xz		vf17,vf19
		vmove.yz		vf18,vf19
	"::"r"(bound_max),"r"(bound_min) : "memory");
#else
	_sceVu0AddVector( &_tmpvf[0], bound_max, bound_min );
	_sceVu0SubVector( &_tmpvf[7], bound_max, bound_min );
	_tmpvf[1].vx = _tmpvf[7].vx ; _tmpvf[1].vy = _tmpvf[0].vy ; _tmpvf[1].vz = _tmpvf[0].vz ;
	_tmpvf[2].vx = _tmpvf[0].vx ; _tmpvf[2].vy = _tmpvf[7].vy ; _tmpvf[2].vz = _tmpvf[0].vz ;
	_tmpvf[3].vx = _tmpvf[7].vx ; _tmpvf[3].vy = _tmpvf[7].vy ; _tmpvf[3].vz = _tmpvf[0].vz ;
	_tmpvf[4].vx = _tmpvf[0].vx ; _tmpvf[4].vy = _tmpvf[0].vy ; _tmpvf[4].vz = _tmpvf[7].vz ;
	_tmpvf[5].vx = _tmpvf[7].vx ; _tmpvf[5].vy = _tmpvf[0].vy ; _tmpvf[5].vz = _tmpvf[7].vz ;
	_tmpvf[6].vx = _tmpvf[0].vx ; _tmpvf[6].vy = _tmpvf[7].vy ; _tmpvf[6].vz = _tmpvf[7].vz ;
#endif

#ifdef BP_PSX2_ASM
	asm volatile("
		lqc2			vf20,0x00(%0)
		lqc2			vf21,0x10(%0)
		lqc2			vf22,0x20(%0)
		lqc2			vf23,0x30(%0)
	"::"r"(&DG_Chanls[0].eye_pers) : "memory");
#else
	DG_SetPos( &DG_Chanls[0].eye_pers ) ;
#endif

	// ８頂点分のクリップ計算を行なう

#ifdef BP_PSX2_ASM
	asm volatile("
		vmulax.xyzw		ACC, vf20,vf12
		vmadday.xyzw	ACC, vf21,vf12
		vmaddaz.xyzw	ACC, vf22,vf12
		vmaddw.xyzw		vf12,vf23,vf00
		vmulax.xyzw		ACC, vf20,vf13
		vmadday.xyzw	ACC, vf21,vf13
		vmaddaz.xyzw	ACC, vf22,vf13
		vmaddw.xyzw		vf13,vf23,vf00
		vmulax.xyzw		ACC, vf20,vf14
		vmadday.xyzw	ACC, vf21,vf14
		vmaddaz.xyzw	ACC, vf22,vf14
		vmaddw.xyzw		vf14,vf23,vf00
		vmulax.xyzw		ACC, vf20,vf15
		vmadday.xyzw	ACC, vf21,vf15
		vmaddaz.xyzw	ACC, vf22,vf15
		vmaddw.xyzw		vf15,vf23,vf00
		vmulax.xyzw		ACC, vf20,vf16
		vmadday.xyzw	ACC, vf21,vf16
		vmaddaz.xyzw	ACC, vf22,vf16
		vmaddw.xyzw		vf16,vf23,vf00
		vmulax.xyzw		ACC, vf20,vf17
		vmadday.xyzw	ACC, vf21,vf17
		vmaddaz.xyzw	ACC, vf22,vf17
		vmaddw.xyzw		vf17,vf23,vf00
		vmulax.xyzw		ACC, vf20,vf18
		vmadday.xyzw	ACC, vf21,vf18
		vmaddaz.xyzw	ACC, vf22,vf18
		vmaddw.xyzw		vf18,vf23,vf00

		vclipw.xyz		vf12,vf12			# vf12 data
		vclipw.xyz		vf13,vf13			# vf13 data
		vclipw.xyz		vf14,vf14			# vf14 data
		vclipw.xyz		vf15,vf15			# vf15 data
		vmulax.xyzw		ACC, vf20,vf19
		vmadday.xyzw	ACC, vf21,vf19
		vmaddaz.xyzw	ACC, vf22,vf19
		vmaddw.xyzw		vf19,vf23,vf00
		cfc2			%0,$18				# clipping
		vclipw.xyz		vf16,vf16			# vf16 data
		vclipw.xyz		vf17,vf17			# vf17 data
		vclipw.xyz		vf18,vf18			# vf18 data
		vclipw.xyz		vf19,vf19			# vf19 data
		vnop
		vnop
		vnop
		vnop
		cfc2			%1,$18				# clipping
	":"=r"(clip_flag1),"=r"(clip_flag2));
#else
	DG_PutVector( &_tmpvf[0], &_tmpvf[0], 8 ) ;
	clip_flag1 = Vclip4( &_tmpvf[0] ) ;
	clip_flag2 = Vclip4( &_tmpvf[4] ) ;
#endif

	check = 0x3f ;
	check &= clip_flag1 ; clip_flag1 >>= 6 ;
	check &= clip_flag1 ; clip_flag1 >>= 6 ;
	check &= clip_flag1 ; clip_flag1 >>= 6 ;
	check &= clip_flag1 ;
	check &= clip_flag2 ; clip_flag2 >>= 6 ;
	check &= clip_flag2 ; clip_flag2 >>= 6 ;
	check &= clip_flag2 ; clip_flag2 >>= 6 ;
	check &= clip_flag2 ;

	if ( check != 0 ){
		clip_flag |= 1 ;
	}

//	if ( clip_flag == 0xf ) clip_flag = -1 ;

	return ( clip_flag );
}

static inline void _SetSsToPosition( FVECTOR *to_pos, FVECTOR *now, FVECTOR *wide,
									 FVECTOR *check_pos, FVECTOR *check_vec,
									 FVECTOR *bound_pos, FVECTOR *bound_size )
{
#if 1
	//レジスタ壊すのでこっちは使えない
	FVECTOR		fvtemp;
	FVECTOR		bound0,bound1;
	float		len;

	_sceVu0SubVector( &fvtemp, now, check_pos );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, 1400.0f );
	_sceVu0AddVector( to_pos, check_pos, &fvtemp );
	
	if( check_vec->vx*fvtemp.vz - check_vec->vz*fvtemp.vx < 0.0f ){
		fvtemp.vx = check_vec->vz;
		fvtemp.vy = 0.0f;
		fvtemp.vz = -check_vec->vx;
		fvtemp.vw = 0.0f;
	}else{
		fvtemp.vx = -check_vec->vz;
		fvtemp.vy = 0.0f;
		fvtemp.vz = check_vec->vx;
		fvtemp.vw = 0.0f;
	}
	len = _Vu0VecLenXZ( &fvtemp, &DG_ZeroVector );
	
	if( len > 90.0f*90.0f ) len = 90.0f*90.0f;
	else if( len < 0.0f ) len = 0.0f;
	len = (1.0f - len/(90.0f*90.0f))*400.0f;
	//printf("len = %f\n",len);
	
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp, len );
	_sceVu0AddVector( to_pos, to_pos, &fvtemp );

	_sceVu0AddVector( &bound0, bound_pos, bound_size );
	_sceVu0SubVector( &bound1, bound_pos, bound_size );
	_sceVu0SubVector( &bound0, &bound0, wide );
	_sceVu0AddVector( &bound1, &bound1, wide );
	//ここでvmax vmin
	if( to_pos->vx > bound0.vx ) to_pos->vx = bound0.vx;
	if( to_pos->vx < bound1.vx ) to_pos->vx = bound1.vx;
	if( to_pos->vz > bound0.vz ) to_pos->vz = bound0.vz;
	if( to_pos->vz < bound1.vz ) to_pos->vz = bound1.vz;


#else

	asm volatile("
		lqc2			vf12,	0x00(%1)	# now
		lqc2			vf13,	0x00(%3)	# check_pos
		qmtc2.ni		%6,		vf17
		lqc2			vf14,	0x00(%4)	# bound_p
		lqc2			vf15,	0x00(%5)	# bound_s
		lqc2			vf16,	0x00(%2)	# wide

		vsub.xyzw		vf18, vf12,	 vf13

		vmul.xyz		vf01, vf18,  vf18		#vec normalize
		vmulax.w		ACC,  vf00,  vf01
		vmadday.w		ACC,  vf00,  vf01
		vmaddz.w		vf1,  vf00,  vf01
		vrsqrt			Q,    vf00w, vf01w
		vwaitq

		vadd.xyzw		vf21, vf14,  vf15
		vsub.xyzw		vf22, vf14,  vf15

		vsub.xyzw		vf23, vf21,  vf16
		vadd.xyzw		vf24, vf22,  vf16

		vmulq.xyz		vf19, vf18, Q			#vec normalize
		vadd.xyzw		vf01, vf13, vf19		#to_pos
		vmulx.xyzw		vf20, vf01, vf17		#to_pos

		vmax			vf25, vf20, vf23
		vmini			vf26, vf25, vf24

		sqc2			vf26,	0x00(%0)

	":: "r"(to_pos), "r"(now), "r"(wide), "r"(check_pos), "r"(bound_pos), "r"(bound_size), "r"(2000.0f) );
	
#endif
}

static inline void _SetSsToPosition2( FVECTOR *to_pos, FVECTOR *now, FVECTOR *wide,
									  FVECTOR *check_pos, FVECTOR *check_vec,
									  FVECTOR *bound_pos, FVECTOR *bound_size )
{
#if 1
	//レジスタ壊すのでこっちは使えない
	FVECTOR		p = { check_vec->vx, 0.0f, check_vec->vz, 0.0f };
	FVECTOR		n;
	FVECTOR		d = { now->vx - check_pos->vx, 0.0f, now->vz - check_pos->vz, 0.0f };
	float		p_len,n_len;
	
	FVECTOR		bound0,bound1;

	_sceVu0Normalize( &p, &p );
	n.vx = -p.vz;
	n.vy = 0.0f;
	n.vz = p.vx;
	n.vw = 0.0f;
	p_len = _sceVu0InnerProduct( &p, &d );
	n_len = bp_sqrtf(1600.0f*1600.0f-p_len*p_len);  //BP_MATH - emulate PS2 sqrtf

	if( DG_FABS(p_len) < 1.0f ) return;

	if( p.vx*d.vz - p.vz*d.vx < 0.0f ) n_len *= -1.0f;

	_sceVu0ScaleVector( &p, &p, p_len );
	_sceVu0ScaleVector( &n, &n, n_len );
	
	_sceVu0AddVector( to_pos, check_pos, &p );
	_sceVu0AddVector( to_pos, to_pos, &n );

	_sceVu0AddVector( &bound0, bound_pos, bound_size );
	_sceVu0SubVector( &bound1, bound_pos, bound_size );
	
	_sceVu0SubVector( &bound0, &bound0, wide );
	_sceVu0AddVector( &bound1, &bound1, wide );
	//ここでvmax vmin
	if( to_pos->vx > bound0.vx ) to_pos->vx = bound0.vx;
	if( to_pos->vx < bound1.vx ) to_pos->vx = bound1.vx;
	if( to_pos->vz > bound0.vz ) to_pos->vz = bound0.vz;
	if( to_pos->vz < bound1.vz ) to_pos->vz = bound1.vz;


#else

	asm volatile("
		lqc2			vf12,	0x00(%1)	# now
		lqc2			vf13,	0x00(%3)	# check_pos
		qmtc2.ni		%6,		vf17
		lqc2			vf14,	0x00(%4)	# bound_p
		lqc2			vf15,	0x00(%5)	# bound_s
		lqc2			vf16,	0x00(%2)	# wide

		vsub.xyzw		vf18, vf12,	 vf13

		vmul.xyz		vf01, vf18,  vf18		#vec normalize
		vmulax.w		ACC,  vf00,  vf01
		vmadday.w		ACC,  vf00,  vf01
		vmaddz.w		vf1,  vf00,  vf01
		vrsqrt			Q,    vf00w, vf01w
		vwaitq

		vadd.xyzw		vf21, vf14,  vf15
		vsub.xyzw		vf22, vf14,  vf15

		vsub.xyzw		vf23, vf21,  vf16
		vadd.xyzw		vf24, vf22,  vf16

		vmulq.xyz		vf19, vf18, Q			#vec normalize
		vadd.xyzw		vf01, vf13, vf19		#to_pos
		vmulx.xyzw		vf20, vf01, vf17		#to_pos

		vmax			vf25, vf20, vf23
		vmini			vf26, vf25, vf24

		sqc2			vf26,	0x00(%0)

	":: "r"(to_pos), "r"(now), "r"(wide), "r"(check_pos), "r"(bound_pos), "r"(bound_size), "r"(2000.0f) );
	
#endif
}


static inline void _SetSsVector( FVECTOR *vec, FVECTOR *now, FVECTOR *to, float ratio )
{
#ifndef BP_PSX2_ASM
	FVECTOR	fvtemp;
	//レジスタ壊すのでこっちは使えない	
	_sceVu0SubVector( vec, to, now );
	vec->vy = 0.0f;
	_sceVu0ScaleVector( &fvtemp, vec, 0.02f );
	_sceVu0AddVector( now, now, &fvtemp );
#else
	asm volatile("
		lqc2			vf13,	0x00(%1)	# now
		lqc2			vf14,	0x00(%2)	# to
		qmtc2.ni		%3,		vf12

		vsub.xyzw		vf15, vf14, vf13
		vmulx.y			vf15, vf00, vf00	#vec

		vmulx.xyzw		vf16, vf15, vf12
		vadd.xyzw		vf17, vf13, vf16	#now

		sqc2			vf15,	0x00(%0)
		sqc2			vf17,	0x00(%1)

	":: "r"(vec), "r"(now), "r"(to), "r"(ratio) : "memory" );
#endif
}


static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num,i;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	//for( i = 0; i < mes_num; i++ ){
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			work->low_cm->flag &= ~DG_COMDL_INVISIBLE;
			break;
		  case 1:
			work->low_cm->flag |= DG_COMDL_INVISIBLE;
			if(!mes_num) return 1;
			break;
		  case 2:
			//起動
			work->group_flags[msg->message[1]] &= ~(GROUP_FLAG_REFUGE|GROUP_FLAG_ESCAPE);
			break;
		  case 3:
			//撤収
			work->group_flags[msg->message[1]] |= GROUP_FLAG_REFUGE;
			DG_COPY_VEC( &work->trap_to_pos[msg->message[1]], &work->refuge_center[msg->message[2]][0] );
			break;

		  case 4:
			//起動
			for( i = msg->message[1]; i <= msg->message[2]; i++ )
				work->group_flags[i] &= ~(GROUP_FLAG_REFUGE|GROUP_FLAG_ESCAPE);
			break;
			
		  case 5:
			//撤収
			for( i = msg->message[1]; i <= msg->message[2]; i++ ){
				work->group_flags[i] |= GROUP_FLAG_REFUGE;
				DG_COPY_VEC( &work->trap_to_pos[i], &work->refuge_center[msg->message[3]][0] );
			}
			break;
		  default:
			break;
		}
		msg--;
	}

	return 0;
}

static void DemoSlaterAct(Work *work)
{
    static FVECTOR	NozzlePoint = { 13.0f, -323.0f, 61.0f, 1.0f };
	int		i,j,k,n_act = 0;
//	DG_COMDL_POS 	*comdl_pos_top = work->low_cm->pos;
	DG_COMDL_POS 	*comdl_pos = work->low_cm->pos;
	SSLTR_DATA		*data = work->ssltr_data;
	FMATRIX		world;
	FVECTOR		*center,*wide,*scale,*to_pos,*now_pos,dvec;//,fvtemp;
	short			*group_index,*group_n_act,*group_timer,*group_max_act,*group_wait;
	u_short			*group_flags;
	float			len;
	int				n_foot,n_bullet;
	FVECTOR			*foot,*foot_top,*bullet,*bullet_top;
	FVECTOR			local_bul[16],nzl_pos,*nzl_vec = NULL;
	int				n_local_bul;
	
	//_sceVu0SubVector( &work->check_vec, &GM_PlayerPosition, &work->pre_check_pos );
	//DG_COPY_VEC( &work->pre_check_pos, &GM_PlayerPosition );	

	if( CheckMesgParam( work ) ){
		GV_WaitMessage( work, work->name );
		return;
	}
	
	center = work->trap_center;
	wide = work->trap_wide;
	scale = work->trap_scale;
	to_pos = work->trap_to_pos;
	now_pos = work->trap_now_pos;

	group_index = work->group_index;
	group_n_act = work->group_n_act;
	group_max_act = work->group_max_act;
	group_timer = work->group_timer;
	group_flags = work->group_flags;
	group_wait = work->group_wait;
	
	DG_COPY_MAT(&world,&DG_UnitMatrix);

	n_foot = GetCheckSltrTrampleNum();
	foot = foot_top = GetCheckSltrTramplePos();

	n_bullet = GetCheckSltrBulletNum();
	bullet = bullet_top = GetCheckSltrBulletPos();
	if( GM_WeaponFire == WP_ColdSpray ){
		_RotTrans( &nzl_pos, GM_PlayerSubWeaponBody->objs->root, &NozzlePoint );
		nzl_vec = (FVECTOR*)GM_PlayerSubWeaponBody->objs->root->m[1];
		//HZX_ViewMatrix( GM_PlayerSubWeaponBody->objs->root, 1000.0f );
	}
	LoadMatHermiteLerp();
	for( i = 0; i < N_SSLT_GROUP; i++ ){
		FVECTOR	check_pos,check_vec;
		//バウンドチェック
		
		//printf("invis %d[%x]\n",i,flags);
#if 1
		//思考部分
#if 0
		//スプレイ反応レジスタ破壊があります。
		if( work->flags&0x0001 && GM_WeaponFire == WP_ColdSpray && !(group_flags[0]&GROUP_FLAG_REFUGE) ){
			//float 		len;
			FVECTOR		diff;
			_sceVu0SubVector( &diff, now_pos, &nzl_pos );
			if( _sceVu0InnerProduct( &diff, &diff ) < 2000.0f * 2000.0f ){
				_sceVu0Normalize( &diff, &diff );
				if( _sceVu0InnerProduct( nzl_vec, &diff ) < -0.95f/*-0.995f*/ ){
					//printf("いででーよ\n");
					//AN_Test_Eye2( now_pos, 2 );
					float	t_len = FLOAT_MAX;
					group_flags[0] |= GROUP_FLAG_REFUGE;
					group_wait[0] = 60*60* 1;

					for( j = 0; j < work->n_refuge; j++ ){
						float temp = _Vu0VecLenXYZ( now_pos, &work->refuge_center[j][0] );
						if( temp < t_len ){
							t_len = temp;
							DG_COPY_VEC( to_pos, &work->refuge_center[j][0] );

						}
					}

				}
			}
		}

		//レジスタ破壊があります。当たってた場合のエフェクト呼び出し
		CheckSltrBlast( now_pos, wide );

		//エリアを撃たれた
		if(group_flags[0] & GROUP_FLAG_HASTY){
			group_flags[0]++;
			if( (group_flags[0]&0x00ff) > 120 ){
				group_flags[0] = (group_flags[0]&0xff00)&(~GROUP_FLAG_HASTY);
			}
		}else if( _Vu0VecLenXYZ( now_pos, to_pos ) >= 10.0f * 10.0f ){
			//移動中は全員動く
			if( group_n_act[0] < N_SSLT_IN_GROUP ) group_n_act[0]++;
		}else{
			if( group_n_act[0] > group_max_act[0] ) group_n_act[0]--;
			if( group_n_act[0] < group_max_act[0] ) group_n_act[0]++;
		}
#else

		if( _Vu0VecLenXYZ( now_pos, to_pos ) >= 10.0f * 10.0f ){
			//移動中は全員動く
			if( group_n_act[0] < N_SSLT_IN_GROUP ) group_n_act[0]++;
		}else{
			if( group_n_act[0] > group_max_act[0] ) group_n_act[0]--;
			if( group_n_act[0] < group_max_act[0] ) group_n_act[0]++;
		}
#endif
		len = GetCheckNearPosLen( &check_pos, &check_vec, now_pos );//_Vu0VecLenXYZ( now_pos, &GM_PlayerPosition );
		//AN_Test_Eye2( &check_pos, 2 );

		if( group_flags[0] & GROUP_FLAG_REFUGE ){
			if( !(group_flags[0] & GROUP_FLAG_ESCAPE) && _Vu0VecLenXYZ( now_pos, to_pos ) < 500.0f * 500.0f ){
				float	t_len = FLOAT_MAX;
				group_flags[0] |= GROUP_FLAG_ESCAPE;
				for( j = 0; j < work->n_refuge; j++ ){
					float temp = _Vu0VecLenXYZ( now_pos, &work->refuge_center[j][1] );
					if( temp < t_len ){
						t_len = temp;
						DG_COPY_VEC( to_pos, &work->refuge_center[j][1] );
					}
				}
			}
			_SetSsVector( &dvec, now_pos, to_pos, 0.01f );
			//if( --group_wait[0] < 0 ){
			//	group_flags[0] &= ~(GROUP_FLAG_REFUGE|GROUP_FLAG_ESCAPE);
			//}
		}else if( len > 4000.0f*4000.0f ){
			DG_COPY_VEC( to_pos, center );
			_SetSsVector( &dvec, now_pos, to_pos, 0.008f );
			group_max_act[0] = 1;
		}else if( len > 2000.0f*2000.0f ){
			DG_COPY_VEC( to_pos, center );
			_SetSsVector( &dvec, now_pos, to_pos, 0.01f );
			group_max_act[0] = 2;
		}else if( len > 1600.0f*1600.0f ){
			DG_COPY_VEC( to_pos, center );
			_SetSsVector( &dvec, now_pos, to_pos, 0.01f );
			group_max_act[0] = 4;
		}else if( len > 1000.0f*1000.0f ){
			//DG_COPY_VEC( to_pos, center );
			_SetSsVector( &dvec, now_pos, to_pos, 0.02f );
			group_max_act[0] = 8;
		}else{
			//２メーター以内
			group_max_act[0] = N_SSLT_IN_GROUP;
			//逃げる処理いれるとこなのね(マギー)
			_SetSsVector( &dvec, now_pos, to_pos, 0.04f );
			_SetSsToPosition( to_pos, now_pos, wide,
							  &check_pos, &check_vec,
							  &work->alltrap_center, &work->alltrap_wide );
		}
		//_SetSsVector( &dvec, now_pos, to_pos, 0.02f );

//		_SetSsVector( &dvec, now_pos, to_pos, 0.1f );
#if 0
		foot = foot_top;
		for( j = 0; j < n_foot; j++ ){
			if( (DG_FABS( now_pos->vx - foot->vx ) < wide->vx) &&
				(DG_FABS( now_pos->vz - foot->vz ) < wide->vz) ){
				//AN_Test_Eye2( foot, 2 );
				SetTrmplSlater( foot, &DG_UnitMatrix, 20.0f*rnd()+40.0f );
			}
			foot++;
		}
#endif
		n_local_bul = 0;
		bullet = bullet_top;
		for( j = 0; j < n_bullet; j++ ){
			if( (DG_FABS( now_pos->vx - bullet->vx ) < wide->vx) &&
				(DG_FABS( now_pos->vz - bullet->vz ) < wide->vz) ){

				DG_COPY_VEC( &local_bul[n_local_bul], bullet );
				group_n_act[0] = N_SSLT_IN_GROUP;
				group_flags[0] = ((group_flags[0]&0xff00)|GROUP_FLAG_HASTY);
				n_local_bul++;
			}
			bullet++;
		}
		LoadMatHermiteLerp();
#else
		//	group_n_act[0] = N_SSLT_IN_GROUP;
		DG_COPY_VEC( to_pos, &GM_PlayerPosition );
		to_pos->vy = now_pos->vy;
		//移動中は全員動く
		len = _Vu0VecLenXYZ( now_pos, to_pos );
		if( len >= 10.0f * 10.0f ){
			//group_n_act[0] = N_SSLT_IN_GROUP;
			if( group_n_act[0] < N_SSLT_IN_GROUP ) group_n_act[0]++;
			
		}else{
			if( group_n_act[0] > 1 ) group_n_act[0]--;
			printf("aaaqaaa\n");
		}
		_SetSsVector( &dvec, now_pos, to_pos );
		//
#endif
		if(!(BoundCheckFlag( now_pos, wide, 1 )&1)){
//		if( i == 0 ){
#if 0
			{
				FVECTOR	bound[2];
				_sceVu0AddVector( &bound[0], now_pos, wide );
				_sceVu0SubVector( &bound[1], now_pos, wide );				
				NewBoundingBoxView( &bound[0], &bound[1], 16, 16, 16 );
				LoadMatHermiteLerp();
			}
#endif
			for( j = 0; j < N_SSLT_IN_GROUP; j++ ){
				int			srot_data = data->rot;
				//int			timer = data->timer;
				//int			index = data->index;
				static FVECTOR outer = { 0.0f, 1.0f, 0.0f, 0.0f };


				// c 0 s
				// 0 1 0
				//-s 0 c
				
				world.m[1][1] = 1.0f;
				world.m[0][0] = scale->vx*TS_COSs(srot_data);
				world.m[2][2] = scale->vz*TS_COSs(srot_data);
				world.m[2][0] = scale->vz*TS_SINs(srot_data);
				world.m[0][2] = -scale->vx*TS_SINs(srot_data);
				if( 1 ){//j == 0 ){
					FVECTOR		debug[3];
					SetSsMatrix_Refuge( (FMATRIX*)comdl_pos,
										&world, &SsltrMovePos[data->index],
										(float)data->timer/4096.0f,
										&outer, now_pos, &dvec, 1.0f, debug );//RndTable[j&7] );
					//PRINT_PFVEC(1,(FVECTOR*)((FMATRIX*)comdl_pos)->m[2]);

//	PRINT_PFVEC(0,&debug[0]);
//	PRINT_PFVEC(1,&debug[1]);
//	PRINT_PFVEC(2,&debug[2]);
					
				}else{
					SetSsMatrix_Normal( (FMATRIX*)comdl_pos,
										&world, &SsltrMovePos[data->index],
										(float)data->timer/4096.0f,
										&outer, center, RndTable[j&7] );
				}

				for( k = 0; k < n_local_bul; k++ ){
					if( (DG_FABS( ((FMATRIX*)comdl_pos)->m[3][0] - local_bul[k].vx ) < 20.0f) &&
						(DG_FABS( ((FMATRIX*)comdl_pos)->m[3][2] - local_bul[k].vz ) < 20.0f) ){
						SetTrmplSlater( (FVECTOR*)((FMATRIX*)comdl_pos)->m[3],
										&DG_UnitMatrix, 20.0f*rnd()+40.0f );
				
					}
					
				}
				if( (group_index[0] <= j && j < group_index[0]+group_n_act[0]) ||
					( j <= group_index[0] + group_n_act[0] - N_SSLT_IN_GROUP ) ){
					data->timer += MOVE_TIME;
					if(data->timer >= 0x1000){
						if(++(data->index) == N_SSLTRMOVE_INDEX) data->index = 0;
						data->timer &= 0x0fff;
					}
				}
				data++;
				comdl_pos++;
			}

			group_timer[0] += MOVE_TIME;
			if( group_timer[0] >= 0x1000 ){
				group_index[0] += group_n_act[0];
				if( group_index[0] >= N_SSLT_IN_GROUP )
					group_index[0] -= N_SSLT_IN_GROUP;
				group_timer[0] &= 0x0fff;

				//printf("%d[ %d, %d ]\n",i,work->group_index[i],work->group_n_act[i]);
			}
			
			n_act += N_SSLT_IN_GROUP;
		}else{
			data += N_SSLT_IN_GROUP;
		}
		center++; wide++;
		scale++; to_pos++;
		now_pos++;
		group_index++;
		group_n_act++;
		group_max_act++;
		group_timer++;
		group_flags++;
		group_wait++;
	}
	if( !n_act ) n_act = 1;
	work->low_cm->n_objs = n_act;
	//printf( "n_act = %d\n", n_act );

	foot = foot_top;
	for( j = 0; j < n_foot; j++ ){
		wide = work->trap_wide;
		now_pos = work->trap_now_pos;
		if( foot->vw > 0.0f ){
			for( i = 0; i < N_SSLT_GROUP; i++ ){
				if( (DG_FABS( now_pos->vx - foot->vx ) < wide->vx) &&
					(DG_FABS( now_pos->vz - foot->vz ) < wide->vz) ){
					//AN_Test_Eye2( foot, 2 );
					SetTrmplSlater( foot, &DG_UnitMatrix, 20.0f*rnd()+40.0f );
					foot->vw = -1.0f;
					break;
				}
				wide++;
				now_pos++;
			}
		}
		foot++;
	}

#if 0
	if(0){
		int		i;
		for( i = 0; i < work->n_refuge; i++ ){
			FVECTOR	bound[2];
			_sceVu0AddVector( &bound[0], &work->refuge_center[i][0], &work->refuge_wide[i][0] );
			_sceVu0SubVector( &bound[1], &work->refuge_center[i][0], &work->refuge_wide[i][0] );
			NewBoundingBoxView( &bound[0], &bound[1], 128, 16, 16 );
			
			_sceVu0AddVector( &bound[0], &work->refuge_center[i][1], &work->refuge_wide[i][1] );
			_sceVu0SubVector( &bound[1], &work->refuge_center[i][1], &work->refuge_wide[i][1] );
			NewBoundingBoxView( &bound[0], &bound[1], 16, 16, 128 );
		}
		
	}
	if(0){
		int		i;
		for( i = 0; i < N_SSLT_GROUP; i++ ){
			FVECTOR	bound[2];
			_sceVu0AddVector( &bound[0], &work->trap_now_pos[i], &work->trap_wide[i] );
			_sceVu0SubVector( &bound[1], &work->trap_now_pos[i], &work->trap_wide[i] );
			NewBoundingBoxView( &bound[0], &bound[1], 16, 16, 16 );
		}
	}
	if(0){
		FVECTOR	bound[2];
		_sceVu0AddVector( &bound[0], &work->alltrap_center, &work->alltrap_wide );
		_sceVu0SubVector( &bound[1], &work->alltrap_center, &work->alltrap_wide );
		NewBoundingBoxView( &bound[0], &bound[1], 16, 16, 160 );
	}

#endif
}

static void Die(Work *work)
{
	if(work->low_cm){
		DG_DequeueComdlObjs( work->low_cm );
		DG_FreeComdl( work->low_cm );
	}
	SubEmmSsltr( work );
}

static void InitSsltrPosData( SSLTR_DATA *w_data, int n_ssltr )
{
	SSLTR_DATA		*data = MEM_SCR_TOP;
	int			i;

	for( i = 0; i < n_ssltr; i++ ){
		data->index = irnd()%N_SSLTRMOVE_INDEX;
		data->rot = irnd()%4096;
		data->timer = irnd()%4096;
		data++;
	}
	TS_Scr_Mem( w_data, MEM_SCR_TOP, sizeof(SSLTR_DATA), n_ssltr );
}

static DG_COMDL *InitSsltrComdl( int cm_code, int n_comdl, int col, int map )
{
	//InitSsltrPosの後に呼び出してスクラッチパッドのデータを使う
	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 	*comdl_pos;
	FVECTOR			*pos = MEM_SCR_TOP;
	int				i;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( cm_code, 'k' ) );
	comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, n_comdl, 0 );
	//comdl = DG_MakeComdl( def->models[0].packs, 0, n_comdl, 0 );
	if( !comdl ) return NULL;
	DG_QueueComdlObjs( comdl );
	comdl->group_id = GM_GetDGGroupID( map );

	comdl_pos = comdl->pos;
	for( i = 0; i < n_comdl; i++ ){
		//float	rot_y = PI*frnd()*0.5f;
		//PRINT_PFVEC(i,pos);

		//_sceVu0RotMatrixZ( &comdl_pos->world, &DG_UnitMatrix, PI );
		//_sceVu0RotMatrixY( &comdl_pos->world, &comdl_pos->world, rot_y );
		//_sceVu0RotMatrixY( &comdl_pos->world, &DG_UnitMatrix, rot_y );

		//ここにルートの初期かも入れる
		DG_COPY_MAT( &comdl_pos->world, &DG_UnitMatrix );
		//DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], pos );
		//comdl_pos->world.m[3][3] = 1.0f;

		//PRINT_PFVEC(0,(FVECTOR*)comdl_pos->world.m[0]);
		//PRINT_PFVEC(1,(FVECTOR*)comdl_pos->world.m[1]);
		//PRINT_PFVEC(2,(FVECTOR*)comdl_pos->world.m[2]);
		//PRINT_PFVEC(3,(FVECTOR*)comdl_pos->world.m[3]);
//		if( i == 0 ){
//			comdl_pos->color.vx = 255;
//			comdl_pos->color.vy = 255;
//			comdl_pos->color.vz = 255;
//		}else{
			comdl_pos->color.vx = col&0xff;
			comdl_pos->color.vy = (col>>8)&0xff;
			comdl_pos->color.vz = (col>>16)&0xff;
//		}
		comdl_pos->color.vw = 128;
		comdl_pos++;
		pos++;
	}

	
	return comdl;
}

static int GetTrapInfo( FVECTOR *center, FVECTOR *wide, FVECTOR *scale, FVECTOR *all_cnt, HZX_GROUP_ID group_id, int name_id )
{
	int				cnt = 0,i;
	HZX_GRP			*group;
	HZX_BLOCK		*block;
	HZX_TRP			*trap;
	FVECTOR			bound0,bound1,fvtemp0;

	group = HZX_GetGroup( group_id );
	ASSERT( group );
	block = group->blocks;
	ASSERT( block );
	//printf( "segs %d[%08x]\n", block->n_segs, block->segs );
	//printf( "flrs %d[%08x]\n", block->n_flrs, block->flrs );
	//printf( "trap %d[%08x]\n", block->n_traps, block->traps );
	trap = block->traps;
	ASSERT( trap );

	all_cnt->vx = 0.0f;
	all_cnt->vy = 0.0f;
	all_cnt->vz = 0.0f;
	all_cnt->vw = 0.0f;

	for ( i = 0; i < block->n_traps; i++, trap++ ) {
		if( trap->name_id != name_id ) continue;
		if( cnt >= N_SSLT_GROUP ){
			return cnt;
		}

		bound0.vx = (float)trap->b1.vx + (float)block->tx;
		bound0.vy = (float)trap->b1.vy + (float)block->ty;
		bound0.vz = (float)trap->b1.vz + (float)block->tz;
		bound0.vw = 1.0f;

		bound1.vx = (float)trap->b2.vx + (float)block->tx;
		bound1.vy = (float)trap->b2.vy + (float)block->ty;
		bound1.vz = (float)trap->b2.vz + (float)block->tz;
		bound1.vw = 1.0f;

		_sceVu0SubVector( &fvtemp0, &bound1, &bound0 );
		_sceVu0ScaleVector( &fvtemp0, &fvtemp0, 0.5f );
		_sceVu0AddVector( &center[cnt], &fvtemp0, &bound0 );
		_sceVu0SubVector( &wide[cnt], &center[cnt], &bound0 );

		//center[cnt].vy += 20.0f;
		//PRINT_PFVEC(cnt,&wide[cnt]);
		_sceVu0AddVector( all_cnt, all_cnt, &center[cnt] );

		scale[cnt].vx = wide[cnt].vx/160.0f;
		scale[cnt].vz = wide[cnt].vz/160.0f;
		cnt++;
	}

	_sceVu0ScaleVector( all_cnt, all_cnt, 1.0f/(float)N_SSLT_GROUP);

	return (cnt - N_SSLT_GROUP);
}


static int GetOneTrapInfo( FVECTOR *center, FVECTOR *wide, HZX_GROUP_ID group_id, int name_id )
{
	int				i;
	HZX_GRP			*group;
	HZX_BLOCK		*block;
	HZX_TRP			*trap;
	FVECTOR			bound0,bound1,fvtemp0;

	group = HZX_GetGroup( group_id );
	ASSERT( group );
	block = group->blocks;
	ASSERT( block );
	//printf( "segs %d[%08x]\n", block->n_segs, block->segs );
	//printf( "flrs %d[%08x]\n", block->n_flrs, block->flrs );
	//printf( "trap %d[%08x]\n", block->n_traps, block->traps );
	trap = block->traps;
	ASSERT( trap );

	for ( i = 0; i < block->n_traps; i++, trap++ ) {
		if( trap->name_id != name_id ) continue;

		bound0.vx = (float)trap->b1.vx + (float)block->tx;
		bound0.vy = (float)trap->b1.vy + (float)block->ty;
		bound0.vz = (float)trap->b1.vz + (float)block->tz;
		bound0.vw = 1.0f;

		bound1.vx = (float)trap->b2.vx + (float)block->tx;
		bound1.vy = (float)trap->b2.vy + (float)block->ty;
		bound1.vz = (float)trap->b2.vz + (float)block->tz;
		bound1.vw = 1.0f;

		//PRINT_PFVEC(0,&bound0);
		//PRINT_PFVEC(1,&bound1);
		
		_sceVu0SubVector( &fvtemp0, &bound1, &bound0 );
		_sceVu0ScaleVector( &fvtemp0, &fvtemp0, 0.5f );
		_sceVu0AddVector( center, &fvtemp0, &bound0 );
		_sceVu0SubVector( wide, center, &bound0 );

		//PRINT_PFVEC(0,center);
		//PRINT_PFVEC(1,wide);
		return 0;
	}

	return (-1);
}


static int GetResources( Work *work )
{
	int				trap_code;
	int				atrap_code;
	HZX_GROUP_ID	hzx_group;
	int				n_ssltr;
	FVECTOR			center;
	FMATRIX			light[2];
	int				color,i,n_trap;


	n_ssltr = N_SSLTS;
	if( GCL_GetOption( 'g' ) == NULL ){ printf("not input funa hzx group\n"); return -1; }
	hzx_group = ( HZX_GROUP_ID )GV_GetBit( GCL_GetNextInt() );
	
	if( GCL_GetOption( 'a' ) == NULL ){ printf("not input all funa trap group\n"); return -1; }
	atrap_code = GCL_GetNextInt();

	if( GCL_GetOption( 't' ) == NULL ){ printf("not input funa trap group\n"); return -1; }
	trap_code = GCL_GetNextInt();

	n_trap = GCL_GetOptionValue( 'n', 0 );
	if( n_trap > N_MAX_REFUGE ) n_trap = N_MAX_REFUGE;

	work->n_refuge = n_trap;
	if( 0 < n_trap && GCL_GetOption( 'r' ) ){
		work->flags |= 0x0001;
		for( i = 0; i < n_trap; i++ ){
			int		temp0 = GCL_GetNextInt();
			int		temp1 = GCL_GetNextInt();
			if( GetOneTrapInfo( &work->refuge_center[i][0], &work->refuge_wide[i][0], hzx_group, temp0 ) ){
				printf("info init err!! refuge traps\n");
				return -1;
			}
			if( GetOneTrapInfo( &work->refuge_center[i][1], &work->refuge_wide[i][1], hzx_group, temp1 ) ){
				printf("info init err!! escape traps\n");
				return -1;
			}
		}
	}

	if( GetOneTrapInfo( &work->alltrap_center, &work->alltrap_wide, hzx_group, atrap_code ) ){
		printf("info init err!![%d]\n");
		return -1;
	}
	
	if( GetTrapInfo( work->trap_center, work->trap_wide, work->trap_scale, &center, hzx_group, trap_code ) ){
		printf("info init err!!\n");
		return -1;
	}

	DG_GetLightMatrix( &center, light );
#ifdef BP_PS2
	color = GetRGBFromLightNorm( &(FVECTOR){0.0f,1.0f,0.0f,0.0f}, light );
#else
	{
		FVECTOR tmp = {0.0f,1.0f,0.0f,0.0f} ;
		color = GetRGBFromLightNorm( &tmp, light );
	}
#endif
	//printf("color [%x]\n");
	color = 0x00606060;
	work->low_cm = InitSsltrComdl( 10389881, n_ssltr, color, work->map );
	if(!work->low_cm) { printf("cm init err!!\n"); return -1; }

	InitSsltrPosData( work->ssltr_data, n_ssltr );

	for( i = 0; i < N_SSLT_GROUP; i++ ){
		float	t_len = FLOAT_MAX;
		int j;
		work->group_timer[i] = 0;
		work->group_index[i] = 0;
		work->group_flags[i] = 0;
		work->group_n_act[i] = 2;
		work->group_max_act[i] = 2;
#if 1

		work->group_flags[i] |= GROUP_FLAG_REFUGE;

		for( j = 0; j < work->n_refuge; j++ ){
			float temp = _Vu0VecLenXYZ( &work->trap_center[i], &work->refuge_center[j][0] );
			if( temp < t_len ){
				t_len = temp;
				DG_COPY_VEC( &work->trap_now_pos[i], &work->refuge_center[j][0] );
				DG_COPY_VEC( &work->trap_to_pos[i], &work->refuge_center[j][0] );		
			}
		}
		
		//DG_COPY_VEC( &work->trap_now_pos[i], &work->trap_center[i] );
#else
		DG_COPY_VEC( &work->trap_now_pos[i], &work->trap_center[i] );
		DG_COPY_VEC( &work->trap_to_pos[i], &work->trap_center[i] );
#endif

		
		//NewDbugSprite(&work->trap_now_pos[i],100.0f );
	}

	work->n_ssltr = n_ssltr;
	printf("%d匹の",n_ssltr);
	return 0;
}

void *NewDemoSlater( int name, int map )
{
	Work *work = NULL;

//	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	work = (Work*)GV_CreateActor( GV_ACTOR_EFFECT, GV_CLASS_EFFECT, sizeof(Work), 16 );
	if(work){
		GV_SetActor(&(work->actor),DemoSlaterAct,Die) ;
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work);
			return NULL;
		}
		AddEmmSsltr( work );
		printf("エマ虫！！\n");
		printf("n_max %d\n", 0x4000/sizeof( SSLTR_DATA ) );
	}
	
	return (void *)work ;
}
