//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_drop.c
	歪みモデルを使った奴(歪み変化)

	2001/07/10 T.Shibata
	
	$Id: scr_drop.c,v 1.1.1.3 2002/11/19 11:48:40 Yoshizawa1 Exp $

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

#define MDL_CODE 	(GV_StrCode("awa_model2"))

#define MDL_PRIM	( SCE_GS_SET_PRIM( 4, 1, 1, 0, 1, 0, 1, 0, 0 ) )
#define MDL_TEST	( SCE_GS_SET_TEST( 0, 7, 0, 0, 0, 0, 1, 1 ) )
#define MDL_ALPHA	( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ) )

#define		N_DROPS		(64)
#define		N_DROPS_MIN	(64)

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

typedef struct {
	float		scl_x, scl_y, x, y;
	float		weight, key;
	int			life;
	short		timer;
	u_char		s_index,flags;
} DROP_DATA;

typedef struct {
	DG_DMATAG		dmatag_tr;				// ＧＩＦ接続ＤＭＡタグ
	TR_BUFFER		tr_buffer;				//								退避(フレームをバックに)
	DG_DMATAG		dmatag_mdl;				// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;				//								モデル描画の設定
	DG_DMATAG		dmatag_mverts;			// ＧＩＦ接続ＤＭＡタグ			モデル頂点(変化)
	DG_DMATAG		dmatag_m_mverts;		// ＧＩＦ接続ＤＭＡタグ			モデル頂点(変化)
	DG_DMATAG		dmatag_test;			// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;				//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;			// オフセット環境復元用
	DG_DMATAG		dmatag_end;				// RETタグ
} DROPS_PACK;

typedef	struct	{
	GV_ACT_EX		actor ;

	ALIGN16_PRE DROPS_PACK		*drops_pack[2] ALIGN16_POST;
	ALIGN16_PRE void			*drow_verts[2] ALIGN16_POST;
	ALIGN16_PRE void			*drow_m_verts[2] ALIGN16_POST;
	ALIGN16_PRE DROP_DATA		drop_data[N_DROPS] ALIGN16_POST;
	ALIGN16_PRE DROP_DATA		drop_m_data[N_DROPS_MIN] ALIGN16_POST;

	CV2_DEF			*cv2_def;
	DG_DMAPACK		*dmapack;

	int				puru_timer;
	int				max_life;
	int				life;
	int				flags;
	int				act_m_drops;
	SVECTOR			*cam_rot;
	SVECTOR			pre_cam_rot;
	SVECTOR			pre_cam_diff_rot;
	float			drops_move_x;
	float			drops_move_y;

	float			force_x,force_y;


#ifndef PSX2
	DG_DEF              *def ;
	void				*packet_mem;
	void                *prim ;
	int xbox_packet_size;
#endif

} Work;

static Work *g_work = NULL;

#ifdef PSX2
#define		N_CV2_VERTS		(37)
#define		N_KMS_VERTS		(72)
#else
#define		N_CV2_VERTS		(40)
#define		N_KMS_VERTS		(40)
#endif

#ifdef BP_PS2
#define		MEM_SCR_VERTS1		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VERTS2		((void *)(MEM_SCR_VERTS1) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS3		((void *)(MEM_SCR_VERTS2) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS4		((void *)(MEM_SCR_VERTS3) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS5		((void *)(MEM_SCR_VERTS4) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS6		((void *)(MEM_SCR_VERTS5) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS7		((void *)(MEM_SCR_VERTS6) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_VERTS8		((void *)(MEM_SCR_VERTS7) + sizeof(FVECTOR)*N_CV2_VERTS)

#define		MEM_SCR_NORMS1		((void *)(MEM_SCR_VERTS8) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS2		((void *)(MEM_SCR_NORMS1) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS3		((void *)(MEM_SCR_NORMS2) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS4		((void *)(MEM_SCR_NORMS3) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS5		((void *)(MEM_SCR_NORMS4) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS6		((void *)(MEM_SCR_NORMS5) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS7		((void *)(MEM_SCR_NORMS6) + sizeof(FVECTOR)*N_CV2_VERTS)
#define		MEM_SCR_NORMS8		((void *)(MEM_SCR_NORMS7) + sizeof(FVECTOR)*N_CV2_VERTS)

#define		MEM_SCR_INDEX		((void *)(MEM_SCR_NORMS8) + sizeof(FVECTOR)*N_CV2_VERTS)
#else

#define		MEM_SCR_VERTS1		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VERTS2		((void *)((char *)MEM_SCR_VERTS1 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS3		((void *)((char *)MEM_SCR_VERTS2 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS4		((void *)((char *)MEM_SCR_VERTS3 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS5		((void *)((char *)MEM_SCR_VERTS4 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS6		((void *)((char *)MEM_SCR_VERTS5 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS7		((void *)((char *)MEM_SCR_VERTS6 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS8		((void *)((char *)MEM_SCR_VERTS7 + sizeof(FVECTOR)*N_CV2_VERTS))

#define		MEM_SCR_NORMS1		((void *)((char *)MEM_SCR_VERTS8 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS2		((void *)((char *)MEM_SCR_NORMS1 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS3		((void *)((char *)MEM_SCR_NORMS2 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS4		((void *)((char *)MEM_SCR_NORMS3 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS5		((void *)((char *)MEM_SCR_NORMS4 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS6		((void *)((char *)MEM_SCR_NORMS5 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS7		((void *)((char *)MEM_SCR_NORMS6 + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS8		((void *)((char *)MEM_SCR_NORMS7 + sizeof(FVECTOR)*N_CV2_VERTS))

#define		MEM_SCR_INDEX		((void *)((char *)MEM_SCR_NORMS8 + sizeof(FVECTOR)*N_CV2_VERTS))
#endif

#if 0
#define		MEM_SCR_VERT		((FVECTOR*)((void *)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
#define		MEM_SCR_NORM		((FVECTOR*)((char*)(MEM_SCR_VERT) + sizeof(FVECTOR) ))
#define		MEM_SCR_X			((int*)((char*)(MEM_SCR_NORM) + sizeof(FVECTOR) ))
#define		MEM_SCR_Y			((int*)((void *)(MEM_SCR_X) + sizeof(int) ))
#define		MEM_SCR_IX			((int*)((void *)(MEM_SCR_Y) + sizeof(int) ))
#define		MEM_SCR_IY			((int*)((void *)(MEM_SCR_IX) + sizeof(int) ))
#define		MEM_SCR_IU			((int*)((void *)(MEM_SCR_IY) + sizeof(int) ))
#define		MEM_SCR_IV			((int*)((void *)(MEM_SCR_IU) + sizeof(int) ))

