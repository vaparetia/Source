//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_file.c
   紙舞い壊れ

   2000/04/25 T. Morita
   $Id: brk_file.c,v 1.1.1.3 2002/11/19 11:45:28 Yoshizawa1 Exp $
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

#include "brk_file.h"


static void Die( Work *work )
{
    int    i, j  ;
    PILED *piled ;
    FILES *p     ;

    work->n_piled &= ~BRK_FIL_INACTIVE ;
    work->n_piece &= ~BRK_FIL_INACTIVE ;
    work->n_dust  &= ~BRK_FIL_INACTIVE ;

    if ( work->piece_s )
        DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
        DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->dust )
	GM_FreePrim2( work->dust ) ;

    /* 積まれた紙を解放する */
    for ( i=work->n_piled, piled=work->piled ; --i>=0 ; piled++ )
    {
	for ( j=BRK_FIL_N_PAPER, p=piled->file ; --j>=0 ; p++ )
	    if ( p->objs )
		DG_DequeueObjs( p->objs ), DG_FreeObjs( p->objs ) ;
	if ( piled->objs )
	    DG_DequeueObjs( piled->objs ), DG_FreeObjs( piled->objs ) ;
    }
    if ( work->piled )
	GV_Free( work->piled ) ;

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;
}


static void Act( Work *work )
{
    BRK_FIL_ReceiveMessage( work ) ;

    if ( !(work->n_piled & BRK_FIL_INACTIVE) )
	BRK_FIL_ActPiled( work ) ;
    if ( !(work->n_piece & BRK_FIL_INACTIVE) )
	BRK_FIL_ActPiece( work ) ;
    if ( !(work->n_dust  & BRK_FIL_INACTIVE) )
	BRK_FIL_ActDust( work ) ;
}


static int GetResources( Work *work, int name, int where )
{
    work->where = where ;
    work->name  = name  ;
    if ( BRK_FIL_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_FIL_InitPiece( work, where ) )
	return -1 ;
    if ( BRK_FIL_InitPrimitive( work ) )
	return -1 ;
    if ( BRK_FIL_InitHazard( work ) )
	return -1 ;
    BRK_FIL_InitEneFind( work ) ;
    BRK_FIL_InitBroken( work ) ;

    return 0 ;
}


void *NewPutFileObject( int name, int where )
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
