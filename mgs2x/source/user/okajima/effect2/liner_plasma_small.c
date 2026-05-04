//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	liner_plasma_small.c
	二点を結ぶプラズマ（リニアガン用、軽い）
	2001/04/17 S.Okajima
	$Id: liner_plasma_small.c,v 1.1.1.3 2002/11/19 11:47:22 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_LENGTH		( 0x4000 )
#define	VEC_LENGTH		( SCR_LENGTH / 16    /   16 )
#define N_VERTS	(32)
#define N_NODES	(VEC_LENGTH-N_VERTS)	/* max */
#define	N_PRIMS	(N_NODES/N_VERTS)

#define	SKIP	(N_VERTS*4)
//#define	SKIP	(N_VERTS)



#define	P_RGB_R_MAX		(64)
#define	P_RGB_G_MAX		(64)
#define	P_RGB_B_MAX		(255)
#define	P_ALPHA_MAX		(255)

#define	FOG_R_MAX		(96)
#define	FOG_G_MAX		(96)
#define	FOG_B_MAX		(255)
#define	FOG_LIFE0		(2)
#define	FOG_LIFE1		(16)

#define	SIZE_MIN		(30)
#define	SIZE_RND		(30)

#define	WIDE_RATIO			(1.01f)
#define	WIDE_RATIO_SPREAD	(1.07f)

#define	INFLUENCE_RADIUS	(2000.0f)


extern CVECTOR OK_FogColorBase;
extern float   OK_FogNear;
extern float   OK_FogFar;


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	float		height;

	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*pos2;
	float		len;
	int			life;
	int			life_max;

	int			width;
	float		*side_ratio;

	FMATRIX		world;

	FVECTOR		node[N_NODES+1];
	DG_PRIM2	*prim ;
} Work ;


/* ---------------------------------------------------------------- */
static void CalcInitNode( FVECTOR *vec, int node_num0, int node_num1 )
{
	FVECTOR	*fv0;
	FVECTOR	*fv1;
	FVECTOR	*fv2;
	int	node_num;
	float	th;

	node_num = node_num1 - node_num0;
	if( node_num <= 1 ) return;
	node_num = node_num0 + node_num/2;

	fv0  = fv1 = fv2 = vec;
	fv0 += node_num0;
	fv1 += node_num1;
	fv2 += node_num;

	fv0->vw = fv1->vy - fv0->vy;
	fv1->vw = fv1->vz - fv0->vz;
	fv2->vw = fv1->vw * 0.25f * frnd();		/* len */

	th = asinf( fv0->vw / fv1->vw );
	th = (float)(((int)(th*4096.0f/TPI))&4095)*TPI/4096.0f;
	fv2->vx = fv0->vx + (fv1->vx - fv0->vx)*0.5f + fv2->vw * frnd();
	fv2->vy = fv0->vy + fv0->vw*0.5f + fv2->vw * cosf(th);
	fv2->vz = fv0->vz + fv1->vw*0.5f - fv2->vw * sinf(th);

	CalcInitNode( vec, node_num0, node_num  );
	CalcInitNode( vec, node_num,  node_num1 );
}

/* ---------------------------------------------------------------- */
static void CalcWorld( Work *work )
{
	SVECTOR	rot;

	OK_DirVecXY( work->pos0, work->pos1, &rot );
	DG_SetPos2( work->pos0, &rot );
	DG_GetPos(  &work->world );
}

/* ---------------------------------------------------------------- */
static void CalcNextNodeSpread( FVECTOR *vec, int node_num0, int node_num1 )
{
	FVECTOR	*fv0;
	int	node_num;

	node_num = node_num1 - node_num0;
	if( node_num <= 1 ) return;
	node_num = node_num0 + node_num/2;

	fv0 = vec;
	fv0 += node_num;
	fv0->vx *= WIDE_RATIO_SPREAD;
	fv0->vy *= WIDE_RATIO_SPREAD;
	CalcNextNodeSpread( vec, node_num0, node_num  );
	CalcNextNodeSpread( vec, node_num,  node_num1 );
}

/* ---------------------------------------------------------------- */
static void CalcNextNode( FVECTOR *vec, int node_num0, int node_num1 )
{
	FVECTOR	*fv0;
	int	node_num;

	node_num = node_num1 - node_num0;
	if( node_num <= 1 ) return;
	node_num = node_num0 + node_num/2;

	fv0 = vec;
	fv0 += node_num;
	fv0->vx *= WIDE_RATIO;
	fv0->vy *= WIDE_RATIO;
	CalcNextNode( vec, node_num0, node_num  );
	CalcNextNode( vec, node_num,  node_num1 );
}

