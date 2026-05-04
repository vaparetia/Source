//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_dummy.c
   ダミープレイヤー

   2001/05/15	M.Sonoyama
   $Id: pl_dummy.c,v 1.1.1.3 2002/11/19 11:50:58 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

typedef	struct	{
	GV_ACT			actor ;
	OBJECT			body ;
	CONTROL_NOEVT	control ;
} Work ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{

}

static	void	Die( Work *work )
{
	GM_FreeControl( ( CONTROL * )&work->control ) ;
	GM_FreeObject( &work->body ) ;

	GM_PlayerControl = NULL ;
	GM_PlayerBody = NULL ;
}

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work, int name )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;

	ctrl = ( CONTROL * )&work->control ;
	GM_InitControlEx( ctrl, 10163495, 0, CTRL_ATR_NOCHK_HZX ) ; /* ライデン */
	ctrl->name = name ;
	PL_GetOptionFV( 'p', &ctrl->mov ) ;
	GM_ConfigControlMapID( ctrl ) ;

	body = &work->body ;
	GM_InitObject( body, GCL_GetOptionValue( 'm', GV_StrCode( "rai_def" ) ), 
				   DG_FLAG_SHADE ) ;
	DG_InvisibleObjs( body->objs ) ;

	/* プレイヤー変数のセット */
	GM_PlayerControl = ctrl ;
	GM_PlayerBody = body ;
	GM_PlayerAddress = ctrl->addr ;
	GM_PlayerMap = ctrl->map ;
	GM_PlayerPosition = ctrl->mov ;
	GM_PlayerPosX = ctrl->mov.vx ;
	GM_PlayerPosY = ctrl->mov.vy ;
	GM_PlayerPosZ = ctrl->mov.vz ;
	GM_PlayerDir = 0 ;
	GM_PlayerMotion = 0 ;
	GM_PlayerStance = 0 ;
	GM_PlayerFindPos = ctrl->mov ;

	/* ＡＣＴはなにもしない */
	GV_SleepActor( &work->actor, GV_CLASS_WAITING ) ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewDummyPlayer( int name, int where )
{
	Work		*work ;

	ASSERT( GM_PlayerControl == NULL ) ;
	
	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_BODY_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
