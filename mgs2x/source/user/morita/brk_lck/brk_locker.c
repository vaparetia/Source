/*
   brk_locker.c
   ロッカー壊れ

   1999/12/08 T.Morita
   $Id: brk_locker.c,v 1.1.1.3 2002/11/19 11:45:33 Yoshizawa1 Exp $
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

#include "brk_locker.h"


static void Die( Work *work )
{
    int  i ;

    for ( i=work->obj->n_models ; --i>=0 ; )
	DG_FreeAnimVertsBuffer( &work->doors[i].v_anm ) ;
    GV_Free( work->doors ) ;
    GV_Free( work->child ) ;

    DG_DequeueObjs( work->obj ) ;
    DG_FreeObjs( work->obj );
}

static void Act( Work *work )
{
    static int flag = 0 ;
    if ( GV_PadData[0].press & PAD_L1 )
	flag ^= 1 ;
    if ( flag )
	DG_InvisiblePrim( work->dbg ) ;
    else
	DG_VisiblePrim( work->dbg ) ;
}

static int GetResources( Work *work, int name, int where )
{
    if ( BRK_LCK_GetOptions( work ) )
	return -1 ;
    if ( BRK_LCK_InitTarget( work ) )
	return -1 ;
    if ( BRK_LCK_SetupPolydata( work ) )
	return -1 ;
    if ( BRK_LCK_InitDbgPrimitive( work ) )
	return -1 ;
    return 0 ;
}

void *NewPutLockerObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}


