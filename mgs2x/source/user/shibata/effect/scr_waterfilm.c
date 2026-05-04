//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scr_waterfilm.c
	歪みモデルを使った奴(歪み変化)

	2001/08/13 T.Shibata
	
	$Id: scr_waterfilm.c,v 1.1.1.3 2002/11/19 11:48:42 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

//#define MDL_CODE 	(GV_StrCode("water_drop_def01"))
//#define MDL_CODE 	(GV_StrCode("water_drop_rand01"))
//#define MDL_CODE 	(GV_StrCode("water_drop_rand02"))
#define MDL_CODE 	(GV_StrCode("scr_waterline01"))


#define MDL_PRIM		(SCE_GS_SET_PRIM( 4, 1, 1, 0, 1, 0, 1, 0, 0 ))
#define MDL_TEST		(SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 ))
#define MDL_ALPHA		(SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x80 ))

#define		N_DROPS		(2*4)

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

typedef struct {
	float		scl_x, x, y;
	short		k_param, l_param;	
} DROP_DATA;

typedef struct {
	DG_DMATAG		dmatag_flush;			// ＧＩＦ接続ＤＭＡタグ
	DROW_FLUSH 		drow_flush;				//								フラッシュ
	DG_DMATAG		dmatag_mdl;				// ＧＩＦ接続ＤＭＡタグ
	MDL_DRAW		mdl_draw;				//								モデル描画の設定
	DG_DMATAG		dmatag_mverts;			// ＧＩＦ接続ＤＭＡタグ			モデル頂点(変化)

	DG_DMATAG		dmatag_test;			// ＧＩＦ接続ＤＭＡタグ
	PACKET_END		end_paket;				//								ピクセルテスト復元
	DG_DMATAG		dmatag_offset;			// オフセット環境復元用
	DG_DMATAG		dmatag_end;				// RETタグ
} DROPS_PACK;

#define	N_MAX_OBJ_PACKS		(64)
typedef	struct	scr_waterfilm_Work {
	GV_ACT_EX		actor ;

	ALIGN16_PRE DROPS_PACK		*drops_pack[2] ALIGN16_POST;
	ALIGN16_PRE void			*drow_verts[2] ALIGN16_POST;
	ALIGN16_PRE DROP_DATA		drop_data[N_DROPS] ALIGN16_POST;
	
	FVECTOR			pre_campos;
	FVECTOR			*campos;
	FVECTOR			*camdir;
	SVECTOR			pre_camdir;
	CV2_DEF			*cv2_def;
	DG_DMAPACK		*dmapack;
	
	int				flags;
	int				interval;

	int				n_packs;
	u_char			n_packs_verts[N_MAX_OBJ_PACKS];  /* XBOXでは、頂点が無限なのでとりあえずアンサインドにした */

	void				*packet_mem;
	void                *prim ;
	DG_DEF              *def ;
	int xbox_packet_size;

} Work;

extern float GM_WaterLevel;
//static Work *g_work = NULL;


//モデルによって変更				
#define		N_CV2_VERTS		(85)
#define		N_KMS_VERTS		(154)		//cv2の奴ね
#define		N_MDL_JOINT		(4)

#define		MEM_SCR_VERTS1		((void *)(SCRPAD_ADDR))
#define		MEM_SCR_VERTS2		((void *)((char *)(MEM_SCR_VERTS1) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS3		((void *)((char *)(MEM_SCR_VERTS2) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_VERTS4		((void *)((char *)(MEM_SCR_VERTS3) + sizeof(FVECTOR)*N_CV2_VERTS))

#define		MEM_SCR_NORMS1		((void *)((char *)(MEM_SCR_VERTS4) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_NORMS2		((void *)((char *)(MEM_SCR_NORMS1) + sizeof(FVECTOR)*N_CV2_VERTS))

