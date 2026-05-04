//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    test_slater.c
	舟りんテスト
	2000/02/07 T.Shibata
	
	$Id: body_slater.c,v 1.1.1.3 2002/11/19 11:48:48 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"body_route.h"

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )


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

#define		N_SSLTS			(3)
//#define		N_SSLTS			(128)
#define		N_MEM_INDEX		(12)

typedef struct {
	u_char		index[N_MEM_INDEX];
	u_short		timer;
	short		flags;
} SSLTR_DATA;


typedef struct _work {
	GV_ACT_EX		actor;

	SSLTR_DATA		ssltr_data[N_SSLTS];
	int				name;
	int				map;
	int				*body_map;
	DG_COMDL		*low_cm;
	CV2_DEF			*cv2_def;
	OBJECT			*body;
	int				joint_index;
	FMATRIX			*root;					//

	short			flags;
	short			se_frip;
} Work;

//extern int DG_QueueComdlObjs( DG_COMDL *comdl );
//extern void DG_DequeueComdlObjs( DG_COMDL *comdl );
//extern void *NewSquareView( FVECTOR*, int, u_char, u_char, u_char );

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
	ans = _sceVu0InnerProduct(&tmp,&tmp);
#endif
	return (ans);
}

#ifndef BP_PS2
static FMATRIX *Mat2, *Mat3 ;
#endif

static void LoadMatHermiteLerp()//FMATRIX *root )
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

	":: "r"(&c_mat2), "r"(&c_mat3) );
#else
	Mat2 = &c_mat2;
	Mat3 = &c_mat3;
#endif

}

#ifndef BP_PS2
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

