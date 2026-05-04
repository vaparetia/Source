//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_book.c
   雑誌壊れ

   2000/06/19 T. Morita
   $Id: put_book.c,v 1.1.1.3 2002/11/19 11:46:30 Yoshizawa1 Exp $
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

#include "put_book.h"


void PUT_BOK_FreeBook( BOOK *p )
{
    if ( p->objs )
    {
	DG_DequeueObjs( p->objs ) ;
	DG_FreeObjs( p->objs ) ;
	p->objs = NULL ;
    }
    GM_FreeTarget( &p->target[0] ) ;
    GM_FreeTarget( &p->target[1] ) ;
}

void PUT_BOK_FreePage( PAGE *p )
{
    if ( p->objs )
    {
	DG_DequeueObjs( p->objs ) ;
	DG_FreeObjs( p->objs ) ;
	p->objs  = NULL ;
	if ( p->anime )
	    ExitVertexAnimation( p->anime ) ;
	p->anime = NULL ;
    }    
}

static void Die( Work *work )
{
    int i ;
    PAGE *p ;

#if 0
    /* 本を解放する */
    if ( work->piece_s )
        DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
        DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
#endif

    /* 本を解放する */
    PUT_BOK_FreeBook( &work->book ) ;

    /* 動的関数を解放する */
    if ( HZX_IsDynamic( &work->seg ) )
	HZX_RemoveDynamicCallback( work->seg.ptr, &work->dyn_clb ) ;

    /* ページを解放する */
    p = work->page ;
    for( i=PUT_BOK_N_PAGE ; --i>=0 ; p++ )
	PUT_BOK_FreePage( p ) ;
}

static void Act( Work *work )
{
    PUT_BOK_ReceiveMessage( work ) ;

    if ( work->non_dmg > 0 )
	work->non_dmg-- ;

    if ( work->blink )
    {
	if ( GV_Time & 1 )
	    DG_InvisibleObjs( work->book.objs ) ;
	else
	    DG_VisibleObjs( work->book.objs ) ;
    }
    if ( work->book.flag > 0 )
	PUT_BOK_ActBook( work ) ;
    if ( !(work->n_page & PUT_BOK_INACTIVE) )
	PUT_BOK_ActPage( work ) ;
#if 0
    if ( !(work->n_piece & PUT_BOK_INACTIVE) )
	PUT_BOK_ActPiece( work ) ;
#endif
}

static int GetResources( Work *work, DG_OBJS *objs, SVECTOR *rot )
{
    //work->name  = name  ;
    work->hzx   = GM_GetHzxGroupID( GM_CurrentMap ) ;
    work->where = GM_CurrentMap ;
    if ( PUT_BOK_InitParam( work, objs, GM_CurrentMap, rot ) )
	return -1 ;
    if ( PUT_BOK_InitPage( work ) )
	return -1 ;
#if 0
    if ( PUT_BOK_InitPiece( work, GM_CurrentMap ) )
	return -1 ;
#endif
    PUT_BOK_InitHazard( work ) ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    Work *work = pwork ;

    switch( signal )
    {
    case 1:
	work->blink = 1 ;
	break ;

    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

void *NewPutBookObject( OBJECT *body, SVECTOR *rot )
{
    Work *work ;

    work = (Work *)GV_NewActorPrio( GV_ACTOR_USER, sizeof(Work), 0x10 ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;
        if( GetResources( work, body->objs, rot ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
