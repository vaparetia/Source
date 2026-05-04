//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	2002/08/20 K.Sigeno
	$Id: dm_fr.c,v 1.2 2002/11/25 01:54:45 Yoshizawa1 Exp $

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
	int			time;
	int			life ;
	int			name ;
} Work ;

extern	int	DM_FrameSkip ;

/*----------------------------------------------------------------*/
#define	DFAULT_FRAMECOUNT 0

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
			work->life = 0;
			break;
		}
		msg--;
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	CheckMesg( work );

	if(DG_FrameCount>0) {
		DM_FrameSkip = (DG_FrameCount-1) ;
	}else {
		DM_FrameSkip = (DG_FrameCount) ;
	}
	if( work->life >= 0 ){
		if( work->life-- < 0 ) GV_DestroyActor( work ) ;
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DM_FrameSkip = DFAULT_FRAMECOUNT;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->life = GCL_GetNextInt();
	}else{
		work->life = -1;
	}
	return (0);
}


/* ---------------------------------------------------------------- */
void *NewDM_FrameCheck( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