static void SetSsMatrix( FMATRIX *out_mat,
						 FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2,
						 FVECTOR *nor0, FVECTOR *nor1,
						 FVECTOR *her )
{

#ifdef BP_PSX2_ASM
	asm volatile("
		lqc2			vf12,0x00(%1)	# pos0
		lqc2			vf13,0x00(%2)	# pos1
		lqc2			vf24,0x00(%3)	# pos2
		lqc2			vf01,0x00(%6)	# her係数

		vsub.xyzw		vf14, vf13, vf12	#pos1 - pos0
		#vmulx.w			vf14, vf00, vf00	#vec0.vw = 0.0f
		vsub.xyzw		vf15, vf24, vf12	#pos2 - pos1
		#vmulx.w			vf15, vf00 ,vf00	#vec1.vw = 0.0f

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

		# norms load
		lqc2			vf12,0x00(%4)	# nor0
		lqc2			vf13,0x00(%5)	# nor1
		
		vmulax.xyzw		ACC ,vf16,vf02	#pos
		vmadday.xyzw	ACC ,vf17,vf02
		vmaddaz.xyzw	ACC ,vf18,vf02
		vmaddw.xyzw		vf15,vf19,vf00	#pos

		vmulax.xyzw		ACC ,vf20,vf02	#vec
		vmadday.xyzw	ACC ,vf21,vf02
		vmaddaz.xyzw	ACC ,vf22,vf02
		vmaddw.xyzw		vf25,vf23,vf00	#vec

		vmulay.xyzw		ACC ,vf12, vf01y	#norm
		vmaddx.xyzw		vf13 ,vf12, vf01x

		vmul.xyz		vf01, vf25,  vf25	#vec normalize
		vmulax.w		ACC,  vf00,  vf01
		vmadday.w		ACC,  vf00,  vf01
		vmaddz.w		vf01, vf00,  vf01
		vrsqrt			Q,    vf00w, vf01w
		vwaitq
		vmulq.xyz		vf14, vf25, Q

		vopmula.xyz		ACC,  vf14, vf13
		vopmsub.xyz		vf12, vf13, vf14

		vmulx.w		vf13,vf0,vf0	#out_mat y.vw = 0.0f
		vmulx.w		vf12,vf0,vf0	#out_mat x.vw = 0.0f		

		sqc2		vf12 ,0x00(%0)
		sqc2		vf13 ,0x10(%0)
		sqc2		vf14 ,0x20(%0)
		sqc2		vf15 ,0x30(%0)

	": : "r"(out_mat), "r"(pos0), "r"(pos1), "r"(pos2), "r"(nor0), "r"(nor1), "r"(her) :"memory");
#else
	FMATRIX	m1 ;
	FVECTOR vec, vec2, opvec, her_vec1, her_vec2, tmp1, tmp2, vec3 ;
	float q;
	
	*(FVECTOR *)m1.m[0] = *pos0 ;
	*(FVECTOR *)m1.m[1] = *pos1 ;
	_sceVu0SubVector( (FVECTOR *)m1.m[2], pos1, pos0 ) ;
	_sceVu0SubVector( (FVECTOR *)m1.m[3], pos2, pos0 ) ;
	HermiteLerpVec( &her_vec1, &her_vec2, &m1, her->vx ) ;

	_sceVu0ScaleVector( &tmp1, nor0, her->vy );
	_sceVu0ScaleVector( &tmp2, nor0, her->vx );
	_sceVu0AddVector( &vec3, &tmp1, &tmp2 );
	_sceVu0Normalize( &vec2, &her_vec2 );  
	vec2.vw = 0.0f; /*_sceVu0Normalize の vw は 0.0f とは限らない yano 2002.04.01*/
	_sceVu0OuterProduct( &opvec, &vec2, &vec3 );
	opvec.vw = 0.0f ;
	vec3.vw = 0.0f ;

	*(FVECTOR *)out_mat->m[3] = her_vec1 ;
	*(FVECTOR *)out_mat->m[2] = vec2 ;
	*(FVECTOR *)out_mat->m[1] = vec3 ;
	*(FVECTOR *)out_mat->m[0] = opvec ;
#endif
}


static inline FVECTOR* GetCV2Verts( CV2_DEF *def, int now_joint, int now_index )
{
	int		joint = now_joint;
	int		index = now_index;
	CV2_MDL *mdl;
	if( joint >= N_CV2_MDL ) joint = N_CV2_MDL-1;
	if( index >= FunaJointNextTableNum[joint] ){
		printf("ERR!! INDEX OVER\n");
		index = FunaJointNextTableNum[joint] - 1;
	}
	mdl = &def->models[joint];
	return &mdl->verts[index];
}

static inline FVECTOR* GetCV2Norms( CV2_DEF *def, int now_joint, int now_index )
{
	int		joint = now_joint;
	int		index = now_index;
	CV2_MDL *mdl;
	if( joint >= N_CV2_MDL ) joint = N_CV2_MDL-1;
	if( index >= FunaJointNextTableNum[joint] ){
		printf("ERR!! INDEX OVER\n");
		index = FunaJointNextTableNum[joint] - 1;
	}
	mdl = &def->models[joint];
	return &mdl->norms[index];
}

static inline u_char *GetNextVertsTable( int now_joint, int now_index )
{
	int		joint = now_joint;
	int		index = now_index;
	u_char	*list;
	if( joint >= N_CV2_MDL ) joint = N_CV2_MDL-1;
	if( index >= FunaJointNextTableNum[joint] ){
		//printf("ERR!! INDEX OVER\n");
		index = FunaJointNextTableNum[joint] - 1;
	}

	list = FunaJointNextTable[joint];
	list = &list[ index*8 ];

	return list;
}

static void SetNextIndex( u_char *index_list, int joint )
{
	u_char	*next_list;
	int		i,j,cnt = 0,index_cun[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int		tmp_index_cnt = 256;
	int		tmp_next_index[8];

	next_list = GetNextVertsTable( joint, index_list[N_MEM_INDEX-1] );

	for( i = 0; i < N_MEM_INDEX; i++ ){
		for( j = 0; j < 8; j++ ){
			if( next_list[j] == index_list[i] )
				index_cun[j]++;
		}
	}
	for( i = 0; i < 8; i++ ){
		if( next_list[i] != 255 ){
			if( tmp_index_cnt > index_cun[i] ){
				tmp_index_cnt = index_cun[i];
				tmp_next_index[0] = next_list[i];
				cnt = 1;
			}else if( tmp_index_cnt == index_cun[i] ){
				tmp_next_index[cnt] = next_list[i];
				cnt++;
			}
		}
	}
	//printf("Index %d] %3d, %3d, %3d, %3d  =>", index_list[N_MEM_INDEX-1],
	//	   next_list[0], next_list[1], next_list[2], next_list[3] );

	memmove( &index_list[0],  &index_list[1], sizeof(u_char)*(N_MEM_INDEX-1) );
	index_list[N_MEM_INDEX-1] = tmp_next_index[(irnd()>>13)%cnt];

	//printf("next_index = %d\n", index_list[N_MEM_INDEX-1] );
}

static void CrawlAct( FMATRIX *pos, SSLTR_DATA *data, CV2_DEF *cv2_def, FMATRIX *root, int joint_index )
{
	FMATRIX		world;
	FVECTOR		her;
	FVECTOR		*pos0,*pos1,*pos2;//,shft_pos0,shft_pos1;
	FVECTOR		*nor0,*nor1,norm0,norm1;
	// func ( An Bn ) -> Sn;
	// func ( Av Bv Bn ) -> Smat
	her.vx = (float)data->timer/4096.0f;
	her.vy = 1.0f - (float)data->timer/4096.0f;
		
	pos0 = GetCV2Verts( cv2_def, joint_index, data->index[N_MEM_INDEX-3] );
	pos1 = GetCV2Verts( cv2_def, joint_index, data->index[N_MEM_INDEX-2] );
	pos2 = GetCV2Verts( cv2_def, joint_index, data->index[N_MEM_INDEX-1] );
		
	nor0 = GetCV2Norms( cv2_def, joint_index, data->index[N_MEM_INDEX-3] );
	nor1 = GetCV2Norms( cv2_def, joint_index, data->index[N_MEM_INDEX-2] );
	_sceVu0ScaleVector( &norm0, nor0, -1.0f );
	_sceVu0ScaleVector( &norm1, nor1, -1.0f );
#if 0
	_sceVu0ScaleVector( &shft_pos0, nor0, -5.0f );
	_sceVu0AddVector( &shft_pos0, &shft_pos0, pos0 );
	//if( i == 0 ) PRINT_PFVEC( i, &shft_pos0 );
	shft_pos0.vw = 1.0f;
	_sceVu0ScaleVector( &shft_pos1, nor1, -5.0f );
	_sceVu0AddVector( &shft_pos1, &shft_pos1, pos1 );
	shft_pos1.vw = 1.0f;
	LoadMatHermiteLerp( /*work->root*/ );
	SetSsMatrix( &world, &shft_pos0, &shft_pos1, pos2, &norm0, &norm1, &her );
#else
	LoadMatHermiteLerp( /*work->root*/ );
	SetSsMatrix( &world, pos0, pos1, pos2, &norm0, &norm1, &her );
#endif
	//(FMATRIX*)comdl_pos
	//ここでマトリックスがけ
	_sceVu0MulMatrix( pos, root, &world );
		
	data->timer += 128;
	if( data->timer > 4096 ){
		data->timer -= 4096;
		SetNextIndex( data->index, joint_index );
		//pos0 = GetCV2Verts( work->cv2_def, work->joint_index, data->index[N_MEM_INDEX-1] );
	}
			
}

static void InitSsltrPosData( SSLTR_DATA *w_data, int n_ssltr )
{
	SSLTR_DATA		*data = w_data;
	int			i;

	for( i = 0; i < n_ssltr; i++ ){
		memset( data->index, 0 , sizeof(u_char)*N_MEM_INDEX );
		data->flags = 0;
		data->timer = irnd()%4096;
		data++;
	}
}

static void InitSsltrPosDataDrop( SSLTR_DATA *w_data, int n_ssltr )
{
	SSLTR_DATA		*data = w_data;
	int			i;

	for( i = 0; i < n_ssltr; i++ ){
		data->flags = 16+irnd()%128;
		data++;
	}

}


static int CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;//,i;

	mes_num = GV_ReceiveMessage( work->name, &msg );

	msg += mes_num-1;
	//for( i = 0; i < mes_num; i++ ){
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case 0:
			//身体を這います。
			if( work->flags == 0 ){
				InitSsltrPosData( work->ssltr_data, N_SSLTS );
				work->flags = 1;
			}
			break;
		  case 1:
			//身体から落ち始めます。
			if( work->flags == 1 ){
				InitSsltrPosDataDrop( work->ssltr_data, N_SSLTS );
				work->flags = 2;
			}
			break;
		  default:
			break;
		}
		msg--;
	}

	return work->flags;
}