#if 1
#define		MEM_SCR_INDEX		((void *)((char *)(MEM_SCR_NORMS2) + sizeof(FVECTOR)*N_CV2_VERTS))
#define		MEM_SCR_X			((int*)((char*)(MEM_SCR_INDEX) + sizeof(short)*N_KMS_VERTS))
#else
#define		MEM_SCR_X			((int*)((char *)(MEM_SCR_NORMS4) + sizeof(FVECTOR)*N_CV2_VERTS))
#endif
#define		MEM_SCR_Y			((int*)((char*)(MEM_SCR_X) + sizeof(int) ))
#define		MEM_SCR_BOTTOM		((void *)((char *)(MEM_SCR_Y) + sizeof(int) ))

//#define		SCALE_PRIMA		(256.0f/5000.0f)
#define		SCALE_PRIMA		(320.0f/5000.0f)
#define		UV_SCALE		(128.0f)

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
	DG_SetPos( world ) ;
	DG_RotVectorW(in, out, 1 ) ;
#endif
}


static FVECTOR	_acc, _vftmp, _vf19, _vf18, _vf17, _vf16, _vf04, _vf05, _vf06, _vf07 ;

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

//２回に分けてkを計算する

static inline void _GetDividingPoint_1st( FVECTOR *verts,
										  FVECTOR *a_vert0, FVECTOR *a_vert1, FVECTOR *b_vert0, FVECTOR *b_vert1,
										  FVECTOR *a_norm0, FVECTOR *a_norm1, FVECTOR *b_norm0, FVECTOR *b_norm1 )
{
#ifdef BP_PSX2_ASM
	// s = l
	// t = 1.0f - l
	asm volatile ("
		lqc2			vf08, 0x00(%1)	# a_vert0
		lqc2			vf09, 0x00(%2)	# a_vert1
		lqc2			vf10, 0x00(%3)	# b_vert0
		lqc2			vf11, 0x00(%4)	# b_vert1

		lqc2			vf12, 0x00(%5)	# a_norm0
		lqc2			vf13, 0x00(%6)	# a_norm1
		lqc2			vf14, 0x00(%7)	# b_norm0
		lqc2			vf15, 0x00(%8)	# b_norm1

		vmulax.xyz		ACC,  vf08, vf16x	#   s * a_vert0
		vmaddy.xyz		vf04, vf09, vf16y	# + t * a_vert1

		vmulax.xyz		ACC,  vf10, vf16x	#   s * b_vert0
		vmaddy.xyz		vf05, vf11, vf16y	# + t * b_vert1

		vmulax.xyz		ACC,  vf12, vf16x	#   s * a_norm0
		vmaddy.xyz		vf06, vf13, vf16y	# + t * a_norm1

		vmulax.xyz		ACC,  vf14, vf16x	#   s * b_norm0
		vmaddy.xyz		vf07, vf15, vf16y	# + t * b_norm1

		sqc2	        vf04, 0x00(%0)
		#sqc2	        vf05, 0x00(%1)
		#sqc2	        vf06, 0x00(%2)
		#sqc2	        vf07, 0x00(%3)

	": : "r"(verts),
		 "r"(a_vert0), "r"(a_vert1), "r"(b_vert0), "r"(b_vert1),
		 "r"(a_norm0), "r"(a_norm1), "r"(b_norm0), "r"(b_norm1) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, a_vert0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf04, a_vert1, _vf16.vy ) ;
	_sceVu0AddVector( &_vf04, &_vf04, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, b_vert0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf05, b_vert1, _vf16.vy ) ;
	_sceVu0AddVector( &_vf05, &_vf05, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, a_norm0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf06, a_norm1, _vf16.vy ) ;
	_sceVu0AddVector( &_vf06, &_vf06, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, b_norm0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vf07, b_norm1, _vf16.vy ) ;
	_sceVu0AddVector( &_vf07, &_vf07, &_acc );

	*verts = _vf04 ;
#endif
}


