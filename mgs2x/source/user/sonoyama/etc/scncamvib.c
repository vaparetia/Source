//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   scncamvib.c
   シナリオカメラ振動

   2001/07/16	M.Sonoyama
   $Id: scncamvib.c,v 1.1.1.3 2002/11/19 11:50:45 Yoshizawa1 Exp $
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
#include	"rand.h"

typedef struct	{
	GV_ACT_EX		actor ;
	int				time ;
	int				xlevel ;
	int				ylevel ;
	int				mode ;
} Work ;

static	void	Act( Work *work )
{
	FVECTOR		adj ;

	if ( -- work->time <= 0 ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	if ( work->xlevel > 0 ) adj.vx = ( float )( ( irnd() % work->xlevel ) ) ;
	else					adj.vx = 0.0F ;
	if ( work->ylevel > 0 ) adj.vy = ( float )( ( irnd() % work->ylevel ) ) ;
	else				    adj.vy = 0.0F ;
	GM_SetCameraAdjust( 0, &adj ) ;

	if ( work->mode == 1 ) {
		work->xlevel -- ;
		work->ylevel -- ;
	}
}

void	*PL_CHARA_ScnVibCamera( int name, int where )
{
	Work	*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( Work ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	work->time = GCL_GetOptionValue( 't', 0 ) ;
	work->xlevel = GCL_GetOptionValue( 'x', 0 ) ;
	work->ylevel = GCL_GetOptionValue( 'y', 0 ) ;
	work->mode = GCL_GetOptionValue( 'm', 0 ) ;
	return work ;
}

/* プログラム呼び */
void	*NewVibrateCamera( int time, int xlevel, int ylevel, int mode )
{
	Work	*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( Work ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	work->time = time ;
	work->xlevel = xlevel ;
	work->ylevel = ylevel ;
	work->mode = mode ;
	return work ;
}
