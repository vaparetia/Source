//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_radle.c
   レードル揺れ

   1999/12/13 T. Morita
   $Id: brk_radle.c,v 1.1.1.3 2002/11/19 11:45:43 Yoshizawa1 Exp $
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

#include "brk_radle.h"


static void Die( Work *work )
{
    int i ;

    if ( work->radle )
    {
	work->n_radle &= ~BRK_RDL_INACTIVE ;
	for ( i=work->n_radle ; --i>=0 ; )
	{
	    if ( work->radle[i].objs )
	    {
		DG_DequeueObjs( work->radle[i].objs ) ;
		DG_FreeObjs( work->radle[i].objs ) ;
	    }
#if MAKING
	    if ( work->radle[i].wireframe )
		GV_DestroyOtherActor( work->radle[i].wireframe ) ;
#endif

	}
	GV_Free( work->radle ) ;
    }
}

static void Act( Work *work )
{
    if ( !(work->n_radle & BRK_RDL_INACTIVE) )
    {
	BRK_RDL_CheckCollide( work ) ;
	BRK_RDL_Act( work ) ;
    }
}

static int GetResources( Work *work, int name, int where )
{
    if ( BRK_RDL_GetOptions( work, name, where ) < 0 )
	return -1 ;
    return 0 ;
}

void *NewPutRadleObject( int name, int where )
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
