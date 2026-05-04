/*
	wave.c
	    波メイン

	1999/08/10 T.Morita
	$Id: water.c,v 1.1.1.3 2002/11/19 11:46:38 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "water.h"


static void Act( Work *work )
{
    static int flag = 1 ;

//    if ( GV_PadData[0].press & PAD_STA ) flag ^= 1 ;
    if ( flag || GV_PadData[0].press & PAD_L1 )
    {
	WTR_ActAllWater( work ) ;
    }
}

static void Die( Work *work )
{
    int x, z, i ;
    DG_OBJS **o ;

    if ( (o=work->m_wave.objs) )
	for ( z=work->z_blk ; --z>=0 ; )
	    for ( x=work->x_blk ; --x>=0 ; o++ )
		DG_FreeObjs( *o ) ;
    WTR_MemDie() ;
}

static void GetOptionValue( Work *work, int col, int row )
{
    /* initialize wave parameter */
    work->wave0 = 0            ;
    work->speed = WATR_SPEED   ;
    work->center.vx = 0.0f     ;
    work->center.vz = 15000.0f ;
    work->center.vy = 2000.0f  ;
    work->x_blk = work->z_blk = WATR_BLK ;
}

static int GetResources( Work *work )
{
    if ( !WTR_MemInit( work ) )
	return -1 ;
    GetOptionValue( work, WATR_WAVE_COL, WATR_WAVE_ROW ) ;
    if ( !WTR_InitModelsAndPrims( work ) )
	return -1 ;

    return 0 ;
}

void *NewWaterWave( int name, int where )
{
    Work *work ;
    
    if ( (work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) )) != NULL )
    {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	if ( GetResources( work ) )
	    GV_DestroyActor( work ), work = NULL ;
    }
    return (void *)work ;
}
