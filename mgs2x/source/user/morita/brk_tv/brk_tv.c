//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tv.c
   TV壊れ

   1999/12/13 T. Morita
   $Id: brk_tv.c,v 1.1.1.3 2002/11/19 11:45:49 Yoshizawa1 Exp $
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

#include "brk_tv.h"


static void Die( Work *work )
{
    if ( work->scn_al )
	GM_FreePrim2( work->scn_al ) ;
    if ( work->scn_iv )
	GM_FreePrim2( work->scn_iv ) ;
    if ( work->spec   )
	GM_FreePrim2( work->spec   ) ;
    if ( work->inv    )
	GM_FreePrim2( work->inv    ) ;

    if ( work->wipe   )
	GM_FreePrim2( work->wipe   ) ;
    GM_FreeTarget( &work->target ) ;
}

static void Act( Work *work )
{
    BRK_TV_ReceiveMessage( work ) ;

    if ( work->scn_iv && work->scn_al && work->inv && !work->count )
	BRK_TV_ActMoveImage( work ) ;
    if ( work->count > 0 )
    {
	/*黒いので消してしまう*/
	if ( work->count > 8  )
	    BRK_TV_ActWipeOut( work ) ;
	/*終了 カウンタ */
	if ( work->count++ > 512 )
	{
	    work->count =-1 ;
	    return ;
	    if ( work->scn_iv )
		GM_FreePrim2( work->scn_iv ), work->scn_iv = NULL ;
	    if ( work->scn_al )
		GM_FreePrim2( work->scn_al ), work->scn_al = NULL ;
	    if ( work->inv    )
		GM_FreePrim2( work->inv    ), work->inv    = NULL ;
	}
    }
}

static int GetResources( Work *work, int name, int where )
{
    work->name = name ;
    if ( BRK_TV_GetOptions( work, name, where ) )
	return -1 ;
    if ( BRK_TV_InitTarget( work, work->def, where ) )
	return -1 ;
    return 0 ;
}

void *NewPutTVObject( int name, int where )
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
