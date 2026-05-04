//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bsensB.c
   ÇúÃÆ¥»¥ó¥µ¡¼£Â

   2000/05/10 M.Sonoyama
   $Id: bsensB.c,v 1.1.1.3 2002/11/19 11:50:14 Yoshizawa1 Exp $
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

#include "BP_Misc.h"

/*------------------------------------------------------------------*/

extern	float	GM_GetNearC4Length( FVECTOR *, FVECTOR * ) ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT		actor ;
	CONTROL		*ctrl ;
	int			last_se_time ;
} Work ;

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	float		len ;
	int			decay ;
	FVECTOR		pos ;

	len = GM_GetNearC4Length( &work->ctrl->mov, &pos ) ;
	if ( len > 11000.0F ) return ;
	decay = 48 + ( int )( 42.0F * ( len - 11000.0F ) / 10000.0F ) ;
	if ( decay < 6 ) decay = 6 ;

   if ( BP_IsPAL()==TRUE )
	   decay = DIRECT_TICK( decay ) ;

   if ( GV_Time - work->last_se_time > decay ) {
		GM_SeSetMode( SD_I_SENS_B01 , &work->ctrl->mov, GM_SEMODE_BOMB ) ;
		work->last_se_time = GV_Time ;
	}
}

/*------------------------------------------------------------------*/

void		*NewBombSenserB( ctrl, body, unit, trigger )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, NULL ) ;
		work->ctrl = ctrl ;
		work->last_se_time = GV_Time ;
	}
    return work ;
}

