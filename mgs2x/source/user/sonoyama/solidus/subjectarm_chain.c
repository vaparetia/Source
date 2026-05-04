//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   subjectarm_chain.c
   ¼ç´ÑÏÓ¤Î¼ê¾û

   2001/07/10	M.Sonoyama
   $Id: subjectarm_chain.c,v 1.1.1.3 2002/11/19 11:51:07 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

extern	void *NewRopeModel3_called(int model_name,int sample_num,
								   OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
								   float oval_param,int collision_flag,	
								   int visible_flag,int mode, int boundmodel_name) ;


typedef	struct	{ 
	GV_ACT_EX			actor ;
	void				*right ;
	void				*left ;
} Work ;

static	void	Start( Work *work )
{
	if ( work->right == NULL ) {
		work->right = NewRopeModel3_called( GV_StrCode( "demo_tjo_rai_broken_r" ),
										   4, GM_PlayerArmBody, 5, NULL, NULL, 
										   1.0F, 0, 0, 1, 0 ) ;
		GV_SetActorChild( GM_PlayerWork, work->right ) ;
    }
	if ( work->left == NULL ) {
		work->left = NewRopeModel3_called( GV_StrCode( "demo_tjo_rai_broken_l" ),
										  4, GM_PlayerArmBody, 9, NULL, NULL, 
										  1.0F, 0, 0, 1, 0 ) ;
		GV_SetActorChild( GM_PlayerWork, work->left ) ;
	}
}

static	void	Kill( Work *work )
{
	if ( work->right != NULL ) GV_DestroyOtherActor( work->right ) ;
	if ( work->left != NULL ) GV_DestroyOtherActor( work->left ) ;
	work->right = NULL ;
	work->left = NULL ;
}

static	void	Act( Work *work )
{
	if ( PL_GetPlayerWeapon() == WP_Blade ) {
		Kill( work ) ;
	} else {
		Start( work ) ;
	}
}

int		PL_COM_SubjectArmChain( void )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work == NULL ) return 0 ;
	GV_SetActor( &work->actor, Act, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	return 0 ;
}

