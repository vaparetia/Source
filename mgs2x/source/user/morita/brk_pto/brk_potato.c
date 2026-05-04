//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_potato.c
   じゃがいも壊れ

   1999/11/26 T. Morita
   2000/10/16 1.17 T.Morita 
   $Id: brk_potato.c,v 1.1.1.3 2002/11/19 11:45:41 Yoshizawa1 Exp $
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

#include "brk_potato.h"
#include "../brk_utl/brk_utl.x"

static void Die( Work *work )
{
    int i ;
    POTATO *p ;

    /* ポテト関係を全て解放 */
    for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
    {
#if MAKING
	if ( p->wireframe )
	    GV_DestroyOtherActor( p->wireframe ) ;
#endif
	if ( p->n_parts )
	    GV_Free( p->parts.prof ) ;
	else
	    GM_FreeTarget( &p->target ) ;
	if ( p->objs )
	    DG_DequeueObjs( p->objs ), DG_FreeObjs( p->objs ) ;
    }
    if ( work->potato )
	GV_Free( work->potato ) ;

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;

    /* 箱関係を全て解放 */
    for ( i=5 ; --i>=0 ; )
	GM_FreePrim2( work->box.scar[i] ) ;
    for ( i=work->box.objs->n_models ; --i>=0 ;  )
	if ( work->box_hzd[i] )
	    BRK_FreeHazard( work->box_hzd[i] ) ;
    if ( work->box.objs )
	DG_DequeueObjs( work->box.objs ),DG_FreeObjs( work->box.objs ) ;
    GM_FreeTarget( &work->box.target ) ;
}



static void Act( Work *work )
{
    int     i ;
    POTATO *p ;

    BRK_PTO_ReceiveMessage( work ) ;

    if ( work->box.act )
	(*work->box.act)( work ) ;
    if ( work->box.vitality <= 0 )
    {
	for ( i=work->n_foot, p=work->foot ; --i>=0 ; p++ )
	    work->flag += (*p->act)( work, p ) ;
	if ( work->flag )
	{
	    //if ( work->box.non_dmg <= 4 )
		BRK_PTO_HzdPotatoCheck( work, BRK_POTATO_RAD*BRK_POTATO_RAD ) ;
	    work->flag = 0 ;
	    for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
	    {
		BRK_UTL_GetLightMatrix( &p->pos, p->lights, 1.0f, work->where ) ;
		if ( p->act && p->act != BRK_PTO_ActNone )
		    work->flag += (*p->act)( work, p ) ;
	    }
	}
    }
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;

    if ( BRK_PTO_InitBox( work, where ) )
	return -1 ;
    if ( BRK_PTO_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_PTO_InitHazard( work ) )/* 必ずGetOptions()のあと */
	return -1 ;
    if ( BRK_PTO_InitPrims( work ) )
	return  -1 ;
    BRK_PTO_InitEneFind( work ) ;
    BRK_PTO_InitBroken( work ) ;

    return 0 ;
}


void *NewPutPotatoObject( int name, int where )
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
