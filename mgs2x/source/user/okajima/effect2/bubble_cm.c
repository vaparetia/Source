//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	bubble_cm.c
	カメラ前の空間に泡

	2000/12/27 S.Okajima
	$Id: bubble_cm.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $
*/
#endif
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
#define	RAISE			(0)

#define	SCALE_DOWN		(0.1f)

#define	SCREEN_NEAR			( 51.0f )

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)
//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_RND		(SCRPAD_ADDR + 0x2000)

#define N_PRIMS		(8)
#define N_VERTS		(32)
#define N_LOOPS		(2)

#define	RANDAM_FIELD_NUM	(0x2000/4)

/* 画面前バンダリ */
#define	BOUND_WIDTH	(1000.0f)
#define	BOUND_WIDTH_DOUBLE	(BOUND_WIDTH*2.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(1000.0f)

#define	SIZE_MIN	(5.0f)
#define	SIZE_RAND	(20.0f)

#define	THIS_ALPHA	(32)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int		n_loops;

	int		invisible_flag;
	int		col;
	FVECTOR bound_0;
	FVECTOR bound_1;

	DG_PRIM2	*prim ;
} Work ;

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

/*----------------------------------------------------------------*/
static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->invisible_flag = 1;
			break;
		  case REQ_ON:
			work->invisible_flag = 0;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i,j;
	FVECTOR	*pos;
	DG_PRIM2	*prim;
	FVECTOR	*sc_pos0;
	FVECTOR	*sc_pos1;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	FVECTOR cam;
	float	*p_randam;

	prim = work->prim;

	CheckMesgParam( work );

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	if( work->invisible_flag  ||  cam.vy > GM_WaterLevel + 2000.0f ){
//	if( work->invisible_flag ){
		DG_InvisiblePrim2( prim );
		return;
	}else{
		DG_VisiblePrim2( prim );
	}

	/* カメラのまえのバンダリをつくる */
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &work->bound_0 ) ;
	_sceVu0AddVector( &bound_1, &b_dis, &work->bound_1 ) ;


	if( bound_0.vy > GM_WaterLevel - (SIZE_MIN + SIZE_RAND) ){
		bound_0.vy = GM_WaterLevel;
		bound_1.vy = GM_WaterLevel - BOUND_WIDTH_DOUBLE;
	}

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	//AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;


	sc_pos0 = prim->pos[1-clock];
	sc_pos1 = prim->pos[  clock];
	for ( j=0; j<work->n_loops; j++ ){
		OK_Mem_Scr( SCRPAD_ADDR, sc_pos0, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		pos   = SCRPAD_ADDR;

		OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );
		p_randam = (float *)SCR_RND;

		for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
			if( vu0_CheckBoundingBox( pos, &bound_0, &bound_1 ) ){
				pos->vx += pos->vw * (*(p_randam++));
				pos->vy += pos->vw * BUBBLE_SPEED;
				pos->vz += pos->vw * (*(p_randam++));
			}else{
				pos->vx = bound_1.vx + BOUND_WIDTH_DOUBLE * rnd();
				pos->vy = bound_1.vy + BOUND_WIDTH_DOUBLE * rnd();
				pos->vz = bound_1.vz + BOUND_WIDTH_DOUBLE * rnd();
			}
			pos++;
		}
		OK_Scr_Mem( sc_pos1, SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		sc_pos0+= N_VERTS*N_PRIMS;
		sc_pos1+= N_VERTS*N_PRIMS;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacketSingle( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos;
	FVECTOR				*dest_pos0;
	FVECTOR				*dest_pos1;
	DG_PRIM2_UVRGBWH	*uvrgb ;		/* スプライト用 */
	DG_PRIM2_UVRGBWH	*dest_uvrgb0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*dest_uvrgb1 ;	/* スプライト用 */
	int		i,j ;
	int		alpha;
	int		isize;
	int		col_r;
	int		col_g;
	int		col_b;
	float	size;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );

	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		alpha = THIS_ALPHA;
	}else{
		alpha = BUBBLE_ALPHA;
	}

	col_r = (work->col>>24)&0xff;
	col_g = (work->col>>16)&0xff;
	col_b = (work->col>> 8)&0xff;

	dest_pos0   = prim->pos[ 0 ];
	dest_pos1   = prim->pos[ 1 ];
	dest_uvrgb0 = prim->uvrgb[ 0 ];
	dest_uvrgb1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < work->n_loops ; i++ ){
		pos   = SCR_POS;
		uvrgb = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS*N_VERTS ; j++ ){
			size = rnd();
			size*= size*size;
			pos->vw = SIZE_MIN + size*SIZE_RAND;
			isize  = (int)pos->vw;

			uvrgb->w  = isize;
			uvrgb->h  = isize;
			uvrgb->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgb->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgb->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgb->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgb->q0 = 4096 ;
			uvrgb->q1 = 4096 ;
			uvrgb->f0 = 0x0fff ;
			uvrgb->f1 = 0x0fff ;

			uvrgb->r  = col_r;
			uvrgb->g  = col_g;
			uvrgb->b  = col_b;
			uvrgb->a  = alpha ;

			pos++;
			uvrgb++ ;
		}
		OK_Scr_Mem( dest_pos0,   SCR_POS, sizeof(FVECTOR),          N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_pos1,   SCR_POS, sizeof(FVECTOR),          N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb0, SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb1, SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
		dest_pos0  += N_PRIMS*N_VERTS;
		dest_pos1  += N_PRIMS*N_VERTS;
		dest_uvrgb0+= N_PRIMS*N_VERTS;
		dest_uvrgb1+= N_PRIMS*N_VERTS;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	/* 表示範囲 */
	work->bound_0.vx =  BOUND_WIDTH ;
	work->bound_0.vy =  BOUND_WIDTH ;
	work->bound_0.vz =  BOUND_WIDTH ;
	work->bound_1.vx = -BOUND_WIDTH ;
	work->bound_1.vy = -BOUND_WIDTH ;
	work->bound_1.vz = -BOUND_WIDTH ;

	if( GCL_GetOption( 'i' ) != NULL ){
		work->invisible_flag = GCL_GetNextInt();
	}else{
		work->invisible_flag = 0 ;
	}

//柴田追加 2001/4/20
	if( GCL_GetOption( 'c' ) != NULL ){
		work->col = GCL_GetNextInt()<<24;
		work->col |= GCL_GetNextInt()<<16;
		work->col |= GCL_GetNextInt()<<8;
		work->col |= BUBBLE_ALPHA;		
	}else{
		work->col = ((BUBBLE_COL_R<<24)|(BUBBLE_COL_G<<16)|(BUBBLE_COL_B<<8)|(BUBBLE_ALPHA));
	}

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_loops*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	if( !InitPacketSingle( work, prim, tex ) ) return -1;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 2, 1, (work->col&255)   ) );

	return 0 ;
}

#ifdef KP_WINDOWS
static	void	DmyAct( Work *work ){}
static	void	DmyDie( Work *work ){}
#endif

void *NewBubbleOnCamera( int name, int map )
{
	Work		*work ;
	int			buf_size;
	int			num;

	OPERATOR() ;

	if( GCL_GetOption( 'n' ) != NULL ){
		num = GCL_GetNextInt() ;
	}else{
		num = N_LOOPS;
	}

	buf_size = sizeof( Work );
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		OK_frnd_to_scr( SCR_RND, 0 );	// 仮初期化

		work->name = name;
		work->map = map;
		work->n_loops = num;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		num = (num * DG_GetInWaterEffectLevel()) >> 8 ;	// 量調整

		if( num )
		{
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			OK_frnd_to_scr( SCR_RND, 0 );	// 仮初期化

			work->name    = name;
			work->map     = map;
			work->n_loops = num;

			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		else
		{
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor )
			work->name = name;
		}
#endif
	}
	return (void *)work ;
}
