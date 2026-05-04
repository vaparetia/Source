//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   command4.c
   プレイヤー／シナリオリンク関数（その４）
   
   2001/06/13 M.Sonoyama
   $Id: command4.c,v 1.1.1.3 2002/11/19 11:50:54 Yoshizawa1 Exp $
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

#define	MSG_ENTER	(6753643)
#define	MSG_LEAVE	(6411627)
#define	MSG_INSIDE	(5547371)

typedef	struct	{
	GV_ACT			actor ;
	int				flag ;
	int				seNo ;
} Work ;

static	void	Act( Work *work )
{
	GM_CameraSet	*now, *next ;
	float			len ;

	if ( GM_PlayerWork == NULL ||
		 !Flag( FLAG_SUBJECTCAMERA_ADJUST ) ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	if ( !GM_CheckPlayerStatus( PLAYER_WATCH ) ||
		 GM_PlayerWork->action2 != NULL ) {
		work->flag = 0 ;
		return ;
	}

	now = GM_GetCurrentCamera( 0 ) ;
	next = GM_GetNextCamera( 0 ) ;
	len = GV_VecLen3F2( &now->position, &next->position ) ;

	GM_SubjectVMaxTmp[ 0 ] = 1 ;

	if ( work->flag == 0 ) {
		if ( len < 160.0F ) {
			GM_SeSetMode( work->seNo, &next->position, GM_SEMODE_BOMB ) ;
			work->flag = 1 ;
		}
	} else {
		if ( len > 320.0F ) {
			//work->flag = 0 ;
		}
	}
}	

/* イントルード主観金網寄り設定 */
int		PL_COM_IntrudeSubjectCameraPositionMove( void )
{
	float		len ;
	int			inout, on = 0 ;
	int			seNo ;

	if ( GM_PlayerWork == NULL ) return 0 ;

	inout = GCL_GetOptionValue( 'i', 0 ) ;
	if ( inout == 1 || inout == MSG_ENTER ) on = 1 ;
	else if ( inout == 0 || inout == MSG_LEAVE ) on = 0 ;
	else return 0 ;	/* 「いる」 */

	if ( on == 0 ) {
		UnsetFlag( FLAG_SUBJECTCAMERA_ADJUST ) ;
		DG_COPY_VEC( &PL_SubjectCameraShift, &DG_ZeroVector ) ;
		return 0 ;
	}

	len = ( float )GCL_GetOptionValue( 'l', 1000 ) ;
	seNo = GCL_GetOptionValue( 's', 0 ) ;

	if ( !Flag( FLAG_SUBJECTCAMERA_ADJUST ) &&
		 GM_CheckPlayerStatus( PLAYER_INTRUDE ) ) {
		SetFlag( FLAG_SUBJECTCAMERA_ADJUST ) ;
		PL_SubjectCameraShift.vz = len ;
		if ( seNo != 0 ) {
			Work		*work ;

			work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
			if ( work != NULL ) {
				GV_SetActor( work, Act, NULL ) ;
				work->seNo = seNo ;
			}
		}
	} else if ( Flag( FLAG_SUBJECTCAMERA_ADJUST ) ) {
		UnsetFlag( FLAG_SUBJECTCAMERA_ADJUST ) ;
	}
	return 0 ;
}