static void Act(Work *work)
{
	int				i, check = N_SSLTS;
	DG_COMDL_POS 	*comdl_pos = work->low_cm->pos;
	SSLTR_DATA		*data = work->ssltr_data;
#if 0
	if( GV_PadDataDirect[0].press & PAD_X ){
		InitSsltrPosData( work->ssltr_data, N_SSLTS );
		work->flags = 1;
	}else if( GV_PadDataDirect[0].press & PAD_Y ){
		if( work->flags == 1 ){
			InitSsltrPosDataDrop( work->ssltr_data, N_SSLTS );
			work->flags = 2;
		}
	}
	//printf("drop out %d\n",work->flags);
#else
	if( !CheckMesgParam( work ) ){
		GV_WaitMessage( work, work->name );
		return;
	}
#endif
/*
	//関数テスト
	{
		u_char		*dbg_list = GetNextVertsTable( work->joint_index, data->index[N_MEM_INDEX-1] );
	}
*/
	if( !work->flags ||
		(GM_CheckPlayerStatus( PLAYER_WATCH ) &&
		(GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0)) ){
		
		work->low_cm->flag |= DG_COMDL_INVISIBLE;
		return;
	}else{
		work->low_cm->flag &= ~DG_COMDL_INVISIBLE;
	}
	work->low_cm->group_id = GM_GetDGGroupID( *work->body_map );
	//LoadMatHermiteLerp( /*work->root*/ );

	for( i = 0; i < N_SSLTS; i++ ){
		if( work->flags == 1 ){
			CrawlAct( (FMATRIX*)comdl_pos, data, work->cv2_def, work->root, work->joint_index );
		}else{
			
			//落下処理初期化
			if( --data->flags < -120 ){
				//スリープへ
				check--;
			}else if( data->flags < 16 ){
				//落下中

				FVECTOR *nor0 = GetCV2Norms( work->cv2_def, work->joint_index, data->index[N_MEM_INDEX-3] );
				FVECTOR vec;
				_sceVu0ScaleVector( &vec, nor0, -8.0f );
				_sceVu0AddVector( (FVECTOR*)((FMATRIX*)comdl_pos)->m[3],
								  (FVECTOR*)((FMATRIX*)comdl_pos)->m[3], &vec );
				((FMATRIX*)comdl_pos)->m[3][3] = 1.0f;

				((FMATRIX*)comdl_pos)->m[3][1] += (float)data->flags * 2.0f;
				

			}else{
				
				CrawlAct( (FMATRIX*)comdl_pos, data, work->cv2_def, work->root, work->joint_index );
			}
		}

#if 0
		{
			extern void *NewTsDebugLine( FVECTOR *verts, int n_verts, int color );
			FVECTOR		line[N_MEM_INDEX];
			for( j = 0; j < N_MEM_INDEX; j++ ){
				//DG_COPY_VEC( &line[j], GetCV2Verts( work->cv2_def, work->joint_index, data->index[j] ) );
				//PRINT_PFVEC( j, GetCV2Verts( work->cv2_def, work->joint_index, data->index[j] ) );
				_RotTrans( &line[j], work->root, GetCV2Verts( work->cv2_def, work->joint_index, data->index[j] ) );
			}
			NewTsDebugLine( line, N_MEM_INDEX, 0xa0404080 );
		}
#endif
		comdl_pos++;
		data++;
	}
	if( !check ){
		work->flags = 0;
		work->low_cm->flag |= DG_COMDL_INVISIBLE;
	}

}

