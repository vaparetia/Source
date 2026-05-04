//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_book.c
   セットされた雑誌

   2001/05/17	M.Sonoyama
   $Id: bul_book.c,v 1.1.1.3 2002/11/19 11:49:59 Yoshizawa1 Exp $
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

//#include <libutl.h>
#include	"gameheader.h"


/*------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	BODY_NAME	MDL_BOOK_MSL

/*------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX	actor ;
	DG_OBJS		*body ;
	FMATRIX		lights[ 2 ] ;	
	ENEFIND		ef ;
	GM_BOMB		this ;
	int			map ;
	int			time ;
} Work ;

#define		ACTIVE_COUNT	(DIRECT_TICK( 60 * 60 * 2 ))	/* 2分 */

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	++ work->time ;
	if ( work->time == ACTIVE_COUNT - DIRECT_TICK( 90 ) ) {
		GV_CallChildSignalFunc( work, 1, 0 ) ;
	}
	if ( work->time >= ACTIVE_COUNT ||
		 work->this.flag & GM_BMB_FLAG_DESTROY ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	if ( work->ef.type & EF_TYPE_GET ) {
		GV_DestroyActor( work ) ;
		return ;
	}
}

static	void	Die( work )
Work		*work ;
{
#if 0  /* 表示はいらない 2001.06.15 */
	DG_DequeueObjs( work->body ) ;
	DG_FreeObjs( work->body ) ;
#endif
	GM_FreeEneFind( &work->ef ) ;
	GM_RemoveBombList( &work->this ) ;
	GM_N_Books -- ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( Work *work, OBJECT *pbody, SVECTOR *rot ) 
{
    //DG_OBJS		*body ;
	FVECTOR		mov ;

#if 0  /* 表示はいらない 2001.06.15 */
	work->body = body = DG_MakeObjs( ( DG_DEF * )GV_GetCache( GV_CacheID( BODY_NAME, 'k' ) ),
									BODY_FLAG, 0 ) ;
	if ( body == NULL ) return -1 ;

	DG_COPY_MAT( &body->world, &pbody->objs->world ) ;
	DG_COPY_MAT( &body->objs[ 0 ].world, &pbody->objs->objs[ 0 ].world ) ;
	DG_COPY_MAT( &body->objs[ 1 ].world, &pbody->objs->objs[ 1 ].world ) ;
	GM_GroupObjs( body, pbody->map_name ) ;
	DG_QueueObjs( body ) ;

	GV_MatToVec( &body->world, &mov ) ;
	DG_SetLightMatrix( body, work->lights ) ;
	DG_GetLightMatrix( &mov, work->lights ) ;
#else
	GV_MatToVec( &pbody->objs->world, &mov ) ;
#endif

	/* ENEFIND */
	{
		int				g, z, map_id, address ;
		HZX_GROUP_ID	hzx_id ;

		HZX_Pos2Zone( &mov, &g, &z ) ;
		hzx_id = GM_GetBit( g ) ;
		map_id = GM_GetMapIDfromHzxGroupID( hzx_id ) ;
		address = HZX_GetAddress( hzx_id, &mov, z ) ;
		GM_SetEneFind( &work->ef, &mov, address, EF_TYPE_ADULT ) ;
		GM_PutEneFind( &work->ef ) ;
		work->map = map_id ;
	}

	{/* 表示ようのきゃらを起動 2001.06.15 */
	  void *NewPutBookObject( OBJECT *body, SVECTOR *rot ) ;
	  int  mapbuf ;

	  mapbuf = GM_CurrentMap ;
	  GM_SetCurrentMap( work->map ) ;
	  GV_SetActorChild( work,
					    NewPutBookObject( pbody, rot ) ) ;
	  GM_SetCurrentMap( GM_CurrentMap ) ;
	}

	work->this.weapon = WP_Book ;
	work->this.mov = &work->ef.pos ;
	work->this.step = NULL ;
	GM_AddBombList( &work->this ) ;
	
	work->time = 0 ;

	GM_N_Books ++ ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    Work *work = pwork ;

    switch( signal )
    {
    case 0:
	work->ef.type |=  EF_TYPE_MOVE ;
	break ;
    case 1:
	work->time = ACTIVE_COUNT - DIRECT_TICK( 90 )-1 ;
	break ;
    case 2:
	work->ef.type &= ~EF_TYPE_MOVE ;
	break ;

    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewBulletBook( OBJECT *pbody, SVECTOR *rot ) 
{
    Work	*work ;

	if ( GM_N_Books >= GM_BOOK_MAX ) return NULL ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;
		if ( GetResources( work, pbody, rot ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		if ( GM_ShootCount < 30000 ) ++ GM_ShootCount ;
		GM_SetWeaponFire( WP_Book ) ;
    }
    return work ;
}




