//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cb_splash.c
	段ボール水飛沫

	2000/10/11 S.Okajima
	$Id: cb_splash.c,v 1.1.1.3 2002/11/19 11:47:03 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
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
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"


extern int ok_flush_status;

#define	N_PRIMS		(2)
#define	N_VERTS		(32)

#define	P_RGB_MAX		(80)
#define	ALPHA			(50)

#define	SIZE			(80)
#define	RAND_Y			(100)
#define	SHIFT_UP		(200)
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;
	int			map;

	int			*flag;
	int			pat;
	FMATRIX		*world;
	FVECTOR		size;

	DG_PRIM2	*prim ;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i,j;
	int		clock;
	FVECTOR	*pos;

	if( (ok_flush_status!=0)
	 || (*work->flag & 2)
	 || (work->pat==0 &&  GM_CheckPlayerStatus(PLAYER_CB_BOX_CANCELED) )
	 || (work->pat==1 && !GM_CheckPlayerStatus(PLAYER_CB_BOX_CANCELED) )
	 || GM_CheckPlayerStatus(PLAYER_WATCH)
	  ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos     = work->prim->pos[clock];
	for(i=0; i<N_PRIMS; i++){
		for ( j = 0 ; j < N_VERTS ; j++ ){
			pos->vx = work->size.vx*frnd();
			pos->vy = work->size.vy + RAND_Y*rnd();
			pos->vz = work->size.vz*frnd() - work->size.vw;
			pos++;
		}
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = SIZE ;
			uvrgbwh0->h = uvrgbwh1->h = SIZE ;

			uvrgbwh0->r = uvrgbwh1->r = P_RGB_MAX ;
			uvrgbwh0->g = uvrgbwh1->g = P_RGB_MAX ;
			uvrgbwh0->b = uvrgbwh1->b = P_RGB_MAX ;
			uvrgbwh0->a = uvrgbwh1->a = ALPHA ;

			uvrgbwh0++;	uvrgbwh1++;
		}
	}
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	tex = DG_GetTexture( 7733153 /*"splash06_alp"*/ );
	prim = work->prim = GM_MakePrim2(  DG_PRIM2_SPRT
	                                  |DG_PRIM2_TEX
	                                  |DG_PRIM2_ALPHA,
	                                  N_PRIMS, N_VERTS );
	if( prim==NULL ) return -1;
	InitPacket2( prim, tex );
	DG_VisiblePrim2( prim );

	prim->root = work->world;

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewCBoxSplash( FMATRIX *world, float size_x, float size_z, float size_h, float shift_z, int *flag, int pat )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;
		work->size.vx = size_x * 0.5f  + SIZE;
		work->size.vy = size_h * 0.5f  + SHIFT_UP;
		work->size.vz = size_z * 0.5f  + SIZE;
		work->size.vw = shift_z;
		work->flag = flag;
		work->pat = pat;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
