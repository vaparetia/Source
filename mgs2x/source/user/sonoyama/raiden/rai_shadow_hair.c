//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   rai_shadow_hair.c
   映り込み／影用ライデン髪の毛

   2001/06/18	M.Sonoyama
   $Id: rai_shadow_hair.c,v 1.1.1.3 2002/11/19 11:51:01 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#define	BODY_NAME		GV_StrCode( "rai_hair_shadow_mt" ) 
#define	GOL_BODY_NAME	GV_StrCode( "rai_gbs_gbshead" ) 
#define	BODY_FLAG		(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define	BODY_FLAG_GBSCAP		(DG_FLAG_SHADE|DG_FLAG_FINISHEDCALC)

typedef	struct	{
	GV_ACT_EX			actor ;
	OBJECT				*pbody ;
	DG_OBJS				*objs ;
	DG_OBJS				*golhead ;
	int					connected ;
} Work ;

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	/* ゴルキャップ対策 */
	if ( PL_GetPlayerItem() == IT_Uniform &&
		( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ) {
		if ( work->connected == 0 && work->golhead != NULL ) {
			DG_DisconnectObjs( work->pbody->objs, work->objs ) ;
			if ( work->objs->next_dgobjs != NULL ) work->objs->next_dgobjs = NULL ;
			DG_ConnectObjs( work->pbody->objs, work->golhead ) ;
			work->connected = 1 ;
		}
		if ( work->pbody->objs != NULL && work->golhead != NULL ){
			work->golhead->world = work->pbody->objs->objs[2].world ;
			work->golhead->objs[0].world = work->pbody->objs->objs[2].world ;
			work->golhead->objs[1].world = work->pbody->objs->objs[11].world ;
			work->golhead->objs[2].world = work->pbody->objs->objs[12].world ;
		}
	} else {
		if ( work->connected == 1 && work->golhead != NULL ) {
			DG_DisconnectObjs( work->pbody->objs, work->golhead ) ;
			if ( work->golhead->next_dgobjs != NULL ) work->golhead->next_dgobjs = NULL ;
			DG_ConnectObjs( work->pbody->objs, work->objs ) ;
			work->connected = 0 ;
		}
	}

	/* 親オブジェの表示状態にあわせる */
	{
		int		vflag ;

		vflag = work->pbody->objs->flag & DG_FLAG_INVISIBLE ;
		work->objs->objs[ 0 ].flag &= ~DG_FLAG_INVISIBLE ;
		work->objs->objs[ 0 ].flag |= vflag ;
	}

#if 0
	{
		DG_OBJS		*link ;

		link = work->pbody->objs ;
		printf( "%x", link ) ;
		while( link->next != NULL ) {
			printf( " - %x", link->next ) ;
			link = link->next ;
		}
		printf( "\n" ) ;
	}
#endif
}

static	void	Die( Work *work )
{
	if ( work->objs != NULL ) {	
		if ( work->connected == 0 ) {
			DG_DisconnectObjs( work->pbody->objs, work->objs ) ;
		}
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs ) ;
	}
	if ( work->golhead != NULL ) {
		if ( work->connected == 1 ) {
			DG_DisconnectObjs( work->pbody->objs, work->golhead ) ;
		}
		DG_DequeueObjs( work->golhead ) ;
		DG_FreeObjs( work->golhead ) ;
	}
	PL_ShadowHairObjs = NULL ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work, OBJECT *body )
{
	DG_DEF		*def ;
	DG_OBJS		*objs ;

	def = ( DG_DEF * )GV_GetCache( GV_CacheID( BODY_NAME, 'k' ) ) ;
	ASSERT( def != NULL ) ;
	objs = work->objs = DG_MakeObjs( def, BODY_FLAG, 0 ) ;
	ASSERT( objs != NULL ) ;
	DG_QueueObjs( objs ) ;
	DG_SetLightMatrix( objs, body->objs->light ) ;
	DG_InvisibleObjs( objs ) ;
	DG_ConnectObjs( body->objs, objs ) ;
	objs->root = &body->objs->objs[ HUMAN21_ATAMA ].world ;

	if ( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) {
		def = ( DG_DEF * )GV_GetCache( GV_CacheID( GOL_BODY_NAME, 'k' ) ) ;
		ASSERT( def != NULL ) ;
		objs = work->golhead = DG_MakeObjs( def, BODY_FLAG_GBSCAP, 0 ) ;
		ASSERT( objs != NULL ) ;
		DG_QueueObjs( objs ) ;
		DG_SetLightMatrix( objs, body->objs->light ) ;
//		DG_InvisibleObjs( objs ) ;
//		objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE ;
		objs->flag |= DG_FLAG_PLUGINDRAW ;	/* DG_FLAG_INVISIBLEの代わり（モデル自体の非表示） */
//		objs->root = &body->objs->objs[ HUMAN21_ATAMA ].world ;
	}

	work->pbody = body ;

	PL_ShadowHairObjs = objs ;
	work->connected = 0 ;

	//GV_SleepActor( work, GV_CLASS_WAITING ) ;
	return 0 ;
}

/*----------------------------------------------------------------*/

void		*PL_RaidenShadowHair( OBJECT *body )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, sizeof( Work ),
									 PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	GetResources( work, body ) ;
	return work ;
}
