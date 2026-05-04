//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
   brk_magazine.c
   雑誌壊れ

   2000/06/19 T. Morita
   $Id: brk_magazine.c,v 1.2 2002/12/11 14:02:42 takaki Exp $
*/
#endif

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

#include "brk_magazine.h"


Work *MGZ_Work = NULL ;


void BRK_MGZ_FreeMagazine( MAGAZINE *p )
{
    int i ;

    if ( p->objs )
    {
	for ( i=3 ; --i>=0 ; )
	{
	    if( p->v_anime[i].mem[0] )
	    {
		DG_FreeAnimVertsBuffer( &p->v_anime[i] ) ;
		p->v_anime[i].mem[0] = NULL ;
	    }
	}
	DG_DequeueObjs( p->objs ) ;
	DG_FreeObjs( p->objs ) ;
	p->objs = NULL ;
    }
    GM_FreeTarget( &p->target[0] ) ;
    GM_FreeTarget( &p->target[1] ) ;
#if MAKING
    if ( p->view[0] ) GV_DestroyOtherActor( p->view[0] ) ;
    if ( p->view[1] ) GV_DestroyOtherActor( p->view[1] ) ;
    if ( p->view[2] ) GV_DestroyOtherActor( p->view[2] ) ;
#endif

}

void BRK_MGZ_FreePage( PAGE *p )
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
    int       i ;

    work->n_magazine  &= ~BRK_MGZ_INACTIVE ;
    work->n_piece     &= ~BRK_MGZ_INACTIVE ;
    work->n_page      &= ~BRK_MGZ_INACTIVE ;
    work->n_dust      &= ~BRK_MGZ_INACTIVE ;

    if ( work->piece_s )
        DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
        DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;

    for( i=BRK_MGZ_N_PAGE ; --i>=0 ; )
	BRK_MGZ_FreePage( &work->page[i] ) ;

    if ( work->magazine )
    {
	MAGAZINE *p = work->magazine ;

	for ( i=work->n_magazine ; --i>=0 ; p++ )
	    BRK_MGZ_FreeMagazine( p ) ;
	GV_Free( work->magazine ) ;
    }
    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;
    MGZ_Work = NULL ;
}

static void Act( Work *work )
{
    BRK_MGZ_ReceiveMessage( work ) ;

    if ( !(work->n_magazine & BRK_MGZ_INACTIVE) )
	BRK_MGZ_ActMagazine( work ) ;
    if ( !(work->n_page & BRK_MGZ_INACTIVE) )
	BRK_MGZ_ActPage( work ) ;
    if ( !(work->n_piece & BRK_MGZ_INACTIVE) )
	BRK_MGZ_ActPiece( work ) ;
//    if ( !(work->n_dust  & BRK_MGZ_INACTIVE) )
//	BRK_MGZ_ActDust( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_MGZ_InitHazard( work ) )
	return -1 ;
    if ( BRK_MGZ_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_MGZ_InitPage( work ) )
	return -1 ;
    if ( BRK_MGZ_InitPiece( work, where ) )
	return -1 ;
//    if ( BRK_MGZ_InitPrimitive( work ) )
//	return -1 ;
    BRK_MGZ_InitEneFind( work ) ;
    BRK_MGZ_InitBroken( work ) ;
    MGZ_Work = work ;

    return 0 ;
}


void *NewPutMagazineObject( int name, int where )
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
