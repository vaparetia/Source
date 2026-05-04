//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tree.c
   植物揺れ

   1999/12/17 T. Morita
   $Id: brk_tree.c,v 1.1.1.3 2002/11/19 11:45:48 Yoshizawa1 Exp $
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

#include "brk_tree.h"


static void Die( Work *work )
{
    if ( work->leaf )
    {
	DG_DequeueComdlObjs( work->leaf ) ;
	DG_FreeComdl( work->leaf ) ;
    }
    if ( work->n_parts )
	GV_Free( work->parts ) ;
    if ( work->objs )
    {
        DG_DequeueObjs( work->objs ) ;
        DG_FreeObjs( work->objs );
	work->objs = NULL ;
    }
    if ( work->target )
    {
	GM_FreeTarget( work->target ) ;
	GV_Free( work->target ) ;
    }
}

static void Act( Work *work )
{
    int i, flag ;
    PART *p ;

    BRK_TRE_ReceiveMessage( work ) ;

#if 0
    /* 葉の動き管理 */
    if ( (i = work->front - work->n_pos) )
    {
	int  j ;

	i = BRK_MAX_DIFF - (i>0 ? i : BRK_N_LEAVES+i) ;
	for ( j=work->front ; --i>=0 ; j=(j+1)&(BRK_N_LEAVES-1) )
	    work->leaves[j] *= -1 ;
	work->front &= (BRK_N_LEAVES-1) ;
    }
#endif
    /* 木の動き */
    if ( !(work->n_parts & BRK_TRE_INACTIVE) )
    {
	flag = 0 ;
	for ( i=work->n_parts, p=work->parts ; --i>=0 ; p++ )
	    flag |= (*p->act)( work, p ) ;
	if ( !flag )
	    work->n_parts |= BRK_TRE_INACTIVE ;
    }
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_TRE_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_TRE_InitTarget( work, where ) )
	return -1 ;
    if ( BRK_TRE_InitHazard( work, where ) )
	return -1 ;
    BRK_TRE_InitBroken( work ) ;

    return 0 ;
}

void *NewPutTreeObject( int name, int where )
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
