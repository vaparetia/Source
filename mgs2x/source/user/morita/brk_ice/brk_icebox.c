//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_icebox.c
   アイスクーラー

   1999/11/26 T. Morita
   $Id: brk_icebox.c,v 1.1.1.3 2002/11/19 11:45:33 Yoshizawa1 Exp $
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

#include "brk_icebox.h"


void BRK_ICE_FreeIce( Work *work )
{
    int i ;
    ICEBOX *p ;

    if ( work->ice )
    {
	work->n_ice &= ~BRK_ICE_INACTIVE ;
	for ( i=work->n_ice, p=work->ice ; --i>=0 ; p++ )
	{
	    if(&p->target)GM_FreeTarget( &p->target ) ;
	    if ( p->objs )
		DG_DequeueObjs( p->objs ), DG_FreeObjs( p->objs ) ;
//	    if ( p->prim )
//		GM_FreePrim2( p->prim ) ;

		if(p->frg_objs){
			DG_DequeueComdlObjs( p->frg_objs );
			DG_FreeComdl( p->frg_objs );
		}
	    p->objs = NULL ;
		p->frg_objs = NULL;
//	    p->prim = NULL ;
	}
		
	GV_Free( work->ice ) ;
	work->ice = NULL ;
    }
	//if(work->spray) GM_FreePrim2(work->spray);
    work->n_ice |= BRK_ICE_INACTIVE ;
}

static void Die( Work *work )
{
    BRK_ICE_FreeIce( work ) ;
	if(work->spray) GM_FreePrim2(work->spray);
    if ( work->box.objs )
    {
	DG_DequeueObjs( work->box.objs ) ;
	DG_FreeObjs( work->box.objs ) ;
    }
    GM_FreeTarget( &work->box.target ) ;
}


static void Act( Work *work )
{
    BRK_ICE_ReceiveMessage( work ) ;

    if ( !(work->box.flag & BRK_ICE_INACTIVE) )
        BRK_ICE_ActBox( work ) ;
    if ( work->ice )
	//BRK_ICE_HzdIceboxCheck( work, BRK_ICEBOX_RAD*BRK_ICEBOX_RAD ) ;
    if ( !(work->n_ice & BRK_ICE_INACTIVE) )
        BRK_ICE_ActIce( work ) ;

    if ( work->sp_flag & SPRAY_ACTIVE )
	if ( BRK_ICE_ActSpray( work ) )
	    work->sp_flag &= ~(SPRAY_ACTIVE) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_ICE_GetOptions( work ) )
	return -1 ;
    if ( BRK_ICE_InitBox( work ) )
	return -1 ;
    if ( BRK_ICE_InitHazard( work ) )
	return -1 ;
    if ( BRK_ICE_SprayPrimInit( work ) )
	return -1 ;
    BRK_ICE_InitBroken( work ) ;

    return 0 ;
}


void *NewPutIceboxObject( int name, int where )
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
