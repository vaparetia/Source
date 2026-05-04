//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  brk_plate.c
  皿壊れ

  2000/04/25 T. Morita
  $Id: brk_plate.c,v 1.1.1.3 2002/11/19 11:45:40 Yoshizawa1 Exp $
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

#include "brk_plate.h"


int BRK_PLT_SeState  ;
int BRK_PLT_SeState2 ;
int BRK_PLT_SeCount  ;

void BRK_PLT_FreePiled( PILED *piled, int i )
{
    if ( piled )
    {
	while( --i>=0 )
	{
	    GM_FreeTarget( &piled[i].target ) ;
	    if ( piled[i].objs )
	    {
		//DG_FreePreshade( piled[i].objs ) ;
		DG_DequeueObjs( piled[i].objs ) ;
		DG_FreeObjs( piled[i].objs ) ;
	    }
	}
	GV_Free( piled ) ;
    }
}
void BRK_PLT_FreePlate( PLATE *plt, int i )
{
    if ( plt )
    {
	while( --i>=0 )
	    if ( plt[i].objs )
		DG_DequeueObjs( plt[i].objs ), DG_FreeObjs( plt[i].objs ) ;
	GV_Free( plt ) ;
    }
}
void BRK_PLT_FreePart( Work *work )
{
    int  i ;

    if ( work->comdl )
    {
	for ( i=work->p_def->n_models ; --i>=0 ; )
	    if ( work->comdl[i] )
		DG_DequeueComdlObjs( work->comdl[i] ), DG_FreeComdl( work->comdl[i] ) ;
        GV_Free( work->comdl ) ;
    }
}

static void Die( Work *work )
{
    if ( work->piece_s )
        DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
        DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->dust )
	GM_FreePrim2( work->dust ) ;

    work->n_piled &= ~BRK_PLT_INACTIVE ;
    BRK_PLT_FreePlate( work->plate, work->n_piled * BRK_PLT_N_PLATE ) ;
    BRK_PLT_FreePart( work ) ;
    BRK_PLT_FreePiled( work->piled, work->n_piled ) ;

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;
}

static void Act( Work *work )
{
    int n_part, n_plate ;

    BRK_PLT_ReceiveMessage( work ) ;

    if ( BRK_PLT_SeState > 0 )
	BRK_PLT_SeState-- ;
    if ( BRK_PLT_SeState2 > 0 )
	BRK_PLT_SeState2-- ;

    n_plate = (work->n_piled & ~BRK_PLT_INACTIVE) * BRK_PLT_N_PLATE ;
    n_part  = n_plate * work->p_def->n_models ;

    if ( !(work->n_dust & BRK_PLT_INACTIVE) )
	BRK_PLT_ActDust( work, BRK_PLT_N_DUST ) ;
    if ( !(work->n_piece & BRK_PLT_INACTIVE) && work->d_def )
	BRK_PLT_ActPart( work, BRK_PLT_N_PIECE, work->piece, 0 ) ;
    if ( !(work->n_part & BRK_PLT_INACTIVE) )
	BRK_PLT_ActPart ( work, n_part , work->part, 1 ) ;
    if ( !(work->n_plate & BRK_PLT_INACTIVE) )
	BRK_PLT_ActPlate( work, n_plate, work->plate ) ;
    if ( !(work->n_piled & BRK_PLT_INACTIVE) )
	BRK_PLT_ActPiled( work, work->n_piled, work->piled ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_PLT_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_PLT_InitHazard( work ) )
	return -1 ;
    if ( BRK_PLT_InitPart( work, where ) )  /* 必ず GetOptions をやってから */
	return -1 ;
    if ( BRK_PLT_InitPlate( work, where ) ) /* 必ず InitPart をやってから */
	return -1 ;
    if ( BRK_PLT_InitPiece( work, where ) ) /* 必ず GetOptions をやってから */
	return -1 ;
    if ( BRK_PLT_InitPrimitive( work ) )
	return -1 ;
    BRK_PLT_InitEneFind( work ) ;
    BRK_PLT_InitBroken( work ) ;

    /* グローバルだけどキャラが起動したら初期化してしまう */
    BRK_PLT_SeState  = 0 ;
    BRK_PLT_SeState2 = 0 ;
    BRK_PLT_SeCount  = 0 ;

    return 0 ;
}

void *NewPutPlateObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
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
