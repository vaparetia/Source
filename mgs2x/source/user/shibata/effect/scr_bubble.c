//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_drop.c
	歪みモデルを使った奴(歪み変化)

	2001/07/10 T.Shibata
	
	$Id: scr_bubble.c,v 1.4 2002/11/23 12:36:04 Yoshizawa1 Exp $

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
#include "libutl.h"

#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include	"def_dma.h"
#include	"../util/ts_util.h"
#include	"../util/dma_set.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

#define XY_OFF			(2048.0f)
#define XY_SCL			(2048.0f)
#define DWIDTH_OFF		((float)(DRAW_WIDTH>>1))
#define HEIGHT_OFF		((float)(DRAW_HEIGHT>>1))
#define DWIDTH_SCL		((float)(DRAW_WIDTH>>1))
#define HEIGHT_SCL		((float)(DRAW_HEIGHT>>1))

#define MDL_CODE 	(GV_StrCode("scr_kihou01"))

#define MDL_PRIM		(SCE_GS_SET_PRIM( 4, 1, 1, 0, 1, 0, 1, 0, 0 ))
#define MDL_TEST		(SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA		(SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ))

#define TEX_PRIM		(SCE_GS_SET_PRIM( 4, 1, 1, 0, 1, 0, 0, 0, 0 ))
#define TEX_RGBA		(SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0x3F800000))
#define TEX_ALPHA_MODE	(SCE_GS_SET_ALPHA(0,1,0,1,0x80))

#define		N_DROPS		(8)

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
#define		TRI_CODE	(GV_StrCode("kihou"))
#define		TEX_CODE	(GV_StrCode("scr_kihou01_alp_ovl"))

typedef struct {
	float		scl_x, scl_y, x, y;
	float		z;
	short		rot_x, rot_y;
	u_short		timer, r_timer;
	u_char		trans_x, trans_y, flags, step;
	
} DROP_DATA;

//#define		DEF_TEX_DRAW

typedef struct {
	DG_DMATAG		dmatag_flush;				// ＧＩＦ接続ＤＭＡタグ
	DROW_FLUSH 		drow_flush;				//								退避(フレームをバックに)
	DG_DMATAG		dmatag_mdl;				// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;				//								モデル描画の設定
	DG_DMATAG		dmatag_mverts;			// ＧＩＦ接続ＤＭＡタグ			モデル頂点(変化)
#ifdef DEF_TEX_DRAW
	DG_DMATAG		dmatag_flush0;			// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush0;			//								フラッシュ
	u_long128		tex_packet_alp[2];		//								テクスチャー転送
	DG_DMATAG		dmatag_flush1;			// ＧＩＦ接続ＤＭＡタグ			フラッシュ
	DROW_FLUSH 		drow_flush1;			//								フラッシュ
	DG_DMATAG		dmatag_tex;				// ＧＩＦ接続ＤＭＡタグ			スプライト描画の設定
	TEX_DRAW		tex_draw;				//								スプライト描画の設定
	DG_DMATAG		dmatag_tverts;			// ＧＩＦ接続ＤＭＡタグ			モデル頂点(変化)
#endif
	DG_DMATAG		dmatag_test;			// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;				//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;			// オフセット環境復元用
	DG_DMATAG		dmatag_end;				// RETタグ
} DROPS_PACK;

typedef	struct	{
	GV_ACT_EX		actor ;

	ALIGN16_PRE DROPS_PACK		*drops_pack[2] ALIGN16_POST;
	ALIGN16_PRE void			*drow_verts[2] ALIGN16_POST;
	ALIGN16_PRE DROP_DATA		drop_data[N_DROPS] ALIGN16_POST;
#ifdef DEF_TEX_DRAW
	ALIGN16_PRE void			*tex_verts[2] ALIGN16_POST;
	DG_TEX_PACKET	*tex_packet[2];
#endif
	
	FVECTOR			pre_campos;
	FVECTOR			*campos;
	FVECTOR			*camdir;
	SVECTOR			pre_camdir;
	CV2_DEF			*cv2_def;
	DG_DMAPACK		*dmapack;
	
	int				flags;
	int				interval;


#ifndef PSX2
	DG_DEF              *def ;
	void				*packet_mem;
	void                *prim ;
	int 				xbox_packet_size;
#endif

} Work;

extern float GM_WaterLevel;
static Work *g_work = NULL;

//モデルによって変更

/*
関節０～２：デフォルト揺らぎ
関節３：右上溜り
関節４：左上溜り
関節５：上溜り
*/
#ifdef PSX2
#define		N_CV2_VERTS		(75)
#define		N_KMS_VERTS		(154)
#define		N_MDL_JOINT		(3)
#else
#define		N_CV2_VERTS		(80)
#define		N_KMS_VERTS		(100)
#define		N_MDL_JOINT		(3)
#endif

#ifdef BP_PS2
#define		MEM_SCR_VERTS1		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VERTS2		((void *)(MEM_SCR_VERTS1) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS3		((void *)(MEM_SCR_VERTS2) + sizeof(FVECTOR)*N_CV2_VERTS)

#define		MEM_SCR_NORMS1		((void *)(MEM_SCR_VERTS3) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS2		((void *)(MEM_SCR_NORMS1) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS3		((void *)(MEM_SCR_NORMS2) + sizeof(FVECTOR)*N_CV2_VERTS)

#define		MEM_SCR_INDEX		((void *)(MEM_SCR_NORMS3) + sizeof(FVECTOR)*N_CV2_VERTS)

