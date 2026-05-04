//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    test_slater.c
	舟りんテスト
	2000/02/07 T.Shibata
	
	$Id: prev_test.c,v 1.1.1.3 2002/11/19 11:48:50 Yoshizawa1 Exp $

*/

#if 0 //BP_TODO

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
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../util/ts_util.h"
//#include	"sea_slater.h"

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

#define		N_SSLTR_INTEX	(8)
#define		N_MOVE_SSLTR	(64)
#define		MOVE_TIME		(4096/64)
#define		INDEX_ADD_TIME		(1)
//[0-7]index
//[8-19]timer
//[20-31]rot
#define		GET_DATA_ROT(_data)		( ((*(u_int*)&(_data))>>20) )
#define		GET_DATA_TIMER(_data)	( ((*(u_int*)&(_data))>>8)&0x0fff )
#define		GET_DATA_INDEX(_data)	( (*(u_int*)&(_data))&0xff )


//[0-11] timer
//[12]	move flag
#define		CHECK_FLAGMOVE(_flags)	((_flags)&0x0001)
#define		GET_ESCAPE_FLAGS(_data)		((*(u_int*)&(_data))>>12)
#define		GET_ESCAPE_TIMER(_data)		((*(u_int*)&(_data))&0x0fff)

#define		SET_DATA(_rot,_time,_index)	(((_rot)<<20)|((_time)<<8)|(_index))
#define		SET_ESCAPE_DATA(_flags,_time)	(((_flags)<<12)|(_time))

#define		MEM_SCR_TOP		((void*)(SCRPAD_ADDR))

typedef struct {
	GV_ACT_EX		actor;
#if 1
	FVECTOR			bound[2];
#endif
	FVECTOR			local_bound[2];
	FVECTOR			escape_bound[2];
	int				name;
	int				map;
	int				move_index;
	int				no_move;
	int				n_ssltr;		//テクスチャー枚数(コモデルは×N_SSLT_INTEX)
	
	DG_COMDL		*high_cm;
	DG_COMDL		*low_cm;

	FVECTOR			*ssltr_data;		//n_sslt×N_SSLT_INTEX	vwはインデックスとMoveのrot
	FVECTOR			*ssltr_pos;			//n_sslt×N_SSLT_INTEX	
	FVECTOR			*ssltr_to_pos;			//n_sslt×N_SSLT_INTEX	
} Work;

#define		SIZE_MUL	(1.0f)

extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
//extern void *NewSquareView( FVECTOR*, int, u_char, u_char, u_char );
extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );
//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )

#define N_SSLTRMOVE_INDEX	(sizeof(SsltrMovePos)/sizeof(SsltrMovePos[0]))
static float RndTable[8] = { 1.2f, 1.0f, 0.9f, 0.8f, 1.1f, 1.05f, 0.95f, 1.075f };

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
};

//640匹中128匹

#ifndef PSX2
static FVECTOR _vf04, _vf05, _vf06, _vf07, _vf08, _vf09, _vf10, _vf11, _vf12 ;
#endif

#ifndef PSX2
static FVECTOR Mat2Vec[4], Mat3Vec[4] ;
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

#ifdef PSX2
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
	Mat2Vec[0] = *(FVECTOR *)&c_mat2.m[0] ;
	Mat2Vec[1] = *(FVECTOR *)&c_mat2.m[1] ;
	Mat2Vec[2] = *(FVECTOR *)&c_mat2.m[2] ;
	Mat2Vec[3] = *(FVECTOR *)&c_mat2.m[3] ;

	Mat3Vec[0] = *(FVECTOR *)&c_mat3.m[0] ;
	Mat3Vec[1] = *(FVECTOR *)&c_mat3.m[1] ;
	Mat3Vec[2] = *(FVECTOR *)&c_mat3.m[2] ;
	Mat3Vec[3] = *(FVECTOR *)&c_mat3.m[3] ;
#endif
}