#define		MEM_SCR_BOTTOM		(((void *)(MEM_SCR_IV) + sizeof(int) ))

#else

#ifdef BP_PS2
#define		MEM_SCR_DROPS		((FVECTOR*)((void *)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
#define		MEM_SCR_X			((int*)((void *)(MEM_SCR_DROPS) + sizeof(DROP_DATA)*N_DROPS ))
#define		MEM_SCR_Y			((int*)((void *)(MEM_SCR_X) + sizeof(int) ))

#define		MEM_SCR_BOTTOM		(((void *)(MEM_SCR_Y) + sizeof(int) ))
#else
#define		MEM_SCR_DROPS		((FVECTOR*)((char *)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
#define		MEM_SCR_X			((int*)((char *)MEM_SCR_DROPS + sizeof(DROP_DATA)*N_DROPS ))
#define		MEM_SCR_Y			((int*)( (char *)MEM_SCR_X + sizeof(int) ))

#define		MEM_SCR_BOTTOM		((void *)( (char *)MEM_SCR_Y + sizeof(int) ))
#endif

#endif


#define		SCALE_PRIMA		(24.0f/1000.0f)
#define		UV_SCALE		(128.0f)

#ifndef BP_PS2
static	FVECTOR _vf14, _vf12, _vf13, _vf11, _vf10, _vf09, _vf08, _vf04, _vf05, _vf06, _acc ;
#endif

static void _LoadScaleDividingPoint1( FVECTOR *scl_norms )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf14, 0x00(%0)	#scale_norm
	": : "r"(scl_norms) );
#else
	_vf14 = *scl_norms ;
#endif
}

static void _LoadScaleDividingPoint2( FVECTOR *key, FVECTOR *scl_verts )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf12, 0x00(%0)	#各係数
		lqc2			vf13, 0x00(%1)	#scale_vec
	": : "r"(key), "r"(scl_verts) );
#else
	_vf12 = *key ;
	_vf13 = *scl_verts ;
#endif
}

static void _GetDividingPointVerts( FVECTOR *out,
									FVECTOR	*ls_verts, FVECTOR *lt_verts,
									FVECTOR *ms_verts, FVECTOR *mt_verts )
{
	// l*( s*ls_verts + t*lt_verts ) + m*( s*ms_verts + t*mt_verts )
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf08, 0x00(%1)	#ls_verts
		lqc2			vf09, 0x00(%2)	#lt_verts
		lqc2			vf10, 0x00(%3)	#ms_verts
		lqc2			vf11, 0x00(%4)	#mt_verts

		vmulax.xyz		ACC,  vf08, vf12x	#s*ls_verts
		vmaddy.xyz		vf04, vf09, vf12y	#s*ls_verts + t*lt_verts

		vmulax.xyz		ACC,  vf10, vf12x	#s*ms_verts
		vmaddy.xyz		vf05, vf11, vf12y	#s*ms_verts + t*mt_verts
		
		vmulaz.xyz		ACC,  vf04, vf12z	#l*(s*ls_verts + t*lt_verts)
		vmaddw.xyz		vf06, vf05, vf12w	#l*(s*ls_verts + t*lt_verts) + m*(s*ms_verts + t*mt_verts)

		vmul.xyz		vf07, vf06, vf13	# * scale

		sqc2	        vf07,0x00(%0)

	": : "r"(out), "r"(ls_verts), "r"(lt_verts), "r"(ms_verts), "r"(mt_verts) :"memory" );
#else
	_vf08 = *ls_verts ;
	_vf09 = *lt_verts ;
	_vf10 = *ms_verts ;
	_vf11 = *mt_verts ;

	_sceVu0ScaleVectorXYZ( &_acc, ls_verts, _vf12.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf04, lt_verts, _vf12.vy ) ;
	_sceVu0AddVector( &_vf04, &_vf04, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, ms_verts, _vf12.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf05, mt_verts, _vf12.vy ) ;
	_sceVu0AddVector( &_vf05, &_vf05, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, &_vf04, _vf12.vz ) ;
	_sceVu0ScaleVectorXYZ( &_vf06, &_vf05, _vf12.vw ) ;
	_sceVu0AddVector( &_vf06, &_vf06, &_acc );

	_sceVu0MulVector( out, &_vf06, &_vf13 ) ;
#endif
}


//