static inline void _GetDividingPoint_2nd( FVECTOR *verts, FVECTOR *norms, FVECTOR *k_param )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf19, 0x00(%2)	# a_vert0

		vmulax.xyz		ACC,  vf04, vf19x	#   s * a_vert0
		vmaddy.xyz		vf04, vf05, vf19y	# + t * b_vert1

		vmulax.xyz		ACC,  vf06, vf19x	#   s * a_norm0
		vmaddy.xyz		vf06, vf07, vf19y	# + t * b_norm1

		vmul.xyz		vf05, vf04, vf17	# verts * scale
		vmul.xyz		vf07, vf06, vf18	# norms * scale

		sqc2	        vf05, 0x00(%0)
		sqc2	        vf07, 0x00(%1)

	": : "r"(verts), "r"(norms), "r"(k_param) :"memory" );
#else
	_vf19 = *k_param ;

	_sceVu0ScaleVectorXYZ( &_acc, &_vf04, _vf19.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vftmp, &_vf05, _vf19.vy ) ;
	_sceVu0AddVector( &_vf04, &_vftmp, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, &_vf06, _vf19.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vftmp, &_vf07, _vf19.vy ) ;
	_sceVu0AddVector( &_vf06, &_vftmp, &_acc );

	_sceVu0MulVector( verts, &_vf04, &_vf17 ) ;
	_sceVu0MulVector( norms, &_vf06, &_vf18 ) ;
#endif
}


static inline void _GetDividingPoint( FVECTOR *verts, FVECTOR *norms,// FVECTOR *uv_scl,
									  FVECTOR *a_vert0, FVECTOR *a_vert1, FVECTOR *b_vert0, FVECTOR *b_vert1,
									  FVECTOR *a_norm0, FVECTOR *a_norm1, FVECTOR *b_norm0, FVECTOR *b_norm1 )
{
#ifdef BP_PSX2_ASM
	// s = k
	// t = 1.0f - k
	// m = l
	// n = 1.0f - l
	// m*s = vf16.x
	// m*t = vf16.y
	// n*s = vf16.z
	// n*t = vf16.w
	// verts = m*(s*a_vert0 + t*a_vert1) + n*(s*b_vert0 + t*b_vert1)
	//		 = m*s*a_vert0 + m*t*a_vert1 + n*s*b_vert0 + n*t*b_vert1
	asm volatile ("
		lqc2			vf08, 0x00(%2)	# a_vert0
		lqc2			vf09, 0x00(%3)	# a_vert1
		lqc2			vf10, 0x00(%4)	# b_vert0
		lqc2			vf11, 0x00(%5)	# b_vert1

		lqc2			vf12, 0x00(%6)	# a_norm0
		lqc2			vf13, 0x00(%7)	# a_norm1
		lqc2			vf14, 0x00(%8)	# b_norm0
		lqc2			vf15, 0x00(%9)	# b_norm1

		vmulax.xyz		ACC,  vf08, vf16x	#   (m*s) * a_vert0
		vmadday.xyz		ACC,  vf09, vf16y	# + (m*t) * a_vert1
		vmaddaz.xyz		ACC,  vf10, vf16z	# + (n*s) * b_vert0
		vmaddw.xyz		vf04, vf11, vf16w	# + (n*t) * b_vert1 = verts

		vmulax.xyz		ACC,  vf12, vf16x	#   (m*s) * a_norm0
		vmadday.xyz		ACC,  vf13, vf16y	# + (m*t) * a_norm1
		vmaddaz.xyz		ACC,  vf14, vf16z	# + (n*s) * b_norm0
		vmaddw.xyz		vf06, vf15, vf16w	# + (n*t) * b_norm1 = norms

		vmul.xyz		vf05, vf04, vf17	# verts * scale
		vmul.xyz		vf07, vf06, vf18	# norms * scale

		sqc2	        vf05,0x00(%0)
		sqc2	        vf07,0x00(%1)

	": : "r"(verts), "r"(norms),
		 "r"(a_vert0), "r"(a_vert1), "r"(b_vert0), "r"(b_vert1),
		 "r"(a_norm0), "r"(a_norm1), "r"(b_norm0), "r"(b_norm1) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, a_vert0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vftmp, a_vert1, _vf16.vy ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_vert0, _vf16.vz ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_vert1, _vf16.vw ) ;
	_sceVu0AddVector( &_vf04, &_vftmp, &_acc );

	_sceVu0ScaleVectorXYZ( &_acc, a_norm0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vftmp, a_norm1, _vf16.vy ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_norm0, _vf16.vz ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_norm1, _vf16.vw ) ;
	_sceVu0AddVector( &_vf06, &_vftmp, &_acc );

	_sceVu0MulVector( verts, &_vf04, &_vf17 ) ;
	_sceVu0MulVector( norms, &_vf06, &_vf18 ) ;
#endif
}