//#define		MEM_SCR_DROPS		((FVECTOR*)((void *)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
//#define		MEM_SCR_X			((int*)((void *)(MEM_SCR_DROPS) + sizeof(DROP_DATA)*N_DROPS ))
#define		MEM_SCR_X			((int*)((void*)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
#define		MEM_SCR_Y			((int*)((void*)(MEM_SCR_X) + sizeof(int) ))
#define		MEM_SCR_BOTTOM		(((void *)(MEM_SCR_Y) + sizeof(int) ))

#else
#define		MEM_SCR_VERTS1		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VERTS2		((void *)((char *)(MEM_SCR_VERTS1) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS3		((void *)((char *)(MEM_SCR_VERTS2) + sizeof(FVECTOR)*N_CV2_VERTS))

#define		MEM_SCR_NORMS1		((void *)((char *)(MEM_SCR_VERTS3) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS2		((void *)((char *)(MEM_SCR_NORMS1) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS3		((void *)((char *)(MEM_SCR_NORMS2) + sizeof(FVECTOR)*N_CV2_VERTS))

#define		MEM_SCR_INDEX		((void *)((char *)(MEM_SCR_NORMS3) + sizeof(FVECTOR)*N_CV2_VERTS))

//#define		MEM_SCR_DROPS		((FVECTOR*)((void *)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
//#define		MEM_SCR_X			((int*)((void *)(MEM_SCR_DROPS) + sizeof(DROP_DATA)*N_DROPS ))
#define		MEM_SCR_X			((int*)((char*)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
#define		MEM_SCR_Y			((int*)((char*)(MEM_SCR_X) + sizeof(int) ))
#define		MEM_SCR_BOTTOM		((void *)((char *)(MEM_SCR_Y) + sizeof(int) ))

#endif


#define		SCALE_PRIMA		(24.0f/1000.0f)
//#define		SCALE_PRIMA		(24.0f/200.0f)
#define		UV_SCALE		(32.0f)

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
	": : "r"(out), "r"(world), "r"(in) );//:"memory" );
#else
#if 0 ///
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#else
	_sceVu0ApplyMatrix( out, world, in );
#endif
#endif
}

#ifndef BP_PS2
static FVECTOR	_acc, _vf18, _vf17, _vf16, _vf04, _vf06 ;
#endif

static void _LoadScaleDividingPoint1( FVECTOR *scl_norms )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf18, 0x00(%0)	#scale_norm
	": : "r"(scl_norms) );
#else
	_vf18 = *scl_norms ;
#endif
}

static void _LoadScaleDividingPoint2( FVECTOR *key, FVECTOR *scl_verts )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf16, 0x00(%0)	#各係数
		lqc2			vf17, 0x00(%1)	#scale_vec
	": : "r"(key), "r"(scl_verts) );
#else
	_vf16 = *key ;
	_vf17 = *scl_verts ;
#endif
}

static void _GetDividingPoint( FVECTOR *out_v, FVECTOR *out_n,
							   FVECTOR	*k_vert, FVECTOR *l_vert, FVECTOR *m_vert,
							   FVECTOR	*k_norm, FVECTOR *l_norm, FVECTOR *m_norm )
{
#ifdef BP_PSX2_ASM
	// k*l_verts + l*l_verts + m*m_verts
	asm volatile ("
		lqc2			vf08, 0x00(%2)	# k_vert
		lqc2			vf09, 0x00(%3)	# l_vert
		lqc2			vf10, 0x00(%4)	# m_vert

		lqc2			vf11, 0x00(%5)	# k_norm
		lqc2			vf12, 0x00(%6)	# l_norm
		lqc2			vf13, 0x00(%7)	# m_norm

		vmulax.xyz		ACC,  vf08, vf16x	# k*k_vert
		vmadday.xyz		ACC,  vf09, vf16y	# l*l_vert
		vmaddz.xyz		vf04, vf10, vf16z	# m*m_vert

		vmulax.xyz		ACC,  vf11, vf16x	# k*k_norm
		vmadday.xyz		ACC,  vf12, vf16y	# l*l_norm
		vmaddz.xyz		vf06, vf13, vf16z	# m*m_norm

		vmul.xyz		vf05, vf04, vf17	# verts * scale
		vmul.xyz		vf07, vf06, vf18	# norms * scale

		sqc2	        vf05,0x00(%0)
		sqc2	        vf07,0x00(%1)

	": : "r"(out_v), "r"(out_n),
		 "r"(k_vert), "r"(l_vert), "r"(m_vert),
		 "r"(k_norm), "r"(l_norm), "r"(m_norm) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, k_vert, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf04, l_vert, _vf16.vy ) ;
	_sceVu0AddVector( &_acc, &_vf04, &_acc );
	_sceVu0ScaleVectorXYZ( &_vf04, m_vert, _vf16.vz ) ;
	_sceVu0AddVector( &_vf04, &_vf04, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, k_norm, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf06, l_norm, _vf16.vy ) ;
	_sceVu0AddVector( &_acc, &_vf06, &_acc );
	_sceVu0ScaleVectorXYZ( &_vf06, m_norm, _vf16.vz ) ;
	_sceVu0AddVector( &_vf06, &_vf06, &_acc );

	_sceVu0MulVector( out_v, &_vf04, &_vf17 ) ;
	_sceVu0MulVector( out_n, &_vf06, &_vf18 ) ;
#endif
}

