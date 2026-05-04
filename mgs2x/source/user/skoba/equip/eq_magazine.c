//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   eq_magazine.c
   放られた雑誌
   
   2000/06/16 M.Sonoyama
   $Id: eq_magazine.c,v 1.1.1.3 2002/11/19 11:50:15 Yoshizawa1 Exp $
*/

/* 装備品でしたが、武器に移行しました。
   2001/05/17	M.Sonoyama */


/* !!! どうやら使用されていないようです。by YANO !!! */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE)
#define	BODY_NAME	MDL_BOOK_MSL

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT		actor ;
    DG_OBJS		*body ;
	FMATRIX		lights[ 2 ] ;
	ENEFIND		ef ;
	int			map ;
} Work ;

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	if ( work->ef.type & EF_TYPE_GET ) {
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( work )
Work		*work ;
{
	DG_DequeueObjs( work->body ) ;
	DG_FreeObjs( work->body ) ;
	GM_FreeEneFind( &work->ef ) ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, world, page )
Work		*work ;
FMATRIX		*world ;
int			page ;
{
    DG_OBJS		*body ;

	work->body = body = DG_MakeObjs( ( DG_DEF * )GV_GetCache( GV_CacheID( BODY_NAME, 'k' ) ),
									BODY_FLAG, 0 ) ;
	if ( body == NULL ) return -1 ;
	DG_COPY_MAT( &body->world, world ) ;
	GM_GroupObjs( body, GM_PlayerMap ) ;
	DG_QueueObjs( body ) ;
	DG_SetLightMatrix( body, work->lights ) ;
	DG_GetLightMatrix( ( FVECTOR * )world->m[ 3 ], work->lights ) ;
	GM_SetEneFind( &work->ef, ( FVECTOR * )world->m[ 3 ], 
				   GM_PlayerAddress, EF_TYPE_ADULT ) ;
	GM_PutEneFind( &work->ef ) ;
	work->map = GM_PlayerMap ;
    return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewEquipMagazine( world, page )
FMATRIX		*world ;
int			page ;
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world, page ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}