static inline void _GetDividingPointVerts( FVECTOR *verts,
										   FVECTOR *a_vert0, FVECTOR *a_vert1, FVECTOR *b_vert0, FVECTOR *b_vert1 )
{
#ifdef BP_PSX2_ASM
	// s = k
	// t = 1.0f - k
	// m = l
	// n = 1.0f - l
	// m*s = vf16.x
	// m*t = vf16.y
	// n*s = vf16.z
	// n*t = vf16.w
	// verts = m*(s*a_vert0 + t*a_vert1) + n*(s*b_vert0 + t*b_vert1)
	//		 = m*s*a_vert0 + m*t*a_vert1 + n*s*b_vert0 + n*t*b_vert1
	asm volatile ("
		lqc2			vf08, 0x00(%1)	# a_vert0
		lqc2			vf09, 0x00(%2)	# a_vert1
		lqc2			vf10, 0x00(%3)	# b_vert0
		lqc2			vf11, 0x00(%4)	# b_vert1

		vmulax.xyz		ACC,  vf08, vf16x	#   (m*s) * a_vert0
		vmadday.xyz		ACC,  vf09, vf16y	# + (m*t) * a_vert1
		vmaddaz.xyz		ACC,  vf10, vf16z	# + (n*s) * b_vert0
		vmaddw.xyz		vf04, vf11, vf16w	# + (n*t) * b_vert1 = verts

		vmul.xyz		vf05, vf04, vf17	# verts * scale

		sqc2	        vf05,0x00(%0)

	": : "r"(verts),
		 "r"(a_vert0), "r"(a_vert1), "r"(b_vert0), "r"(b_vert1) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, a_vert0, _vf16.vx ) ;
	_sceVu0ScaleVectorXYZ( &_vftmp, a_vert1, _vf16.vy ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_vert0, _vf16.vz ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_vert1, _vf16.vw ) ;
	_sceVu0AddVector( &_vf04, &_vftmp, &_acc );

	_sceVu0MulVector( verts, &_vf04, &_vf17 ) ;
#endif
}


static inline void _GetDividingPointNorms( FVECTOR *norms, FVECTOR *k_param,
										   FVECTOR *a_norm0, FVECTOR *a_norm1, FVECTOR *b_norm0 )
{
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2			vf19, 0x00(%4)	# b_vert1
		lqc2			vf12, 0x00(%1)	# a_vert0
		lqc2			vf13, 0x00(%2)	# a_vert1
		lqc2			vf14, 0x00(%3)	# b_vert0


		vmulax.xyz		ACC,  vf12, vf19x	#
		vmadday.xyz		ACC,  vf13, vf19y	#
		vmaddz.xyz		vf04, vf14, vf16z	#

		vmul.xyz		vf05, vf04, vf18	# verts * scale

		sqc2	        vf05,0x00(%0)

	": : "r"(norms),
		 "r"(a_norm0), "r"(a_norm1), "r"(b_norm0), "r"(k_param) :"memory" );
#else
	_sceVu0ScaleVectorXYZ( &_acc, a_norm0, k_param->vx ) ;
	_sceVu0ScaleVectorXYZ( &_vftmp, a_norm1, k_param->vy ) ;
	_sceVu0AddVector( &_acc, &_vftmp, &_acc );
	_sceVu0ScaleVectorXYZ( &_vftmp, b_norm0, _vf16.vz ) ;
	_sceVu0AddVector( &_vf04, &_vftmp, &_acc );

	_sceVu0MulVector( norms, &_vf04, &_vf18 ) ;
