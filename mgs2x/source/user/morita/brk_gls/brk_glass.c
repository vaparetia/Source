//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_glass.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_glass.c,v 1.1.1.3 2002/11/19 11:45:29 Yoshizawa1 Exp $
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

#include "brk_glass.h"


static void Die( Work *work )
{
    /* パケットメモリ開放 */
    if ( work->packet )
	GV_DelayedFree( work->packet ) ;

    /* ＤＭＡパケットオブジェクト開放 */
    if ( work->dmapack )
    {
	DG_DequeueDmapack( work->dmapack ) ;
	DG_FreeDmapack( work->dmapack ) ;
    }

    if ( work->glass )
	GM_FreePrim2( work->glass ) ;
    if ( work->broken )
	GM_FreePrim2( work->broken ) ;
    if ( work->thick )
	GM_FreePrim2( work->thick ) ;
    if ( work->mirror )
	GM_FreePrim2( work->mirror ) ;
    if ( work->mirror_bg )
	GM_FreePrim2( work->mirror_bg ) ;
    if ( work->mirror_th )
	GM_FreePrim2( work->mirror_th ) ;
    if ( work->mirror_br )
	GM_FreePrim2( work->mirror_br ) ;

#if 0
    work->n_piece &= ~BRK_GLS_INACTIVE ;
    if ( work->piece_s )
	DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->piece_l )
	DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
#endif

    if ( work->fog_mist )
	GV_DestroyOtherActor( work->fog_mist ) ;

    if ( work->poly )
	GV_Free( work->poly ) ;
    GM_FreeEneFind( &work->ene_find ) ;
}

static void Act( Work *work )
{
    BRK_GLS_ReceiveMessage( work ) ;

    if ( work->flag > 0 ) {
		BRK_GLS_ActGlass( work ) ;
	}

#ifdef KP_XBOX
	BRK_GLS_ActFlipTheFace( work ) ;
#endif
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    GM_CurrentMap = where ;

    if ( BRK_GLS_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_GLS_InitPolygons( work ) )
	return -1 ;
    if ( BRK_GLS_InitTarget( work, where ) )
	return -1 ;
    if ( BRK_GLS_InitHazard( work, where ) )
	return -1 ;
    //if ( BRK_GLS_InitPieces( work, where ) )	return -1 ;
    if ( BRK_GLS_FogMist( work, where ) )
	return -1 ;
    BRK_GLS_InitEneFind( work ) ;
    BRK_GLS_InitBroken( work ) ;

    return 0 ;
}

void *NewPutGlassObject( int name, int where )
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
