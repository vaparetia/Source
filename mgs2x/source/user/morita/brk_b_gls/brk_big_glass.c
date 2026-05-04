//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_glass.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_big_glass.c,v 1.1.1.3 2002/11/19 11:45:21 Yoshizawa1 Exp $
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
#include "def_dma.h"
#include "gameheader.h"

#include "brk_big_glass.h"


void BRK_BGLS_FreeComdl( Work *work )
{
    int i ;

    if ( work->comdl )
    {
	for ( i=work->n_comdl ; --i>=0 ; )
	    if ( work->comdl[i] )
		DG_DequeueComdlObjs( work->comdl[i] ), DG_FreeComdl( work->comdl[i] ) ;
	GV_DelayedFree( work->comdl ) ;
    }
}

void BRK_BGLS_FreePiece( Work *work )
{
    if ( work->piece_s )
	DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
	DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->piece )
	GV_Free( work->piece )  ;
    work->piece = NULL ;

    if ( work->objs )
        DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    work->piece_s = work->piece_l = NULL ;
    work->objs = NULL ;
}

static void Die( Work *work )
{
    if ( work->parts )
	GV_Free( work->parts ) ;
    BRK_BGLS_FreePiece( work ) ;

    if ( work->frac[0] )
	GM_FreePrim2( work->frac[0] ) ;
    if ( work->frac[1] )
	GM_FreePrim2( work->frac[1] ) ;

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;

    /* ターゲットの解放 */
    GM_FreeTarget( &work->target ) ;

    BRK_BGLS_FreeComdl( work ) ;
}

static void Act( Work *work )
{
    //DG_StartSprToMem( BRK_BGLS_MatrixTable, SCRPAD_ADDR, sizeof(FMATRIX)*256 ) ;
    BRK_BGLS_ReceiveMessage( work ) ;
    BRK_BGLS_ActFractureWeb( work ) ;
    //DG_EndSprToMem() ;
    if ( work->act )
	(*work->act)( work ) ;
    if ( !(work->n_piece & BRK_BGLS_INACTIVE) )
	BRK_BGLS_ActPieces( work, BRK_N_PIECE, work->piece ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_BGLS_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_BGLS_InitPrimitive( work ) )
	return -1 ;
    if ( BRK_BGLS_InitTarget( work, where ) )
	return -1 ;
    if ( BRK_BGLS_InitHazard( work, where ) )
	return -1 ;
    BRK_BGLS_InitMatrixTable( work ) ;
    BRK_BGLS_InitEneFind( work ) ;
    BRK_BGLS_InitBroken( work ) ;

    return 0 ;
}

void *NewPutBigGlassObject( int name, int where )
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
