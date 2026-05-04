//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bul_m4demo.c
	発射されたスティンガー

	2001/04/09 S.Kobayashi
	$Id: bul_m4demo.c,v 1.1.1.3 2002/11/19 11:50:01 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"
#include	"gameheader.h"
#include	"libmt.h"
#include    "../test/etc.h"

#define	BODY_NAME	(12137178)	/* M4_mdl */
#define	BODY_FLAG	(DG_FLAG_SHADE)

#define	LIFE		(160)

#define	N_PRIMS			(1)
#define	N_VERTS			(32)
#define	N_VP			(N_PRIMS * N_VERTS)

extern	TARGET	*PL_LockonTarget ;

typedef	struct	_Work {
	GV_ACT_EX		actor ;
	CONTROL			ctrl;
	OBJECT			body ;
	FVECTOR			rots[9];
	FMATRIX			lights[ 2 ] ;

	FVECTOR         speed;
	int				life ;
	int				flag ;
} Work ;

/*--------------------------------------------------------------------*/

#if 0
static	void	Hit( off, def, ptr )
TARGET			*off, *def ;
void			*ptr ;
{
	Work		*work ;

	work = ( Work * )ptr ;
	work->flag = 1 ;
}
#endif

/*--------------------------------------------------------------------*/
static	void	Act( work )
Work			*work ;
{
	CONTROL		*ctrl ;

	ctrl = &work->ctrl;

	_sceVu0AddVector( &ctrl->mov, &ctrl->mov, &ctrl->step ) ;
	DG_SetPos2( &ctrl->mov, &ctrl->rot );
	DG_GetPos( &work->body.objs->world );
	work->speed.vz += P_GRAVITY; // 重力
	DG_RotVector( &work->speed , &ctrl->step , 1 );

	if ( work->life < 0 ){
		GV_DestroyActor( work );
	} else {
		work->life--;
	}
}

static	void	Die( work )
Work			*work ;
{
	GM_FreeControl( &work->ctrl ) ;
	GM_FreeObject( &work->body ) ;
}

/*--------------------------------------------------------------------*/
static	int		InitControl( work, world )
Work			*work ;
FMATRIX			*world ;
{
	CONTROL		*ctrl ;
	FVECTOR		step ;

	ctrl = &work->ctrl;
	if ( GM_InitControl( ctrl, WP_M4, GM_CurrentMap ) < 0 ){
		return ( -1 );
	}

	DG_SetPos( world );
	DG_PutVector( &DG_ZeroVector , &ctrl->mov, 1 );
    ctrl->rot = ctrl->turn = DG_ZeroSVector;
	DG_SetPos( world );
	DG_RotVector( &work->speed, &step, 1 );
	DG_COPY_VEC( &ctrl->step, &step );

	GV_VecToRot( &step, &ctrl->rot );
	ctrl->rot.vx -= 1024 ;
	/*
	GM_ConfigControlPosition( ctrl, &mov, &rot ) ;
	GM_ConfigControlMapCheck( ctrl ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE | HZX_FLOOR_IK ;
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;	
	*/

	return 0 ;
}

static	int		InitObject( work, world )
Work			*work ;
FMATRIX			*world ;
{
	FVECTOR		*rots;
	OBJECT		*body ;

	body = &work->body;
	GM_InitObject( body, BODY_NAME, BODY_FLAG );
	if ( body->objs == NULL ){
		return ( -1 );
	}
	GM_ConfigObjectJoint( body, work->rots );
	GM_ConfigObjectLight( body, work->lights );
	DG_GetLightMatrix( ( FVECTOR *)world->m[ 3 ] , work->lights );            // 位置からライトマトリックスの計算

	DG_COPY_MAT( &body->objs->world, world );

	rots = work->rots;
	DG_COPY_VEC( rots, &DG_ZeroVector );

	return 0 ;
}

/*
static	void	SetTarget( work )
Work			*work ;
{
	TARGET		*t ;

	t = &work->target ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | 
				  TARGET_POWER | TARGET_THROUGH, GM_CurrentStageMap, 
				  BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
}
*/

static	int		GetResources( work, world, side )
Work			*work ;
FMATRIX			*world ;
int				side;
{
	FVECTOR		from, to;

	work->speed.vx = 0;
	work->speed.vy = -180.0f;
	work->speed.vz = -2.0f;
	if ( InitControl( work, world ) < 0 ){
		return ( -1 );
	}
	GV_MatToVec( world, &to );
	DG_COPY_VEC( &from, &work->ctrl.mov );
	from.vy = to.vy;
	if ( InitObject( work, world ) < 0 ){
		return ( -1 );
	}
	work->life = LIFE ;
	GM_SeSetMode( SD_W_MISSIL01, &work->ctrl.mov, GM_SEMODE_BOMB ) ; 
	GM_SetNoise( NOISE_M, &work->ctrl.mov, work->ctrl.map ) ;
	return 0 ;
}

/*--------------------------------------------------------------------*/
/* M4 */
void		*NewBulletM4_demo( FMATRIX *world , int side ) 
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, side ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
