//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_plasma_line.c
	二点を結ぶプラズマ（ライン）
	2000/03/23 S.Okajima
	$Id: demo_plasma_color.c,v 1.1.1.3 2002/11/19 11:44:36 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../../okajima/etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define N_VERTS	(32)
#define	N_PRIMS	(8)
#define N_NODES	(N_VERTS*N_PRIMS)

#define	P_ALPHA_MAX		(128)
#define	P_RGB_R_MAX		(48)
#define	P_RGB_G_MAX		(192)
#define	P_RGB_B_MAX		(96)

/* １サイクル */
#define	ALPHA_COUNT_MIN		(16)
#define	ALPHA_COUNT_RND		(16)

#define	SIZE			(50)
//#define	SIZE			(80)

#define	RUNNING_STEP	(10.0f)
#define	LIMIT_RADIUS	(1000.0f)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		real_pos1;
	FVECTOR		running;
	float		radius;
	int			life;
	int			flag;
	int			alpha_count;
	int			init_flag;

    int			max_alpha;

    CVECTOR		color;
    
	short		master_rot_vz;
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
static void CalcNextNode( FVECTOR *vec, int node_num0, int node_num1 )
{
	FVECTOR	*fv0;
	int	node_num;

	node_num = node_num1 - node_num0;
	if( node_num <= 1 ) return;
	node_num = node_num0 + node_num/2;

#if 0
	fv0  = fv1 = fv2 = vec;
	fv0 += node_num0;
	fv1 += node_num1;
	fv2 += node_num;

	fv0->vw = fv1->vy - fv0->vy;
	fv1->vw = fv1->vz - fv0->vz;
	fv2->vw = fv1->vw * 0.25f * frnd();		/* len */

	th = asinf( fv0->vw / fv1->vw );
	th = (float)(((int)(th*4096.0f/TPI))&4095)*TPI/4096.0f;
	fv2->vy = (fv0->vy + fv0->vw*0.5f + fv2->vw * cosf(th))*0.1f  +  fv2->vy*0.9f;
	fv2->vz =  fv0->vz + fv1->vw*0.5f - fv2->vw * sinf(th);
#else
	fv0 = vec;
	fv0 += node_num;
	fv0->vx += fv0->vx * 0.01f * rnd();
	fv0->vy += fv0->vy * 0.05f * rnd();
#endif

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
	FVECTOR	fvtemp;
	SVECTOR master_rot;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float	len;
	int		clock;
	int		i,k;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	/* Ｚ軸方向にマスターノード初期化 */
	node = work->node;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;

	/* 終点を更新 */
	_sceVu0AddVector( &work->real_pos1, &work->real_pos1, &work->running );
	_sceVu0SubVector( &fvtemp, &work->real_pos1, work->pos1 );
	len = GV_VecLen3F( &fvtemp );
	if( irnd()%32==0 || len > LIMIT_RADIUS || work->flag==0){
		work->init_flag = 1;
		work->alpha_count = ALPHA_COUNT_MIN + irnd()%ALPHA_COUNT_RND;
		work->master_rot_vz = irnd()%4096;
		work->running.vx = RUNNING_STEP*frnd();
		work->running.vy = RUNNING_STEP*frnd();
		work->running.vz = RUNNING_STEP*frnd();
		master_rot.vx = irnd()%4096;
		master_rot.vy = irnd()%2048;
		master_rot.vz = 0;
		fvtemp.vx = 0.0f;
		fvtemp.vy = 0.0f;
		fvtemp.vz = work->radius;
		DG_SetPos2( work->pos1, &master_rot );
		DG_PutVector( &fvtemp, &work->real_pos1, 1 );

		work->flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
			GM_GetHzxGroupID( GM_CurrentStageMap ),
			work->pos0,
			&work->real_pos1,
			HZX_CHK_FIX,
			HZX_SEG_NO_PLAYER,
			HZX_FLOOR_NO_PLAYER );
		if( work->flag!=0 ) HZX_GetOnlinePoint( &work->real_pos1 );

		_sceVu0SubVector( &fvtemp, &work->real_pos1, work->pos0 );
		len = GV_VecLen3F( &fvtemp );
		if(len < 10.0f) len = 10.0f;
		node[ N_NODES ].vz = len;
		CalcInitNode( node, 0, N_NODES );

	}else{
		work->init_flag = 0;
		work->flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
			GM_GetHzxGroupID( GM_CurrentStageMap ),
			work->pos0,
			&work->real_pos1,
			HZX_CHK_FIX,
			HZX_SEG_NO_PLAYER,
			HZX_FLOOR_NO_PLAYER );
		if( work->flag!=0 ) HZX_GetOnlinePoint( &work->real_pos1 );

		_sceVu0SubVector( &fvtemp, &work->real_pos1, work->pos0 );
		len = GV_VecLen3F( &fvtemp );
		if(len < 10.0f) len = 10.0f;
		node[ N_NODES ].vz = len;
		if(irnd()%32==0  ||  work->alpha_count<0 ){
			work->alpha_count = ALPHA_COUNT_MIN + irnd()%ALPHA_COUNT_RND;
			CalcInitNode( node, 0, N_NODES );
		}else{
			work->alpha_count--;
			CalcNextNode( node, 0, N_NODES );
			CalcSmoothNode( node, N_NODES );
		}
	}

	OK_DirVecXY( work->pos0, &work->real_pos1, &master_rot );
	work->master_rot_vz += 4;
	master_rot.vz = work->master_rot_vz;
	DG_SetPos2( work->pos0, &master_rot );
	DG_PutVector( node, work->prim->pos[ clock ], N_NODES );

	uvrgbwh = work->prim->uvrgb[ clock ] ;
	if( work->init_flag != 0 ){
		DG_SetTmpLight2(
			&work->real_pos1,
			500.0f,
			1000.0f,
			*(int *)&work->color,
			LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		for ( i=N_PRIMS*N_VERTS; i>0; i-- ){
			uvrgbwh->a = work->max_alpha * i / (N_PRIMS*N_VERTS);
			uvrgbwh++;
		}
	}else if( work->alpha_count > ALPHA_COUNT_MIN ){
		for ( i=N_PRIMS*N_VERTS; i>0; i-- ){
			uvrgbwh->a = work->max_alpha;
			uvrgbwh++;
		}
	}else{
		k = work->max_alpha * work->alpha_count / ALPHA_COUNT_MIN;
		for ( i=N_PRIMS*N_VERTS; i>0; i-- ){
			uvrgbwh->a = k;
			uvrgbwh++;
		}
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
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int size, int color )
{
	FVECTOR				*pos_0 ;
	FVECTOR				*pos_1 ;
	DG_PRIM2_UVRGBWH	*uvrgbwh_0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	work->color.r = ( (color>>24) & 0xff );
	work->color.g = ( (color>>16) & 0xff );
	work->color.b = ( (color>>8) & 0xff );
	work->max_alpha = ( color & 0xff );
	
	
	//-------------------------------
	pos_0  = prim->pos[ 0 ] ;
	pos_1  = prim->pos[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos_0, work->pos0 );
			DG_COPY_VEC( pos_1, work->pos0 );
			pos_0++ ;
			pos_1++ ;
		}
	}

	//-------------------------------
	uvrgbwh_0 = prim->uvrgb[ 0 ] ;
	uvrgbwh_1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
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
			uvrgbwh_0->r  = uvrgbwh_1->r  = work->color.r ;
			uvrgbwh_0->g  = uvrgbwh_1->g  = work->color.g ;
			uvrgbwh_0->b  = uvrgbwh_1->b  = work->color.b ;
			uvrgbwh_0->a  = uvrgbwh_1->a  = work->max_alpha ;
			uvrgbwh_0++ ;
			uvrgbwh_1++ ;
		}
	}
}

