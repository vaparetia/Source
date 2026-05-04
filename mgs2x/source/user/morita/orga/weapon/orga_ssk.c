//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_ssk.c 
   オルガ 武器関数(コンバットナイフ)

   2000/01/11 T.Morita
   $Id: orga_ssk.c,v 1.1.1.3 2002/11/19 11:46:28 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

#include "orga_wep.h"
#include "../include/orga_se.h"


typedef	struct work_usp_t Work ;
struct work_usp_t
{
    GV_ACT   actor  ;
    DG_OBJS *weapon ;
    FMATRIX *world  ;
	   
    short   *trigger ;
} ;



#define ORGA_USP_FRAMES  7
#define ORGA_USP_SCALE   7

static void Die( Work *work )
{
    if ( work->weapon )
	DG_DequeueObjs( work->weapon ), DG_FreeObjs( work->weapon ) ;
}

static void Act( Work *work )
{
    work->weapon->world = *work->world ;

    if ( *work->trigger == -1 )
	GV_DestroyActor( work ) ;
}

static int GetResources( Work *work, FMATRIX *world, short *trigger )
{
    DG_DEF  *def = GV_GetCache( GV_CacheID( GV_StrCode( "ssk" ), 'k' ) ) ;

    DG_QueueObjs( work->weapon = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ) ;
    work->world = world ;

    return 0 ;
}

void *NewOrgaWeaponSSK( FMATRIX *world, short *trigger )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, world, trigger ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
