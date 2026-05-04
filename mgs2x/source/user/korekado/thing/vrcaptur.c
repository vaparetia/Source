//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vrcaptur.c
	skoba/equip/capture.c のＶＲ版

	$Id: vrcaptur.c,v 1.1.1.3 2002/11/19 11:44:28 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
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

/* ---------------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX actor;

	int time;
} Work;

/* ---------------------------------------------------------------------- */
#define FREE_TIME (60)

static void Act( Work *work )
{
	int i ;

	if ( work->time++ > FREE_TIME ) {
		GV_DestroyActor(work);
	}
}

static void Die( Work *work )
{
	GM_SdSet( SNG_PAUSEOFF ); // 演説ならす
	GV_PauseOffActorSystem( GV_PAUSE_STOP );
}

static int GetResources(Work *work)
{
	int i ;
	GM_SdSet( SNG_PAUSEON ); // 演説とめる

	/* ワークの初期化 */
	work->time=0;

	GV_PauseOnActorSystem( GV_PAUSE_STOP );

	return ( 0 );
}

/* ---------------------------------------------------------------------- */
void *NewVrCaptureStart( )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0xFF ) ;
	if ( work != NULL ) {
		GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL );
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
