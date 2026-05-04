//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  demo_bullet.c
  デモ用銃弾

   2000/04/18 T.Morita
   $Id: demo_frmcnt.c,v 1.1.1.3 2002/11/19 11:45:57 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "gameheader.h"

typedef struct work_t
{
    GV_ACT_EX actor  ;
    int name  ;
    int prev  ;
    int frame ;
} Work ;
extern int DG_FrameCount ;

static void Die( Work *work )
{
    DG_FrameCount = work->prev ;
}

static void Act( Work *work )
{
    if ( work->frame )
	work->frame-- ;
    else
	GV_DestroyActor( work ) ;
}

void *NewDemoFrameCountCall( int name, int frame_start, int frame_end )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	work->name = 0 ;
	work->frame = frame_start - frame_end ;
	work->prev = DG_FrameCount ;
	DG_FrameCount = 2 ;
    }
    return work ;
}

void *NewDemoFrameCount( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	work->name = name ;
	work->frame = GCL_GetOptionValue( 'e', 1 ) - GCL_GetOptionValue( 's', 0 ) ;
	work->prev = DG_FrameCount ;
	DG_FrameCount = 2 ;
    }
    return work ;
}