#endif
}

static int SetDropVerts( void *output, DROP_DATA *pdrops, CV2_DEF *cv2_def, int n_drops, int n_packs, u_char *n_packs_vers )
{
	int				i,j,k, cnt = n_drops;
	DROP_DATA		*drops = pdrops;
	short			   *index;
   int            n_index;
	int				packs_index,n_all_verts;
	Work            *work = (Work *)output ;
	float			ix,iy,iu,iv ;
	int             v_cnt ;
	void            *head ;
	DG_MDLPACK 	*objpack;
	int          v0, v1, v2 ;
	FVECTOR			*a_verts0 = (FVECTOR*)MEM_SCR_VERTS1;
	FVECTOR			*a_verts1 = (FVECTOR*)MEM_SCR_VERTS2;
	FVECTOR			*b_verts0 = (FVECTOR*)MEM_SCR_VERTS3;
	FVECTOR			*b_verts1 = (FVECTOR*)MEM_SCR_VERTS4;
	FVECTOR			*a_norms0 = (FVECTOR*)MEM_SCR_NORMS1;
	FVECTOR			*a_norms1 = (FVECTOR*)MEM_SCR_NORMS2;
	static FVECTOR	scale_norms = { UV_SCALE, UV_SCALE, 1.0f, 0.0f };

	if( DG_Chanls[0].eye.m[3][1] < GM_WaterLevel )
   {
		return 0;
	}

	TS_Mem_Scr( MEM_SCR_VERTS1, cv2_def->models[0].verts, sizeof(FVECTOR), N_CV2_VERTS*(N_MDL_JOINT*2-2) );

	_LoadScaleDividingPoint1( &scale_norms );

	for( i = 0; i < n_drops; i++ )
   {
		float l_param = (float)abs(drops->l_param)/4096.0f;// - 4096);
		float k_param = (float)abs(drops->k_param)/4096.0f;// - 4096);
		FVECTOR			stlm = {
			(		k_param) * (	   l_param),
			(		k_param) * (1.0f - l_param),
			(1.0f - k_param) * (	   l_param),
			(1.0f - k_param) * (1.0f - l_param),
		};
		
      FVECTOR			scale_verts = { drops->scl_x, SCALE_PRIMA, 1.0f, 0.0f };

		if( !(GV_PauseLevel & GV_PAUSE_PAUSE) &&
			!(GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON )) ){

#define			WIDTH_P	(256.0f*SCALE_PRIMA)
#define			WIDTH_N	(-1024.0f*SCALE_PRIMA)

			drops->y += 96.0f;
			if( drops->y > 5000.0f+WIDTH_N )
         {
			   cnt--;
			}
		}

		*MEM_SCR_X = (int)drops->x;
		// *MEM_SCR_Y = (int)drops->y;

		_LoadScaleDividingPoint2( &stlm, &scale_verts );

      index = cv2_def->models[0].verts_index;
      n_index = cv2_def->models[0].n_verts_index;

		n_all_verts = 0;
		packs_index = 0;

      for( j = 0; j < n_packs; j++ )
      {
			FVECTOR		verts;
			FVECTOR		norms;
			FVECTOR		scl;
			float		   ftemp;
         
         int numPackVerts = n_packs_vers[j];

         work->prim = DG_SetDmapackTriangleStrip( work->prim, numPackVerts );

         for( k = 0; k < numPackVerts; ++k )
         {
            int alpha = 128;

				_GetDividingPointVerts( &verts,
										&a_verts0[*index], &a_verts1[*index],
										&b_verts0[*index], &b_verts1[*index] );

				ftemp = verts.vy - drops->y*SCALE_PRIMA;
				if( ftemp > 0.0f )
            {
					if( ftemp > WIDTH_P ){
						ftemp = WIDTH_P;
						alpha = 0;
					}
					scl.vx = ftemp = ftemp/WIDTH_P * l_param;
					scl.vy = ftemp = ftemp/WIDTH_P * (1.0f - l_param );
					scl.vz = ftemp = (1.0f - ftemp/WIDTH_P);
					
				}
            else
            {
					if( ftemp < WIDTH_N ){
						ftemp = WIDTH_N;
						alpha = 0;
					}
					scl.vx = ftemp = ftemp/WIDTH_N * l_param;
					scl.vy = ftemp = ftemp/WIDTH_N * (1.0f - l_param );
					scl.vz = ftemp = (1.0f - ftemp/WIDTH_N);
				}

				{
					FVECTOR tmp={ 0.0f, 0.0f, -1.0f, 0.0f } ;
					_GetDividingPointNorms( &norms, &scl,
											&a_norms0[*index], &a_norms1[*index],
											&tmp );
				}
				
				ix = verts.vx/512.0f*DRAW_WIDTH ;
				iy = verts.vy;
				iu = ix - norms.vx;
				iv = iy - norms.vy;
				ix = (DRAW_WIDTH /2 + ix + (*MEM_SCR_X));
				iy = (DRAW_HEIGHT/2 + iy);
				iu = DG_FRAME_U(DRAW_WIDTH /2 + iu + (*MEM_SCR_X)) ;
				iv = DG_FRAME_V(DRAW_HEIGHT/2 + iv) ;

            index++;

            work->prim = DG_SetDmapackVertex( work->prim, ix,iy, iu,iv, DG_MakeDMAPackColor(128,128,128,alpha) ) ;
			}
		}
		drops++;
	}
	return cnt;
}




