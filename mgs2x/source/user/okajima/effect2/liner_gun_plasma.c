//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	liner_gun_plasma.c
	二点を結ぶプラズマ（画面クリップありの点をスプライトで表示）
	2001/02/01 S.Okajima
	$Id: liner_gun_plasma.c,v 1.1.1.3 2002/11/19 11:47:21 Yoshizawa1 Exp $
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
#include	"../etc/ok_util.h"

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_LENGTH		( 0x4000 )
#define	VEC_LENGTH		( SCR_LENGTH / 16 )
#define N_VERTS	(32)
//#define N_NODES	(VEC_LENGTH-N_VERTS)	/* max */
#define N_NODES_D	(VEC_LENGTH/4-N_VERTS)	/* max */
#define N_NODES_G	(VEC_LENGTH  -N_VERTS)	/* max */
#define	N_PRIMS_D	(N_NODES_D/N_VERTS)
#define	N_PRIMS_G	(N_NODES_G/N_VERTS)

#define	SKIP	(N_VERTS*4)
//#define	SKIP	(N_VERTS)


#define	P_RGB_R_MAX		(16)
#define	P_RGB_G_MAX		(32)
#define	P_RGB_B_MAX		(48)
#define	P_ALPHA_MAX		(128)

#define	FOG_R_MAX		(76)
#define	FOG_G_MAX		(108)
#define	FOG_B_MAX		(176)

#define	FOG_LIFE0		(2)
#define	FOG_LIFE1		(180)

#define	FOG_LIFE_INTERVAL	(8)

#define	SIZE_MIN		(50)
#define	SIZE_RND		(30)

//#define	WIDE_RATIO		(1.01f)
#define	WIDE_RATIO		(0.99f)

#define	INFLUENCE_RADIUS	(2000.0f)


extern CVECTOR OK_FogColorBase;
extern float   OK_FogNear;
extern float   OK_FogFar;


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	float		height;

	FVECTOR		pos0;
	FVECTOR		pos1;
	float		len;
	int			life;
	int			life_max;

	int			node_num;
	int			prim_num;

	int			fog_flag;

	FMATRIX		world;

	DG_PRIM2	*prim ;
//	FVECTOR		node[N_NODES+1];
	FVECTOR		node[0];
} Work ;


static int   OK_TotalNum    = 0;
extern void *NewFogSet_Demo( int col_r, int col_g, int col_b, float near, float far, int time );

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
//	fv0->vx += fv0->vx * 0.01f * rnd();
//	fv0->vy += fv0->vy * 0.01f * rnd();
	fv0->vx *= WIDE_RATIO;
	fv0->vy *= WIDE_RATIO;
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
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int		clock;
	float	ratio;
	int		i,k;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	/* Ｚ軸方向にマスターノード初期化 */
	OK_Mem_Scr( SCR_POS, work->node, sizeof(FVECTOR), work->node_num+1 );
	node = SCR_POS;
/*
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;
	node[ N_NODES ].vz = work->len;
*/
	CalcNextNode( node, 0, work->node_num );
	CalcSmoothNode( node, work->node_num );

	DG_SetPos( &work->world );
	DG_PutVector( node, work->prim->pos[ clock ], work->node_num );
	OK_Scr_Mem( work->node, SCR_POS, sizeof(FVECTOR), work->node_num+1 );

	ratio = (float)work->life / (float)work->life_max;
	ratio*= ratio;
	k     = (int)((float)(P_ALPHA_MAX) * ratio);
	uvrgbwh = work->prim->uvrgb[ clock ] ;
	for ( i=work->node_num; i>0; i-- ){
		uvrgbwh->a = k * (work->node_num-i)/work->node_num;
//		uvrgbwh->a = k;
		uvrgbwh++;
	}

	if( work->life > 0 ){
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}


	if( work->life_max - work->life > FOG_LIFE_INTERVAL ){
		if( work->fog_flag == 0 ){
			work->fog_flag = 1;
			NewFogSet_Demo(
				OK_FogColorBase.r,
				OK_FogColorBase.g,
				OK_FogColorBase.b,
				OK_FogNear,
				OK_FogFar,
				FOG_LIFE1 );
		}
	}



}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );

	OK_TotalNum--;
	if( OK_TotalNum <= 0 ){
		OK_TotalNum = 0;
	}
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

	size = SIZE_MIN + (irnd()>>8)%SIZE_RND;
	//-------------------------------
	uvrgbwh_0 = prim->uvrgb[ 0 ] ;
	uvrgbwh_1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->node_num ; j++ ){
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
		uvrgbwh_0->a  = uvrgbwh_1->a  = 0 ;
		uvrgbwh_0++ ;
		uvrgbwh_1++ ;
	}
}