static int SetDropVerts( void *output, void *output1, DROP_DATA *pdrops, CV2_DEF *cv2_def, int n_drops )
{
	int				i,j,n_verts,cnt = 0;
	DROP_DATA		*drops = pdrops;
#ifndef PSX2
	Work            *work = (Work *)output ;
	float			ix,iy,iu,iv ;
	int n_packs, k;
	DG_VERTEX_KMSS *vbuff;

#else /*PSX2*/
	MVERT_DATA_F		*mverts = (MVERT_DATA_F*)(((DG_GIFTAG*)output)+1);
#ifdef DEF_TEX_DRAW
	MVERT_DATA_F		*tverts = (MVERT_DATA_F*)(((DG_GIFTAG*)output1)+1);
#endif
	int				ix,iy,iu,iv;
#endif /*PSX2*/
	short			*index;
	FVECTOR			*k_verts = (FVECTOR*)MEM_SCR_VERTS1;
	FVECTOR			*l_verts = (FVECTOR*)MEM_SCR_VERTS2;
	FVECTOR			*m_verts = (FVECTOR*)MEM_SCR_VERTS3;
	
	FVECTOR			*k_norms = (FVECTOR*)MEM_SCR_NORMS1;
	FVECTOR			*l_norms = (FVECTOR*)MEM_SCR_NORMS2;
	FVECTOR			*m_norms = (FVECTOR*)MEM_SCR_NORMS3;
	static FVECTOR	scale_norms = { UV_SCALE, UV_SCALE, 1.0f, 0.0f };
	
#ifdef PSX2
	n_verts = N_KMS_VERTS;//cv2_def->models[0].n_verts_index;
#else
	n_verts = work->def->models[0].packs->n_indices;
	n_packs = work->def->models[0].n_packs;
#endif

#ifdef PSX2
	TS_Mem_Scr( MEM_SCR_VERTS1, cv2_def->models[0].verts, sizeof(FVECTOR), N_CV2_VERTS*N_MDL_JOINT*2 );
	TS_Mem_Scr( MEM_SCR_INDEX, cv2_def->models[0].verts_index, sizeof(u_long128), (N_KMS_VERTS+3)/4 );
#else //XBOX
	TS_Mem_Scr( MEM_SCR_VERTS1, cv2_def->models[0].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS2, cv2_def->models[1].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS3, cv2_def->models[2].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS1, cv2_def->models[0].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS2, cv2_def->models[1].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS3, cv2_def->models[2].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_INDEX, cv2_def->models[0].verts_index, sizeof(u_long128), (N_KMS_VERTS+3)/4 );
#endif

	_LoadScaleDividingPoint1( &scale_norms );

//	index = MEM_SCR_INDEX;
//	for( j = 0; j < n_verts; j++ ){
//		printf( "index [%d][%d]\n", j,  *index);
//		index++;
//	}
	for( i = 0; i < n_drops; i++ ){
		FVECTOR			stlm = {
			DG_FABS( TS_SINs( drops->rot_x ) * TS_COSs( drops->rot_y ) ),
			DG_FABS( TS_SINs( drops->rot_y ) ),
			DG_FABS( TS_COSs( drops->rot_x ) * TS_COSs( drops->rot_y ) ),
			0.0f,
		};
		FVECTOR			scale_verts = { drops->scl_x, drops->scl_y, 1.0f, 0.0f };
		//float			c = TS_COSs( drops->rot_x );
		//float			s = TS_SINs( drops->rot_x );		
		//float			alpha_ratio = 1.0f;//(float)drops->trans_x/max_life;

/*		if( (GV_Time % 1600) < 400 ){
			stlm.vx = 1.0f;
			stlm.vy = 0.0f;
			stlm.vz = 0.0f;
			stlm.vw = 0.0f;
		}else if( (GV_Time % 1600) < 800 ){
			stlm.vx = 0.0f;
			stlm.vy = 1.0f;
			stlm.vz = 0.0f;
			stlm.vw = 0.0f;
		}else if( (GV_Time % 1600) < 1200 ){
			stlm.vx = 0.0f;
			stlm.vy = 0.0f;
			stlm.vz = 1.0f;
			stlm.vw = 0.0f;
		}else{
*/
//			printf("x[%d]y[%d]",drops->rot_x,drops->rot_y);PRINT_PFVEC(drops->timer,&stlm);
//		}
		drops->trans_x = 0;
		drops->trans_y = 0;
		*MEM_SCR_X = (int)drops->x;
		*MEM_SCR_Y = (int)drops->y;

		_LoadScaleDividingPoint2( &stlm, &scale_verts );
		if( drops->step ){

#ifdef PSX2
		index = MEM_SCR_INDEX;
#else
		//printf("n_pack[%d]\n",n_packs);
		for( k = 0; k < n_packs; k ++ ){
			n_verts = (work->def->models[0].packs + k)->n_indices;
			index = (work->def->models[0].packs + k)->index;
			work->prim = DG_SetDmapackTriangleStrip( work->prim, n_verts );
			vbuff = work->def->models[2].vbuff;
#endif

			for( j = 0; j < n_verts; j++ ){
				FVECTOR		verts;
				FVECTOR		norms;

				_GetDividingPoint( &verts, &norms,
									&k_verts[*index], &l_verts[*index], &m_verts[*index],
									&k_norms[*index], &l_norms[*index], &m_norms[*index] );
				//printf("[%d]%f,%f,%f\n",*index,m_norms[*index].vx,m_norms[*index].vy,m_norms[*index].vz );
				//printf("[%d]%f,%f,%f\n",*index,vbuff[*index].nx/32767.0f,vbuff[*index].ny/32767.0f,vbuff[*index].nz/32767.0f );
#ifdef PSX2
				ix = (int)verts.vx;
				iy = (int)verts.vy;
				iu = ix - (int)norms.vx;
				iv = iy - (int)norms.vy;
				ix = ((2048 + ix + (*MEM_SCR_X))<<4);
				iy = ((2048 + iy + (*MEM_SCR_Y))<<4);
				iu = ((DRAW_WIDTH/2 + iu + (*MEM_SCR_X))<<4);
				iv = ((DRAW_HEIGHT/2 + iv + (*MEM_SCR_Y))<<4);

#else /* PSX2 */

				ix = verts.vx;
				iy = verts.vy;
				iu = (ix - norms.vx);
				iv = (iy - norms.vy);
				ix = DRAW_WIDTH /2 + ix + (*MEM_SCR_X);
				iy = DRAW_HEIGHT/2 + iy + (*MEM_SCR_Y);
				iu = DG_FRAME_U(DRAW_WIDTH /2 + iu + (*MEM_SCR_X));
				iv = DG_FRAME_V(DRAW_HEIGHT/2 + iv + (*MEM_SCR_Y));

#endif /* PSX2 */

				if( iu > MAX_U ) iu = MAX_U;
				else if( iu < MIN_UV ) iu = MIN_UV;
				if( iv > MAX_V ) iv = MAX_V;
				else if( iv < MIN_UV ) iv = MIN_UV;
			
				if( iy > MAX_Y ){
					iy = MAX_Y;
					drops->trans_x++;
				}else if( iy < MIN_Y ){
					iy = MIN_Y;
					drops->trans_x++;
				}
				if( ix > MAX_X ){
					ix = MAX_X;
					drops->trans_y++;
				}else if( ix < MIN_X ){
					ix = MIN_X;
					drops->trans_y++;
				}

#ifdef PSX2
#ifdef DEF_TEX_DRAW
				tverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
				tverts++;
#endif
				mverts->uv.data = SCE_GS_SET_UV(iu,iv);
				mverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
				mverts++;
#else /*XBOX*/
				work->prim = DG_SetDmapackVertex( work->prim, ix,iy, iu,iv,
												 SCE_GS_SET_RGBAQ(148,156,164,128,0) ) ;

#endif /*XBOX*/
				index++;
			}
			cnt++;
#ifdef KP_XBOX
		}
#endif
		}
		drops++;
	}
	return cnt;
}

