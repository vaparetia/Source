//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_thunder_parts.c
	稲光（実部）
	2000/04/08 S.Okajima
	$Id: d_thunder_parts.c,v 1.1.1.3 2002/11/19 11:46:56 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"libutl.h"
#include	"gameheader.h"
//#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCRPAD_ADDR + 0x2000)

#define N_VERTS	(64)
#define N_POLYS	(N_VERTS/4)
#define	N_PRIMS	(4)
#define N_NODES	(N_PRIMS*N_POLYS)

#define	P_ALPHA_MAX		(64)
#define	P_RGB_R_MAX		(48)
#define	P_RGB_G_MAX		(192)
#define	P_RGB_B_MAX		(96)

/* １サイクル */
#define	ALPHA_COUNT_MIN		(12)

#define	WIDTH_SCALE		(0.007f)
/* ---------------------------------------------------------------- */
typedef	struct _d_thunder_parts_Work	{
	GV_ACT_EX		actor ;

	FVECTOR		pos0;
	FVECTOR		pos1;
	FVECTOR		mem_pos0;
	FVECTOR		mem_pos1;
	float		radius;
	int			life;
	int			alpha_count;
	int			branch_num;

	short		master_rot_vz;
	FVECTOR		node[N_NODES+1];

	DG_PRIM2	*prim_x ;
	DG_PRIM2	*prim_y ;
	DG_PRIM2	*prim_z ;

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
	FVECTOR	*fv1;
	FVECTOR	*fv2;
	int	node_num;
	float	th;

	node_num = node_num1 - node_num0;
	if( node_num <= 1 ) return;
	node_num = node_num0 + node_num/2;

#if 1
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
static void MakeThunder( Work *work )
{
	FVECTOR	*node;
	FVECTOR	*pos;
	FVECTOR	fvtemp;
	SVECTOR master_rot;
	float	len;
	float	shifter;
	int		i,k;
	int		mem_num;


	/* Ｚ軸方向にマスターノード初期化 */
	node = work->node;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;

	work->master_rot_vz = irnd()%4096;
	master_rot.vx = irnd()%4096;
	master_rot.vy = irnd()%2048;
	master_rot.vz = 0;
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = work->radius;
	DG_SetPos2( &work->pos1, &master_rot );
	DG_PutVector( &fvtemp, &work->pos1, 1 );

	/* 水平面より高い場所にシフト */
	work->pos0.vy = (work->pos0.vy < 0.0f)?0.0f:work->pos0.vy;
	work->pos1.vy = (work->pos1.vy < 0.0f)?0.0f:work->pos1.vy;

	fvtemp.vx = work->pos1.vx - work->pos0.vx;
	fvtemp.vy = work->pos1.vy - work->pos0.vy;
	fvtemp.vz = work->pos1.vz - work->pos0.vz;
	len = GV_VecLen3F( &fvtemp );
	if(len < 10.0f) len = 10.0f;
	node[ N_NODES ].vz = len;
	CalcInitNode( node, 0, N_NODES );

	OK_DirVecXY( &work->pos0, &work->pos1, &master_rot );

	shifter = len * 0.75f;
	len *= WIDTH_SCALE;
	/* 主流 */
	OK_Mem_Scr( SCR_POS1,  work->node, sizeof(FVECTOR), N_NODES+1 ) ;
	DG_SetPos2( &work->pos0, &master_rot );
	DG_PutVector( SCR_POS1, SCR_POS1, N_NODES+1 );
	node   = SCR_POS1;
	pos    = SCR_POS0;

	mem_num = irnd() % N_NODES / 2;
	DG_COPY_VEC( &work->mem_pos0, &node[mem_num] );

	for ( i=N_PRIMS; i>0; i-- ){
		for ( k=N_POLYS; k>0; k-- ){
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx+len; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			node++;

			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx+len; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
		}
	}
	OK_Scr_Mem( work->prim_x->pos[ 0 ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( work->prim_x->pos[ 1 ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;
	node   = SCR_POS0;
//	mem_num = irnd() % N_NODES;
	mem_num = irnd() % N_NODES / 2 + N_NODES / 2;
	DG_COPY_VEC( &work->mem_pos0, &node[mem_num] );
	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = shifter;
	master_rot.vx += irnd()%1024-512;
	master_rot.vy += irnd()%1024-512;
	master_rot.vz += irnd()%1024-512;
//	mem_num = irnd() % N_NODES / 2 + N_NODES / 2;
	DG_SetPos2( &node[N_NODES], &master_rot );
	DG_PutVector( &fvtemp, &work->mem_pos1, 1 );

	node   = SCR_POS1;
	pos    = SCR_POS0;
	for ( i=N_PRIMS; i>0; i-- ){
		for ( k=N_POLYS; k>0; k-- ){
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy+len; pos->vz = node->vz    ; pos++;
			node++;

			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy+len; pos->vz = node->vz    ; pos++;
		}
	}
	OK_Scr_Mem( work->prim_y->pos[ 0 ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( work->prim_y->pos[ 1 ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;

	node   = SCR_POS1;
	pos    = SCR_POS0;
	for ( i=N_PRIMS; i>0; i-- ){
		for ( k=N_POLYS; k>0; k-- ){
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz+len; pos++;
			node++;

			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz+len; pos++;
		}
	}
	OK_Scr_Mem( work->prim_z->pos[ 0 ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( work->prim_z->pos[ 1 ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2_UVRGB	*uvrgb_x;
	DG_PRIM2_UVRGB	*uvrgb_y;
	DG_PRIM2_UVRGB	*uvrgb_z;
	int		clock;
	int		i,k;
	int		alpha;

	GM_GroupPrim2( work->prim_x, GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_y, GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_z, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim_x );
	DG_SwitchBuffPrim2( work->prim_y );
	DG_SwitchBuffPrim2( work->prim_z );
	clock = work->prim_x->buffer_clock;


	uvrgb_x = work->prim_x->uvrgb[ clock ] ;
	uvrgb_y = work->prim_y->uvrgb[ clock ] ;
	uvrgb_z = work->prim_z->uvrgb[ clock ] ;
	k = P_ALPHA_MAX;

#if 1
	DG_VisiblePrim2( work->prim_x );
	DG_VisiblePrim2( work->prim_y );
	DG_VisiblePrim2( work->prim_z );
	if( work->life < ALPHA_COUNT_MIN ){
		k = P_ALPHA_MAX * work->life / ALPHA_COUNT_MIN;
	}else{
		if(GV_Time%2==0){
			DG_InvisiblePrim2( work->prim_x );
			DG_InvisiblePrim2( work->prim_y );
			DG_InvisiblePrim2( work->prim_z );
		}
	}
#else
	if( work->alpha_count < ALPHA_COUNT_MIN ){
		k = P_ALPHA_MAX * work->alpha_count / ALPHA_COUNT_MIN;
	}
#endif

	if( work->branch_num==2 ){
		for ( i=N_PRIMS*N_POLYS; i>N_PRIMS*N_POLYS*3/4; i-- ){
			alpha = k * (N_PRIMS*N_POLYS - i) / (N_PRIMS*N_POLYS/4);
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
		}
		for ( i=N_PRIMS*N_POLYS*3/4; i>0; i-- ){
			alpha = k * i / (N_PRIMS*N_POLYS*3/4);
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
		}
	}else{
		for ( i=N_PRIMS*N_POLYS; i>0; i-- ){
			alpha = k * i / (N_PRIMS*N_POLYS);
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
		}
	}

#if 1
	if( work->life > 0 ){
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
#else
	if( work->alpha_count > 0 ){
		work->alpha_count--;
	}else{
		if( work->life > 0 ){
			work->alpha_count = work->life;
		}else{
			work->alpha_count = 0;
		}
	}

	if( work->life > 0 ){
		work->life--;
	}else if( work->alpha_count==0 ){
		GV_DestroyActor( work ) ;
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim_x = OK_FreePrim2( work->prim_x );
	work->prim_y = OK_FreePrim2( work->prim_y );
	work->prim_z = OK_FreePrim2( work->prim_z );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos_0 ;
	FVECTOR				*pos_1 ;
	DG_PRIM2_UVRGB		*uvrgb_0 ;
	DG_PRIM2_UVRGB		*uvrgb_1 ;
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	pos_0  = prim->pos[ 0 ] ;
	pos_1  = prim->pos[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos_0, &work->pos0 );
			DG_COPY_VEC( pos_1, &work->pos0 );
			pos_0++ ;
			pos_1++ ;
		}
	}

	//-------------------------------
	uvrgb_0 = prim->uvrgb[ 0 ] ;
	uvrgb_1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb_1->u = uvrgb_0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x8fff ;
			uvrgb_1->r = uvrgb_0->r = P_RGB_R_MAX ;
			uvrgb_1->g = uvrgb_0->g = P_RGB_G_MAX ;
			uvrgb_1->b = uvrgb_0->b = P_RGB_B_MAX ;
			uvrgb_1->a = uvrgb_0->a = P_ALPHA_MAX ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x8fff ;
			uvrgb_1->r = uvrgb_0->r = P_RGB_R_MAX ;
			uvrgb_1->g = uvrgb_0->g = P_RGB_G_MAX ;
			uvrgb_1->b = uvrgb_0->b = P_RGB_B_MAX ;
			uvrgb_1->a = uvrgb_0->a = P_ALPHA_MAX ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x0fff ;
			uvrgb_1->r = uvrgb_0->r = P_RGB_R_MAX ;
			uvrgb_1->g = uvrgb_0->g = P_RGB_G_MAX ;
			uvrgb_1->b = uvrgb_0->b = P_RGB_B_MAX ;
			uvrgb_1->a = uvrgb_0->a = P_ALPHA_MAX ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x0fff ;
			uvrgb_1->r = uvrgb_0->r = P_RGB_R_MAX ;
			uvrgb_1->g = uvrgb_0->g = P_RGB_G_MAX ;
			uvrgb_1->b = uvrgb_0->b = P_RGB_B_MAX ;
			uvrgb_1->a = uvrgb_0->a = P_ALPHA_MAX ;
			uvrgb_0++;
			uvrgb_1++;
		}
	}
}

static int GetResources( Work *work, FVECTOR *pos0, FVECTOR *pos1, float radius, int branch_num, int life )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		fvtemp;
	float		len;

	if( pos0->vy < 0.0f ) return -1;

	DG_COPY_VEC( &work->pos0, pos0 );
	DG_COPY_VEC( &work->pos1, pos1 );

	work->radius = radius;
	work->life   = life;
	work->branch_num = branch_num;

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

	prim = work->prim_x = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL) return -1;
	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim_x );
	prim = work->prim_y = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL) return -1;
	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim_y );
	prim = work->prim_z = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL) return -1;
	InitPacket( work, prim, tex );
	DG_VisiblePrim2( work->prim_z );

	work->alpha_count = 0;

	MakeThunder( work );

	fvtemp.vx = work->pos1.vx - work->pos0.vx;
	fvtemp.vy = work->pos1.vy - work->pos0.vy;
	fvtemp.vz = work->pos1.vz - work->pos0.vz;
	len = GV_VecLen3F( &fvtemp );

	/* E3 暫定 */
	if(0){
		FVECTOR col={ (float)(P_RGB_R_MAX/2), (float)(P_RGB_G_MAX/2), (float)(P_RGB_B_MAX/2), 8.0f };
		extern void *NewSetHola_Demo( FVECTOR *pos, FVECTOR *col, float size, int life );
		NewSetHola_Demo(            pos0, &col, len, life );
		NewSetHola_Demo( &work->mem_pos0, &col, len, life );
		NewSetHola_Demo(            pos1, &col, len, life );
	}

	if( branch_num-- > 0 ){
		extern void *NewThunderParts_Demo( FVECTOR *pos0, FVECTOR *pos1, float radius, int branch_num, int life );
		NewThunderParts_Demo( &work->mem_pos0, &work->mem_pos1, len*0.5f, branch_num, life-1 );
	}

	return (0);
}

/* ---------------------------------------------------------------- */
/*
*pos0     :[ポインタ参照する]発生点
*pos1     :[ポインタ参照する]
radius    :pos1を中心とした球の半径（この球の内部の点を終点とする）０以上
branch_num:枝分かれの数
life      :寿命。フレーム指定。０以上。
*/
void *NewThunderParts_Demo( FVECTOR *pos0, FVECTOR *pos1, float radius, int branch_num, int life )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos0, pos1, radius, branch_num, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

