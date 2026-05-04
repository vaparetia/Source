//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cv2_view.c
	CV2頂点・法線を視覚化

	2001/12/03 S.Okajima
	$Id: cv2_view.c,v 1.1.1.3 2002/11/19 11:47:04 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"


#define	JOINT_NUM		(22)

#define	POS_VS_UVS	(1 + 2)

#define	DIV_VERTS		(1)
#define	DIV_NORMS		(1)
#define	SCR_DIVISION	( DIV_VERTS + DIV_NORMS )

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2		(32)
//#define	N_VERTS2		(16)
//関節毎にスクラッチパット使用
#define	N_PRIMS2		( JOINT_NUM )


#define	PARAM1		( (MAX_FVECTOR - N_VERTS2*POS_VS_UVS) / SCR_DIVISION )
#define	PARAM2		(PARAM1 * 16)

#if 0 //BP
//#ifdef PSX2
#define	SCR_POS		((void *)SCRPAD_ADDR)
#else
#define	SCR_POS		((char *)SCRPAD_ADDR)
#endif

#define	SCR_UVS		(SCR_POS   + N_VERTS2 * 16)
#define	SCR_VERTS	(SCR_UVS   + N_VERTS2 * 16 * 2)
#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)


#if 0
#define	TEX_NAME		(7733153 /*"splash06_alp"*/)
#define	RAND_WIDTH		(80.0f)
//#define	SCALE			(0.95f)
#define	SCALE			(1.1f)
#define	P_ALPHA_MAX		(24)
#define	P_RGB_MAX		(128)
#define	SIZE			(50)
#else
#define	TEX_NAME		(6715088 /*"rcm_l_msk"*/)
#define	RAND_WIDTH		(0.0f)
#define	SCALE			(1.00f)
#define	P_ALPHA_MAX		(128)
#define	P_RGB_MAX		(128)
#define	SIZE			(8)
#endif



#define	SHIFT_ROT		(256.0f)

#define	NORMS_LIMIT		(0.7f)
//#define	NORMS_LIMIT		(0.5f)
#define	LENGTH_LIMIT	(8000.0f)
#define	LENGTH_MIN		(5000.0f)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;
	int			map;

	CONTROL		*control;
	DG_OBJS		*objs;

	int			n_models;

	DG_PRIM2	*prim ;
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		*norms[JOINT_NUM];
	int			n_verts[JOINT_NUM];

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int i, n;
	int	max_num;
	int	clock;
	FVECTOR	*buff;
	FVECTOR *from;
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		*n_verts;
	FVECTOR	**verts;
	FVECTOR	**norms;
	FMATRIX	**world;
	CONTROL		*ctrl;

//	DG_InvisibleObjs( GM_PlayerBody->objs );

	DG_VisiblePrim2( work->prim );

	ctrl = work->control;

	if( GM_CheckPlayerStatus(PLAYER_CB_BOX) ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	n_verts  = work->n_verts;
	pos      = work->prim->pos[ clock ];
	uvrgbwh  = work->prim->uvrgb[ clock ];
	world    = work->world;
	verts    = work->verts;
	norms    = work->norms;
	i = work->n_models ; 
	while( --i>=0 ){
		OK_Mem_Scr( SCR_VERTS, *verts,  sizeof(FVECTOR), *n_verts) ;
		OK_Mem_Scr( SCR_NORMS, *norms,  sizeof(FVECTOR), *n_verts) ;

		DG_SetPos( *world );
		DG_RotVector( SCR_NORMS, SCR_NORMS, *n_verts );

		from = SCR_VERTS;
		buff = SCR_NORMS;
		n    = *n_verts;

		max_num = N_VERTS2;

		while ( -- n >= 0 ) {
			DG_PutVector( from, pos, 1 );
			uvrgbwh->a = P_ALPHA_MAX;
			uvrgbwh++;
			pos++;
			if( --max_num <= 0 ) break;
			buff ++ ;
			from ++ ;
		}

		while ( --max_num >= 0 ) {
			uvrgbwh->a = 0;
			uvrgbwh++;
			pos++;		/* skip させる */
		}

		n_verts++;
		world++;
		verts++;
		norms++;
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	pos  = (FVECTOR *)SCRPAD_ADDR ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f * (float)k;
			pos->vy = 0.0f * (float)j;
			pos->vz = 0.0f ;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
		}
	}
	OK_Scr_Mem( prim->pos[ 0 ], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS2 * work->n_models ) ;
	OK_Scr_Mem( prim->pos[ 1 ], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS2 * work->n_models ) ;


	//-------------------------------
	uvrgbwh = prim->uvrgb[ 0 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = SIZE ;
			uvrgbwh->h = SIZE ;

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh ++ ;
		}
	}
	uvrgbwh = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = SIZE ;
			uvrgbwh->h = SIZE ;

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh ++ ;
		}
	}
}


static int GetResources( Work *work, DG_OBJS *org_objs, CONTROL *control, int model_id )
{
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i ;

	work->objs=org_objs;
	work->control = control;

	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 


	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_models, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( TEX_NAME );

	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( work->prim );

	/* モデルの共有頂点データ取得 */
	cvd_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );
//	cvd_def = GV_GetCache( GV_CacheID( 3075579 /*"sna_skl3"*/, 'c' ) );
//	cvd_def = GV_GetCache( GV_CacheID( 13488099 /*"crg_raindammy"*/, 'c' ) );

//printf("org_objs->n_models:%d\n",org_objs->n_models);

	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > PARAM1 ){
//			printf("%d::%d\n",work->n_verts[i],PARAM1 );
			work->n_verts[i] = PARAM1;
		}
		work->verts[i]   = cvd_def->models[i].verts;
		work->norms[i]   = cvd_def->models[i].norms;
		work->world[i]   = &org_objs->objs[i].world;
	}

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewCV2_View( DG_OBJS *objs, CONTROL *control, int model_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, objs, control, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
