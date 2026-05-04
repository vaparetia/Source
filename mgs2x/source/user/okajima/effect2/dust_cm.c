//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	dust_cm.c
	カメラ前の空間にゴミ

	2000/12/28 S.Okajima
	$Id: dust_cm.c,v 1.4 2002/11/23 12:24:50 Yoshizawa1 Exp $
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

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)


//#define N_VERTS			(32)
#define N_VERTS			(16)	/* rspr */
#define N_PRIMS			(0x2000 / 16 / N_VERTS)
#define N_LOOPS			(8)
#define N_LOOPS_GAME	(8)

#define	RANDAM_FIELD_NUM	(0x2000 / 4)

#if 0
#define DUST_SIZE		(4.0f)
#define DUST_SIZE_GAME	(20.0f)
#else
#define DUST_SIZE		(10.0f)
#define DUST_SIZE_GAME	(10.0f)
#endif

#define SPEED_RND	(1.0f)

/* 画面前バンダリ */
#define	BOUND_WIDTH	(600.0f)
#define	BOUND_WIDTH_DOUBLE	(BOUND_WIDTH*2.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(1200.0f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int		name ;
	int		map ;

	int		n_loops;

	int		invisible_flag;
	int		col;
	int		before_flag;
	FVECTOR before_cam;

	FVECTOR bound_0;
	FVECTOR bound_1;

	DG_PRIM2	*prim ;
//	FVECTOR	vec[N_LOOPS*N_PRIMS*N_VERTS];
	FVECTOR	vec[0];
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
	DG_PRIM2 *prim;
	int	clock;
	int	i, j;
	FVECTOR	*before_pos;
	FVECTOR	*pos;
	FVECTOR	*sc_pos;
	FVECTOR	*vec;
	FVECTOR	*sc_vec;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	FVECTOR diff;
	FVECTOR cam;
	float	len;
	float	*p_randam;
	int		near_flag;

	CheckMesgParam( work );

	prim = work->prim;

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );


	if( (cam.vy > GM_WaterLevel+2000.0f  &&  !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) )
	 || (work->invisible_flag)     ){
//	if( work->invisible_flag ){
		DG_InvisiblePrim2( work->prim );
		work->before_flag = 1;
		return;
	}else{
		DG_VisiblePrim2( work->prim );
	}

	// 前のカメラ座標との差
	_sceVu0SubVector( &diff, &cam, &work->before_cam ) ;
	len = GV_VecLen3F( &diff );
//printf("len:%f\n",len);
	near_flag = 0;
	if( len < BOUND_WIDTH ){
		near_flag = 1;
	}


	/* カメラのまえのバンダリをつくる */
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &work->bound_0 ) ;
	_sceVu0AddVector( &bound_1, &b_dis, &work->bound_1 ) ;


	if( bound_0.vy > GM_WaterLevel ){
		bound_0.vy = GM_WaterLevel;
		bound_1.vy = GM_WaterLevel - BOUND_WIDTH_DOUBLE;
	}

//	NewBoundingBoxView( &bound_1, &bound_0, 255, 128, 64 ) ;



