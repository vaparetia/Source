//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_locker.c
  ロッカーの中のエマ

  2001/07/11 T.Morita Revised
  $Id: emma_locker.c,v 1.1.1.3 2002/11/19 11:46:04 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"libutl.h"
#include	"gameheader.h"

typedef struct
{
    GV_ACT  actor ;
    FVECTOR pos   ;
    int     se[3] ;
    int     n_se  ;
    int     time  ;

    short   heart ;
    short   heart_tic ;
    short   heart_cnt ;
} Work ;

#define HEARD_RAD      2000.0f*2000.0f
#define HEARD_RAD_BOMB 6000.0f*6000.0f

#define HEART_BEAT_NORMAL 120
#define HEART_BEAT_FAST 60

static void EMA_ActHeartBeat( Work *work )
{
    /* 心臓の音 */
    if ( PL_GetPlayerWeapon() == WP_Mic )
    {
	if ( work->heart_cnt <= 0 )
	    GM_SeSetMode( SD_E_EHEART02, &work->pos, GM_SEMODE_MIC ) ;
    }

    if ( --work->heart_cnt < 0 )
	work->heart_cnt = work->heart < HEART_BEAT_FAST ?
	    HEART_BEAT_FAST : work->heart ;

    if ( --work->heart_tic<=0 )
    {
	work->heart_tic = 60/TIME_BASE ;
	if ( ++work->heart > HEART_BEAT_NORMAL )
	    work->heart = HEART_BEAT_NORMAL ;
    }
}

static void Act( Work *work )
{
    FVECTOR l ;
    float rad ;

    EMA_ActHeartBeat( work ) ;

    if ( work->time > 0 )
    {
	work->time-- ;
	return ;
    }
    if ( GM_NoisePower == NOISE_ZERO )
	return ;
    else if ( GM_NoisePower <= NOISE_M )
	rad = HEARD_RAD ; 
    else 
	rad = HEARD_RAD_BOMB ; 

    /* 音との距離を見る */
    _sceVu0SubVector( &l, &GM_NoisePosition, &work->pos ) ;
    if ( _sceVu0InnerProduct( &l, &l ) > rad )
	return ;

    work->heart = 0 ;
    work->time = 60*2*5/TIME_BASE ;
    GM_SeSetMode( work->se[irnd()%work->n_se], &work->pos, GM_SEMODE_BOMB ) ;
}


static int GetResources( Work *work, int name, int where )
{
    work->heart = HEART_BEAT_NORMAL ;
    work->heart_tic= 0  ;

    if ( GCL_GetOption( 'p' ) )
    {
	work->pos.vx = (float)GCL_GetNextInt() ;
	work->pos.vy = (float)GCL_GetNextInt() ;
	work->pos.vz = (float)GCL_GetNextInt() ;
    }
    work->se[0] = SD_V_EMAAFR01 ;
    work->n_se = 1 ;
    if ( GCL_GetOption( 'v' ) )
    {
	work->se[0] = GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->se[work->n_se++] = GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->se[work->n_se++] = GCL_GetNextInt() ;
    }

    return 0 ;
}


void *NewEmmaLocker( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) 
    {
	GV_SetActor( &work->actor, Act, NULL ) ;
	if ( GetResources( work, name, where ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
