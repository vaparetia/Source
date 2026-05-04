//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_speech.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_speech.c,v 1.1.1.3 2002/11/19 11:45:46 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_speech.h"


static void Die( Work *work )
{
    if ( work->piece_s )
	DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
	DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->objs )
    {
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
}

static void Act( Work *work )
{
    if ( !(work->n_piece & BRK_SPC_INACTIVE) )
	BRK_SPC_ActPieces( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_SPC_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_SPC_InitTarget( work, where ) )
	return -1 ;
    if ( BRK_SPC_InitHazard( work, where ) )
	return -1 ;
    if ( BRK_SPC_InitPieces( work, where ) )
	return -1 ;
    return 0 ;
}

void *NewPutSpeechObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
