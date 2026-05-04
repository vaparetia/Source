//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	disparc.c
	範囲表示エフェクト

	2001/05/01 K.Takabe
	$Id: disparc.c,v 1.1.1.3 2002/11/19 11:51:19 Yoshizawa1 Exp $

*/
/*



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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
#define PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_ALPHA)
#define N_PRIMS		(1)
#define N_VERTS		(64)


/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;

	DG_PRIM2	*prim ;
	FMATRIX		*parent ;
} Work ;


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_DestroyActor( work );
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *world, int min_len, int max_len, int min_angle, int max_angle, int col )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		*pos ;
	int			i, j, k ;

	work->map = GM_CurrentMap ;

	work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world = *world ;
	//prim->flag |= DG_PRIM2_INVISIBLE ;

	for ( k = 0 ; k < 2 ; k++ ){

		/* 頂点の設定 */
		pos = prim->pos[ k ] ;
		for ( i = 0 ;  i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j++, pos++ ){
				float	angle, len ;
				if ( j & 1 ){
					len = max_len ;
				} else {
					len = min_len ;
				}
				angle = ( max_angle - min_angle ) * ( j / 2 ) / 31.0f + min_angle ;
				angle = angle / 2048.0f * (float)M_PI ;
				pos->vy = 0.0f ;
				pos->vx = len * sinf( angle ) ;
				pos->vz = len * cosf( angle ) ;
				pos->vw = 1.0f ;
			}
		}

		/* ＵＶ、ＲＧＢ値の設定 */
		uvrgb = prim->uvrgb[ k ];
		for ( i = 0 ;  i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j++, uvrgb++ ){
				uvrgb->r = col & 0xff ;
				uvrgb->g = ( col >> 8 ) & 0xff ;
				uvrgb->b = ( col >> 16 ) & 0xff ;
				uvrgb->a = ( col >> 24 ) & 0xff ;
				//uvrgb->r = 255 ;
				//uvrgb->g = 0 ;
				//uvrgb->b = 0 ;
				//uvrgb->a = 64 ;
				if ( j == 0 ){
					uvrgb->f = 0x8fff ;	/* 最初の頂点は描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* それ以降は描画キックを行う */
				}
			}
		}
	}
	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewDebugArcDisp( FMATRIX *pos, int min_len, int max_len, int min_angle, int max_angle, int col )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, min_len, max_len, min_angle, max_angle, col ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

