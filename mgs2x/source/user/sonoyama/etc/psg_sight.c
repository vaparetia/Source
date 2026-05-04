//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   psg_sight.c
   £Ð£Ó£Ç¡Ý£±¥µ¥¤¥È¡Ê²¾¡Ë
   
   2001/04/03	M.Sonoyama
   $Id: psg_sight.c,v 1.1.1.3 2002/11/19 11:50:45 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

#define		SIGHT_NAME	(118471)	/* psg.o2d */

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX			actor ;
	int					handle_2d ;
	int					action_num ;
	int					action ;
	SPR_EMPTY			*root ;
	SPR_POS				adjpos ;
	SPR_POS				adjaim ;
} Work ;

/*----------------------------------------------------------------*/

static	void	CenterAdjust( Work *work )
{
	FVECTOR			to ;
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;
	_sceVu0SubVector( &to, &cam->target, &cam->position ) ;
	GV_LenVec3F( &to, &to, 0.0F, 1000000.0F ) ;
	_sceVu0AddVector( &to, &cam->position, &to ) ;

	if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &cam->position, &to,
							    HZX_CHK_ALL, 0, 0 ) ) {
		FVECTOR		hit, disp ;

		HZX_GetOnlinePoint( &hit ) ;
		DG_TransPersOne( &disp, &hit ) ;
		disp.vy = disp.vy * 384.0F / ( float )DRAW_HEIGHT ;
		work->adjaim.x = disp.vx - ( float )( DRAW_WIDTH / 2 ) ;
		work->adjaim.y = disp.vy - ( float )( 384 / 2 ) ;
	}
	work->adjpos.x = GV_NearExp4F( work->adjpos.x, work->adjaim.x ) ;
	work->adjpos.y = GV_NearExp4F( work->adjpos.y, work->adjaim.y ) ;
	SPR_SetPosEmpty( ( SPR_OBJ * )work->root, &work->adjpos ) ;
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	if ( L2D_EvokeActionByNumber( work->handle_2d, work->action ) < 0 ) {
		CenterAdjust( work ) ;
		return ;
	} else if ( work->action < work->action_num ) {
		work->action ++ ;
	} else {
		L2D_BreakAction( work->handle_2d ) ;
	}
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ) {
		L2D_ReleaseLayout( work->handle_2d ) ;
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int			handle ;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( SIGHT_NAME, DG_CHANL_MENU, 0, 0 ) ;
#ifdef DEBUG_MODE
	if ( handle < 0 ) {
		printf( "sight data not found\n" ) ;
	}
	printf( "handle %d\n", handle ) ;
#endif
	work->handle_2d = handle ;
	work->action_num = L2D_GetActionNumber( handle ) ;
	work->action = 0 ;

	work->root = ( SPR_EMPTY * )L2D_GetObject( handle, GV_StrCode( "ROOT" ) ) ;
	if ( work->root == NULL ) printf( "????????????????" ) ;

	work->adjpos.x = 0.0F ;
	work->adjpos.y = 0.0F ;
	work->adjaim.x = 0.0F ;
	work->adjaim.y = 0.0F ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewPsgSight( int mode ) 
{
	Work			*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER, sizeof( Work ), 0 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
