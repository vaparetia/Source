//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	off_man.c
	各種ＯＦＦ制御

	1999/11/08 S.Okajima
	$Id: off_man.c,v 1.1.1.3 2002/11/19 11:47:08 Yoshizawa1 Exp $

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

#ifdef PSX2
#include	"def_dma.h"
#endif

int	ok_body_shadow_off=0;

typedef	struct	{
	GV_ACT_EX	actor ;
	int		name;

} Work ;

enum {
	REQ_BODY_SHADOW_OFF=0,
	REQ_BODY_SHADOW_ON,
	REQ_NO
};

static void Act( Work *work )
{
	GV_MSG *msg;

	if( GV_ReceiveMessage( work->name, &msg ) ){
		switch( msg->message[0] ){
			case REQ_BODY_SHADOW_OFF:
				ok_body_shadow_off = 1;
			  break;
			case REQ_BODY_SHADOW_ON:
				ok_body_shadow_off = 0;
			  break;
			default:
			  break;
		}
	}


}

static void Die( Work *work )
{
}

static int GetResources( Work *work, int name )
{
	work->name = name;
	return (0);
}


void *NewOffMan( int name )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
