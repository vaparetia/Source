/*
  roach.c
  ゴキブリ

  2000/04/23 T. Morita
  $Id: roach.c,v 1.1.1.3 2002/11/19 11:46:32 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "roach.h"


static void Die( Work *work )
{
    int i ;
    Roach *r = work->roach ;

    for ( i=work->n_roach ; --i>=0 ; r++ )
	if ( r->objs )
	    DG_DequeueObjs( r->objs ), DG_FreeObjs( r->objs );
}

static void Act( Work *work )
{
    int i ;
    Roach *r = work->roach ;

    for ( i=work->n_roach ; --i>=0 ; r++ )
	if ( r->act )
	    (*r->act)( work, r, i ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name = name ;

    if ( RCH_InitMatrixTable( work ) )
	return -1 ;
    if ( RCH_GetOptions( work, name, where ) )
	return -1 ;
    if ( RCH_InitRoaches( work, where ) )
	return -1 ;

    return 0 ;
}

void *NewCockRoach( int name, int where )
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
