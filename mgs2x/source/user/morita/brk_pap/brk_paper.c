//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_paper.c
   紙舞い壊れ

   2000/04/25 T. Morita
   $Id: brk_paper.c,v 1.1.1.3 2002/11/19 11:45:39 Yoshizawa1 Exp $
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

#include "brk_paper.h"


static void Die( Work *work )
{
    int i ;
    PAPER *p ;
    PILED *piled ;

    work->n_piled &= ~BRK_PAP_INACTIVE ;
    work->n_paper &= ~BRK_PAP_INACTIVE ;
    work->n_piece &= ~BRK_PAP_INACTIVE ;
    work->n_dust  &= ~BRK_PAP_INACTIVE ;

    for ( i=BRK_PAP_N_PAPER, p=work->paper ; --i>=0 ; p++ )
	if ( p->objs )
	    DG_DequeueObjs( p->objs ), DG_FreeObjs( p->objs ) ;
    if ( work->piece_s )
        DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
        DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->dust )
	GM_FreePrim2( work->dust ) ;

    for ( i=work->n_piled, piled=work->piled ; --i>=0 ; piled++ )
	if ( piled->objs )
	    DG_DequeueObjs( piled->objs ), DG_FreeObjs( piled->objs ) ;
    if ( work->piled )
	GV_Free( work->piled ) ;

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;
}

static void Act( Work *work )
{
    BRK_PAP_ReceiveMessage( work ) ;

    if ( !(work->n_piled & BRK_PAP_INACTIVE) )
	BRK_PAP_ActPiled( work ) ;
    if ( !(work->n_paper & BRK_PAP_INACTIVE) )
	BRK_PAP_ActPaper( work ) ;
    if ( !(work->n_piece & BRK_PAP_INACTIVE) )
	BRK_PAP_ActPiece( work ) ;
    if ( !(work->n_dust  & BRK_PAP_INACTIVE) )
	BRK_PAP_ActDust( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->where = where ;
    work->name  = name  ;
    if ( BRK_PAP_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_PAP_InitPaper( work, where ) )
	return -1 ;
    if ( BRK_PAP_InitPiece( work, where ) )
	return -1 ;
    if ( BRK_PAP_InitPrimitive( work ) )
	return -1 ;
    if ( BRK_PAP_InitHazard( work ) )
	return -1 ;
    BRK_PAP_InitEneFind( work ) ;
    BRK_PAP_InitBroken( work ) ;

    return 0 ;
}


void *NewPutPaperObject( int name, int where )
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