enum {
	DROP_STEP_SLEEP = 0,
	DROP_STEP_OFFCAM,
	DROP_STEP_ONCAM,
	DROP_STEP_PRESLEEP,
	DROP_STEP_PRESLEEP2,
};

#define		INIT_Z_POS		(700.0f)

static void ActiveOnCamDropAct( DROP_DATA *drops, float off_x, float off_y, float off_z )
{
	int		temp = 2;
	
	drops->rot_x = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;
	drops->rot_y = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;

	drops->scl_x = SCALE_PRIMA * (1.0f + (float)drops->trans_x/80.0f);
	drops->scl_y = SCALE_PRIMA * (1.0f + (float)drops->trans_y/80.0f);

	//printf("trans_x %d:trans_y %d\n",drops->trans_x,drops->trans_y);
	
	if( drops->x > -256.0f && drops->x < 256.0f ) temp--;
	if( drops->y > -224.0f && drops->y < 224.0f ) temp--;

	if( !temp ) drops->timer = 0;

	drops->x += frnd()*4.0f + off_x/1.5f;
	drops->y += frnd()*4.0f + off_y/1.5f;
	drops->z += frnd()*4.0f - off_z;
#define		ACT_OFFSET	(8.0f)
	if( drops->x < -256.0f-ACT_OFFSET ) drops->x = -256.0f-ACT_OFFSET;
	else if( drops->x > 256.0f+ACT_OFFSET ) drops->x = 256.0f+ACT_OFFSET;
	
	if( drops->y < -224.0f-ACT_OFFSET ) drops->y = -224.0f-ACT_OFFSET;
	else if( drops->y > 224.0f+ACT_OFFSET ) drops->y = 224.0f+ACT_OFFSET;
	
	drops->r_timer += 1;
	if( drops->r_timer >= 4096 ) drops->r_timer -= 4096;

	if( drops->z > 50.0f ){
		//終了判定
		drops->step = DROP_STEP_OFFCAM;
	}

	//進んでるとき
	if( (off_z > 10.0f && drops->timer >= 60) ||
		(off_z < 10.0f && drops->timer >= 360) ){
		drops->step = DROP_STEP_PRESLEEP;
		drops->timer = 0;
	}

	if( drops->z < 0.0f ) drops->z = 0.0f;
}

