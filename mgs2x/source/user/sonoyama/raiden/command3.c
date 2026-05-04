//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   command3.c
   プレイヤー／シナリオリンク関数（その３）
   
   2001/04/17 M.Sonoyama
   $Id: command3.c,v 1.1.1.3 2002/11/19 11:50:54 Yoshizawa1 Exp $
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
#include	"camera.h"

#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_inline.h"

/* 主観カメラ乗っ取り */
typedef	struct	{
	GV_ACT_EX		actor ;
	FVECTOR			target ;
	SVECTOR			rotate ;
	int				flag ; 
} OSC_Work ;

static	void	OSC_Act( OSC_Work *work )
{
	GM_CameraSet	*cam ;

	if ( GM_PlayerWork != NULL && 
		 GM_PlayerWork->subject_camera->on != 0 ) {
		cam = GM_PlayerWork->subject_camera ;
		if ( GM_PlayerWork->weapon_camera->on != 0 ) {
			cam = GM_PlayerWork->weapon_camera ;
		}
		if ( work->flag & 1 ) {
			DG_COPY_VEC( &cam->target, &work->target ) ;
			GM_CameraMakeRotate( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;
		} else if ( work->flag & 2 ) {
			cam->rotate = work->rotate ;
			GM_CameraMakeTarget( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;
		}
		GM_CameraDir = GM_PlayerWork->camdir = cam->rotate ;
		if ( !Flag( FLAG_SUBJECT_HORIZON_LIMIT ) ) {
			GM_PlayerWork->control.turn.vy 
				= GM_PlayerWork->control.rot.vy 
					= cam->rotate.vy ;
		}
	}
	GV_DestroyActor( work ) ;
}

static	void	OSC_Die( OSC_Work *work )
{

}

int			PL_COM_OverrideSubjectCamera( void )
{
	OSC_Work		*work ;

	ASSERT( GM_PlayerWork != NULL ) ;
	work = ( OSC_Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, sizeof( OSC_Work ),
										 PLAYER_ARM_ACTOR_PRIO - 1 ) ;
	if ( work == NULL ) return 0 ;
	GV_SetActor( &work->actor, OSC_Act, OSC_Die ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( GM_PlayerWork, &work->actor ) ;
	if ( PL_GetOptionFV( 't', &work->target ) ) work->flag |= 1 ;
	else if ( PL_GetOptionSV( 'r', &work->rotate ) ) work->flag |= 2 ;
	return 1 ;
}