/*----------------------------------------------------------------*/
static int GetHeight( Work *work )
{
	HZX_FLR		flr[2];
	int			flr_atrs[2];
	int			floor_flag;

	floor_flag = HZX_LevelHazardCheck(
//	                GM_GetHzxGroupID( work->map ),
	                GM_GetHzxGroupID( GM_CurrentStageMap ),
	                &work->pos0,
	                HZX_CHK_F_FLOOR,
	                HZX_FLOOR_NO_PLAYER );
	if( floor_flag & 1 ){
		HZX_GetLevelHazard( flr, flr_atrs );
#if 1 //yano 2001.02.26
		work->height = HZX_GetFloorLevel();
#else
		HZX_GetLevelHeight( &work->height ); /*下のメモリを破壊する可能性があるので、変更しました。*/
#endif
	}else{
		return 0;
	}
	return 1;
}


static int GetResources( Work *work )
{
	FVECTOR	fvtemp;
	FVECTOR	*node;
	SVECTOR	rot;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	float		len;
	int			i;
	CVECTOR		col;

	work->fog_flag = 0;

	OK_DirVecXY( &work->pos0, &work->pos1, &rot );
	rot.vz = (irnd()>>8)&4095;
	DG_SetPos2(  &work->pos0, &rot );
	DG_GetPos(   &work->world );



	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->prim_num, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 13297023 /*"xlit02a"*/ );

	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim );

	/* Ｚ軸方向にマスターノード作成 */
	_sceVu0SubVector( &fvtemp, &work->pos1, &work->pos0 ) ;
	work->len = GV_VecLen3F( &fvtemp );
	if(work->len < 10.0f) work->len = 10.0f;

	node = SCR_POS;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ work->node_num ].vx = 0.0f;
	node[ work->node_num ].vy = 0.0f;
	node[ work->node_num ].vz = work->len;
	CalcInitNode( node, 0, work->node_num );
	OK_Scr_Mem( work->node, SCR_POS, sizeof(FVECTOR), work->node_num+1 );


	fvtemp.vx = (work->pos0.vx + work->pos1.vx) * 0.5f;
	fvtemp.vy = (work->pos0.vy + work->pos1.vy) * 0.5f;
	fvtemp.vz = (work->pos0.vz + work->pos1.vz) * 0.5f;
	_sceVu0SubVector( &fvtemp, &fvtemp, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	len = GV_VecLen3F( &fvtemp );

	if( OK_TotalNum <= 0){
		NewFogSet_Demo(
			FOG_R_MAX,
			FOG_G_MAX,
			FOG_B_MAX,
			0,
			len*2.0f,
			FOG_LIFE0 );
	}

	col.r = P_RGB_R_MAX;
	col.g = P_RGB_G_MAX;
	col.b = P_RGB_B_MAX;
	col.cd= P_ALPHA_MAX/2;

	OK_Mem_Scr( SCR_POS, work->node, sizeof(FVECTOR), work->node_num+1 );
	node = SCR_POS;
	CalcNextNode( node, 0, work->node_num );
	CalcSmoothNode( node, work->node_num );

	DG_SetPos( &work->world );
	OK_Scr_Mem( work->node, SCR_POS, sizeof(FVECTOR), work->node_num+1 );
	DG_PutVector( node, node, work->node_num+1 );
	DG_PutVector( node, work->prim->pos[ 0 ], work->node_num );
	DG_PutVector( node, work->prim->pos[ 1 ], work->node_num );

	if( GetHeight( work ) ){ // 床の照り返し
		i = (irnd()>>8)%SKIP;
		node += i;
		i = work->node_num / SKIP;
		while( --i > 0){
//printf("H:%f %f  %f \n",node->vy, work->height, INFLUENCE_RADIUS);
			OK_PutFloorLight( node, work->height, INFLUENCE_RADIUS, col );
			node += SKIP;
		}
	}


	col.r = P_RGB_R_MAX;
	col.g = P_RGB_G_MAX;
	col.b = P_RGB_B_MAX;
	col.cd= P_ALPHA_MAX;
	if( OK_TotalNum<=0 ){
		extern void *NewDelayTmpLight( FVECTOR *pos, CVECTOR col, int life );
		NewDelayTmpLight( &work->pos0, col, 60 );
	}

	OK_TotalNum++;

	return (0);
}

/* ---------------------------------------------------------------- */
/*
*pos0 :発生点（即時コピー）
*pos1 :発生点（即時コピー）
*/
void *NewLinerGunPlasma( FVECTOR *pos0, FVECTOR *pos1, int life )
{
	Work		*work ;
	int			buf_size;
	int			num;
	int			prim_num;

	OPERATOR() ;

	if( GM_CheckGameStatus( STATE_DEMO ) ) {
		num      = N_NODES_D;
		prim_num = N_PRIMS_D;
	}else{
		num      = N_NODES_G;
		prim_num = N_PRIMS_G;
	}

	buf_size = sizeof( Work ) + sizeof( FVECTOR ) * (num + 1);
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos0, pos0 );
		DG_COPY_VEC( &work->pos1, pos1 );
		life = DIRECT_TICK( life );
		work->life     = life;
		work->life_max = life;

		work->node_num = num;
		work->prim_num = prim_num;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