static void ActiveOffCamDropAct( DROP_DATA *drops, float off_x, float off_y, float off_z )
{
	float	scale;

	drops->x += frnd()*10.0f + off_x;
	drops->y += frnd()*10.0f + off_y;
	drops->z -= off_z;

	drops->rot_x = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;
	drops->rot_y = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;
	
	scale = 1.0f - drops->z/INIT_Z_POS;
	if( scale > 1.0f ) scale = 1.0f;
	else if( scale < 0.0f ) scale = 0.0f;

	drops->scl_x = SCALE_PRIMA * scale;
	drops->scl_y = SCALE_PRIMA * scale;

	drops->r_timer += 32;
	if( drops->r_timer >= 4096 ) drops->r_timer -= 4096;

	if( drops->z < 0.0f ){
#define		SCR_OFF		(16.0f)
		//終了判定
		drops->z = 0.0f;
		if( drops->x > -256.0f-SCR_OFF && drops->x < 256.0f+SCR_OFF &&
			drops->y > -224.0f-SCR_OFF && drops->y < 224.0f+SCR_OFF ){
			drops->step = DROP_STEP_ONCAM;
		}else{
			//drops->step = DROP_STEP_PRESLEEP2;
			drops->step = DROP_STEP_SLEEP;
			drops->timer = 0;
		}
	}else if( drops->z > INIT_Z_POS+500.0f ){
		drops->step = DROP_STEP_SLEEP;
		//printf("sleep z\n");
	}
}

static void ActivePreSleepDropAct( DROP_DATA *drops, float off_x, float off_y, float off_z )
{
	float	scale = 1.0f - (float)drops->timer/320.0f;
	
	drops->x += frnd()*4.0f + off_x/1.5f;
	drops->y += frnd()*4.0f + off_y/1.5f;
	
	drops->rot_x = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;
	drops->rot_y = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;

	drops->scl_x = SCALE_PRIMA * scale * (1.0f + (float)drops->trans_x/80.0f);
	drops->scl_y = SCALE_PRIMA * scale * (1.0f + (float)drops->trans_y/80.0f);

	if( drops->x < -256.0f-ACT_OFFSET ) drops->x = -256.0f-ACT_OFFSET;
	else if( drops->x > 256.0f+ACT_OFFSET ) drops->x = 256.0f+ACT_OFFSET;
	
	if( drops->y < -224.0f-ACT_OFFSET ) drops->y = -224.0f-ACT_OFFSET;
	else if( drops->y > 224.0f+ACT_OFFSET ) drops->y = 224.0f+ACT_OFFSET;
	
	drops->r_timer += 8;
	if( drops->r_timer >= 4096 ) drops->r_timer -= 4096;

	if( drops->timer >= 320 ){
		//終了判定
		drops->step = DROP_STEP_SLEEP;
		//printf("sleep 1\n");
	}
}

static void ActivePreSleepDropAct2( DROP_DATA *drops, float off_x, float off_y, float off_z )
{
	float	scale = 1.0f - (float)drops->timer/320.0f;
	
	drops->x += frnd()*4.0f + off_x;
	drops->y += frnd()*4.0f + off_y;
	
	drops->rot_x = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;
	drops->rot_y = (short)(TS_SINs( (drops->r_timer)&0x7ff ) * 2048.0f) + 2048;

	drops->scl_x = SCALE_PRIMA * scale;
	drops->scl_y = SCALE_PRIMA * scale;

	drops->r_timer += 8;
	if( drops->r_timer >= 4096 ) drops->r_timer -= 4096;

	if( drops->timer >= 320 ){
		//終了判定
		drops->step = DROP_STEP_SLEEP;
		//printf("sleep 2\n");
	}
}



static DROP_DATA *ActDrops( DROP_DATA *pdrops, int n_drops, float off_x, float off_y, float off_z )
{
	int				i;
//	DROP_DATA		*drops = MEM_SCR_DROPS;
	DROP_DATA		*drops = pdrops;
	DROP_DATA		*free = NULL;
//	TS_Mem_Scr( MEM_SCR_DROPS, pdrops, sizeof(DROP_DATA), n_drops );

//	printf("z = %f\n",off_z);
	for( i = 0; i < n_drops; i++ ){
		drops->timer += 1;
#if 0
		if( GV_PadData[0].press & PAD_X ){
			drops->y = 100.0f * rnd() + 50.0f;
			drops->x = frnd() * 200.0f;
			drops->timer = 0;
			drops->step = DROP_STEP_OFFCAM;
			drops->z = INIT_Z_POS;
	}
#endif		
		switch( drops->step ){
		  case DROP_STEP_SLEEP:
			free = drops;
			break;
		  case DROP_STEP_OFFCAM:
			ActiveOffCamDropAct( drops, off_x, off_y, off_z );
			break;
		  case DROP_STEP_ONCAM:
			ActiveOnCamDropAct( drops, off_x, off_y, off_z );
			break;
		  case DROP_STEP_PRESLEEP:
			ActivePreSleepDropAct( drops, off_x, off_y, off_z );
			break;
		  case DROP_STEP_PRESLEEP2:
			ActivePreSleepDropAct2( drops, off_x, off_y, off_z );
			break;
		}

		
		drops++;
	}
	return free;

}

static void AllDropsSleep( DROP_DATA *pdrops, int n_drops )
{
	int				i;
	DROP_DATA		*drops = pdrops;
	for( i = 0; i < n_drops; i++ ){
		drops->step = DROP_STEP_SLEEP;
		drops++;
	}
}

static void ResetDmaVifGifTags( DROPS_PACK *packet, void *gif_output, int n_drops )
{
	DG_GIFTAG	*giftag = (DG_GIFTAG*)gif_output;
	int size = sizeof(DG_GIFTAG) + sizeof(MVERT_DATA_F)*N_KMS_VERTS*n_drops;
	
	packet->dmatag_mverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, size/sizeof(u_long128) );
	packet->dmatag_mverts.vifcode[1] = SCE_VIF1_SET_DIRECT( size/sizeof(u_long128), 0);

	giftag->tag  = SCE_GIF_SET_TAG( sizeof(MVERT_DATA_F)*N_KMS_VERTS*n_drops/sizeof(u_long128), 1, 0, 0, 0, 1);	
	
}

