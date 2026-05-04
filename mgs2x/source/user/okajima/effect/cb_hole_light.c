//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cb_hole_light.c
	霧雨（透視変換バージョン）

	2000/10/12 S.Okajima
	$Id: cb_hole_light.c,v 1.1.1.3 2002/11/19 11:47:03 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
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
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"lit_man.h"
#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern int big_thund_flash_flag ;

/*----------------------------------------------------------------*/
#define	SCREEN_NEAR		( 51.0f )

/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_PRIMS2		(2)
#define	N_VERTS2		(32)

#define	P_SIZE_MIN		(10.0f)
#define	P_SIZE_MAX		(50.0f - P_SIZE_MIN)
#define	P_ALPHA_MAX		(48.0f)
#define	P_RGB_MAX		(48)

#define	LIGHT_LENGTH	(300.0f)
#define	EFFECT_LEN_MAX	(4000.0f)

static FVECTOR LineVec[N_PRIMS2*N_VERTS2];

typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		*world;

	int			sw[MAX_LIGHT_NUM];
	float		len[MAX_LIGHT_NUM];
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	CalcLen( Work *work, FVECTOR *center )
{
	static FVECTOR width={ EFFECT_LEN_MAX, EFFECT_LEN_MAX, EFFECT_LEN_MAX, 0.0f };
	int	i;
	int	*sw;
	FVECTOR	pos;
	float	*len;
	FVECTOR	bound1;
	FVECTOR	bound2;

	_sceVu0SubVector( &bound1, center, &width );
	_sceVu0AddVector( &bound2, center, &width );

	len      = work->len;
	sw       = work->sw;
	for( i=0; i<ok_lit_data_num; i++ ){
		if( ((*sw) = vu0_CheckBoundingBox( ok_lit_pos_sorted[i], &bound1, &bound2 ))==1 ){
			_sceVu0SubVector( &pos, center, ok_lit_pos_sorted[i] );
			(*len) = GV_VecLen3F( &pos );
		}
		len++;
		sw++;
	}
}


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i,j;
	int	min_que[4];
	int	itemp;
	int	alpha;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	center;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	SVECTOR	rot;
	int	*sw;
	float	min;
	float	inner;
	float	*len;

	if( GM_CheckPlayerStatus(PLAYER_CB_BOX) && GM_CheckPlayerStatus(PLAYER_WATCH) ){
		DG_VisiblePrim2( work->prim );
	}else{
		DG_InvisiblePrim2( work->prim );
		return;
	}

	DG_COPY_VEC( &center, (FVECTOR *)work->world->m[3] );


//	GM_GroupPrim2( work->prim, GM_PlayerMap ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;


	/* 個々の距離を求める（work->len[] に 入る） */
	CalcLen( work, &center );

	/* 最も近い光源から得たデータを使用しセットする */
	min = EFFECT_LEN_MAX;
	len = work->len;
	min_que[0] = -1;
	min_que[1] = -1;
	min_que[2] = -1;
	min_que[3] = -1;
	sw    = work->sw;
	for( j=0; j<ok_lit_data_num; j++ ){
		if( *sw ){
			if( *len < min ){
				min    = *len;
				min_que[3] = min_que[2];
				min_que[2] = min_que[1];
				min_que[1] = min_que[0];
				min_que[0] = j;
			}
		}
		len++;
		sw++;
	}

	for( i=0; i<4; i++ ){
		itemp = min_que[i];
		if(itemp != -1){
			fvtemp0.vx = 0.0f;
			fvtemp0.vy = 0.0f;
			fvtemp0.vz = 1.0f;
			fvtemp0.vw = 0.0f;
			fvtemp1.vx = 0.0f;
			fvtemp1.vy = 0.0f;
			fvtemp1.vz =-1.0f;
			fvtemp1.vw = 0.0f;
			DG_SetPos( work->world );
			DG_RotVector( &fvtemp0, &fvtemp0, 1 );

			OK_DirVecXY( ok_lit_pos_sorted[itemp], &center, &rot );
			DG_SetPos2( &center, &rot );
			DG_PutVector( LineVec, work->prim->pos[ clock ], N_PRIMS2*N_VERTS2 );
			DG_RotVector( &fvtemp1, &fvtemp1, 1 );
			inner = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
			if( inner > 0.0f ){
				min = EFFECT_LEN_MAX - work->len[itemp];
				alpha    = (int)(P_ALPHA_MAX * min * inner / EFFECT_LEN_MAX);

				uvrgbwh = work->prim->uvrgb[ clock ];
				for ( i = 0 ; i < N_PRIMS2*N_VERTS2 ; i++ ){
					uvrgbwh->a = alpha;
					uvrgbwh->h = uvrgbwh->w = (int)(P_SIZE_MIN + P_SIZE_MAX*(float)i/(N_PRIMS2*N_VERTS2));
					uvrgbwh++;
				}
				DG_VisiblePrim2( work->prim );
				break;
			}else{
				DG_InvisiblePrim2( work->prim );
			}
		}else{
			DG_InvisiblePrim2( work->prim );
			break;
		}
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			uvrgbwh1->w = uvrgbwh0->w = 100 ;
			uvrgbwh1->h = uvrgbwh0->h = 100 ;

			uvrgbwh1->r = uvrgbwh0->r = P_RGB_MAX ;
			uvrgbwh1->g = uvrgbwh0->g = P_RGB_MAX ;
			uvrgbwh1->b = uvrgbwh0->b = P_RGB_MAX ;
			uvrgbwh1->a = uvrgbwh0->a = 0 ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			i;

	for( i=0; i<N_PRIMS2*N_VERTS2; i++ ){
		LineVec[i].vx = 0.0f;
		LineVec[i].vy = 0.0f;
		LineVec[i].vz = (float)i * LIGHT_LENGTH/(N_PRIMS2*N_VERTS2);
	}
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

//	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS2, N_VERTS2 );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS2, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewCB_HoleLight( FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
