//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   shadowhzx.c
   ±∆≈Í±∆•œ•∂°º•…
   
   2000/05/22 M.Sonoyama
   $Id: shadowhzx.c,v 1.1.1.3 2002/11/19 11:50:50 Yoshizawa1 Exp $
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
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

#if 0

static	HZX_GROUP_ID	ShadowHazard[ GM_MAX_MAPS ] = { -1 } ;
static	int	StartFlag = 0 ;

static	PL_PollingSet	PollingSet ;

/* hzx_group_no <--> shadow_hzx_id §Œ£±¬–£±¬–±˛ */

/*----------------------------------------------------------------*/

/* ±∆∞Ã√÷∑◊ªª */
static	int		ExprShadowPos( work )
PlayerWork		*work ;
{
	FVECTOR		from, to, dir ;
	int			c ;
	HZX_GROUP_ID	hzx_id, tmp ;

	if ( StartFlag == 0 || GM_N_PlayerShadowPos == GM_MAX_PLAYER_SHADOW_POS ) return 0 ;
	hzx_id = ( HZX_GROUP_ID )ShadowHazard[ GV_GetNo( work->control.hzx_id ) ] ;
	if ( hzx_id == -1 ) return 0 ;

	ObjPos( work, HUMAN21_ATAMA, &from ) ;
	dir.vx = work->lights[ 0 ].m[ 0 ][ 0 ] ;
	dir.vy = work->lights[ 0 ].m[ 1 ][ 0 ] ;
	dir.vz = work->lights[ 0 ].m[ 2 ][ 0 ] ;
	GV_LenVec3F( &dir, &dir, 0.0F, 10000.0F ) ; /* £±£∞£Ì */
	_sceVu0AddVector( &to, &from, &dir ) ;

	tmp = HZX_CurrentGroupID ;
	HZX_CurrentGroupID = 0 ;
	c = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL,
							   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;
	HZX_CurrentGroupID = tmp ;

	if ( c ) {
		HZX_GetOnlinePoint( &GM_PlayerShadowPos[ GM_N_PlayerShadowPos ] ) ;
#if 0
		{
			FVECTOR		v[ 2 ] ;

			DG_COPY_VEC( &v[ 0 ], &from ) ;
			DG_COPY_VEC( &v[ 1 ], &GM_PlayerShadowPos[ GM_N_PlayerShadowPos ] ) ;
			NewLineView( v, 1, 23, 232, 23 ) ;
		}
#endif
		GM_N_PlayerShadowPos ++ ;
	}
	return 0 ;
}

#endif

/*----------------------------------------------------------------*/

int		NewShadowDropHazard( void )
{
	extern	int		NewSpotDropHazard( void ) ;
#if 0
	HZX_GROUP_ID	hzx_id, shadow_id ;
	int				no ;
#endif
	/* «—ªﬂ */
	NewSpotDropHazard() ;
	return 0 ;
#if 0
	/* «∞§Œ§ø§· */
	if ( StartFlag == 0 ) {
		int			i ;
		for ( i = 0; i < GM_MAX_MAPS; i ++ ) ShadowHazard[ i ] = -1 ;
	}

	/* •›°º•Í•Û•∞∆ÛΩ≈≈–œøÀ…ªﬂ */
	PL_RemovePollingFunc( &PollingSet ) ;
	PL_AddPollingFunc( &PollingSet, ExprShadowPos ) ;

	if ( GCL_GetOption( 'n' ) != NULL ) {
		hzx_id = GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt() ) ) ;
		ASSERT( hzx_id != 0 ) ;
	} else {
		hzx_id = GM_GetHzxGroupID( GM_CurrentMap ) ;
	}
	GCL_GetOption( 'g' ) ;
	shadow_id = ( HZX_GROUP_ID )GV_GetBit( GCL_GetNextInt() ) ;
	
	while( hzx_id != 0 ) {
		no = GV_GetNo( hzx_id ) ;
		hzx_id &= ~GV_GetBit( no ) ;
		ShadowHazard[ no ] = shadow_id ;
	}
	StartFlag = 1 ;
	return 0 ;
#endif
}