static void Act( Work *work )
{
#ifndef PSX2
	work->prim = work->dmapack->autopacket ;
	work->prim = InitDrowFlush( work->prim ) ;
	work->prim = InitTrBuffer( work->prim, 0 ) ;
	work->prim = InitMdlDraw( work->prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;
#endif

	if( !(GV_PauseLevel & GV_PAUSE_PAUSE) &&
		!(GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON )) &&
		DG_Chanls[0].eye.m[3][1] < GM_WaterLevel &&
		!GM_CheckGameStatus( STATE_DISP_GAMEOVER )){
		DROP_DATA		*free;
		FVECTOR			diff;
		FVECTOR			flow;
		SVECTOR			dir;

		_sceVu0SubVector( &diff, work->campos, &work->pre_campos );
		TS_MatToRot( &dir, &DG_Chanls[0].eye );

		_RotTrans( &diff, &DG_Chanls[0].eye_inv, &diff );
#ifdef BP_PS2
		_RotTrans( &flow, &DG_Chanls[0].eye_inv, &(FVECTOR){ 0.0f, 8.0f, 0.0f, 0.0f } );
#else
		{
			FVECTOR tmp={ 0.0f, 8.0f, 0.0f, 0.0f };
			_RotTrans( &flow, &DG_Chanls[0].eye_inv, &tmp );
		}
#endif

		DG_COPY_VEC( &work->pre_campos, work->campos );
		work->pre_camdir = dir;

		free = ActDrops( work->drop_data, N_DROPS, -diff.vx+flow.vx, diff.vy+flow.vy, diff.vz-flow.vz );
		if( --work->interval < 0 && free ){
			float		off_y = 80.0f * (1.0f - (float)(dir.vx)/1024.0f);
			work->interval = 120;
			free->x = frnd() * 256.0f;
			free->timer = 0;
			free->step = DROP_STEP_OFFCAM;


			free->y = 150.0f + 50.0f * rnd() + off_y;
			free->z = INIT_Z_POS;// - (float)(dir.vx/2);

			free->scl_x = 0.0f;
			free->scl_y = 0.0f;
			
			printf("y %f: z %f\n",free->y,free->z);
			//free->z = INIT_Z_POS;
		}

	}
	if( DG_Chanls[0].eye.m[3][1] > GM_WaterLevel ){
		AllDropsSleep( work->drop_data, N_DROPS );
	}
#ifdef PSX2
	{
		int		cnt;
#ifdef DEF_TEX_DRAW
		cnt = SetDropVerts( work->drow_verts[DG_Clock], work->tex_verts[DG_Clock], work->drop_data, work->cv2_def, N_DROPS );
#else
		cnt = SetDropVerts( work->drow_verts[DG_Clock], NULL, work->drop_data, work->cv2_def, N_DROPS );
#endif
		ResetDmaVifGifTags( work->drops_pack[DG_Clock], work->drow_verts[DG_Clock], cnt );
	}
#else /*PSX2*/

	{
#ifdef DEF_TEX_DRAW
		SetDropVerts( work, work->tex_verts[DG_Clock], work->drop_data, work->cv2_def, N_DROPS );
#else
		SetDropVerts( work, NULL, work->drop_data, work->cv2_def, N_DROPS );
#endif
	}

	work->prim = DG_SetDmapackEnd( work->prim ) ;
	//printf( "scr_bubble.c need Size%d\n",  (int)work->prim-(int)work->packet_mem  ) ;
	if( work->xbox_packet_size < ((int)work->prim-(int)work->packet_mem) ){
		printf("xbox_packet_size 足りません(%d)\n",((int)work->prim-(int)work->packet_mem));
		ASSERT( 0 );
	}
#endif /*PSX2*/
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}

#ifdef PSX2
	if(work->drops_pack[0]) GV_DelayedFree(work->drops_pack[0]);
#else
	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
#endif
	if(work->drow_verts[0]) GV_DelayedFree(work->drow_verts[0]);
	
#ifdef DEF_TEX_DRAW
	if(work->tex_verts[0]) GV_DelayedFree(work->tex_verts[0]);
#endif
	g_work = NULL;
}

/* ---------------------------------------------------------------- */
static void InitRndDropData( DROP_DATA *p_drop, int n_drops )
{
	int i;
	DROP_DATA	*drops = p_drop;

	for( i = 0; i < n_drops; i++ ){

		drops->scl_x = SCALE_PRIMA;// *0.5f + SCALE_PRIMA*rnd();
		//if( irnd() & 0x00400000 ) drops->scl_x *= -1.0f;
		drops->scl_y = SCALE_PRIMA;// *0.5f + SCALE_PRIMA*rnd();
		drops->x = drops->x = frnd() * 200.0f;//(float)((DRAW_WIDTH/2) - 32)*frnd();
		drops->y = 0;//(float)((DRAW_HEIGHT/2) - 32)*frnd();
		drops->trans_x = 0;//trans_x - (int)((float)(trans_x/4)*rnd());
		drops->timer = 0;//irnd()%4096;
		drops->r_timer = irnd()%4096;
		drops->step = 0;
		drops++;
	}
}

