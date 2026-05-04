//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splash_rot.c
	段ボール水飛沫

	2000/10/18 S.Okajima
	$Id: splash_rot.c,v 1.1.1.3 2002/11/19 11:47:14 Yoshizawa1 Exp $
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

#define	N_VERTS		(32)

#define	P_RGB_MAX		(80)
#define	ALPHA			(50)

#define	SIZE			(80)
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;
	int			map;

	FMATRIX		world;
	FVECTOR		width;

	int			max_prims;
	DG_PRIM2	*prim ;

	int			invisible_flag;

} Work ;
/* ---------------------------------------------------------------- */
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

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
		  case REQ_KILL:
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i;
	int		clock;
	FVECTOR	*pos;
	FVECTOR	*pos_mem;

	CheckMesgParam( work );

	if( ok_flush_status!=0 || work->invisible_flag ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	GM_GroupPrim2( work->prim, work->map ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = pos_mem = work->prim->pos[clock];
	for(i=0; i<work->max_prims*N_VERTS; i++){
		pos->vx = work->width.vx*frnd();
		pos->vy = work->width.vy* rnd();
		pos->vz = work->width.vz*frnd();
		pos++;
	}
	DG_SetPos( &work->world );
	DG_PutVector( pos_mem, pos_mem, work->max_prims*N_VERTS );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( DG_PRIM2 *prim, DG_TEX *tex, int max_prims )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < max_prims ; j++ ){
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

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * max_prims * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * max_prims * N_VERTS );

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		center;
	SVECTOR		rot;

	work->invisible_flag = 0;

	work->max_prims = 1;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->max_prims = GCL_GetNextInt();
	}

	if ( GCL_GetOption( 'p' ) != NULL ) {
		center.vx = (float)GCL_GetNextInt();
		center.vy = (float)GCL_GetNextInt();
		center.vz = (float)GCL_GetNextInt();
	}else{
		ASSERT(0);
	}

	if ( GCL_GetOption( 'r' ) != NULL ) {
		rot.vx = (short)GCL_GetNextInt();
		rot.vy = (short)GCL_GetNextInt();
		rot.vz = 0;
	}else{
		ASSERT(0);
	}

	DG_SetPos2( &center, &rot );
	DG_GetPos( &work->world );

	if ( GCL_GetOption( 'w' ) != NULL ) {
		work->width.vx = (float)GCL_GetNextInt();
		work->width.vy = (float)GCL_GetNextInt();
		work->width.vz = (float)GCL_GetNextInt();
	}else{
		ASSERT(0);
	}


	tex = DG_GetTexture( 7733153 /*"splash06_alp"*/ );
	prim = work->prim = GM_MakePrim2(  DG_PRIM2_SPRT
	                                  |DG_PRIM2_TEX
	                                  |DG_PRIM2_ALPHA,
	                                  work->max_prims, N_VERTS );
	if( prim==NULL ) return -1;
	InitPacket2( prim, tex, work->max_prims );
	DG_VisiblePrim2( prim );


	return (0);
}

/* ---------------------------------------------------------------- */
void *NewSplashRotate( int name, int map )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		GV_SetActorMessageKill( work, name );

		work->name = name;
		work->map  = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
