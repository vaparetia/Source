//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_btl_act.c
   瓶壊れ アクト

   1999/12/02 T. Morita
   $Id: playerwire.c,v 1.1.1.3 2002/11/19 11:45:52 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"


#define MAX_PLAYER_OBJS 21 
typedef struct work_t
{
    GV_ACT  actor ;

    void   *wire[MAX_PLAYER_OBJS] ;
} Work  ;


static Work *PL_Wireframe = NULL ;

static void Die( Work *work )
{
    int  i ;
    PL_Wireframe = NULL ;

    for (i=MAX_PLAYER_OBJS ; --i>=0 ; )
	if ( work->wire[i] )
	    GV_DestroyOtherActor( work->wire[i] ) ;
}

static void Act( Work *work )
{
    DG_InvisibleObjs( GM_PlayerBody->objs ) ;
}

static int GetResources( Work *work )
{
    int i ;
    DG_OBJS *objs ;
    extern void *NewDrawWireframe( DG_OBJ *obj ) ;

    objs = GM_PlayerBody->objs ;
    for ( i=objs->n_models ; --i>=0 ; )
    {
	if ( !(work->wire[i] = NewDrawWireframe( &objs->objs[i] )) )
	    return -1 ;
    }
    return 0 ;
}

void *NewPlayerWireframe()
{
    Work *work ;

    if ( PL_Wireframe )
	return NULL ;
    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if ( GetResources( work ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }

    return PL_Wireframe = work ;
}
