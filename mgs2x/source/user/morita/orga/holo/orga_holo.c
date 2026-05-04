//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_holo.c 
   オルガ オルガ戦専用プットホロオブジェ

   2000/01/21 T.Morita
   $Id: orga_holo.c,v 1.1.1.3 2002/11/19 11:46:24 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#define _MAIN_FILE_
#include "orga_holo.h"



static void Die( Work *work )
{
    if ( work->splash )
	GV_DestroyOtherActor( work->splash ) ;

    DG_FreeAnimVertsBuffer( &work->v_anm ) ;
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    if ( work->objs_r )
	DG_DequeueObjs( work->objs_r ), DG_FreeObjs( work->objs_r ) ;
    GM_FreeTarget( &work->target ) ;
    GM_FreeTarget( &work->bul_wall ) ;
    if ( work->buffer[0] )
	GV_Free( work->buffer[0] ) ;
    if ( work->lzh )
	UTL_LzshDecodeFree( work->lzh ) ; 

    ORG_HOL_Work = NULL ;
}


static void Act( Work *work )
{
    ORG_HOL_Message( work ) ;

    if ( work->key_idx >= 0 )
    {
	ORG_HOL_ActSound( work ) ;    /* サウンドルーチン */
	ORG_HOL_ActDecode( work ) ;
	ORG_HOL_ActTurnOffMirror( work ) ;
    }
    else if ( ++work->key > 150 )
    {
	GV_DestroyActor( work ) ;
	return ;
    }
    ORG_HOL_ActHoloGone( work ) ;
}


static int GetResources( Work *work, int name, int where )
{
    work->name = name  ;
    work->map  = where ;

    if ( ORG_HOL_InitHoloParam( work ) < 0 )
	return -1 ;
    if ( ORG_HOL_GetOptions( work, name, where ) < 0 )
	return -1 ;
    if ( ORG_HOL_InitLzsh( work, work->id[work->flag] ) < 0 )
	return -1 ;

    ORG_HOL_Work = work ;

    return 0 ;
}


void *NewPutHoloObject( int name, int where )
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