static void _GetDividingPointVertsMin( FVECTOR *out, FVECTOR *l_verts, FVECTOR *m_verts )
{
#ifdef BP_PSX2_ASM
	// l*( l_verts ) + m*( m_verts )
	asm volatile ("
		lqc2			vf08, 0x00(%1)	#l_verts
		lqc2			vf09, 0x00(%2)	#m_verts

		vmulaz.xyz		ACC,  vf08, vf12z	#l*l_verts
		vmaddw.xyz		vf06, vf09, vf12w	#l*l_verts + m*m_verts

		vmul.xyz		vf07, vf06, vf13	# * scale

		sqc2	        vf07,0x00(%0)

	": : "r"(out), "r"(l_verts), "r"(m_verts) :"memory" );
#else
	_vf08 = *l_verts ;
	_vf09 = *m_verts ;

	_sceVu0ScaleVectorXYZ( &_acc, &_vf08, _vf12.vz ) ;
	_sceVu0ScaleVectorXYZ( &_vf06, &_vf09, _vf12.vw ) ;
	_sceVu0AddVector( &_vf06, &_vf06, &_acc );

	_sceVu0MulVector( out, &_vf06, &_vf13 );
#endif
}

static void _GetDividingPointNorms( FVECTOR *out,
									FVECTOR	*ls_verts, FVECTOR *lt_verts,
									FVECTOR *ms_verts, FVECTOR *mt_verts )
{
#ifdef BP_PSX2_ASM
	// l*( s*ls_verts + t*lt_verts ) + m*( s*ms_verts + t*mt_verts )
	asm volatile ("
		lqc2			vf08, 0x00(%1)	#ls_verts
		lqc2			vf09, 0x00(%2)	#lt_verts
		lqc2			vf10, 0x00(%3)	#ms_verts
		lqc2			vf11, 0x00(%4)	#mt_verts

		vmulax.xyz		ACC,  vf08, vf12x	#s*ls_verts
		vmaddy.xyz		vf04, vf09, vf12y	#s*ls_verts + t*lt_verts

		vmulax.xyz		ACC,  vf10, vf12x	#s*ms_verts
		vmaddy.xyz		vf05, vf11, vf12y	#s*ms_verts + t*mt_verts

		vmulaz.xyz		ACC,  vf04, vf12z	#l*(s*ls_verts + t*lt_verts)
		vmaddw.xyz		vf06, vf05, vf12w	#l*(s*ls_verts + t*lt_verts) + m*(s*ms_verts + t*mt_verts)

		vmul.xyz		vf07, vf06, vf14	# * scale

		sqc2	        vf07,0x00(%0)

	": : "r"(out), "r"(ls_verts), "r"(lt_verts), "r"(ms_verts), "r"(mt_verts) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, ls_verts, _vf12.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf04, lt_verts, _vf12.vy ) ;
	_sceVu0AddVector( &_vf04, &_vf04, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, ms_verts, _vf12.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf05, mt_verts, _vf12.vy ) ;
	_sceVu0AddVector( &_vf05, &_vf05, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, &_vf04, _vf12.vz ) ;
	_sceVu0ScaleVectorXYZ( &_vf06, &_vf05, _vf12.vw ) ;
	_sceVu0AddVector( &_vf06, &_vf06, &_acc );

	_sceVu0MulVector( out, &_vf06, &_vf14 );
#endif
}

static void _GetDividingPointNormsMin( FVECTOR *out, FVECTOR *l_verts, FVECTOR *m_verts )
{
#ifdef BP_PSX2_ASM
	// l*( l_verts ) + m*( m_verts )
	asm volatile ("
		lqc2			vf08, 0x00(%1)	#l_verts
		lqc2			vf09, 0x00(%2)	#m_verts

		vmulaz.xyz		ACC,  vf08, vf12z	#l*l_verts
		vmaddw.xyz		vf06, vf09, vf12w	#l*l_verts + m*m_verts

		vmul.xyz		vf07, vf06, vf14	# * scale

		sqc2	        vf07,0x00(%0)

	": : "r"(out), "r"(l_verts), "r"(m_verts) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, l_verts, _vf12.vz ) ;
	_sceVu0ScaleVectorXYZ( &_vf06, m_verts, _vf12.vw ) ;
	_sceVu0AddVector( &_vf06, &_vf06, &_acc );

	_sceVu0MulVector( out, &_vf06, &_vf14 );
#endif
}


static void SetDropVerts( void *output, DROP_DATA *pdrops, CV2_DEF *cv2_def, int n_drops, float max_life )
{
	int				i,j,n_verts;
	DROP_DATA		*drops = (DROP_DATA*)MEM_SCR_DROPS;
//	DROP_DATA		*drops = pdrops;
	FVECTOR			*l_verts0,*m_verts0;
	FVECTOR			*l_verts1,*m_verts1;
	FVECTOR			*l_norms0,*m_norms0;
	FVECTOR			*l_norms1,*m_norms1;
	short			*index;
	static FVECTOR	scale_norms = { UV_SCALE, UV_SCALE, 1.0f, 0.0f };
#ifndef PSX2
	Work            *work = (Work *)output ;
	float			ix,iy,iu,iv ;
	int n_packs, k;
#else
	MVERT_DATA		*mverts = (MVERT_DATA*)(((DG_GIFTAG*)output)+1);
	int				ix,iy,iu,iv;
#endif

	void *verts_data[8] = {
		MEM_SCR_VERTS1,
		MEM_SCR_VERTS2,
		MEM_SCR_VERTS3,
		MEM_SCR_VERTS4,
		MEM_SCR_VERTS5,
		MEM_SCR_VERTS6,
		MEM_SCR_VERTS7,
		MEM_SCR_VERTS8,
	};
	void *norms_data[8] = {
		MEM_SCR_NORMS1,
		MEM_SCR_NORMS2,
		MEM_SCR_NORMS3,
		MEM_SCR_NORMS4,
		MEM_SCR_NORMS5,
		MEM_SCR_NORMS6,
		MEM_SCR_NORMS7,
		MEM_SCR_NORMS8,
	};
#ifdef PSX2
	n_verts = cv2_def->models[0].n_verts_index;
#else
	n_packs = work->def->models[0].n_packs;
	n_verts = work->def->models[0].packs->n_indices;
#endif

#ifdef PSX2
	TS_Mem_Scr( MEM_SCR_VERTS1, cv2_def->models[0].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts*16 );
	/*
	TS_Mem_Scr( MEM_SCR_VERTS2, cv2_def->models[1].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS3, cv2_def->models[2].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS4, cv2_def->models[3].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS5, cv2_def->models[4].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS6, cv2_def->models[5].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS7, cv2_def->models[6].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS8, cv2_def->models[7].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );

	TS_Mem_Scr( MEM_SCR_NORMS1, cv2_def->models[0].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS2, cv2_def->models[1].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS3, cv2_def->models[2].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS4, cv2_def->models[3].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS5, cv2_def->models[4].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS6, cv2_def->models[5].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS7, cv2_def->models[4].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS8, cv2_def->models[5].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
*/
#else //XBOX では上記の方法では法線情報がきちんととれないため
	TS_Mem_Scr( MEM_SCR_VERTS1, cv2_def->models[0].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS2, cv2_def->models[1].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS3, cv2_def->models[2].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS4, cv2_def->models[3].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS5, cv2_def->models[4].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS6, cv2_def->models[5].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS7, cv2_def->models[6].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_VERTS8, cv2_def->models[7].verts, sizeof(FVECTOR), cv2_def->models[0].n_verts );

	TS_Mem_Scr( MEM_SCR_NORMS1, cv2_def->models[0].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS2, cv2_def->models[1].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS3, cv2_def->models[2].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS4, cv2_def->models[3].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS5, cv2_def->models[4].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS6, cv2_def->models[5].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS7, cv2_def->models[6].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
	TS_Mem_Scr( MEM_SCR_NORMS8, cv2_def->models[7].norms, sizeof(FVECTOR), cv2_def->models[0].n_verts );
#endif
	TS_Mem_Scr( MEM_SCR_INDEX, cv2_def->models[0].verts_index, sizeof(short), cv2_def->models[0].n_verts_index );
	TS_Mem_Scr( MEM_SCR_DROPS, pdrops, sizeof(DROP_DATA), n_drops );

	_LoadScaleDividingPoint1( &scale_norms );

	for( i = 0; i < n_drops; i++ ){
		FVECTOR			stlm = { drops->weight, 1.0f-drops->weight, drops->key, 1.0f-drops->key };
		FVECTOR			scale_verts = { drops->scl_x, drops->scl_y, 1.0f, 0.0f };
		float			alpha_ratio = 1.0f;//(float)drops->life/max_life;
		
		*MEM_SCR_X = (int)drops->x;
		*MEM_SCR_Y = (int)drops->y;

		l_verts0 = verts_data[drops->s_index+0];
		m_verts0 = verts_data[drops->s_index+1];
		l_verts1 = verts_data[drops->s_index+2];
		m_verts1 = verts_data[drops->s_index+3];
		l_norms0 = norms_data[drops->s_index+0];
		m_norms0 = norms_data[drops->s_index+1];
		l_norms1 = norms_data[drops->s_index+2];
		m_norms1 = norms_data[drops->s_index+3];

		_LoadScaleDividingPoint2( &stlm, &scale_verts );

#ifdef PSX2
		index = MEM_SCR_INDEX;
#else
		for( k = 0; k < n_packs; k ++ ){
			n_verts = (work->def->models[0].packs + k)->n_indices;
			index = (work->def->models[0].packs + k)->index;
			work->prim = DG_SetDmapackTriangleStrip( work->prim, n_verts );
#endif



		for( j = 0; j < n_verts; j++ ){

			float	alpha;
#if 0
			//遅くなった
			_GetDividingPointVerts( MEM_SCR_VERT,
									&l_verts0[*index], &l_verts1[*index],
									&m_verts0[*index], &m_verts1[*index] );

			_GetDividingPointNorms( MEM_SCR_NORM,
									&l_norms0[*index], &l_norms1[*index],
									&m_norms0[*index], &m_norms1[*index] );


			*MEM_SCR_IX = (int)MEM_SCR_VERT->vx;
			*MEM_SCR_IY = -(int)MEM_SCR_VERT->vy;
			*MEM_SCR_IU = -(*MEM_SCR_IX + (int)MEM_SCR_NORM->vx);
			*MEM_SCR_IV = -(*MEM_SCR_IY + (int)MEM_SCR_NORM->vy);
			alpha = 96.0f * MEM_SCR_VERT->vz/400.0f * alpha_ratio;
			
			*MEM_SCR_IX = ((2048 + *MEM_SCR_IX + (*MEM_SCR_X))<<4);
			*MEM_SCR_IY = ((2048 + *MEM_SCR_IY + (*MEM_SCR_Y))<<4);
			*MEM_SCR_IU = ((DRAW_WIDTH/2  + *MEM_SCR_IU + (*MEM_SCR_X))<<4);
			*MEM_SCR_IV = ((DRAW_HEIGHT/2 + *MEM_SCR_IV + (*MEM_SCR_Y))<<4);

			if( *MEM_SCR_IU > MAX_U ) *MEM_SCR_IU = MAX_U;
			else if( *MEM_SCR_IU < MIN_UV ) *MEM_SCR_IU = MIN_UV;
			if( *MEM_SCR_IV > MAX_V ) *MEM_SCR_IV = MAX_V;
			else if( *MEM_SCR_IV < MIN_UV ) *MEM_SCR_IV = MIN_UV;

			if( alpha < 0.0f ) alpha = 0.0f;

			mverts->uv.data = SCE_GS_SET_UV(*MEM_SCR_IU,*MEM_SCR_IV);
			mverts->xyz.data = SCE_GS_SET_XYZ(*MEM_SCR_IX,*MEM_SCR_IY,0);
			mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,((int)alpha),0);
			mverts++;
			index++;

#else
			FVECTOR		verts;
			FVECTOR		norms;

			_GetDividingPointVerts( &verts,
									&l_verts0[*index], &l_verts1[*index],
									&m_verts0[*index], &m_verts1[*index] );

			_GetDividingPointNorms( &norms,
									&l_norms0[*index], &l_norms1[*index],
									&m_norms0[*index], &m_norms1[*index] );

			alpha = 96.0f * verts.vz/400.0f * alpha_ratio;
			if( alpha < 0.0f ) alpha = 0.0f;

#ifdef PSX2
			ix = (int)verts.vx;
			iy = -(int)verts.vy;
			iu = -(ix + (int)norms.vx);
			iv = -(iy + (int)norms.vy);

			ix = ((2048 + ix + (*MEM_SCR_X))<<4);
			iy = ((2048 + iy + (*MEM_SCR_Y))<<4);
			iu = ((DRAW_WIDTH/2 + iu + (*MEM_SCR_X))<<4);
			iv = ((DRAW_HEIGHT/2 + iv + (*MEM_SCR_Y))<<4);

			if( iu > MAX_U ) iu = MAX_U;
			else if( iu < MIN_UV ) iu = MIN_UV;
			if( iv > MAX_V ) iv = MAX_V;
			else if( iv < MIN_UV ) iv = MIN_UV;

			mverts->uv.data = SCE_GS_SET_UV(iu,iv);
			mverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
			mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,((int)alpha),0);
			mverts++;
#else /*PSX2*/
			ix = verts.vx;
			iy = -verts.vy;
			iu = -(ix + norms.vx);
			iv = -(iy + norms.vy);

			ix = DRAW_WIDTH /2 + ix + (*MEM_SCR_X);
			iy = DRAW_HEIGHT/2 + iy + (*MEM_SCR_Y);
			iu = DG_FRAME_U(DRAW_WIDTH /2 + iu + (*MEM_SCR_X));
			iv = DG_FRAME_V(DRAW_HEIGHT/2 + iv + (*MEM_SCR_Y));

			work->prim = DG_SetDmapackVertex( work->prim, ix,iy, iu,iv,
											 SCE_GS_SET_RGBAQ(128,128,128,(int)alpha,0) ) ;
#endif /*PSX2*/
			index++;
#endif
		}
#ifdef KP_XBOX
		}
#endif

		drops++;
	}

}

static void ReSetDmsTags( DROPS_PACK *packet, int n_verts )
{
	// ＤＭＡタグ		モデル頂点
	packet->dmatag_m_mverts.qwc =
		DMATAG_SET_QWC( DMATAG_ID_REF, (sizeof(DG_GIFTAG)+n_verts*sizeof(MVERT_DATA))/sizeof(u_long128) );
	packet->dmatag_m_mverts.vifcode[1] =
		SCE_VIF1_SET_DIRECT( (sizeof(DG_GIFTAG)+n_verts*sizeof(MVERT_DATA))/sizeof(u_long128), 0 );
}

static void SetDropVertsMin( void *output, DROP_DATA *pdrops, CV2_DEF *cv2_def, int n_drops, float max_life )
{
	int				i,j,n_verts;
	DROP_DATA		*drops = (DROP_DATA*)MEM_SCR_DROPS;
//	DROP_DATA		*drops = pdrops;
	FVECTOR			*l_verts0,*m_verts0;
	FVECTOR			*l_norms0,*m_norms0;
	short			*index;
	int				active = 0;
	static FVECTOR	scale_norms = { UV_SCALE, UV_SCALE, 1.0f, 0.0f };
	DG_GIFTAG		*giftag = (DG_GIFTAG*)output;
#ifndef PSX2
	Work            *work = (Work *)output ;
	float			ix,iy,iu,iv ;
	int n_packs , k;
#else
	int				ix,iy,iu,iv;
	MVERT_DATA		*mverts = (MVERT_DATA*)(giftag+1);
#endif
	
#ifdef PSX2
	n_verts = cv2_def->models[0].n_verts_index;
#else
	n_verts = work->def->models[0].packs->n_indices;
	n_packs = work->def->models[0].n_packs;
#endif

	TS_Mem_Scr( MEM_SCR_DROPS, pdrops, sizeof(DROP_DATA), n_drops );

	_LoadScaleDividingPoint1( &scale_norms );

	for( i = 0; i < n_drops; i++, drops++ ){
		if( !(drops->flags & 0x0001) ) continue;
		{
		FVECTOR			stlm = { 0.0f, 0.0f, drops->key, 1.0f-drops->key };
		FVECTOR			scale_verts = { drops->scl_x, drops->scl_y, 1.0f, 0.0f };
		float			alpha_ratio = (float)drops->life/max_life;

		*MEM_SCR_X = (int)drops->x;
		*MEM_SCR_Y = (int)drops->y;
		
		l_verts0 = MEM_SCR_VERTS1;
		m_verts0 = MEM_SCR_VERTS2;
		l_norms0 = MEM_SCR_NORMS1;
		m_norms0 = MEM_SCR_NORMS2;

#ifdef PSX2
		index = MEM_SCR_INDEX;
#else //XBOX
		for( k = 0; k < n_packs; k ++ ){
			//objpack = work->def->models[0].packs ;
			n_verts = (work->def->models[0].packs + k )->n_indices;
			index = (work->def->models[0].packs + k )->index;
			work->prim = DG_SetDmapackTriangleStrip( work->prim, n_verts );
#endif
		_LoadScaleDividingPoint2( &stlm, &scale_verts );
		for( j = 0; j < n_verts; j++ ){
			float		alpha;
			FVECTOR		verts;
			FVECTOR		norms;

			_GetDividingPointVertsMin( &verts,
									&l_verts0[*index], &m_verts0[*index] );

			_GetDividingPointNormsMin( &norms,
									&l_norms0[*index], &m_norms0[*index] );

			alpha = 96.0f * verts.vz/400.0f * alpha_ratio;
			if( alpha < 0.0f ) alpha = 0.0f;

#ifdef PSX2
			ix = (int)verts.vx;
			iy = -(int)verts.vy;
			iu = -(ix + (int)norms.vx);
			iv = -(iy + (int)norms.vy);

			ix = ((2048 + ix + (*MEM_SCR_X))<<4);
			iy = ((2048 + iy + (*MEM_SCR_Y))<<4);
			iu = ((DRAW_WIDTH/2 + iu + (*MEM_SCR_X))<<4);
			iv = ((DRAW_HEIGHT/2 + iv + (*MEM_SCR_Y))<<4);

			if( iu > MAX_U ) iu = MAX_U;
			else if( iu < MIN_UV ) iu = MIN_UV;
			if( iv > MAX_V ) iv = MAX_V;
			else if( iv < MIN_UV ) iv = MIN_UV;

			mverts->uv.data = SCE_GS_SET_UV(iu,iv);
			mverts->xyz.data = SCE_GS_SET_XYZ(ix,iy,0);
			mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,((int)alpha),0);
			mverts++;
#else /*PSX2*/
			ix = verts.vx;
			iy = -verts.vy;
			iu = -(ix + norms.vx);
			iv = -(iy + norms.vy);

			ix = DRAW_WIDTH /2 + ix + (*MEM_SCR_X);
			iy = DRAW_HEIGHT/2 + iy + (*MEM_SCR_Y);
			iu = DG_FRAME_U(DRAW_WIDTH /2 + iu + (*MEM_SCR_X));
			iv = DG_FRAME_V(DRAW_HEIGHT/2 + iv + (*MEM_SCR_Y));

			work->prim = DG_SetDmapackVertex( work->prim, ix,iy, iu,iv,
											 SCE_GS_SET_RGBAQ(128,128,128,(int)alpha,0) ) ;
#endif /*PSX2*/
			index++;
		}
#ifdef KP_XBOX
		}
#endif
		active++;


		}
	}

#ifdef PSX2
	giftag->tag  = SCE_GIF_SET_TAG( n_verts*active*sizeof(MVERT_DATA)/sizeof(u_long128), 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD;
#endif

}

static DROP_DATA *GetFreeMinDrops()
{
	int i;
	DROP_DATA *drops = g_work->drop_m_data;

	for( i = 0; i < N_DROPS_MIN; i++ ){
		if( !(drops->flags & 0x0001) ){
			//printf("粒粒[%d]\n",i);
			return drops;
		}
		drops++;
	}
	
	return NULL;
}

static void ActDrops( DROP_DATA *pdrops, int n_drops, float off_x, float off_y, int pururun )
{
	int				i;
//	DROP_DATA		*drops = MEM_SCR_DROPS;
	DROP_DATA		*drops = pdrops;

//	TS_Mem_Scr( MEM_SCR_DROPS, pdrops, sizeof(DROP_DATA), n_drops );
	for( i = 0; i < n_drops; i++ ){
		drops->x += off_x*drops->scl_x/SCALE_PRIMA;
		drops->y += off_y*drops->scl_y/SCALE_PRIMA + DG_FABS(off_x*drops->scl_x/SCALE_PRIMA)*(1.0f - drops->weight);
		
		drops->life -= 1;
		if( drops->life < 0 ) drops->life = 0;
		drops->timer += 192;

		if( drops->timer >= 4096*4 ) drops->timer -= 4096*4;
		drops->key = 0.25f * TS_SINs( drops->timer ) * ((float)pururun/4096.0f) ;

		if( /*!drops->s_index &&*/ drops->weight < 0.2f && drops->scl_y - drops->scl_x > SCALE_PRIMA*0.2f/*SCALE_PRIMA*0.5f*/ ){
			float add_y = 4.0f*rnd();
			drops->y += add_y;
			//if( drops->y > (float)(DRAW_HEIGHT/2+32) ){
			//	drops->y *= -1.0f;
			//	drops->x = (float)((DRAW_WIDTH/2) - 32)*frnd();
			//}
			drops->key = 0.25f * TS_SINs( drops->timer ) * (add_y/2.0f) ;
			if( add_y < 1.0f ){
				DROP_DATA	*data = GetFreeMinDrops();
				if( data ){
					data->scl_x = (SCALE_PRIMA/3.5f*0.5f)+(SCALE_PRIMA/3.5f*rnd()*0.5f);
					data->scl_y = (SCALE_PRIMA/3.5f*0.5f)+(SCALE_PRIMA/3.5f*rnd()*0.5f);
					data->x = drops->x + 2.0f*TS_SINs( drops->timer/4 );
					data->y = drops->y;
					data->flags |= 0x0001;
					data->life = 200 - (int)((float)(200/4)*rnd());

					drops->scl_y *= 0.957f;
					drops->scl_x *= 0.958f;

				}
				//data->timer = 0;//irnd()%4096;
			}
		}
		drops++;
	}

}

static int ActDropsMin( DROP_DATA *pdrops, int n_drops )
{
	int				i,cnt = 0;
//	DROP_DATA		*drops = MEM_SCR_DROPS;
	DROP_DATA		*drops = pdrops;

//	TS_Mem_Scr( MEM_SCR_DROPS, pdrops, sizeof(DROP_DATA), n_drops );
	for( i = 0; i < n_drops; i++ ){
		if( drops->flags & 0x0001 ){
			drops->life -= 1;
			if( drops->life < 0 ){
				drops->life = 0;
				drops->flags &= ~0x0001;
				//printf("蒸発\n");
			}else{
				cnt++;
			}
		}
		drops++;
	}
	return cnt;
}


static void Act( Work *work )
{
#ifndef PSX2
	work->prim = work->dmapack->autopacket ;
	work->prim = InitDrowFlush( work->prim ) ;
	work->prim = InitTrBuffer( work->prim, 0 ) ;
	work->prim = InitMdlDraw( work->prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;
#endif

	if( GM_CheckPlayerStatus( PLAYER_WATCH ) && GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ){
		SVECTOR		rot_diff;
		SVECTOR		rot_diffdiff;

      // BP_WARNING - The follow would need the & in parens to work as expected.  Not fixing.
		if( !work->flags&0x0001 ){
			//主観になります			init rot
			//printf("主観になります。\n");
			work->flags |= 0x0001;
			
			work->pre_cam_rot = *work->cam_rot;
			work->pre_cam_diff_rot.vx = 0;
			work->pre_cam_diff_rot.vy = 0;
			work->pre_cam_diff_rot.vz = 0;
			rot_diff.vx = 0;
			rot_diff.vy = 0;
			rot_diff.vz = 0;
			rot_diffdiff.vx = 0;
			rot_diffdiff.vy = 0;
			rot_diffdiff.vz = 0;
			work->drops_move_x = 0.0f;
			work->drops_move_y = 0.0f;
			
		}else{
			//ずっと主観でした			rot reset
			//printf("主観ですので。\n");
			work->dmapack->flag &= ~DG_DMAPACK_INVISIBLE0;

			rot_diff.vx = work->cam_rot->vx - work->pre_cam_rot.vx;
			rot_diff.vy = work->cam_rot->vy - work->pre_cam_rot.vy;
			rot_diff.vz = work->cam_rot->vz - work->pre_cam_rot.vz;
			rot_diffdiff.vx = rot_diff.vx - work->pre_cam_diff_rot.vx;
			rot_diffdiff.vy = rot_diff.vy - work->pre_cam_diff_rot.vy;
			rot_diffdiff.vz = rot_diff.vz - work->pre_cam_diff_rot.vz;

		}
		if( !(GV_PauseLevel & GV_PAUSE_PAUSE) ){
			ActDrops( work->drop_data, N_DROPS,
					  work->drops_move_x + work->force_x,
					  work->drops_move_y + work->force_y, work->puru_timer );
			work->act_m_drops = ActDropsMin( work->drop_m_data, N_DROPS_MIN );
			
#ifdef PSX2
			SetDropVerts( work->drow_verts[DG_Clock], work->drop_data, work->cv2_def, N_DROPS, (float)work->max_life );
			SetDropVertsMin( work->drow_m_verts[DG_Clock], work->drop_m_data, work->cv2_def, N_DROPS_MIN, 200 );
			ReSetDmsTags( work->drops_pack[DG_Clock], work->act_m_drops*N_KMS_VERTS );
#else
			SetDropVerts( work, work->drop_data, work->cv2_def, N_DROPS, (float)work->max_life );
			SetDropVertsMin( work, work->drop_m_data, work->cv2_def, N_DROPS_MIN, 200 );
#endif
			work->flags &= ~0x0002;
			{
				int  max_diff_rot = abs(rot_diffdiff.vx);
				if( max_diff_rot < abs(rot_diffdiff.vy) ) max_diff_rot = abs(rot_diffdiff.vy);
				if( max_diff_rot < abs(rot_diffdiff.vz) ) max_diff_rot = abs(rot_diffdiff.vz);
				
				//printf("%d --> %d\n",max_diff_rot, work->puru_timer );
				if( max_diff_rot > 32.0f ) max_diff_rot = 32.0f;
				max_diff_rot = (int)((4096.0f*2.0f)*(float)(max_diff_rot)/32.0f);
				if( work->puru_timer < max_diff_rot ){
					work->drops_move_x = 5.0f*rot_diffdiff.vy/38.0f;
					work->drops_move_y = -5.0f*rot_diffdiff.vx/38.0f;
					work->puru_timer = max_diff_rot;
				}
			}
			
			
			//printf("(%d,%d,%d)->(%d,%d,%d):[%d,%d,%d]\n", rot_diff.vx, rot_diff.vy, rot_diff.vz,
			//	   work->pre_cam_diff_rot.vx, work->pre_cam_diff_rot.vy, work->pre_cam_diff_rot.vz,
			//	   rot_diff.vx - work->pre_cam_diff_rot.vx,
			//	   rot_diff.vy - work->pre_cam_diff_rot.vy,
			//	   rot_diff.vz - work->pre_cam_diff_rot.vz );

		}else if( !(work->flags & 0x0002) ){
#ifdef PSX2
			SetDropVerts( work->drow_verts[DG_Clock], work->drop_data, work->cv2_def, N_DROPS, (float)work->max_life );
			SetDropVertsMin( work->drow_m_verts[DG_Clock], work->drop_m_data, work->cv2_def, N_DROPS_MIN, 200 );
			ReSetDmsTags( work->drops_pack[DG_Clock], work->act_m_drops*N_KMS_VERTS );
#else
			SetDropVerts( work, work->drop_data, work->cv2_def, N_DROPS, (float)work->max_life );
			SetDropVertsMin( work, work->drop_m_data, work->cv2_def, N_DROPS_MIN, 200 );
#endif			
			work->flags |= 0x0002;
		}
		work->pre_cam_rot = *work->cam_rot;
		work->pre_cam_diff_rot = rot_diff;
	}else{
		
		if( work->flags&0x0001 ){
			//主観解除					rot clear
			//printf("主観やーめた\n");
			work->flags &= ~0x0001;
			work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
			//SetDropVerts( work->drow_verts[DG_Clock], work->drop_data, work->cv2_def, N_DROPS, (float)work->max_life );
		}else{
			//ずっと主観じゃありません	sleep
			//printf("主観しとりません。\n");

			GV_DestroyActor( work );
		}
	}
	if( !(GV_PauseLevel & GV_PAUSE_PAUSE) ){
		work->puru_timer -= 128;
		if( work->puru_timer <= 1024 ) work->puru_timer = 1024;

		work->drops_move_x *= 0.7f;
		work->drops_move_y *= 0.7f;
		if( DG_FABS(work->drops_move_x) < 0.01f ) work->drops_move_x = 0.0f;
		if( DG_FABS(work->drops_move_y) < 0.01f ) work->drops_move_y = 0.0f;
	}

	work->force_x = 0.0f;
	work->force_y = 0.0f;

#ifndef PSX2
	work->prim = DG_SetDmapackEnd( work->prim ) ;
//printf( "scr_drop.c need Size%d\n",  (int)work->prim-(int)work->packet_mem  ) ;
	if( work->xbox_packet_size < ((int)work->prim-(int)work->packet_mem) ){
		printf("xbox_packet_size not enough!!\n");
		ASSERT( 0 );
	}
#endif
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
	if(work->drow_m_verts[0]) GV_DelayedFree(work->drow_m_verts[0]);
	g_work = NULL;
}




static void InitRndDropData( DROP_DATA *p_drop, int n_drops, int life )
{
	int i;
	DROP_DATA	*drops = p_drop;

	for( i = 0; i < n_drops; i++ ){

		drops->scl_x = SCALE_PRIMA*0.5f + SCALE_PRIMA*rnd();
		//if( irnd() & 0x00400000 ) drops->scl_x *= -1.0f;
		drops->scl_y = SCALE_PRIMA*0.5f + SCALE_PRIMA*rnd();
		drops->x = (float)((DRAW_WIDTH/2) - 32)*frnd();
		drops->y = (float)((DRAW_HEIGHT/2) - 32)*frnd();
		drops->key = 0.0f;
		drops->weight = rnd();
		
		drops->s_index = (irnd()>>5)%3 * 2;
		drops->flags = 0;//drops->l_index+1;

		drops->life = life - (int)((float)(life/4)*rnd());
		drops->timer = irnd()%4096;

		drops++;
	}
}

static void InitRndDropData_Rnd( DROP_DATA *p_drop, int n_drops, int life )
{
	int i;
	DROP_DATA	*drops = p_drop;
	DROP_DATA	temp;

	for( i = 0; i < n_drops; i++ ){
		int index = i + (irnd()>>6)%(n_drops-i);
		temp = drops[index];
		drops[index] = drops[i];
		drops[i] = temp;
	}

	for( i = 0; i < n_drops/2; i++ ){
		
		drops->scl_x = SCALE_PRIMA*0.5f + SCALE_PRIMA*rnd();
		//if( irnd() & 0x00400000 ) drops->scl_x *= -1.0f;
		drops->scl_y = SCALE_PRIMA*0.5f + SCALE_PRIMA*rnd();
		drops->x = (float)((DRAW_WIDTH/2) - 32)*frnd();
		drops->y = (float)((DRAW_HEIGHT/2) - 32)*frnd();
		drops->key = 0.0f;
		drops->weight = rnd();
		
		drops->s_index = (irnd()>>5)%3 * 2;
		drops->flags = 0;//drops->l_index+1;

		drops->life = life - (int)((float)(life/4)*rnd());
		drops->timer = irnd()%4096;

		drops++;
	}
}

static void InitRndDropData2( DROP_DATA *p_drop, int n_drops, int life )
{
	int i;
	DROP_DATA	*drops = p_drop;

	for( i = 0; i < n_drops; i++ ){

		drops->scl_x = 0.0f;
		drops->scl_y = 0.0f;
		drops->x = 0.0f;
		drops->y = 0.0f;
		drops->key = 5.0f;
		drops->weight = 1.0f;
		drops->s_index = 0;
		drops->flags = 0;

		drops->life = 0;//life - (int)((float)(life/4)*rnd());
		drops->timer = 0;//irnd()%4096;

		drops++;
	}
}


#ifdef PSX2  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ---------------------------------------------------------------- */
static void InitDmaTags( Work *work, DROPS_PACK *packet, int which, int verts_size )
{
	// --------  DMATag  ----------------------------
	// ＤＭＡタグ		退避
	packet->dmatag_tr.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(TR_BUFFER) );
	packet->dmatag_tr.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_tr.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(TR_BUFFER), 0);
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

	// ＤＭＡタグ		モデル頂点
	packet->dmatag_m_mverts.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, sizeof(DG_GIFTAG)/sizeof(u_long128) );
	packet->dmatag_m_mverts.addr = work->drow_m_verts[which];
	packet->dmatag_m_mverts.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_m_mverts.vifcode[1] = SCE_VIF1_SET_DIRECT( sizeof(DG_GIFTAG)/sizeof(u_long128), 0 );
}

