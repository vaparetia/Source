//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_monitor.c
   制御室モニター壊れ

   1999/12/13 T. Morita
   $Id: brk_monitor.c,v 1.1.1.3 2002/11/19 11:45:36 Yoshizawa1 Exp $
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

#define __MAIN_FILE__
#include "brk_monitor.h"


static void Die( Work *work )
{
    int i ;

    for ( i=BRK_MON_N_TYPE ; --i>=0 ; )
	if ( work->prim[i] )
	    GM_FreePrim2( work->prim[i] ) ;
    if ( work->back )
	GM_FreePrim2( work->back ) ;
    if ( work->line )
	GM_FreePrim2( work->line ) ;
    
    GM_FreeTarget( &work->target ) ;
}

static void Act( Work *work )
{
    BRK_MON_ActLaster( work ) ;
    BRK_MON_ActMonitor( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->where = where ;
    work->name  = name  ;

    //if ( BRK_MON_InitBack( work, name, where ) < 0 )	return -1 ;
    if ( BRK_MON_InitMonitor( work, name, where ) < 0 )
	return -1 ;
    if ( BRK_MON_InitLaster( work, name, where ) < 0 )
	return -1 ;
    if ( BRK_MON_InitParam( work, name, where ) < 0 )
	return -1 ;
    if ( BRK_MON_InitTarget( work, name, where ) < 0 )
	return -1 ;
	
    return 0 ;
}

void *NewPutMonitor( int name, int where )
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
