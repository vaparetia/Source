//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_vending.c 
   プット自動販売機

   1999/12/26 T.Morita
   $Id: brk_vending.c,v 1.1.1.3 2002/11/19 11:45:52 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "brk_vending.h"


static void Die( Work *work )
{
    int i ;

    if ( work->vender )
    {
	for ( i=work->n_vender ; --i>=0 ; )
	    if ( work->vender[i].objs )
	    {
		GM_FreeTarget( &work->vender[i].target ) ;
		DG_DequeueObjs( work->vender[i].objs ) ;
		DG_FreeObjs( work->vender[i].objs ) ;
	    }
	GV_Free( work->vender ) ;
    }
    if ( work->can )
    {
	work->n_can &= ~BRK_VND_INACTIVE ;
	for ( i=work->n_can ; --i>=0 ; )
	    if ( work->can[i].objs )
	    {
		GM_FreeTarget( &work->can[i].target ) ;
		DG_DequeueObjs( work->can[i].objs ) ;
		DG_FreeObjs( work->can[i].objs ) ;
	    }
	GV_Free( work->can ) ;
    }
}

static void Act( Work *work ) 
{
    if ( !(work->n_can & BRK_VND_INACTIVE) ) 
	BRK_VND_ActCan( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name    = name  ;
    work->where   = where ;

    if ( BRK_VND_InitHazard( work ) < 0 )
	return -1 ;

    if ( BRK_VND_GetOptions( work, where ) < 0 )
	return -1 ;
    if ( BRK_VND_InitCan( work, where ) < 0 )
	return -1 ;
    if ( BRK_VND_InitVending( work, where ) < 0 )
	return -1 ;

    return 0 ;
}

void *NewVendingMachine( int name, int where )
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
