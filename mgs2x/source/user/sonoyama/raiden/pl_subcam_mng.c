//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_subcam_mng.c
   主観カメラＯＮ／ＯＦＦ制御

   2001/08/13	M.Sonoyama
   $Id: pl_subcam_mng.c,v 1.1.1.3 2002/11/19 11:50:59 Yoshizawa1 Exp $
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

typedef	struct	{
	GV_ACT_EX		actor ;
	GM_CameraSet	*subject ;
	GM_CameraSet	*weapon ;
	int				order ;
	int				next ;

	int             mode ;// T.Morita Added   1:Submode 0:Normal
} Work ;

static	Work	*SubcamMngWork ;

static	void	Act( Work *work )
{
	if ( GV_PauseLevel != 0 ) return ;
	//printf( "[%d] %d %d %d\n", GV_Time, work->next, work->subject->on, work->weapon->on ) ;
	if ( work->next == 0 ) {
		/* ＯＦＦだけここで制御 */
		if ( work->subject->on == 1 ||
			 work->weapon->on == 1 ) {
			work->subject->on = 0 ;
			work->weapon->on = 0 ;
			GM_ChangeCamera( 0 ) ;
		}
	} 
	GV_SleepActor( work, GV_CLASS_WAITING ) ;
}

void		*PL_SubjectCameraManager( GM_CameraSet *subject, GM_CameraSet *weapon, int mode )
{
	Work	*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_SUBJECTCAM_ONOFF_PRIO ) ;
	if ( work == NULL ) ASSERT( 0 ) ;
	GV_SetActor( work, Act, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	work->subject = subject ;
	work->weapon = weapon ;
	work->next = subject->on || mode ;
	work->mode = mode ;
	SubcamMngWork = work ;
	//printf( "[%d]-- subcam mng start !! %d %d %d\n", GV_Time, work->next,	
			  //work->subject->on, work->weapon->on ) ;
	return work ;
}

void   		PL_SubjectCameraOn( void )
{
	if ( SubcamMngWork->mode ) return ;//主観ライデンのときは無視する

	//printf( "---------- subcam on %d \n", GV_Time ) ;
	if ( SubcamMngWork == NULL ) return ;
	SubcamMngWork->next = 1 ;
	GV_WakeupActor( SubcamMngWork, GV_CLASS_WAITING ) ;
}

void   		PL_SubjectCameraOff( void )
{
	if ( SubcamMngWork->mode ) return ;//主観ライデンのときは無視する

	//printf( "----------subcam off %d\n", GV_Time ) ;
	if ( SubcamMngWork == NULL ) return ;
	SubcamMngWork->next = 0 ;
	GV_WakeupActor( SubcamMngWork, GV_CLASS_WAITING ) ;
}
