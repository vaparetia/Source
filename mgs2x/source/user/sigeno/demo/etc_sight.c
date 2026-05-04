//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	etc_sight.c
	サイト呼び出し
	2002/08/22 K.Sigeno
	$Id: etc_sight.c,v 1.2 2002/11/25 01:49:05 Yoshizawa1 Exp $
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include "gameheader.h"
#include "libutl.h"

extern void *NewRaySight( void ) ;
typedef	struct _Work {
	GV_ACT_EX	actor ;
} Work ;

static void Act(Work *work)
{
}
static void Die(Work *work)
{
}
static int GetResources( work,name,where )
Work *work ;
int	name ;
int	where ;
{
	GV_SetActorChild( &(work->actor) ,NewRaySight());
	GV_SetActorMessageKill( &(work->actor), name );
	return 1;
}
void *NewSightEtc( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