static void Die(Work *work)
{
	if(work->low_cm){
		DG_DequeueComdlObjs( work->low_cm );
		DG_FreeComdl( work->low_cm );
	}	
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
		DG_COPY_MAT( &comdl_pos->world, &DG_UnitMatrix );
		comdl_pos->color.vx = col&0xff;
		comdl_pos->color.vy = (col>>8)&0xff;
		comdl_pos->color.vz = (col>>16)&0xff;
		comdl_pos->color.vw = 128;
		comdl_pos++;
		pos++;
	}
	return comdl;
}


static int GetResources( Work *work )
{
//	FVECTOR			center;
//	SSLTR_DATA			*ssltr_data;
//	int					color,i;

	//work->body = EMA_CommandGetBody();
	work->body = GM_PlayerBody;
	work->body_map = &GM_PlayerMap;
	work->joint_index = GCL_GetOptionValue( 'j', 0 );//18;
	work->root = &work->body->objs->objs[work->joint_index].world;

	work->flags = 0;

	work->cv2_def = (CV2_DEF*)GV_GetCache( GV_CacheID( ROOTMDL_CODE, 'c' ));
	if( work->cv2_def == NULL){ printf("ERR!! NO MODEL cv2!!\n"); return -1; }

//	color = 0x00606060;
	work->low_cm = InitSsltrComdl( 10389881, N_SSLTS, 0x00a0a0a0, work->map );
	if(!work->low_cm) { printf("cm init err!!\n"); return -1; }

	InitSsltrPosData( work->ssltr_data, N_SSLTS );
	return 0;
}

void *NewBodySlater( int name, int map )
{
	Work *work = NULL;

//	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	work = (Work*)GV_CreateActor( GV_ACTOR_EFFECT, GV_CLASS_EFFECT, sizeof(Work), 16 );
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		work->name = name;
		work->map = map;
		if(GetResources( work ) < 0){
			GV_DestroyActor(work);
			return NULL;
		}
		printf("舟虫！！\n");
	}
	
	return (void *)work ;
}