//	// 再設置点基準をづらす
//	_sceVu0AddVector( &b_dis, &b_dis, &diff ) ;
//	AN_Test_Eye2( &b_dis, 2 );

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	before_pos = prim->pos[1-clock];
	pos        = prim->pos[  clock];
	vec = work->vec;
	if( near_flag ){
		for ( j=0 ; j < work->n_loops ; j++ ){
			OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			OK_Mem_Scr( SCR_TMP, vec,        sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			sc_pos = SCR_POS;
			sc_vec = SCR_TMP;
			for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
				if(       sc_pos->vx > bound_0.vx ){
					sc_pos->vx-= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vy > bound_0.vy ){
					sc_pos->vy-= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vz > bound_0.vz ){
					sc_pos->vz-= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vx < bound_1.vx ){
					sc_pos->vx+= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vy < bound_1.vy ){
					sc_pos->vy+= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vz < bound_1.vz ){
					sc_pos->vz+= BOUND_WIDTH_DOUBLE;
				}else{
					sc_pos->vx += sc_vec->vx;
					sc_pos->vy += sc_vec->vy;
					sc_pos->vz += sc_vec->vz;
				}
				sc_vec++;
				sc_pos++;
			}
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			before_pos += N_VERTS*N_PRIMS;
			pos        += N_VERTS*N_PRIMS;
			vec        += N_VERTS*N_PRIMS;
		}
	}else{
		for ( j=0 ; j < work->n_loops ; j++ ){
			OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			sc_pos = SCR_POS;
			// ランダムテーブルシャッフル
			OK_rnd_to_scr( SCR_TMP, RANDAM_FIELD_NUM );
			p_randam = (float *)SCR_TMP;
			for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
				sc_pos->vx = bound_1.vx + BOUND_WIDTH_DOUBLE * (*(p_randam++));
				sc_pos->vy = bound_1.vy + BOUND_WIDTH_DOUBLE * (*(p_randam++));
				sc_pos->vz = bound_1.vz + BOUND_WIDTH_DOUBLE * (*(p_randam++));
				sc_pos++;
			}
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			before_pos += N_VERTS*N_PRIMS;
			pos        += N_VERTS*N_PRIMS;
		}
	}

	DG_COPY_VEC( &work->before_cam, &cam );
	work->before_flag = 0;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	int		isize;
	float	fsize;
	float	speed;
	float	angle;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		isize  = (int)DUST_SIZE;
		fsize  = DUST_SIZE;
		speed  = SPEED_RND;
	}else{
		isize  = (int)DUST_SIZE_GAME;
		fsize  = DUST_SIZE_GAME;
		speed  = SPEED_RND;
	}

	GV_ZeroMemory( prim->pos[0],    sizeof( FVECTOR ) * work->n_loops * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1],    sizeof( FVECTOR ) * work->n_loops * N_PRIMS * N_VERTS );

	vec        = work->vec;
	pos0       = prim->pos[ 0 ];
	pos1       = prim->pos[ 1 ];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < work->n_loops*N_PRIMS*N_VERTS ; i++ ){
		vec->vx = speed*frnd();
		vec->vy = speed*frnd();
		vec->vz = speed*frnd();

		pos1->vw = pos0->vw = fsize;

		angle = TPI*rnd();
		uvrgbwh1->w  = uvrgbwh0->w  = fsize*sinf(angle);
		uvrgbwh1->h  = uvrgbwh0->h  = fsize*cosf(angle);
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
#if 0
		uvrgbwh1->r  = uvrgbwh0->r  = BUBBLE_COL_R ;
		uvrgbwh1->g  = uvrgbwh0->g  = BUBBLE_COL_G ;
		uvrgbwh1->b  = uvrgbwh0->b  = BUBBLE_COL_B ;
		uvrgbwh1->a  = uvrgbwh0->a  = BUBBLE_ALPHA ;
#else
		//柴田改造 2001/4/20
		uvrgbwh1->r  = uvrgbwh0->r  = ((work->col>>24)&0xff);
		uvrgbwh1->g  = uvrgbwh0->g  = ((work->col>>16)&0xff);
		uvrgbwh1->b  = uvrgbwh0->b  = ((work->col>> 8)&0xff);
		uvrgbwh1->a  = uvrgbwh0->a  = BUBBLE_ALPHA ;
#endif
		vec++;
		pos0 ++;
		pos1 ++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->before_flag = 1;

#if 1
	DG_COPY_VEC( &work->before_cam, (FVECTOR *)DG_Chanls->eye.m[3] );
#else
	work->before_cam.vx = FLOAT_MAX;
	work->before_cam.vy = FLOAT_MAX;
	work->before_cam.vz = FLOAT_MAX;
	work->before_cam.vw = FLOAT_MAX;
#endif

	if( GCL_GetOption( 'i' ) != NULL ){
		work->invisible_flag = GCL_GetNextInt() ;
	}else{
		work->invisible_flag = 0 ;
	}

//柴田追加 2001/4/20
	if( GCL_GetOption( 'c' ) != NULL ){
		work->col = (GCL_GetNextInt()<<24);
		work->col |= (GCL_GetNextInt()<<16);
		work->col |= (GCL_GetNextInt()<<8);
		work->col |= BUBBLE_ALPHA;		
	}else{
		work->col = ((BUBBLE_COL_R<<24)|(BUBBLE_COL_G<<16)|(BUBBLE_COL_B<<8)|(BUBBLE_ALPHA));
	}


	/* 表示範囲 */
	work->bound_0.vx =  BOUND_WIDTH ;
	work->bound_0.vy =  BOUND_WIDTH ;
	work->bound_0.vz =  BOUND_WIDTH ;
	work->bound_1.vx = -BOUND_WIDTH ;
	work->bound_1.vy = -BOUND_WIDTH ;
	work->bound_1.vz = -BOUND_WIDTH ;

//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 13081966 /*"chi03_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_loops*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

void *NewDustOnCamera( int name, int map )
{
	Work		*work ;
	int			buf_size;
	int			num;

	OPERATOR() ;

	if( GCL_GetOption( 'n' ) != NULL ){
		num = GCL_GetNextInt() ;
	}else{
		if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
			num=N_LOOPS_GAME;
		}else{
			num=N_LOOPS;
		}
	}


	buf_size = sizeof( Work ) + sizeof( FVECTOR ) * num*N_PRIMS*N_VERTS;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		OK_rnd_to_scr( SCR_TMP, 0 );

		work->name = name;
		work->map = map;
		work->n_loops = num;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		num = (num * DG_GetInWaterEffectLevel()) >> 8 ;	// 量調整

		if ( num ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			OK_rnd_to_scr( SCR_TMP, 0 );

			work->name    = name;
			work->map     = map;
			work->n_loops = num;

			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor )
			work->name = name;
		}
#endif
	}
	return (void *)work ;
}
