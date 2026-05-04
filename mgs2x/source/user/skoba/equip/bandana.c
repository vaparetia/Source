//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bandana.c
   無限バンダナ

   2001/07/30	M.Sonoyama
   $Id: bandana.c,v 1.1.1.3 2002/11/19 11:50:13 Yoshizawa1 Exp $
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

extern	void *NewRopeModel3_called(int model_name,int sample_num,
								   OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
								   float oval_param,int collision_flag,int visible_flag,int mode,
								   int boundmodel_name) ;

typedef	struct	{
	GV_ACT_EX		actor ;
	FVECTOR			r_shift ;
	FVECTOR			l_shift ;
	void			*right ;
	void			*left ;
} Work ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* オリジナルバンダナ消し */
static	inline	void	VisibleBandana( int id )
{
	extern void	PL_SendMessage( int to, int *mesg, int v ) ;
	int			buf[ 2 ] ;

	buf[ 0 ] = 0 ;
	buf[ 1 ] = id ;
	PL_SendMessage( GV_StrCode( "バンダナ１" ), buf, 2 ) ;
	PL_SendMessage( GV_StrCode( "バンダナ２" ), buf, 2 ) ;
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	VisibleBandana( 0 ) ;
}

static	void	Die( Work *work )
{
	VisibleBandana( 1 ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	GV_SetVec3( &work->r_shift, 0.0F, 60.0F, -90.0F ) ;
	GV_SetVec3( &work->l_shift, 0.0F, 50.0F, -90.0F ) ;
	work->right = NewRopeModel3_called( GV_StrCode( "sna_mugen_bdn1" ), 1,
									    GM_PlayerBody, HUMAN21_ATAMA, &work->r_shift, NULL,
									    1.0F, 1, 0, 0, 0 ) ;
	if ( work->right == NULL ) return -1 ;
	GV_SetActorChild( work, work->right ) ;
	work->left = NewRopeModel3_called( GV_StrCode( "sna_mugen_bdn2" ), 1,
									   GM_PlayerBody, HUMAN21_ATAMA, &work->l_shift, NULL,
									   1.0F, 1, 0, 0, 0 ) ;
	if ( work->left == NULL ) return -1 ;
	GV_SetActorChild( work, work->left ) ;
	GM_ClearCodeFlag |= GM_CLEAR_SPECIAL_ITEM_USED | GM_CLEAR_MUGENBANDANA_USED ;
	return 0 ;
}

/*----------------------------------------------------------------*/

void		*NewMugenBandana( CONTROL *ctrl, OBJECT **body, int **unit, u_int *trigger )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