static int GetResources( Work *work, FVECTOR *pos0, FVECTOR *pos1, int radius, int life, int size, int color )
{
	FVECTOR	fvtemp;
	float	len;
	FVECTOR	*node;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->pos0   = pos0;
	work->pos1   = pos1;

	work->radius = (float)radius;
	work->life   = life;
	DG_COPY_VEC( &work->real_pos1, work->pos1 );
	work->flag   = 0;
	work->alpha_count = 0;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	InitPacket( work, prim, tex, size, color );
	DG_VisiblePrim2( work->prim );

	/* Ｚ軸方向にマスターノード作成 */
	_sceVu0SubVector( &fvtemp, &work->real_pos1, work->pos0 );
	len = GV_VecLen3F( &fvtemp );

	node = work->node;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;
	node[ N_NODES ].vz = len;
	CalcInitNode( node, 0, N_NODES );

	return (0);
}

/* ---------------------------------------------------------------- */
/*
*pos0 :[ポインタ参照する]発生点
*pos1 :[ポインタ参照する]
radius:pos1を中心とした球の半径（この球の内部の点を終点とする）０以上
life  :寿命。フレーム指定。０以上。
*/
void *NewDemoPlasmaLineColor( int con_name0, int con_name1, int radius, int life, int size, int color )
{
	Work		*work ;
	EFTCONTROL  *eft_ctrl0 ;
	EFTCONTROL  *eft_ctrl1 ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		eft_ctrl0 = DM_GetEftControl( con_name0 );
		eft_ctrl1 = DM_GetEftControl( con_name1 );
		if ( GetResources( work, &eft_ctrl0->mov, &eft_ctrl1->mov, radius, life, size, color ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

