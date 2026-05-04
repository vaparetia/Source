//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scnvapor.c
	シナリオ呼び湯気
	2002/08/30 K.Sigeno
	$Id: scnvapor.c,v 1.2 2002/11/25 01:54:45 Yoshizawa1 Exp $
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

extern void *NewVapor( FVECTOR *center, float radius, int col, int life ) ;

#define VAPOR_CYCLE	(180)
#define COL_R (80 << 24)
#define COL_G (80 << 16)
#define COL_B (80 << 8)
#define COL_A (24)

#define VAPOR_COL	(COL_R|COL_G|COL_B|COL_A)
typedef	struct _Work {
	GV_ACT_EX	actor ;
	FVECTOR		pos ;
	float		radius ;
	int			cnt ;
} Work ;

static void Act(Work *work)
{
	if(work->cnt == 0 ){
		GV_SetActorChild( &(work->actor) ,NewVapor( &work->pos,work->radius, (int) VAPOR_COL, (VAPOR_CYCLE*5)/6 )) ;
	}
	work->cnt++ ;
	if(work->cnt >= VAPOR_CYCLE ){
		work->cnt = 0 ;
	}

}
static void Die(Work *work)
{
}
static int GetResources( work,name,where )
Work *work ;
int	name ;
int	where ;
{
//	GV_SetActorChild( &(work->actor) ,NewRaySight());
	if ( GCL_GetOption( 'p' ) != NULL ){
		work->pos.vx = (float)GCL_GetNextInt() ;
		work->pos.vy = (float)GCL_GetNextInt() ;
		work->pos.vz = (float)GCL_GetNextInt() ;
		work->pos.vw = 1.0f ;
	}else {
		return 0 ;
	}
	if ( GCL_GetOption( 'r' ) != NULL ){
		work->radius = (float)GCL_GetNextInt() ;
	}else {
		return 0 ;
	}
	work->cnt = 0 ;
	GV_SetActorMessageKill( &(work->actor), name );
	return 1;
}
void *NewScnVapor( name , where )
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

