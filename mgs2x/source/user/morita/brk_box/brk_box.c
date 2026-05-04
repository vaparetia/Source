/*
   brk_box.c
   じゃがいも壊れ

   1999/11/26 T. Morita
   $Id: brk_box.c,v 1.1.1.3 2002/11/19 11:45:22 Yoshizawa1 Exp $
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

#include "brk_box.h"


static void Die( Work *work )
{
    int i ;

    for ( i= work->n_box ; --i>=0 ;  )
	if ( work->box[i].objs ) 
	    DG_FreeObjs( work->box[i].objs ) ;
    if ( work->box )
	GV_Free( work->box ) ;
}

static void Act( Work *work )
{
    int i, flag=0 ;
    BOX *p ;

    BRK_BOX_ReceiveMessage( work ) ;
    for ( i=work->n_box, p=work->box ; --i>=0 ; p++ )
	flag |= (*p->act)( work, p ) ;
    if ( flag )
	BRK_BOX_HzdPotatoCheck( work, 80.0f*80.0f ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name = name ;
    if ( BRK_BOX_InitBox( work ) )
	return -1 ;
    if ( BRK_BOX_GetOptions( work ) )
	return -1 ;
    if ( BRK_BOX_InitHazard( work, where ) )
	return -1 ;
    return 0 ;
}


void *NewPutPotatoObject( int name, int where )
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