#ifndef PSX2
static void MakeHermiteMat( FMATRIX *out_mat, FMATRIX *rot_mat, FVECTOR *pos )
{
	FVECTOR tmp ;
	
	DG_SetPos( rot_mat ) ;
	DG_PutVector(pos, (FVECTOR *)out_mat->m[0], 2 ) ;
	DG_PutVectorW((FVECTOR *)(pos+2), &tmp, 1 ) ;

	_sceVu0SubVector( (FVECTOR *)out_mat->m[2], (FVECTOR *)out_mat->m[0], (FVECTOR *)out_mat->m[1] );
	out_mat->m[2][3] = 0.0f ;
	_sceVu0SubVector( (FVECTOR *)out_mat->m[3], &tmp, (FVECTOR *)out_mat->m[0] );
	out_mat->m[3][3] = 0.0f ;
}

static void HermiteLerpVec( FVECTOR *out_vec1, FVECTOR *out_vec2, FMATRIX *mat, float her_t )
{
	FMATRIX	m1, m2 ;
	FVECTOR her_vec ;

	DG_SetPos( mat ) ;
	DG_PutVectorW( Mat2Vec, (FVECTOR *)m1.m[0], 4 ) ;
	DG_PutVectorW( Mat3Vec, (FVECTOR *)m2.m[0], 4 ) ;
	
	her_vec.vx = her_t*her_t*her_t ;
	her_vec.vy = her_t*her_t ;
	her_vec.vz = her_t ;
	her_vec.vw = 1.0f ;

	DG_SetPos( &m1 ) ;
	DG_PutVectorW( &her_vec, out_vec1, 1 ) ;

	DG_SetPos( &m2 ) ;
	DG_PutVectorW( &her_vec, out_vec2, 1 ) ;
}

#endif

