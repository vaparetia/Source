//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_nkt_smoke.c
   ウェブサイト
   
   2001/07/28	S.Kobayashi
   $Id: demo_nkt_smoke.c,v 1.1.1.3 2002/11/19 11:50:19 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../okajima/etc/ok_util.h"
#include    "../test/etc.h"
#include    "game.h"

#define	RAISE					(0)
#define	RAISE_SHIFT_WORLD		(20.0f)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)
#define	N_PRIMS			(1)
#define	N_VERTS			(32)
#define	N_VP			(N_PRIMS * N_VERTS)

#define	SIZE_BASE		(10)
#define	SIZE_STEP		(3)

#define	LIGHT_LEN		(300.0f)

#define	ALPHA_BASE			(96.0f)
#define	ALPHA_MIN			(16)

#define	COL_R			(32)
#define	COL_G			(24)
#define	COL_B			(8)

#define SK_R (128)
#define SK_G (128)
#define SK_B (128)
#define SK_A (128)

typedef	struct _work {
	GV_ACT_EX			actor ;
	FMATRIX             *pWorld;

	DG_PRIM2	*prim_main; // tex
	
	int                 name;
	u_int               flag;
	void  ( *act )( struct _work * );
} Work ;

static FVECTOR s_pos[N_VP];
static FVECTOR ShiftMain    = {    0.0f,  280.0f,    0.0f, 0.0f };

// define
#define DIE_FLAG (0x1)

// extern
extern void *NewSubMazzleFlushMngNormal( int num , FMATRIX *pWorld , FVECTOR *shift , FVECTOR *pPower , int life , float size_rnd , float alpha , int deg );
extern void *NewSubMazzleFlushMngRed( int num , FMATRIX *pWorld , FVECTOR *shift , FVECTOR *pPower , int life , float size_rnd , float alpha , int deg );

// メッセージを受けとると殺す敵が死んだ時に利用
static int MsgDie( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int         name;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	name = 0;
	while( --msg_num >= 0 ){
		if ( msg->message[ 0 ] == DIE_FLAG ){
			GV_DestroyActor( pWork );
			return ( -1 );
		}
		msg--;
	}
	return ( 0 );
}

static void NormalAct( Work *pWork )
{
	// effect by koba4
	FMATRIX fmtmp;
	FVECTOR fvtmp;
	FVECTOR shift;
	FVECTOR power;

	power.vx = 0.0f;
	power.vy = 80.0f;
	power.vz = 70.0f;
	power.vw = 1.0f;

	DG_COPY_VEC( &shift , (FVECTOR *)pWork->pWorld->m[ 1 ] );
	_sceVu0ScaleVector( &shift , &shift , 300.f ); 
	_sceVu0AddVector( &fvtmp , (FVECTOR *)pWork->pWorld->m[ 3 ] , &shift );
	fmtmp = *pWork->pWorld;
	DG_COPY_VEC( ( FVECTOR * )fmtmp.m[ 3 ] , &fvtmp );
	NewSubMazzleFlushMngNormal( 1 , &fmtmp , &DG_ZeroVector ,&power , 10 , 100.0f, 2.0f , 32 );
	// prim
	{
		DG_PRIM2 	*prim;
		int			clock;
		int			j;
		DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

		DG_VisiblePrim2( pWork->prim_main ) ;
		GM_GroupPrim2( pWork->prim_main,    GM_CurrentStageMap ) ;
		DG_SwitchBuffPrim2( pWork->prim_main );
		clock = pWork->prim_main->buffer_clock;

		prim = pWork->prim_main;
		uvrgbwh = prim->uvrgb[ clock ];
		DG_SetPos( pWork->pWorld );
		DG_MovePos( &ShiftMain );
		DG_PutVector( s_pos, prim->pos[ clock ], N_VP );
		for( j = 0; j< N_VP ; j++ ) {
			uvrgbwh->w = uvrgbwh->h = ( N_VP - j ) * SIZE_STEP + SIZE_BASE;
			uvrgbwh->a = ALPHA_BASE * ( N_VP - j ) / N_VP + ALPHA_MIN;
			uvrgbwh++;
		}
	}
}

static void Act( Work *pWork )
{
	if ( MsgDie( pWork ) < 0 ){
		return;
	}
 	pWork->act( pWork );
}

static	void	Die( Work *pWork )
{
	if( pWork->prim_main != NULL ){
	    GM_FreePrim2( pWork->prim_main );
	}
}

/*----------------------------------------------------------------*/
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int n_prims, int n_verts )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise = RAISE;

	//-------------------------------
	for ( i=0; i < 2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<n_prims; j++ ){
			for ( k=0; k<n_verts; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = COL_R;
				uvrgbwh->g = COL_G;
				uvrgbwh->b = COL_B;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}
}

#define LIGHT05_MSK (762348)
static	int	GetResources( Work *pWork )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	int         i;

	tex = DG_GetTexture( LIGHT05_MSK );
	if ( tex == NULL ){
		SK_Err("prim\0");
		return ( -1 );
	}
	prim = pWork->prim_main = GM_MakePrim2( DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim == NULL ){
#ifdef DEBUG 
		SK_Err("prim\0");
#endif
		return -1;
	}
	InitPacket( pWork , prim, tex, N_PRIMS, N_VERTS );

	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[ i ].vx = 0.0f;
		s_pos[ i ].vy = (float)( i + 1 ) * LIGHT_LEN / (float)( N_VP );
		s_pos[ i ].vz = 0.0f;
	}

	pWork->act = ( void * )NormalAct;

	return (0);
}

/*----------------------------------------------------------------*/
#define	ACTOR_PRIO		(254)

void *NewDemoNktSmoke( int name , FMATRIX *pWorld ) 
{
	Work		*work ;

	work = (Work*)GV_CreateActor( GV_ACTOR_EFFECT , GV_CLASS_OBJECT , sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name = name;
	work->pWorld = pWorld;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}

	return work ;
}

