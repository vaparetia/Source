//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_arkms.c
   ARKMS専用 キャラ

   2000/12/22 T. Morita
   $Id: demo_connect.c,v 1.1.1.3 2002/11/19 11:45:55 Yoshizawa1 Exp $
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
#include "../include/util.h"


typedef struct work_t
{
    GV_ACT_EX   actor ;

	DG_EVMOBJ  *p_evm ;
    DG_OBJS    *p_kms ;
    DG_OBJS    *objs  ;
    int        *flag  ;
    int         joint ;
    int         name  ;
} Work ;


static void Act( Work *work )
{
	if ( work->flag ) {
		switch( *work->flag ) {
		  case 0:
			DG_InvisibleObjs( work->objs ) ;
			work->objs->objs->flag |= DG_FLAG_INVISIBLE ;
			break ;
		  case 1:
			DG_VisibleObjs( work->objs ) ;
			work->objs->objs->flag &= ~DG_FLAG_INVISIBLE ;
			break ;
		  case 2:
			work->objs->flag |= DG_FLAG_INVISIBLE0 ;
			break ;
		}
	} else if ( GM_CheckPlayerStatus( PLAYER_CB_BOX ) ||
			   ((work->p_evm ? work->p_evm->flag & DG_EVMOBJ_INVISIBLE : 1) &&
				(work->p_kms->flag & DG_FLAG_INVISIBLE))) {
		DG_InvisibleObjs( work->objs ) ;
	} else {
		DG_VisibleObjs( work->objs ) ;
		work->objs->objs->flag &= ~DG_FLAG_INVISIBLE ;
    }
	_sceVu0CopyMatrix( &work->objs->world,
					  &work->p_kms->objs[work->joint].world ) ;
	work->objs->group_id = work->p_kms->group_id ;
}

static void Die( Work *work )
{
    if ( work->objs ){
	DG_DisconnectObjs( work->p_kms, work->objs ) ;
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
}

static int GetResourcesCalled( Work *work,
							   DG_EVMOBJ *p_evm, DG_OBJS *p_kms, int joint,
							   int kms, int *flag, int disp_flag )
{
    DG_DEF *def ;

    def = GV_GetCache( GV_CacheID( kms, 'k' ) ) ;
    if ( !def ) {
		return -1 ;
	}

	disp_flag |= DG_FLAG_SHADE|DG_FLAG_ONEPIECE ;
    work->objs = DG_MakeObjs( def, disp_flag, 0 ) ;
	if ( work->objs == NULL ) {
		return -1 ;
	}
    DG_QueueObjs( work->objs ) ;
    DG_ConnectObjs( p_kms, work->objs ) ;
    DG_SetLightMatrix( work->objs, p_kms->light ) ;

    work->flag  = flag  ;
    work->joint = joint ;
    work->p_kms = p_kms ;
	work->p_evm = p_evm ;

	_sceVu0CopyMatrix( &work->objs->world,
					   &work->p_kms->objs[work->joint].world ) ;
	work->objs->group_id = work->p_kms->group_id ;

    return 0 ;
}

void *NewConnectEquip( DG_OBJS *objs, int joint, int kms, int *flag, int disp_flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, NULL, objs, joint, kms, flag, disp_flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewConnectObjectEquip( OBJECT *object, int joint, int kms, int *flag, int disp_flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, object->evmobj, object->objs, joint, kms, flag, disp_flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
