//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   rai_action_blur.c
   ライデン用モーションブラーエフェクト

   2001/07/25	M.Sonoyama
   $Id: rai_action_blur.c,v 1.1.1.3 2002/11/19 11:51:00 Yoshizawa1 Exp $
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

extern	void *NewActionBlurEffect( OBJECT *body, u_long64 mask, int *enable_flag ) ;

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			body ;
	u_long64			mask ;
	int				*enable_flag ;
	DG_OBJS			*objs ;
	void			*actblurwork ;
} Work ;

static	void	Act( Work *work )
{
	if ( PL_ActionBlurObjs != NULL && 
		 work->objs != PL_ActionBlurObjs ) {
		if ( work->actblurwork != NULL ) {
			GV_ACT		*abw ;

			abw = GV_SearchActor( work->actblurwork ) ;
			if ( abw != NULL && abw->name == GV_StrCode( "actblurwork" ) ) {
				GV_DestroyOtherActor( abw ) ;
			}
			work->actblurwork = NULL ;
		}
		work->body.objs = PL_ActionBlurObjs ;
		work->actblurwork = NewActionBlurEffect( &work->body, work->mask, work->enable_flag ) ;
		if ( work->actblurwork != NULL ) {
			( ( GV_ACT * )work->actblurwork )->name = GV_StrCode( "actblurwork" ) ;
			GV_SetActorChild( PL_ActionBlurParent, work->actblurwork ) ;
			work->objs = PL_ActionBlurObjs ;
		}
	}
	PL_ActionBlurObjs = NULL ;
	PL_ActionBlurParent = NULL ;
}

static	void	Die( Work *work )
{
	
}

void	*PL_PlayerActionBlurManager( OBJECT *body, u_long64 mask, int *enable_flag )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_ACTIONBLUR_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->mask = mask ;
	work->enable_flag = enable_flag ;
	work->body.m_ctrl = body->m_ctrl ;
	return work ;
}
