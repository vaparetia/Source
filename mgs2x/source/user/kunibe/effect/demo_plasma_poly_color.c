//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	demo_plasma_poly_color.c
	二点を結ぶプラズマ（ポリゴン）
	2000/04/06 S.Okajima
	$Id: demo_plasma_poly_color.c,v 1.1.1.3 2002/11/19 11:44:37 Yoshizawa1 Exp $
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

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCRPAD_ADDR + 0x2000)

#define N_VERTS	(64)
#define N_POLYS	(N_VERTS/4)
#define	N_PRIMS	(4)
#define N_NODES	(N_PRIMS*N_POLYS)

#define	P_ALPHA_MAX		(128)
#define	P_RGB_R_MAX		(48)
#define	P_RGB_G_MAX		(192)
#define	P_RGB_B_MAX		(96)

/* １サイクル */
#define	ALPHA_COUNT_MIN		(16)
#define	ALPHA_COUNT_RND		(16)

#define	WIDTH_SCALE		(0.01f)

int	OK_plasma_count = 0;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		real_pos1;
	FVECTOR		running;
	float		radius;
	float		width;
	int			branch;
	int			life;
	int			flag;
	int			alpha;
	int			alpha_count;
	int			init_flag;

	short		master_rot_vz;
	FVECTOR		node[N_NODES+1];

	DG_PRIM2	*prim_x ;
	DG_PRIM2	*prim_y ;
	DG_PRIM2	*prim_z ;

	CVECTOR		color;
	int			branch_num_y;
	int			branch_num_z;

	int			who_am_i;

} Work ;

/* ---------------------------------------------------------------- */
static void MakeBranch( Work *work )
{
	work->branch_num_y = irnd()%N_NODES/4 + N_NODES*3/4;
	work->branch_num_z = irnd()%N_NODES*3/4 + N_NODES/4;
}

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
//	FVECTOR	*fv1;
//	FVECTOR	*fv2;
	int	node_num;
//	float	th;

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
	FVECTOR	*pos;
	FVECTOR	fvtemp;
	SVECTOR master_rot;
	DG_PRIM2_UVRGB	*uvrgb_x;
	DG_PRIM2_UVRGB	*uvrgb_y;
	DG_PRIM2_UVRGB	*uvrgb_z;
	float	len;
	float	shifter;
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

	/* Ｚ軸方向にマスターノード初期化 */

	node = work->node;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;

	/* 終点を更新 */
	work->real_pos1.vx += work->running.vx;
	work->real_pos1.vy += work->running.vy;
	work->real_pos1.vz += work->running.vz;
	fvtemp.vx = work->real_pos1.vx - work->pos1->vx;
	fvtemp.vy = work->real_pos1.vy - work->pos1->vy;
	fvtemp.vz = work->real_pos1.vz - work->pos1->vz;
	len = GV_VecLen3F( &fvtemp );
	if( irnd()%32==0 || len > work->radius*0.5f || work->flag==0){
		work->init_flag = 1;
		work->alpha_count = ALPHA_COUNT_MIN + irnd()%ALPHA_COUNT_RND;
		work->master_rot_vz = irnd()%4096;
		work->running.vx = len*frnd()*0.005f;
		work->running.vy = len*frnd()*0.005f;
		work->running.vz = len*frnd()*0.005f;
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

		fvtemp.vx = work->real_pos1.vx - work->pos0->vx;
		fvtemp.vy = work->real_pos1.vy - work->pos0->vy;
		fvtemp.vz = work->real_pos1.vz - work->pos0->vz;
		len = GV_VecLen3F( &fvtemp );
		if(len < 10.0f) len = 10.0f;
		node[ N_NODES ].vz = len;
		CalcInitNode( node, 0, N_NODES );
		MakeBranch( work );
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

		fvtemp.vx = work->real_pos1.vx - work->pos0->vx;
		fvtemp.vy = work->real_pos1.vy - work->pos0->vy;
		fvtemp.vz = work->real_pos1.vz - work->pos0->vz;
		len = GV_VecLen3F( &fvtemp );
		if(len < 10.0f) len = 10.0f;
		node[ N_NODES ].vz = len;
		if(irnd()%32==0  ||  work->alpha_count<0 ){
			work->alpha_count = ALPHA_COUNT_MIN + irnd()%ALPHA_COUNT_RND;
			CalcInitNode( node, 0, N_NODES );
			MakeBranch( work );
		}else{
			work->alpha_count--;
			CalcNextNode( node, 0, N_NODES );
			CalcSmoothNode( node, N_NODES );
		}
	}

	OK_DirVecXY( work->pos0, &work->real_pos1, &master_rot );
	work->master_rot_vz += 4;
	master_rot.vz = work->master_rot_vz;

	shifter = len * 0.25f;
	if( work->width <= 0.0f ){
		len *= WIDTH_SCALE;		/* 長さに比例した太さ */
	}else{
		len  = work->width;		/* 直接指定した太さ */
	}

	len *= 0.5f;

	fvtemp.vx = len;
	fvtemp.vy = len;
	fvtemp.vz = len;

	/* 主流 */
	OK_Mem_Scr( SCR_POS1,  work->node, sizeof(FVECTOR), N_NODES+1 ) ;
	DG_SetPos2( work->pos0, &master_rot );
	DG_PutVector( SCR_POS1, SCR_POS1, N_NODES+1 );
	node   = SCR_POS1;
	pos    = SCR_POS0;

// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2		vf14,0x00(%0)
		"::"r"(&fvtemp):"memory"
	);
#endif

	for ( i=N_PRIMS; i>0; i-- ){
		for ( k=N_POLYS; k>0; k-- ){
#ifndef BP_PSX2_ASM
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx+len; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			node++;

			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx+len; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
#else

			asm volatile ("
				lqc2		vf8 ,0x00(%0)
				lqc2		vf9 ,0x10(%0)
				lqc2		vf10,0x20(%0)
				lqc2		vf11,0x30(%0)
				lqc2		vf12,0x00(%1)
				lqc2		vf13,0x10(%1)

				vmove.xyz	vf8 ,vf12
				vmove.xyz	vf10,vf13
				vmove.xyz	vf9 ,vf12
				vmove.xyz	vf11,vf13
				vadd.x		vf9 ,vf9 ,vf14
				vadd.x		vf11,vf11,vf14

				sqc2		vf8 ,0x00(%0)
				sqc2		vf9 ,0x10(%0)
				sqc2		vf10,0x20(%0)
				sqc2		vf11,0x30(%0)
				"::"r"(pos),"r"(node):"memory"
			);

			pos+=4;
			node++;
#endif
		}
	}
	OK_Scr_Mem( work->prim_x->pos[ clock ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;

	/* 副流１ */
	if( work->branch!=0 ){
		OK_Mem_Scr( SCR_POS1,  work->node, sizeof(FVECTOR), N_NODES+1 ) ;
		node   = SCR_POS1;
		node  += work->branch_num_y;
		alpha  = N_NODES+1-work->branch_num_y;
		for( i=0; i<alpha; i++ ){
			node->vx += shifter * (float)i / (float)alpha;
			node++;
		}
		DG_SetPos2( work->pos0, &master_rot );
		DG_PutVector( SCR_POS1, SCR_POS1, N_NODES+1 );
	}
	node   = SCR_POS1;
	pos    = SCR_POS0;
	for ( i=N_PRIMS; i>0; i-- ){
		for ( k=N_POLYS; k>0; k-- ){
// yano /*未検証*/
#ifndef BP_PSX2_ASM
			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy+len; pos->vz = node->vz    ; pos++;
			node++;

			pos->vx = node->vx    ; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy+len; pos->vz = node->vz    ; pos++;
#else
			asm volatile ("
				lqc2		vf8 ,0x00(%0)
				lqc2		vf9 ,0x10(%0)
				lqc2		vf10,0x20(%0)
				lqc2		vf11,0x30(%0)
				lqc2		vf12,0x00(%1)
				lqc2		vf13,0x10(%1)

				vmove.xyz	vf8 ,vf12
				vmove.xyz	vf9 ,vf12
				vmove.xyz	vf10,vf13
				vmove.xyz	vf11,vf13
				vadd.y		vf9 ,vf9 ,vf14
				vadd.y		vf11,vf11,vf14

				sqc2		vf8 ,0x00(%0)
				sqc2		vf9 ,0x10(%0)
				sqc2		vf10,0x20(%0)
				sqc2		vf11,0x30(%0)
				"::"r"(pos),"r"(node):"memory"
			);
			pos+=4;
			node++;
#endif
		}
	}
	OK_Scr_Mem( work->prim_y->pos[ clock ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;


	/* 副流２ */
	if( work->branch!=0 ){
		shifter *= 0.5f;
		OK_Mem_Scr( SCR_POS1,  work->node, sizeof(FVECTOR), N_NODES+1 ) ;
		node   = SCR_POS1;
		node  += work->branch_num_z;
		alpha  = N_NODES+1-work->branch_num_z;
		for( i=0; i<alpha; i++ ){
			node->vy += shifter * (float)i / (float)alpha;
			node++;
		}
		DG_SetPos2( work->pos0, &master_rot );
		DG_PutVector( SCR_POS1, SCR_POS1, N_NODES+1 );
	}
	node   = SCR_POS1;
	pos    = SCR_POS0;
	for ( i=N_PRIMS; i>0; i-- ){
		for ( k=N_POLYS; k>0; k-- ){
// yano /*未検証*/
#ifndef BP_PSX2_ASM
			pos->vx = node->vx+len; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy+len; pos->vz = node->vz+len; pos++;
			node++;

			pos->vx = node->vx+len; pos->vy = node->vy    ; pos->vz = node->vz    ; pos++;
			pos->vx = node->vx    ; pos->vy = node->vy+len; pos->vz = node->vz+len; pos++;
#else

			asm volatile ("
				lqc2		vf8 ,0x00(%0)
				lqc2		vf9 ,0x10(%0)
				lqc2		vf10,0x20(%0)
				lqc2		vf11,0x30(%0)
				lqc2		vf12,0x00(%1)
				lqc2		vf13,0x10(%1)

				vmove.xyz	vf8 ,vf12
				vmove.xyz	vf9 ,vf12
				vmove.xyz	vf10,vf13
				vmove.xyz	vf11,vf13
				vadd.x		vf9 ,vf9 ,vf14
				vadd.yz		vf9 ,vf9 ,vf14
				vadd.x		vf11,vf11,vf14
				vadd.yz		vf11,vf11,vf14

				sqc2		vf8 ,0x00(%0)
				sqc2		vf9 ,0x10(%0)
				sqc2		vf10,0x20(%0)
				sqc2		vf11,0x30(%0)
				"::"r"(pos),"r"(node):"memory"
			);

			pos+=4;
			node++;
#endif
		}
	}
	OK_Scr_Mem( work->prim_z->pos[ clock ],  SCR_POS0, sizeof(FVECTOR), N_VERTS * N_PRIMS ) ;


	uvrgb_x = work->prim_x->uvrgb[ clock ] ;
	uvrgb_y = work->prim_y->uvrgb[ clock ] ;
	uvrgb_z = work->prim_z->uvrgb[ clock ] ;

	if( work->init_flag != 0 ){
		if( work->who_am_i == OK_plasma_count ){
			DG_SetTmpLight2(
				&work->real_pos1,
				500.0f,
				1000.0f,
				*(int *)&work->color,
				LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		}
		for ( i=N_PRIMS*N_POLYS; i>0; i-- ){
			alpha = work->alpha * i / (N_PRIMS*N_POLYS);
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
		}
	}else if( work->alpha_count > ALPHA_COUNT_MIN ){
		for ( i=N_PRIMS*N_POLYS; i>0; i-- ){
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = work->alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = work->alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = work->alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = work->alpha;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
		}
	}else{
		k = work->alpha * work->alpha_count / ALPHA_COUNT_MIN;
		for ( i=N_PRIMS*N_POLYS; i>0; i-- ){
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = k;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = k;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = k;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
			uvrgb_x->a = uvrgb_y->a = uvrgb_z->a = k;
			uvrgb_x++; uvrgb_y++; uvrgb_z++;
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
			DG_COPY_VEC( pos_0, work->pos0 );
			DG_COPY_VEC( pos_1, work->pos0 );
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
			uvrgb_1->r = uvrgb_0->r = work->color.r ;
			uvrgb_1->g = uvrgb_0->g = work->color.g ;
			uvrgb_1->b = uvrgb_0->b = work->color.b ;
			uvrgb_1->a = uvrgb_0->a = work->alpha ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x8fff ;
			uvrgb_1->r = uvrgb_0->r = work->color.r ;
			uvrgb_1->g = uvrgb_0->g = work->color.g ;
			uvrgb_1->b = uvrgb_0->b = work->color.b ;
			uvrgb_1->a = uvrgb_0->a = work->alpha ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x0fff ;
			uvrgb_1->r = uvrgb_0->r = work->color.r ;
			uvrgb_1->g = uvrgb_0->g = work->color.g ;
			uvrgb_1->b = uvrgb_0->b = work->color.b ;
			uvrgb_1->a = uvrgb_0->a = work->alpha ;
			uvrgb_0++;
			uvrgb_1++;

			uvrgb_1->u = uvrgb_0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb_1->v = uvrgb_0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb_1->q = uvrgb_0->q = 4096 ;
			uvrgb_1->f = uvrgb_0->f = 0x0fff ;
			uvrgb_1->r = uvrgb_0->r = work->color.r ;
			uvrgb_1->g = uvrgb_0->g = work->color.g ;
			uvrgb_1->b = uvrgb_0->b = work->color.b ;
			uvrgb_1->a = uvrgb_0->a = work->alpha ;
			uvrgb_0++;
			uvrgb_1++;
		}
	}
}

static int GetResources( Work *work, FVECTOR *pos0, FVECTOR *pos1, int radius, int width, int branch, int color, int life )
{
	FVECTOR	fvtemp;
	float	len;
	FVECTOR	*node;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->width = (float)width;

	work->branch = branch;

	work->pos0   = pos0;
	work->pos1   = pos1;

	work->radius = (float)radius;
	work->life   = life;
	DG_COPY_VEC( &work->real_pos1, work->pos1 );
	work->flag   = 0;
	work->alpha_count = 0;

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

	work->color.r = ( ( color >> 24 ) & 0xff );
	work->color.g = ( ( color >> 16 ) & 0xff );
	work->color.b = ( ( color >>  8 ) & 0xff );
	work->alpha   = ( color & 0xff );

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



	/* Ｚ軸方向にマスターノード作成 */
	fvtemp.vx = work->pos1->vx - work->pos0->vx;
	fvtemp.vy = work->pos1->vy - work->pos0->vy;
	fvtemp.vz = work->pos1->vz - work->pos0->vz;
	len = GV_VecLen3F( &fvtemp );

	node = work->node;
	node[       0 ].vx = 0.0f;
	node[       0 ].vy = 0.0f;
	node[       0 ].vz = 0.0f;
	node[ N_NODES ].vx = 0.0f;
	node[ N_NODES ].vy = 0.0f;
	node[ N_NODES ].vz = len;
	CalcInitNode( node, 0, N_NODES );
	MakeBranch( work );

	OK_plasma_count++;
	work->who_am_i = OK_plasma_count;

	return (0);
}

/* ---------------------------------------------------------------- */
/*
*pos0 :[ポインタ参照する]発生点
*pos1 :[ポインタ参照する]
radius:pos1を中心とした球の半径（この球の内部の点を終点とする）０以上
width :０以下）長さに比例した幅とする １以上）この値を幅とする
branch:０）枝分かれ無し １）枝分かれ有り
life  :寿命。フレーム指定。０以上。
*/
void *NewPlasmaPolyColor_Demo( FVECTOR *pos0, FVECTOR *pos1, int radius, int width, int branch, int color, int life )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos0, pos1, radius, width, branch, color, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