static void InitMVerts( void *verts, DG_DEF *def, int n_drops, int size )
{
	DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack;
	SVECTOR 	*sverts;
	SVECTOR 	*snorms;
	DG_GIFTAG	*giftag = (DG_GIFTAG*)verts;
	MVERT_DATA	*mverts = (MVERT_DATA*)(giftag+1);
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
				mverts->rgbq.reg = SCE_GS_RGBAQ;
				mverts->uv.reg = SCE_GS_UV;
				mverts->xyz.reg = (snorms->pad&0x8000)?SCE_GS_XYZ3:SCE_GS_XYZ2;
				mverts->rgbq.data = SCE_GS_SET_RGBAQ(0x80,0x80,0x80,0x80,0);
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

#endif  /*!!!!!!!!!!!!!!!!!!!!!!! PSX2でしか使用しない関数!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


static int GetResources( Work *work, int life )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;
	DG_DEF			*def;
	CV2_DEF			*cv2_def;
	int				n_verts,verts_size;
	GM_CameraSet	*cam;
#ifdef PSX2
	DROPS_PACK		*packet;
	int  i ;
#endif
	
//	DG_GIFTAG		*giftag;
#if 1
	//デモ用で切り替える
	cam = GM_PlayerSubjectCamera[ 0 ];
	if(cam == NULL){
		printf("ERR!! NO CAMERA[0]!!\n");
		return -1;
	}

	work->cam_rot = &cam->rotate;
	work->pre_cam_rot = cam->rotate;
	work->pre_cam_diff_rot.vx = 0;
	work->pre_cam_diff_rot.vy = 0;
	work->pre_cam_diff_rot.vz = 0;
#endif
	def = (DG_DEF*)GV_GetCache( GV_CacheID( MDL_CODE, 'k' ));
	if(def == NULL){ printf("ERR!! NO MODEL!!\n"); return -1; }

	cv2_def = work->cv2_def = (CV2_DEF*)GV_GetCache( GV_CacheID( MDL_CODE, 'c' ));
	if( cv2_def == NULL){ printf("ERR!! NO MODEL!!\n"); return -1; }

	n_verts = GetMdlVertsJoint( def, 0 );//GetMdlVerts( def );				//モデルの頂点数
	verts_size = sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*n_verts*N_DROPS;

	printf("n_verts kms %d: cv2 %d scr_bottom %x\n", n_verts, cv2_def->models[0].n_verts, MEM_SCR_BOTTOM );
	printf("n_verts %d: dma size %x: gif size %x\n", n_verts, verts_size/sizeof(FVECTOR),
		   sizeof(MVERT_DATA)*n_verts*N_DROPS/sizeof(FVECTOR) );

	// 頂点データメモり確保
	verts = GV_Malloc( verts_size * 2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->drow_verts[0] = (MVERT_DATA*)( verts );
	work->drow_verts[1] = (MVERT_DATA*)( verts + verts_size );

	// 頂点データメモり確保
	verts = GV_Malloc( (sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*n_verts*N_DROPS_MIN) * 2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->drow_m_verts[0] = (MVERT_DATA*)( verts );
	work->drow_m_verts[1] = (MVERT_DATA*)( verts + (sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*n_verts*N_DROPS_MIN) );

//	giftag = (DG_GIFTAG*)work->drow_m_verts;
//	giftag->tag  = SCE_GIF_SET_TAG( 0, 1, 0, 0, 0, 1);
//	giftag->regs = GS_REGS_AD;
	
	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											 DG_DMAPACK_PHASE_AFTER );
	if(dmapack == NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	DG_QueueDmapack( dmapack );

	InitRndDropData( work->drop_data, N_DROPS, life );
	InitRndDropData2( work->drop_m_data, N_DROPS_MIN, life );

#ifdef PSX2
	// パケットメモリ割り当て
	packet = GV_Malloc( sizeof(DROPS_PACK) * 2 );
	if(packet == NULL){ printf("ERR!! MALLOC DROPS_PACK!!\n"); return -1; }
	work->drops_pack[0] = dmapack->packet[0] = &packet[0];
	work->drops_pack[1] = dmapack->packet[1] = &packet[1];

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaTags( work, &packet[i], i, verts_size );
		InitTrBuffer( &packet[i].tr_buffer, i );
		InitMdlDraw( &packet[i].mdl_draw, MDL_TEST, MDL_ALPHA, MDL_PRIM );
		InitMVerts( work->drow_verts[i], def, N_DROPS, sizeof(MVERT_DATA)*n_verts*N_DROPS );
		InitMVerts( work->drow_m_verts[i], def, N_DROPS_MIN, 0 );
		InitEndPacket( &packet[i].end_paket );

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
	work->xbox_packet_size = 180000;/* Actを回して、このサイズを下回ることがないのを確認 */
	if ( !(work->packet_mem = GV_Malloc( work->xbox_packet_size )) ) {
		printf("ERR!! MALLOC scr_drop.c!!\n");
		return -1 ;
	}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( dmapack->autopacket ) ;

#endif /*PSX2*/


	work->max_life = work->life = life;
#if 0
	if( GM_CheckPlayerStatus( PLAYER_WATCH ) && GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ){
		if( !work->flags&0x0001 ){
			//主観になります			init rot
			//printf("主観になります。\n");
			work->flags |= 0x0001;
			ActDrops( work->drop_data, N_DROPS, NULL, NULL, 4*4096-1 );
		}		
	}else{
		if( work->flags&0x0001 ){
			ActDrops( work->drop_data, N_DROPS, NULL, NULL, 4*4096-1 );
		}
	}
#endif
	work->puru_timer = 0;
	work->force_x = 0.0f;
	work->force_y = 0.0f;
	return (0);
}

void *NewScrDrop( int life )
{
	Work		*work ;

	printf("------------- NewScrDrop run ---------------\n");
	if(g_work){
		//水滴情報だけ更新すればいいと思う
		InitRndDropData_Rnd( g_work->drop_data, N_DROPS, life );
		g_work->max_life = life;
		return g_work;
	}
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, life) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		g_work = work;
		printf("----------------- NewScrDrop OK ---------------\n");
	}

	return (void *)work ;
}
void SetDropForce( float x, float y )
{
	if( !g_work ) return;
	g_work->force_x += x;
	g_work->force_y += y;
}
