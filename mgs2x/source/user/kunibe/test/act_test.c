//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    water_con.c
	水面のコントロール
	2000/01/05 T.Shibata
	
	$Id: act_test.c,v 1.1.1.3 2002/11/19 11:44:54 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

typedef	struct	{
	GV_ACT_EX	actor;
	int		name;
	int		map;
    int life;
} Work;

static void Act( Work *work )
{
    printf("life %d\n",work->life);
    if(--work->life < 0){
	GV_DestroyActor( work );
    }
}

static void Die(Work *work )
{
    printf("Die\n");
}

static int GetResources( Work *work )
{
    work->life = GCL_GetOptionValue( 'l', -1 );

    if( work->life < 0 ) return -1;
    return 0;
}

void *NewActTest( int name, int map )
{
	Work		*work ;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ));
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return (void *)work ;
}