#ifdef PSX2  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
static void InitDmaTags( Work *work, DROPS_PACK *packet, int which, int verts_size )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		退避
	packet->dmatag_flush.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
	// ＤＭＡタグ		モデル描画設定
	packet->dmatag_mdl.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(MDL_DRAW) );
	packet->dmatag_mdl.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mdl.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(MDL_DRAW), 0);
	// ＤＭＡタグ		ピクセルテスト復元
	packet->dmatag_test.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(PACKET_END) );
	packet->dmatag_test.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_test.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(PACKET_END), 0);
	// オフセット環境復元用
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) );
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] );
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0);
	// RETタグ
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 );
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	// ＤＭＡタグ		モデル頂点
	packet->dmatag_mverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, verts_size/sizeof(u_long128) );
	packet->dmatag_mverts.addr = work->drow_verts[which];
	packet->dmatag_mverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_mverts.vifcode[1] = SCE_VIF1_SET_DIRECT( verts_size/sizeof(u_long128), 0);

#ifdef DEF_TEX_DRAW
	// ＤＭＡタグ		フラッシュ
	packet->dmatag_flush0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush0.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);

	packet->dmatag_flush1.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(DROW_FLUSH) );
	packet->dmatag_flush1.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_flush1.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DROW_FLUSH), 0);
	// ＤＭＡタグ		スプライト描画設定
	packet->dmatag_tex.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TEX_DRAW) ) ;
	packet->dmatag_tex.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_tex.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TEX_DRAW), 0) ;
	// ＤＭＡタグ		モデル頂点
	packet->dmatag_tverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, verts_size/sizeof(u_long128) );
	packet->dmatag_tverts.addr = work->tex_verts[which];
	packet->dmatag_tverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_tverts.vifcode[1] = SCE_VIF1_SET_DIRECT( verts_size/sizeof(u_long128), 0);

#endif

}

static void InitMVerts( void *verts, DG_DEF *def, int n_drops, int size )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	DG_GIFTAG	*giftag = (DG_GIFTAG*)verts;
	MVERT_DATA_F	*mverts = (MVERT_DATA_F*)(giftag+1);
	int			i,j,k,cnt=0;
	//short		svx,svy;//,snx,sny;
	//float		temp;

	giftag->tag  = SCE_GIF_SET_TAG( size/sizeof(u_long128), 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;

	for( k = 0; k < n_drops; k++ ){
		objpack = mdl->packs;
		for( i = 0; i < mdl->n_packs; i++ ){
			sverts = (SVECTOR*)objpack->verts;
			snorms = (SVECTOR*)objpack->norms;
			for( j = 0; j < objpack->n_verts; j++ ){
				//mverts->rgbq.reg = SCE_GS_RGBAQ;
				mverts->uv.reg = SCE_GS_UV;
				mverts->xyz.reg = (snorms->pad&0x8000)?SCE_GS_XYZ3:SCE_GS_XYZ2;
				//mverts->rgbq.data = SCE_GS_SET_RGBAQ(255,255,255,0x80,0);
				mverts->uv.data = SCE_GS_SET_UV(0,0);
				mverts->xyz.data = SCE_GS_SET_XYZ(0,0,0);

				mverts++;
				sverts++;
				snorms++;

				cnt++;
			}
			objpack++;
		}
	}
	//printf("verts = %d\n",cnt);
}
#ifdef DEF_TEX_DRAW
static void InitTVerts( void *verts, DG_DEF *def, DG_TEX *tex, int n_drops, int size )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	DG_GIFTAG	*giftag = (DG_GIFTAG*)verts;
	MVERT_DATA_F	*mverts = (MVERT_DATA_F*)(giftag+1);
	int				i,j,k,cnt=0;
	float			st[2];
	int				*ist = (int*)st;
	short			*uvs;

	//short		svx,svy;//,snx,sny;
	//float		temp;

	giftag->tag  = SCE_GIF_SET_TAG( size/sizeof(u_long128), 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;

	for( k = 0; k < n_drops; k++ ){
		objpack = mdl->packs;
		for( i = 0; i < mdl->n_packs; i++ ){
			sverts = (SVECTOR*)objpack->verts;
			snorms = (SVECTOR*)objpack->norms;
			uvs = objpack->uvs[0];
			for( j = 0; j < objpack->n_verts; j++ ){
				//mverts->rgbq.reg = SCE_GS_RGBAQ;
				//mverts->rgbq.data = SCE_GS_SET_RGBAQ(255,255,255,0x80,0);

				st[0] = (((float)uvs[0]/1024.0f) * tex->u_scale) + tex->u_offset;
				st[1] = (((float)uvs[1]/1024.0f) * tex->v_scale) + tex->v_offset;

				printf("uv[%d,%d]->[%f,%f]->[%f,%f]\n",
					   uvs[0], uvs[1],
					   (float)uvs[0]/1024.0f, (float)uvs[1]/1024.0f,
					   st[0], st[1] );
				
				mverts->uv.reg = SCE_GS_ST;
				mverts->xyz.reg = (snorms->pad&0x8000)?SCE_GS_XYZ3:SCE_GS_XYZ2;
				mverts->uv.data = SCE_GS_SET_ST(ist[0],ist[1]);
				mverts->xyz.data = SCE_GS_SET_XYZ(0,0,0);

				mverts++;
				sverts++;
				snorms++;
				uvs+=2;

				cnt++;
			}
			objpack++;
		}
	}
	//printf("verts = %d\n",cnt);
}
#endif

#endif  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


static int GetResources( Work *work )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;
	DG_DEF			*def;
	CV2_DEF			*cv2_def;
	int				n_verts,verts_size;
#ifdef PSX2
	DROPS_PACK		*packet;
	int				i ;