static void Act( Work *work )
{
	work->prim = work->dmapack->autopacket ;
	work->prim = InitDrowFlush( work->prim ) ;
	work->prim = InitTrBuffer( work->prim, 0 ) ;
	work->prim = InitMdlDraw( work->prim, MDL_TEST, MDL_ALPHA, MDL_PRIM ) ;

	if( !SetDropVerts( work, work->drop_data, work->cv2_def, N_DROPS, work->n_packs, work->n_packs_verts ) )
   {
		work->dmapack->flag |= DG_DMAPACK_INVISIBLE0;
		GV_DestroyActor( work );
	}
	work->prim = DG_SetDmapackEnd( work->prim );

	//printf( "scr_waterfilm.c need Size%d\n",  (int)work->prim-(int)work->packet_mem  ) ;

	if( work->xbox_packet_size < ((int)work->prim-(int)work->packet_mem) ){
		printf("xbox_packet_size not enough!!\n");
		ASSERT( 0 );
	}
}

/* ---------------------------------------------------------------- */

static void Die( Work *work )
{
	if(work->dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}

	if(work->packet_mem) GV_DelayedFree( work->packet_mem );
	
	if(work->drow_verts[0]) GV_DelayedFree(work->drow_verts[0]);

//	g_work = NULL;
}

static void InitRndDropData( DROP_DATA *p_drop, int n_drops )
{
	int i;
	DROP_DATA	*drops = p_drop;

	for( i = 0; i < n_drops/2; i++ ){

		drops->scl_x = SCALE_PRIMA * 0.5f + SCALE_PRIMA*rnd();
		drops->x = frnd() * 200.0f;//(float)((DRAW_WIDTH/2) - 32)*frnd();
		drops->y = -6000.0f + -2000.0f * rnd();
		drops->k_param = 4096;
		drops->l_param = irnd()%4096;
		drops++;

		drops->scl_x = drops[-1].scl_x;
		drops->x = drops[-1].x;
		drops->y = drops[-1].y-2000.0f;
		drops->k_param = 1024;
		drops->l_param = drops[-1].l_param;
		drops++;
	}
}

