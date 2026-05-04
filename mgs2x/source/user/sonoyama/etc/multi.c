//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   multi.c
   マルチウェイトモデル表示用
   
   2000/04/06 M.Sonoyama
   $Id: multi.c,v 1.1.1.3 2002/11/19 11:50:44 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#define	ACTOR_PRIO	(254)

typedef	struct	{
    GV_ACT		actor ;
	CONTROL		*ctrl ;
    OBJECT		*body ;
    int			flag ;
} Work ;

typedef	struct	{
    GV_ACT	actor ;
    OBJECT	*body ;
    int		*flag ;
} Work2 ;

static	void	Act( work )
Work		*work ;
{
    int			flag, i ;
	FVECTOR		mov, mov2, diff ;
	FMATRIX		*mats ;

	GV_MatToVec( &work->body->objs->world, &mov ) ;
	GV_MatToVec( &work->body->evmobj->world, &mov2 ) ;

	DG_COPY_MAT( &work->body->evmobj->world, &work->body->objs->world ) ;

	_sceVu0SubVector( &diff, &mov, &mov2 ) ;
	mats = work->body->evmobj->matrix[ work->body->evmobj->use_buffer ] ;
	for ( i = 0; i < work->body->evmobj->n_skeleton; i ++ ) {
		GV_MatToVec( mats, &mov ) ;
		_sceVu0AddVector( &mov, &mov, &diff ) ;
		GV_VecToMat( &mov, mats ) ;
		mats ++ ;
	}

    work->flag = flag = work->body->objs->flag & DG_FLAG_INVISIBLE ;
    if ( flag & DG_FLAG_INVISIBLE ) {
		work->body->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
    } else {
		work->body->evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
    }
    DG_InvisibleObjs( work->body->objs ) ;
}

static	void	Act2( work )
Work2		*work ;
{
    work->body->objs->flag &= ~DG_FLAG_INVISIBLE ;
    work->body->objs->flag |= *( work->flag ) ;
}

static	int	GetResources( work, object )
Work		*work ;
OBJECT		*object ;
{
    int		model ;
	int		evm_flag ;

    GCL_GetOption( 'm' ) ;
    model = GCL_GetNextInt() ;

	evm_flag = ( object->objs->flag & DG_FLAG_IRREACTION ) ? DG_EVMOBJ_IRREACTION : 0 ;
    GM_ConfigObjectEvm( object, model, evm_flag ) ;
    GM_ConfigObjectLight( object, object->objs->light ) ;
    work->body = object ;
    work->flag = object->objs->flag & DG_FLAG_INVISIBLE ;
    DG_InvisibleObjs( object->objs ) ;
    return 0 ;
}

void *NewMultiweightChara( name, where )
int	name, where ;
{
    Work		*work ;
    Work2		*work2 ;
    CONTROL		*ctrl ;
    OBJECT		*object ;
    int			*tmp ;

    GCL_GetOption( 'n' ) ;
    ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
    ASSERT( ctrl != NULL ) ;
    object = ctrl->object ;
    ASSERT( object != NULL ) ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, 
								  sizeof( Work ), PLAYER_MULTI_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, NULL ) ;
		if ( GetResources( work, object ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
	work->ctrl = ctrl ;
    tmp = &work->flag ;

    work2 = (Work2 *)GV_CreateActor( GV_ACTOR_PREV2, GV_CLASS_OBJECT, 
									sizeof( Work2 ), PLAYER_MULTI_ACTOR2_PRIO ) ;
    if ( work2 != NULL ) {
		GV_SetActor( &( work2->actor ), Act2, NULL ) ;
		work2->flag = tmp ;
		work2->body = object ;
    }
    return work ;
}