/* ---------------------------------------------------------------- */
static void CalcSmoothNode( FVECTOR *vec, int node_num )
{
	FVECTOR	*fv0;
	FVECTOR	*fv1;
	FVECTOR	*fv2;

	fv0 = fv1 = fv2 = vec;
	fv1+= 2;
	fv2++;
	node_num -= 2;
	while( --node_num > 0){
		fv2->vx = (fv0->vx + fv1->vx) * 0.5f;
		fv2->vy = (fv0->vy + fv1->vy) * 0.5f;
		fv0++;
		fv1++;
		fv2++;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	*node;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int		clock;
	float	ratio;
	int		i,k;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_VisiblePrim2( work->prim );

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	/* Ｚ軸方向にマスターノード初期化 */
	OK_Mem_Scr( SCR_POS, work->node, sizeof(FVECTOR), N_NODES+1 );
	node = SCR_POS;

	if( (*work->side_ratio) < 0.95f ){
		CalcNextNode( node, 0, N_NODES );
	}else{
		CalcNextNodeSpread( node, 0, N_NODES );
	}
	CalcSmoothNode( node, N_NODES );

	CalcWorld( work );

	DG_SetPos( &work->world );
	DG_PutVector( node, work->prim->pos[ clock ], N_NODES );
	OK_Scr_Mem( work->node, SCR_POS, sizeof(FVECTOR), N_NODES+1 );

#if 0
	if( work->life > work->life_max/4 ){
		ratio = 1.0f;
	}else{
		ratio = (float)(work->life) / (float)(work->life_max/4);
	}
#else
	ratio = (float)(work->life) / (float)(work->life_max);
#endif

	ratio*= ratio;
	k     = (int)((float)(P_ALPHA_MAX) * ratio);
	uvrgbwh = work->prim->uvrgb[ clock ] ;
	for ( i=N_NODES; i>0; i-- ){
		uvrgbwh->a = k;
		uvrgbwh++;
	}

	if( work->life > 0 ){
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );

}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh_0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_1 ;	/* スプライト用 */
	int		j ;
	int		size;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	size = work->width;
	//-------------------------------
	uvrgbwh_0 = prim->uvrgb[ 0 ] ;
	uvrgbwh_1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_NODES ; j++ ){
		uvrgbwh_0->u0 = uvrgbwh_1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh_0->v0 = uvrgbwh_1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh_0->u1 = uvrgbwh_1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh_0->v1 = uvrgbwh_1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh_0->q0 = uvrgbwh_1->q0 = 4096 ;
		uvrgbwh_0->q1 = uvrgbwh_1->q1 = 4096 ;
		uvrgbwh_0->f0 = uvrgbwh_1->f0 = 0x0fff ;
		uvrgbwh_0->f1 = uvrgbwh_1->f1 = 0x0fff ;
		uvrgbwh_0->w  = uvrgbwh_1->w  = size;
		uvrgbwh_0->h  = uvrgbwh_1->h  = size;
		uvrgbwh_0->r  = uvrgbwh_1->r  = P_RGB_R_MAX ;
		uvrgbwh_0->g  = uvrgbwh_1->g  = P_RGB_G_MAX ;
		uvrgbwh_0->b  = uvrgbwh_1->b  = P_RGB_B_MAX ;
		uvrgbwh_0->a  = uvrgbwh_1->a  = P_ALPHA_MAX ;
		uvrgbwh_0++ ;
		uvrgbwh_1++ ;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	FVECTOR	fvtemp;
	FVECTOR	*node;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	float		len;
	CVECTOR		col;

	CalcWorld( work );

	/* Ｚ軸方向にマスターノード作成 */
	_sceVu0SubVector( &fvtemp, work->pos1, work->pos0 ) ;
	work->len = GV_VecLen3F( &fvtemp );
	if(work->len < 1.0f) work->len = 1.0f;

	node = SCR_POS;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;
	node[ N_NODES ].vz = work->len;
	CalcInitNode( node, 0, N_NODES );
	OK_Scr_Mem( work->node, SCR_POS, sizeof(FVECTOR), N_NODES+1 );


	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 13297023 /*"xlit02a"*/ );

	InitPacket( work, prim, tex );
	DG_InvisiblePrim2( work->prim );



	fvtemp.vx = (work->pos0->vx + work->pos1->vx) * 0.5f;
	fvtemp.vy = (work->pos0->vy + work->pos1->vy) * 0.5f;
	fvtemp.vz = (work->pos0->vz + work->pos1->vz) * 0.5f;
	_sceVu0SubVector( &fvtemp, &fvtemp, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	len = GV_VecLen3F( &fvtemp );

	col.r = P_RGB_R_MAX;
	col.g = P_RGB_G_MAX;
	col.b = P_RGB_B_MAX;
	col.cd= P_ALPHA_MAX;


//	OK_TotalNum++;

	OK_Mem_Scr( SCR_POS, work->node, sizeof(FVECTOR), N_NODES+1 );
	node = SCR_POS;
	CalcNextNode( node, 0, N_NODES );
	CalcSmoothNode( node, N_NODES );

	DG_SetPos( &work->world );
	OK_Scr_Mem( work->node, SCR_POS, sizeof(FVECTOR), N_NODES+1 );
	DG_PutVector( node, node, N_NODES+1 );
	DG_PutVector( node, work->prim->pos[ 0 ], N_NODES );
	DG_PutVector( node, work->prim->pos[ 1 ], N_NODES );

	return (0);
}


/* ---------------------------------------------------------------- */
/*
*pos0 :[ポインタ参照する]端点Ａ
*pos1 :[ポインタ参照する]端点Ｂ
*pos2 :[ポインタ参照する]中継点
width :この値を幅とする
life  :寿命。フレーム指定。０以上。
*/
void *NewLinerPlasmaMini( FVECTOR *pos0, FVECTOR *pos1, FVECTOR *pos2, int width, int life, float *ratio )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pos0       = pos0;
		work->pos1       = pos1;
		work->pos2       = pos2;
		work->width      = width;
		work->life_max   = work->life = life;
		work->side_ratio = ratio;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

