/*
   brk_fedral.c
   雑誌壊れ

   2000/06/19 T. Morita
   $Id: brk_fedralex.c,v 1.1.1.3 2002/11/19 11:45:27 Yoshizawa1 Exp $
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

#include "brk_fedralex.h"


Work *FED_Work = NULL ;

void BRK_FED_FreeFedral( FEDRAL *p )
{
    if ( p->objs )
    {
	if( p->v_anime.mem[0] )
	{
	    DG_FreeAnimVertsBuffer( &p->v_anime ) ;
	    p->v_anime.mem[0] = NULL ;
	}
	DG_DequeueObjs( p->objs ) ;
	DG_FreeObjs( p->objs ) ;
	p->objs = NULL ;
    }
    if ( p->scar )
	GM_FreePrim2( p->scar ) ;
    GM_FreeTarget( &p->target ) ;
}

static void Die( Work *work )
{
    int       i ;

    work->n_fedral  &= ~BRK_FED_INACTIVE ;
    work->n_piece   &= ~BRK_FED_INACTIVE ;
    if ( work->piece_s )
        DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
        DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;

    if ( work->fedral )
    {
	FEDRAL *p = work->fedral ;

	for ( i=work->n_fedral ; --i>=0 ; p++ )
	    BRK_FED_FreeFedral( p ) ;
	GV_Free( work->fedral ) ;
    }

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;
}

static void Act( Work *work )
{
    BRK_FED_ReceiveMessage( work ) ;

    if ( !(work->n_fedral & BRK_FED_INACTIVE) )
	BRK_FED_ActFedral( work ) ;
    if ( !(work->n_piece & BRK_FED_INACTIVE) )
	BRK_FED_ActPiece( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_FED_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_FED_InitPiece( work, where ) )
	return -1 ;
    if ( BRK_FED_InitHazard( work ) )
	return -1 ;
    BRK_FED_InitEneFind( work ) ;
    BRK_FED_InitBroken( work ) ;
    FED_Work = work ;

    return 0 ;
}


void *NewPutFedralExObject( int name, int where )
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
