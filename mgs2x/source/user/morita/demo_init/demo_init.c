//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_init.c
   ARKMS専用 キャラ

   2000/03/05 T. Morita
   $Id: demo_init.c,v 1.1.1.3 2002/11/19 11:45:58 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "../../mode/demo/libdemo.h"
#include "../include/util.h"


typedef struct work_t
{
    GV_ACT_EX   actor   ;

    int name ;
} Work ;

void DM_EffectInitCommand( int *buffer, int num )
{
    switch( buffer[0] )
    {
    case 0:
	DM_EffectInitFlag |= 1 ;
    }
    printf( "DM_EffectInitCommand() %d\n", DM_EffectInitFlag ) ;
}

static void Act( Work *work )
{
}

static void Die( Work *work )
{
    DM_EffectInitName = 0 ;
}

static int GetResources( Work *work, int name )
{
    if ( DM_EffectInitName )
	PERROR( "Second NewDemoEffectInitialize() maybe lanched.\n" ) ;

    work->name        = name ;
    DM_EffectInitName = name ;

    return 0 ;
}

void *NewDemoEffectInitialize( int name )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, name ) < 0 )
        {
	    ASSERT( 0 ) ; /* 必ず落ちてもらう（「システム エフェクト初期化」が2つ以上置かれた） */
        }
    }
    return work ;
}