static void SetSsMatrix_Normal( FMATRIX *out_mat,
								FMATRIX *rot_mat, FVECTOR *pos,
								float her_t,
								FVECTOR *outer, FVECTOR *data, float scale )
{
#ifdef PSX2
	asm volatile("
		lqc2			vf16,0x00(%1)	# RotMatrix ..x
		lqc2			vf20,0x00(%2)	# pos0
		lqc2			vf17,0x10(%1)	# ... y
		lqc2			vf18,0x20(%1)	# ... z
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

	_sceVu0MulVectorXYZ( &vec, &her_vec2, &her_vec2 );
	q = 1.0f/bp_sqrtf( vec.vx + vec.vy + vec.vz ) ; //BP_MATH - emulate PS2 sqrtf
	_sceVu0ScaleVector( &vec2, &her_vec2, q ) ;
	_sceVu0OuterProduct( &opvec, &vec2, outer );
	opvec.vw = 0.0f ;

	_sceVu0AddVector( (FVECTOR *)out_mat->m[3], &her_vec1, data );
	out_mat->m[0][3] = 1.0f ;
	_sceVu0ScaleVector( (FVECTOR *)out_mat->m[2], &vec2, scale ) ;
	_sceVu0ScaleVector( (FVECTOR *)out_mat->m[1], outer, scale ) ;
	_sceVu0ScaleVector( (FVECTOR *)out_mat->m[0], &opvec, scale ) ;
#endif
}

static inline void _RotVector( FVECTOR *out, FMATRIX *world, FVECTOR *in )
{
#ifdef PSX2
	asm volatile("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddz.xyzw			vf8, vf6,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
	DG_SetPos( world ) ;
	DG_RotVector(in, out, 1 ) ;
#endif
}

static void NormalAct( Work *work )
{
	int				i,j;
	FVECTOR			*data = MEM_SCR_TOP;
	FVECTOR			*pos = work->ssltr_pos;
	DG_COMDL_POS 	*comdl_pos_top = work->low_cm->pos;
	DG_COMDL_POS 	*comdl_pos;

	if( work->move_index+128 > work->n_ssltr ){
		TS_Scr_Mem( MEM_SCR_TOP, &work->ssltr_data[work->move_index],
					sizeof(FVECTOR), work->n_ssltr-work->move_index );
		TS_Scr_Mem( (u_char*)(MEM_SCR_TOP)+sizeof(FVECTOR)*(work->n_ssltr-work->move_index),
					work->ssltr_data, sizeof(FVECTOR), 128 - (work->n_ssltr-work->move_index) );
	}else{
		TS_Scr_Mem( MEM_SCR_TOP, &work->ssltr_data[work->move_index], sizeof(FVECTOR), 128 );
	}
//	NewTsDebugLine( SsltrMovePos, N_SSLTRMOVE_INDEX, 0x80808080 );

	//vf04<=>vf11に定数マトリックスロード
	LoadMatHermiteLerp();

	for( i = work->move_index, j = 0; j < N_MOVE_SSLTR; i++,j++ ){
		int			srot_data = GET_DATA_ROT(data->vw);
		int			timer = GET_DATA_TIMER(data->vw);
		int			index = GET_DATA_INDEX(data->vw);
		FMATRIX		world;
		FVECTOR		rot_pos[3];
		static FVECTOR outer = { 0.0f, 1.0f, 0.0f, 0.0f };

		comdl_pos = &comdl_pos_top[i%work->n_ssltr];

		DG_COPY_MAT(&world,&DG_UnitMatrix);
		// c 0 s
		// 0 1 0
		//-s 0 c
		world.m[0][0] = world.m[2][2] = TS_COSs(srot_data);
		world.m[2][0] = TS_SINs(srot_data);
		world.m[0][2] = -world.m[2][0];
		DG_COPY_VEC( &rot_pos[0], &SsltrMovePos[index]);
		DG_COPY_VEC( &rot_pos[1], &SsltrMovePos[(index+1)%N_SSLTRMOVE_INDEX]);
		DG_COPY_VEC( &rot_pos[2], &SsltrMovePos[(index+2)%N_SSLTRMOVE_INDEX]);

		SetSsMatrix_Normal( (FMATRIX*)comdl_pos,
							&world, rot_pos,
							(float)timer/4096.0f,
							&outer, data, SIZE_MUL );

		DG_COPY_VEC( &pos[i%work->n_ssltr],(FVECTOR*)comdl_pos->world.m[3]);
		timer += MOVE_TIME;
		if(timer >= 0x1000){
			if(++index == N_SSLTRMOVE_INDEX) index = 0;
			timer &= 0x0fff;
		}

		index = SET_DATA(srot_data,timer,index);
		data->vw = *(float*)&index;

		data++;
	}

	if( work->move_index+128 > work->n_ssltr){
		TS_Mem_Scr( &work->ssltr_data[work->move_index], MEM_SCR_TOP,
					sizeof(FVECTOR), work->n_ssltr-work->move_index );
		TS_Mem_Scr( work->ssltr_data,
					(u_char*)(MEM_SCR_TOP)+sizeof(FVECTOR)*(work->n_ssltr-work->move_index),
					sizeof(FVECTOR), 128 - (work->n_ssltr-work->move_index) );
	}else{
		TS_Mem_Scr( &work->ssltr_data[work->move_index], MEM_SCR_TOP, sizeof(FVECTOR), 128 );
	}

}
static inline float GetXYLen2( FVECTOR *a, FVECTOR *b )
{
	float len;
	FVECTOR temp = { b->vx, b->vy, 0.0f, 0.0f };
#ifdef PSX2
//retouch kore	02.2.18
	asm volatile("
		lqc2			vf16,0x00(%1)	# pos
		lqc2			vf17,0x00(%2)	# player

		vsub.xy			vf18,vf16,vf17	#pos - player
		vmul.xy			vf2,vf18,vf18	#len
		vmulax.w		ACC,vf0,vf2
		vmaddy.w		vf1,vf0,vf2
		vaddw.x			vf2,vf0,vf1
		#vsqrt			Q,vf1w
		#vwaitq
		#vaddq.x			vf1,vf0,Q
		qmfc2.i			$8,vf2
		sw				$8,0(%0)
	"::"r"(&len),"r"(a),"r"(&temp) : "$8", "memory" );
#else
	len = ((a->vx - b->vx)*(a->vx - b->vx)) + ((a->vy - b->vy)*(a->vy - b->vy)) ;
#endif

	return len;
}

#ifndef PSX2
static void HermiteLerpVec( FVECTOR *out_vec1, FVECTOR *out_vec2, FMATRIX *mat, float her_t )
{
	FMATRIX	m1, m2 ;
	FVECTOR her_vec ;

	DG_SetPos( mat ) ;
	DG_PutVectorW( Mat2Vec, (FVECTOR *)m1.m[0], 4 ) ;
	DG_PutVectorW( Mat3Vec, (FVECTOR *)m2.m[0], 4 ) ;
	
	her_vec.vx = her_t*her_t*her_t ;
	her_vec.vy = her_t*her_t ;
	her_vec.vz = her_t ;
	her_vec.vw = 1.0f ;

	DG_SetPos( &m1 ) ;
	DG_PutVectorW( &her_vec, out_vec1, 1 ) ;

	DG_SetPos( &m2 ) ;
	DG_PutVectorW( &her_vec, out_vec2, 1 ) ;
}
#endif

static void RunAwayAct( Work *work )
{
	int				i;
	FVECTOR			*data = MEM_SCR_TOP;
	FVECTOR			*pos = work->ssltr_pos;
	FVECTOR			*to_pos = work->ssltr_to_pos;
	FVECTOR			*bound0 = &work->local_bound[0];
	FVECTOR			*bound1 = &work->local_bound[1];
	DG_COMDL_POS 	*comdl_pos = work->low_cm->pos;
   float          q;//BP_ASM

	TS_Scr_Mem( MEM_SCR_TOP, work->ssltr_data, sizeof(FVECTOR), work->n_ssltr );

	//vf04<=>vf11に定数マトリックスロード
	LoadMatHermiteLerp();
	for( i = 0; i < work->n_ssltr; i++ ){
		int			timer = GET_ESCAPE_TIMER(pos->vw);
		int			flags = GET_ESCAPE_FLAGS(pos->vw);
		//バンダリと円チェック
		if( (bound0->vx < to_pos->vx && to_pos->vx < bound1->vx) &&
			(bound0->vz < to_pos->vy && to_pos->vy < bound1->vz) &&
			(GetXYLen2( to_pos, &GM_PlayerPosition ) < 1200.0f*1200.0f) ){
			FVECTOR		fvtemp;
			int			temp = (irnd()>>16)&0x07;
			//新しいto_posセット
			DG_COPY_VEC( pos, ((FMATRIX*)comdl_pos)->m[3]);

			//no check
#ifdef PSX2
			asm volatile("
				lqc2			vf16,0x00(%1)	# pos
				lqc2			vf17,0x00(%2)	# player
				qmtc2.ni		%3,vf03			#HermiteLerpVecここから

				vsub.xz			vf18,vf16,vf17	#pos - player
				vmul.xz			vf2,vf18,vf18	#len
				vmulax.w		ACC,vf0,vf2
				vmaddz.w		vf1,vf0,vf2
				vrsqrt			Q,vf0w,vf1w
				vwaitq
				vmulq.xyz		vf1,vf18,Q
				vmulx.xyz		vf2,vf1,vf3
				sqc2			vf2,0x00(%0)
			"::"r"(&fvtemp),"r"(pos),"r"(&GM_PlayerPosition),"r"(RndTable[temp]) );
#else
         //BP_ASM - fixed to compile
			fvtemp.vx = (pos.vx - GM_PlayerPosition.vx)*(pos.vx - GM_PlayerPosition.vx) ;
			fvtemp.vz = (pos.vz - GM_PlayerPosition.vz)*(pos.vz - GM_PlayerPosition.vz) ;

			q = 1.0f/bp_sqrtf( fvtemp.vx + fvtemp.vz ) ; //BP_MATH - emulate PS2 sqrtf

			fvtemp.vx *= q*RndTable[temp] ;
			fvtemp.vz *= q*RndTable[temp] ;

#endif

			to_pos->vx = fvtemp.vx;
			to_pos->vy = fvtemp.vz;
			to_pos->vz = ((FMATRIX*)comdl_pos)->m[2][0]*160.0f;
			to_pos->vw = ((FMATRIX*)comdl_pos)->m[2][2]*160.0f;

			
			//フラグセット
			flags |= 0x0001;
			timer = MOVE_TIME<<1;
		}
		//act
		if(CHECK_FLAGMOVE(flags)){
			//posの座標から　to_posへの補間
			FVECTOR to,dvec0;
			static FVECTOR outer = { 0.0f, 1.0f, 0.0f, 0.0f };
			static FVECTOR zero = { 0.0f, 0.0f, 0.0f, 0.0f };

			to.vx = to_pos->vx;
			to.vy = work->bound[0].vy;
			to.vz = to_pos->vy;
			to.vw = 1.0f;
			
			dvec0.vx = to_pos->vz;
			dvec0.vy = 0.0f;
			dvec0.vz = to_pos->vw;
			dvec0.vw = 0.0f;

			//pos,to, dvec0, Zero, (float)timer/4096.0fで補間
			//printf("%d\n",timer);
#ifdef PSX2
			asm volatile("
				qmtc2.ni		%5,vf01			#HermiteLerpVecここから
				lqc2			vf12 ,0x00(%1)	#
				lqc2			vf13 ,0x00(%2)	#
				lqc2			vf14 ,0x00(%3)	#
				lqc2			vf15 ,0x00(%4)	#

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
				vmaddw.xyzw		vf15,vf19,vf00

				lqc2		vf13 ,0x00(%6)	#outer
				qmtc2.ni	%7,vf17

				vmul.xyz	vf1,vf25,vf25	#vec normalize
				vmulax.w	ACC,vf0,vf1
				vmadday.w	ACC,vf0,vf1
				vmaddz.w	vf1,vf0,vf1
				vrsqrt		Q,vf0w,vf1w
				vwaitq
				vmulq.xyz	vf14,vf25,Q

				vopmula.xyz	ACC,vf14,vf13
				vopmsub.xyz	vf12,vf13,vf14

				#vadd.xyz	vf15,vf24,vf16
				vmulx.w		vf12,vf0,vf0	#out_mat x.vw = 0.0f

				vmulx.xyz		vf14,vf14,vf17	#mul matrix
				vmulx.xyz		vf13,vf13,vf17	#mul matrix
				vmulx.xyz		vf12,vf12,vf17	#mul matrix

				vmulw.w		vf15,vf0,vf0	#out_mat w.vw = 1.0f

				sqc2		vf12 ,0x00(%0)
				sqc2		vf13 ,0x10(%0)
				sqc2		vf14 ,0x20(%0)
				sqc2		vf15 ,0x30(%0)
		
			"::"r"(comdl_pos),"r"(pos),"r"(&to),"r"(&dvec0),"r"(&zero),"r"((float)timer/4096.0f),
			   "r"(&outer),"r"(SIZE_MUL) );
#else
			FMATRIX	m1 ;
			FVECTOR vec, vec2, opvec, her_vec1, her_vec2 ;

			*(FVECTOR *)m1.m[0] = *pos ;
			*(FVECTOR *)m1.m[1] = *to ;
			*(FVECTOR *)m1.m[2] = dvec0 ;
			*(FVECTOR *)m1.m[3] = zero ;

			HermiteLerpVec( &her_vec1, &her_vec2, &m1, ((float)timer/4096.0f) ) ;

			_sceVu0MulVectorXYZ( &vec, &her_vec2, &her_vec2 );
			q = 1.0f/bp_sqrtf( vec.vx + vec.vy + vec.vz ) ; //BP_MATH - emulate PS2 sqrtf
			_sceVu0ScaleVector( &vec2, &her_vec2, q ) ;
			_sceVu0OuterProduct( &opvec, &vec2, outer );
			opvec.vw = 0.0f ;

			*(FVECTOR *)out_mat->m[3] = her_vec1 ;
			out_mat->m[0][3] = 1.0f ;
			_sceVu0ScaleVector( (FVECTOR *)out_mat->m[2], &vec2, scale ) ;
			_sceVu0ScaleVector( (FVECTOR *)out_mat->m[1], outer, scale ) ;
			_sceVu0ScaleVector( (FVECTOR *)out_mat->m[0], &opvec, scale ) ;
#endif

			//pos,to, dvec0, Zero, (float)timer/4096.0fで補間
			//posに着いたか
			timer += MOVE_TIME<<1;
			if(timer > 4096){
				timer = 0;
				flags &= ~0x0001;
			}
			//AN_Test_Eye2( (FVECTOR*)((FMATRIX*)comdl_pos)->m[3], 2 );
		}else{
			//nop戻る？
		}
		flags = SET_ESCAPE_DATA(flags,timer);
		pos->vw = *(float*)&flags;
		data++;
		pos++;
		to_pos++;
		comdl_pos++;
	}
	TS_Mem_Scr( work->ssltr_data, MEM_SCR_TOP, sizeof(FVECTOR), work->n_ssltr );

}

//ＸＺ平面のみ
static int CheckBound( FVECTOR *bound, FVECTOR *a0, FVECTOR *a1, FVECTOR *b0, FVECTOR *b1 )
{
	FVECTOR		max,min;
	FVECTOR		a_diff,b_diff;

	_sceVu0SubVector( &a_diff, a1, a0 );
	_sceVu0SubVector( &b_diff, b1, b0 );
	//
	min.vx = (a0->vx < b0->vx)?a0->vx:b0->vx;
	min.vy = a0->vy;
	min.vz = (a0->vz < b0->vz)?a0->vz:b0->vz;
	min.vw = 1.0f;
	max.vx = (a1->vx > b1->vx)?a1->vx:b1->vx;
	max.vy = a1->vy;
	max.vz = (a1->vz > b1->vz)?a1->vz:b1->vz;
	max.vw = 1.0f;

	if( max.vx - min.vx < a_diff.vx + b_diff.vx && max.vz - min.vz < a_diff.vz + b_diff.vz ){
		//printf("atari\n");
		if( a0->vx < b0->vx && b0->vx < a1->vx ){
			bound[0].vx = b0->vx;
			if( a0->vx < b1->vx && b1->vx < a1->vx ){
				//	a -------------------
				//		b ----------			の時
				bound[1].vx = b1->vx;
			}else{
				//	a -------------------
				//		b ------------------------
				bound[1].vx = a1->vx;
			}
		}else{
			bound[0].vx = a0->vx;
			if( a0->vx < b1->vx && b1->vx < a1->vx ){
				//		a -------------------
				//	b ---------------			の時
				bound[1].vx = b1->vx;
			}else{
				//		 a ----------
				//	b ---------------------		の時
				bound[1].vx = a1->vx;
			}
		}
		if( a0->vz < b0->vz && b0->vz < a1->vz ){
			bound[0].vz = b0->vz;
			if( a0->vz < b1->vz && b1->vz < a1->vz ){
				bound[1].vz = b1->vz;
			}else{
				bound[1].vz = a1->vz;
			}
		}else{
			bound[0].vz = a0->vz;
			if( a0->vz < b1->vz && b1->vz < a1->vz ){
				bound[1].vz = b1->vz;
			}else{
				bound[1].vz = a1->vz;
			}
		}
		bound[0].vy = a0->vy;
		bound[0].vw = 1.0f;
		bound[1].vy = a0->vy;
		bound[1].vw = 1.0f;
		return 1;
	}
	return 0;
}

static void Act(Work *work)
{
	int		flags;
	//バンダリチェック
	work->low_cm->flag &= ~DG_COMDL_INVISIBLE;
	flags = DG_BoundCheck( &DG_UnitMatrix, &work->bound[0], &work->bound[1] );
	work->low_cm->flag |= (flags&0x03)<<4;
	//プレイヤーとのバンダリチェック

	{
		FVECTOR p_bound[2];
		//FVECTOR bound[2];
		static FVECTOR bound_wide = { 1200.0f, 0.0f, 1200.0f, 0.0f };
		_sceVu0SubVector( &p_bound[0], &GM_PlayerPosition, &bound_wide );
		_sceVu0AddVector( &p_bound[1], &GM_PlayerPosition, &bound_wide );
		if(CheckBound( work->local_bound, &work->bound[0], &work->bound[1], &p_bound[0], &p_bound[1] )){
			//NewBoundingBoxView( &work->local_bound[0], &work->local_bound[1], 16, 16, 16 );
			RunAwayAct( work );
		}else{
			NormalAct( work );
		}
		p_bound[0].vy = work->bound[0].vy;
		p_bound[1].vy = work->bound[0].vy;
		//NewBoundingBoxView( &p_bound[0], &p_bound[1], 16, 16, 16 );
	}
	//
	//NormalAct( work );
	
	//NewBoundingBoxView( &work->escape_bound[0], &work->escape_bound[1], 16, 16, 16 );

	work->move_index+=1;
	if(work->move_index >= work->n_ssltr) work->move_index = 0;

}

static void Die(Work *work)
{

	if(work->high_cm){
		DG_DequeueComdlObjs( work->high_cm );
		DG_FreeComdl( work->high_cm );
	}
	if(work->low_cm){
		DG_DequeueComdlObjs( work->low_cm );
		DG_FreeComdl( work->low_cm );
	}
	
	if(work->ssltr_data) GV_Free(work->ssltr_data);
	if(work->ssltr_pos) GV_Free(work->ssltr_pos);
	if(work->ssltr_to_pos) GV_Free(work->ssltr_to_pos);
}

#if 0
static int InitSsltrObjs( SSLTR_WORK *ssltr_work, int obj_code, int n_objs )
{
	DG_DEF		*def;
	DG_OBJS		*objs;
	int i;
	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("sky_sphere_5"), 'k' ) ) ;
	if(!def){
		printf("ERR!! NO MODEL!!\n");
		return -1;
	}
//	printf("index %d\n",index);
	for( i = 0; i < N_NEAR_SSLTR; i++ ){
		objs = ssltr_work->objs[i] = DG_MakeObjs( def, DG_FLAG_SHADE, 0 );
		if(!objs) return -1;
		if(DG_QueueObjs( objs )<0) return -1;
		
		DG_SetPos( &DG_UnitMatrix);
		DG_PutObjs( objs );
	}
	return 0;

}
#endif

static FVECTOR *InitSsltrData( FVECTOR *bound, int n_ssltr )
{
	FVECTOR		*verts,*data = MEM_SCR_TOP;
	int			i;
	FVECTOR		center,add;

	verts = GV_Malloc( n_ssltr*sizeof(FVECTOR)*N_SSLTR_INTEX );
	if(!verts) return NULL;

	DG_COPY_VEC( &center, &bound[0] );
	_sceVu0SubVector( &add, &bound[1], &bound[0] );
	
	for( i = 0; i < n_ssltr*N_SSLTR_INTEX; i++ ){
		short	rot,index,timer;
		int		rot_index;
		rot = (irnd()>>8)&0x0fff;

		index = (irnd()>>4)%N_SSLTRMOVE_INDEX;
		//timer = ((i < 128)?(MOVE_TIME*(63-i/2)):(0));
		timer = 0;
		rot_index = SET_DATA(rot,timer,index);

		data->vx = center.vx + add.vx*rnd();
		data->vy = center.vy + add.vy*rnd();
		data->vz = center.vz + add.vz*rnd();
		data->vw = *(float*)&rot_index;

		//NewDbugSprite( &verts[i*(n_ssltr*N_SSLTR_INTEX/4)+j], 100 );
		//printf("%d:",i);PRINT_PFVEC(j,data);
		data++;
	}	
	
	TS_Scr_Mem( verts, MEM_SCR_TOP, sizeof(FVECTOR), n_ssltr*N_SSLTR_INTEX );
	return verts;
}

static FVECTOR *InitSsltrToPos( int n_ssltr )
{
	FVECTOR		*verts,*data = MEM_SCR_TOP;
	int			i;

	verts = GV_Malloc( n_ssltr*sizeof(FVECTOR)*N_SSLTR_INTEX );
	if(!verts) return NULL;
	
	for( i = 0; i < n_ssltr*N_SSLTR_INTEX; i++ ){
		data->vy = data->vz;
		data++;
	}	
	TS_Scr_Mem( verts, MEM_SCR_TOP, sizeof(FVECTOR), n_ssltr*N_SSLTR_INTEX );
	return verts;
}

static FVECTOR *InitSsltrPos( int n_ssltr )
{
	FVECTOR		*verts,*data = MEM_SCR_TOP;
	int			i;

	verts = GV_Malloc( n_ssltr*sizeof(FVECTOR)*N_SSLTR_INTEX );
	if(!verts) return NULL;
	
	for( i = 0; i < n_ssltr*N_SSLTR_INTEX; i++ ){
		data->vw = 0.0f;
		data++;
	}	
	TS_Scr_Mem( verts, MEM_SCR_TOP, sizeof(FVECTOR), n_ssltr*N_SSLTR_INTEX );
	return verts;
}

static DG_COMDL *InitSsltrComdl( int cm_code, int n_comdl, int map )
{
	//InitSsltrPosの後に呼び出してスクラッチパッドのデータを使う
	DG_DEF			*def;
	DG_COMDL		*comdl;
	DG_COMDL_POS 	*comdl_pos;
	FVECTOR			*pos = MEM_SCR_TOP;
	int				i;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( cm_code, 'k' ) );
	comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, n_comdl*N_SSLTR_INTEX, 0 );
	if( !comdl ) return NULL;
	DG_QueueComdlObjs( comdl );
	comdl->group_id = GM_GetDGGroupID( map );

	comdl_pos = comdl->pos;
	for( i = 0; i < n_comdl*N_SSLTR_INTEX; i++ ){
		//float	rot_y = PI*frnd()*0.5f;
		//PRINT_PFVEC(i,pos);

		//_sceVu0RotMatrixZ( &comdl_pos->world, &DG_UnitMatrix, PI );
		//_sceVu0RotMatrixY( &comdl_pos->world, &comdl_pos->world, rot_y );
		//_sceVu0RotMatrixY( &comdl_pos->world, &DG_UnitMatrix, rot_y );

		//ここにルートの初期かも入れる
		DG_COPY_MAT( &comdl_pos->world, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR*)comdl_pos->world.m[3], pos );
		comdl_pos->world.m[3][3] = 1.0f;
		//PRINT_PFVEC(0,(FVECTOR*)comdl_pos->world.m[0]);
		//PRINT_PFVEC(1,(FVECTOR*)comdl_pos->world.m[1]);
		//PRINT_PFVEC(2,(FVECTOR*)comdl_pos->world.m[2]);
		//PRINT_PFVEC(3,(FVECTOR*)comdl_pos->world.m[3]);

		comdl_pos->color.vx = 128;
		comdl_pos->color.vy = 128;
		comdl_pos->color.vz = 128;
		comdl_pos->color.vw = 128;

		comdl_pos++;
		pos++;
	}
	return comdl;
}

