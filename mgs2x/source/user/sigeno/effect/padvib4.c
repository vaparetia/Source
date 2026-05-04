//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	2002/08/20 K.Sigeno
	$Id: padvib4.c,v 1.2 2002/11/25 01:54:45 Yoshizawa1 Exp $

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

extern void *NewPadVibration3( u_char *script, int *ctrl ) ;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	int			flag ;
	int			name ;
} Work ;


/*----------------------------------------------------------------*/

enum {
	KILL=0,
};

static void CheckMesg( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
			case KILL:
				work->flag |= VAR_FLAG_PLAY_STOP ;
				break;
		}
		msg--;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	CheckMesg( work );
	if(work->flag & VAR_FLAG_PLAY_STOP){
		GV_DestroyActor( work ) ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work , int name )
{

	int		file ;
	work->flag = 0 ;
	work->name =name ;
	file = GCL_GetOptionValue( 'v', 0 ) ;
	ASSERT( file != 0 ) ;
	work->flag = VAR_FLAG_PLAYING ;
	GV_SetActorChild( work,NewPadVibration3( GM_FindVibData( file ), &work->flag ) );
	return (0);
}


/* ---------------------------------------------------------------- */
void *NewPadVib4( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		if ( GetResources( work ,name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

