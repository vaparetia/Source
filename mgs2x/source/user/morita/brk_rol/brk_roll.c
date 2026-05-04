/*
   brk_roll.c
   転がりオブジェクト

   1999/11/26 T. Morita
   $Id: brk_roll.c,v 1.1.1.3 2002/11/19 11:45:44 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_roll.h"


static void Die( Work *work )
{
    int i ;
    ROLL *p ;

    if ( work->roll )
    {
	work->n_roll &= ~BRK_ROL_INACTIVE ;
	for ( i=work->n_roll, p=work->roll ; --i>=0 ; p++ )
	{
	    GM_FreeTarget( &p->target ) ;
	    if ( p->objs )
		DG_DequeueObjs( p->objs ), DG_FreeObjs( p->objs ) ;
	}
	GV_Free( work->roll ) ;
    }
}

static void Act( Work *work )
{
    if ( !(work->n_roll & BRK_ROL_INACTIVE) )
	BRK_ROL_ActRoll( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_ROL_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_ROL_InitHazard( work ) )
	return -1 ;
    return 0 ;
}

void *NewPutRollObject( int name, int where )
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