static int GetResources( Work *work )
{
	int		i;
	int		n_ssltr;
	FVECTOR	bound[4];
	n_ssltr = GCL_GetOptionValue( 'n', 0 );
	work->no_move = GCL_GetOptionValue( 'm', 0 );

	if(GCL_GetOption( 'b' ) == NULL) { printf("ERR!! scn err slater.c\n"); return -1; }
	for( i = 0; i < 2; i++ ){
		bound[i].vx = (float)GCL_GetNextInt();
		bound[i].vy = (float)GCL_GetNextInt();
		bound[i].vz = (float)GCL_GetNextInt();
		bound[i].vw = 1.0f;
	}
	DG_COPY_VEC( &work->bound[0], &bound[0] );
	DG_COPY_VEC( &work->bound[1], &bound[1] );
	work->ssltr_data = InitSsltrData( bound, n_ssltr );
	if(!work->ssltr_data) return -1;

	if(GCL_GetOption( 'e' )){
		for( i = 0; i < 2; i++ ){
			bound[i].vx = (float)GCL_GetNextInt();
			bound[i].vy = (float)GCL_GetNextInt();
			bound[i].vz = (float)GCL_GetNextInt();
			bound[i].vw = 1.0f;
		}
	}
	DG_COPY_VEC( &work->escape_bound[0], &bound[0] );
	DG_COPY_VEC( &work->escape_bound[1], &bound[1] );

	//work->high_cm = InitSsltrComdl( 3475835, n_ssltr, work->map );
	//if(!work->high_cm) return -1;
	work->low_cm = InitSsltrComdl( 10389881, n_ssltr, work->map );
	if(!work->low_cm) return -1;
	work->n_ssltr = n_ssltr*N_SSLTR_INTEX;

	work->ssltr_pos = GV_Malloc( n_ssltr*sizeof(FVECTOR)*N_SSLTR_INTEX );
	if(!work->ssltr_pos) return -1;
	work->ssltr_pos = InitSsltrPos( n_ssltr );

	work->ssltr_to_pos = GV_Malloc( n_ssltr*sizeof(FVECTOR)*N_SSLTR_INTEX );
	if(!work->ssltr_to_pos) return -1;
	work->ssltr_to_pos = InitSsltrToPos( n_ssltr );
	
	//printf("%d匹の",n_ssltr*N_SSLTR_INTEX);
	return 0;
}

void *NewPrevTest( int name, int map )
{
	Work *work = NULL;

	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		printf("ふなむし！！第四弾 %d\n",N_SSLTRMOVE_INDEX);
	}
	
	return (void *)work ;
}

#endif