static void InitObjPacksVerts( Work *work, DG_DEF *def )
{
   int			i, j, vertCount;

   DG_MDL 		*mdl = def->models;
	DG_OBJPACK 	*objpack = mdl->packs;

	work->n_packs = 0;

   for( i = 0; i < mdl->n_packs; i++ )
   {
      short* normals = objpack->norms;

      vertCount = 0;

      for( j = 0; j < objpack->n_verts; )
      {
         short flag = normals[3];
       
         if( (flag & 0x8000) && vertCount > 2 )
         {
            work->n_packs_verts[work->n_packs] = vertCount;
            work->n_packs++;
            
            vertCount = 0;
            continue;
         }

         vertCount++;
         j++;
         normals += 4;
      }

      if( vertCount > 0 )
      {
         work->n_packs_verts[work->n_packs] = vertCount;
         work->n_packs++;
         vertCount = 0;
      }

      objpack++;
	}
}

static int GetResources( Work *work )
{
	DG_DMAPACK		*dmapack;
	u_char			*verts;
	DG_DEF			*def;
	CV2_DEF			*cv2_def;
	int				n_verts,verts_size;
	DROPS_PACK		*packet;
	int  i ;

	work->campos = (FVECTOR*)DG_Chanls[0].eye.m[3];
	work->camdir = (FVECTOR*)DG_Chanls[0].eye.m[2];
	DG_COPY_VEC( &work->pre_campos, work->campos );
	TS_MatToRot( &work->pre_camdir, &DG_Chanls[0].eye );
	
	def = (DG_DEF*)GV_GetCache( GV_CacheID( MDL_CODE, 'k' ));
	if(def == NULL){ printf("ERR!! NO MODEL!! kms\n"); return -1; }

	cv2_def = work->cv2_def = (CV2_DEF*)GV_GetCache( GV_CacheID( MDL_CODE, 'c' ));
	if( cv2_def == NULL){ printf("ERR!! NO MODEL cv2!!\n"); return -1; }

	n_verts = GetMdlVertsJoint( def, 0 );//GetMdlVerts( def );				//モデルの頂点数
	verts_size = sizeof(DG_GIFTAG) + sizeof(MVERT_DATA)*n_verts*N_DROPS;

//	printf("n_verts kms %d: cv2 %d scr_bottom %x\n", n_verts, cv2_def->models[0].n_verts, MEM_SCR_BOTTOM );
	
//	printf("n_verts %d: dma size %x: gif size %x\n", cv2_def->models[0].verts_index, verts_size/sizeof(FVECTOR),
//		   sizeof(MVERT_DATA)*n_verts*N_DROPS/sizeof(FVECTOR) );

	// 頂点データメモり確保
	verts = GV_Malloc( verts_size * 2 );
	if(verts == NULL){ printf("ERR!! MALLOC MVERTS!!\n"); return -1; }
	work->drow_verts[0] = (MVERT_DATA*)( verts );
	work->drow_verts[1] = (MVERT_DATA*)( verts + verts_size );

	// ＤＭＡパケット型オブジェクト作成
	work->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3,
											  DG_DMAPACK_PHASE_AFTER, 143 );
	if(dmapack == NULL){ printf("ERR!! MAKE DMAPACK!!\n"); return -1; }
	//dmapack->flag |= DG_DMAPACK_INVISIBLE0;
	DG_QueueDmapack( dmapack );

	/* モデルのn_packsからpacksの配列を初期化する */
	InitObjPacksVerts( work, def );
	InitRndDropData( work->drop_data, N_DROPS );

	work->def = def ;
	
	work->xbox_packet_size = 30000;/* Actを回して、このサイズを下回ることがないのを確認 */
	if ( !(work->packet_mem = GV_Malloc( work->xbox_packet_size )) ) {
			printf("ERR!! MALLOC scr_waterfilm.c!!\n");
			return -1 ;
		}
	dmapack->autopacket = work->packet_mem ;
	DG_SetDmapackEnd( dmapack->autopacket ) ;

	work->interval = 120;
	return (0);
}

void *NewScrWaterFilm( int name, int map )
{
	Work		*work ;

	//return (void*)1;
//	if(g_work){
//		return g_work;
//	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
//		g_work = work;
	}

	return (void *)work ;
}
