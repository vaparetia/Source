//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	slowdown.c

	2000/11/01 S.Okajima
	$Id: slowdown.c,v 1.1.1.3 2002/11/19 11:47:13 Yoshizawa1 Exp $

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


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;

	int			life;
	int			time;
} Work ;

void *OK_SLOW_DOWN_WORK = NULL;

/*----------------------------------------------------------------*/
#define	DFAULT_FRAMECOUNT 0

enum {
	PARAM=0,
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
		  case PARAM:

			work->time = msg->message[1];
//			work->life = msg->message[2];

			work->time = ( work->time < DFAULT_FRAMECOUNT )?DFAULT_FRAMECOUNT: work->time;

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
	if( GM_CheckGameStatus(STATE_GAMEOVER) ){
		DG_FrameCount = DFAULT_FRAMECOUNT;
		return;
	}


	CheckMesgParam( work );

	DG_FrameCount = work->time;
	if( work->life >= 0 ){
		if( work->life-- < 0 ) GV_DestroyActor( work ) ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	OK_SLOW_DOWN_WORK = NULL;
	DG_FrameCount = DFAULT_FRAMECOUNT;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->life = GCL_GetNextInt();
	}else{
		return -1;
	}
	if ( GCL_GetOption( 't' ) != NULL ){
		work->time = GCL_GetNextInt();
	}else{
		return -1;
	}
	return (0);
}


/* ---------------------------------------------------------------- */
void *NewSlowDown( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_SLOW_DOWN_WORK != NULL ) return OK_SLOW_DOWN_WORK;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		OK_SLOW_DOWN_WORK = work;
		work->name = name;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