#endif

#ifdef DEF_TEX_DRAW
	DG_TEXTURE_LIST *texlist;
	DG_TEX			*tex;
	texlist = DG_GetTextureList( TRI_CODE );
	if(texlist==NULL){
		printf("ERR!! NO TRI!!\n");
		return -1;
	}
	
	tex = DG_GetTexture2( TRI_CODE, TEX_CODE );
	if( !tex ){
		printf("ERR!! NO TEX!!\n");
		return -1;
	}
	work->tex_packet[0] = &texlist->tex_packet[0];
	work->tex_packet[1] = &texlist->tex_packet[1];
#endif

	work->campos = (FVECTOR*)DG_Chanls[0].eye.m[3];
	work->camdir = (FVECTOR*)DG_Chanls[0].eye.m[2];
	DG_COPY_VEC( &work->pre_campos, work->campos );
	TS_MatToRot( &work->pre_camdir, &DG_Chanls[0].eye );
	
	def = (DG_DEF*)GV_GetCache( GV_CacheID( MDL_CODE, 'k' ));
	if(def == NULL){ printf("ERR!! NO MODEL!! kms\n"); return -1; }

	cv2_def = work->cv2_def = (CV2_DEF*)GV_GetCache( GV_CacheID( MDL_CODE, 'c' ));
	if( cv2_def == NULL){ printf("ERR!! NO MODEL cv2!!\n"); return -1; }

	n_verts = GetMdlVertsJoint( def, 0 );//GetMdlVerts( def );				//モデルの頂点数
	verts_size = sizeof(DG_GIFTAG) + sizeof(MVERT_DATA_F)*n_verts*N_DROPS;

	printf("n_verts kms %d: cv2 %d scr_bottom %x\n", n_verts, cv2_def->models[0].n_verts, MEM_SCR_BOTTOM );
	
//	printf("n_verts %d: dma size %x: gif size %x\n", cv2_def->models[0].verts_index, verts_size/sizeof(FVECTOR),
//		   sizeof(MVERT_DATA_F)*n_verts*N_DROPS/sizeof(FVECTOR) );
	printf("****************** n_verts %d\n", cv2_def->models[0].n_verts_index );

#ifdef DEF_TEX_DRAW	
	// 頂点データメモり確保
	verts = GV_Malloc( verts_size * 2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->tex_verts[0] = (MVERT_DATA_F*)( verts );
	work->tex_verts[1] = (MVERT_DATA_F*)( verts + verts_size );
#endif
	// 頂点データメモり確保
	verts = GV_Malloc( verts_size * 2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->drow_verts[0] = (MVERT_DATA_F*)( verts );
	work->drow_verts[1] = (MVERT_DATA_F*)( verts + verts_size );

	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											  DG_DMAPACK_PHASE_AFTER, 145 );
	if(dmapack == NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	//dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	DG_QueueDmapack( dmapack );

	InitRndDropData( work->drop_data, N_DROPS );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(DROPS_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC DROPS_PACK!!\n"); return -1; }
	work->drops_pack[0] = dmapack->packet[0] = &packet[0];
	work->drops_pack[1] = dmapack->packet[1] = &packet[1];
	
	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i, verts_size );
		InitDrowFlush( &packet[i].drow_flush );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitMVerts( work->drow_verts[i], def, N_DROPS, sizeof(MVERT_DATA_F)*n_verts*N_DROPS );
		InitEndPacket( &packet[i].end_paket );

#ifdef DEF_TEX_DRAW
		InitDrowFlush( &packet[i].drow_flush0 );
		InitDrowFlush( &packet[i].drow_flush1 );

		InitSetTexDraw( &packet[i].tex_draw, TEX_RGBA, TEX_PRIM );
		InitTVerts( work->tex_verts[i], def, tex, N_DROPS, sizeof(MVERT_DATA_F)*n_verts*N_DROPS );

		TexDataSet( packet[i].tex_packet_alp, work->tex_packet[i],
					&packet[i].tex_draw, &tex->tex_trans, TEX_ALPHA_MODE );
#endif

		packet[i].mdl_draw.data.rgbq.data = SCE_GS_SET_RGBAQ(148,156,164,128,0x3F800000);

#if 0
		//ブラーモード
		packet[i].mdl_draw.data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(1-i)/64,
															 BUFFER_WIDTH/64,
															 FRAME_BUFFER_COLOR_MODE(),
															 10,10,0,0,0,0,0,0,0);
#endif
	}
#else /*PSX2*/

	work->def = def ;
	//work->xbox_packet_size = 14000;/* Actを回して、このサイズを下回ることがないのを確認 */
	work->xbox_packet_size = 36000;/* Actを回して、このサイズを下回ることがないのを確認 */
	if ( !(work->packet_mem = GV_Malloc( work->xbox_packet_size )) ) {
		printf("ERR!! MALLOC scr_bubble.c!!\n");
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( dmapack->autopacket ) ;

#endif /*PSX2*/

	work->interval = 120;
	return (0);
}

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

void *NewScrBubble( int name, int map )
{
	Work		*work ;

	//return (void*)1;
	if(g_work){
		return g_work;
	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
#else
		if ( DG_CheckUseBlur() ) {
			GV_SetActor( &( work->actor ), Act, Die );
			GV_ActorEX( &work->actor ) ;
			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work );
				return NULL;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie );
			GV_ActorEX( &work->actor ) ;
		}
#endif
		g_work = work;
	}

	return (void *)work ;
}
