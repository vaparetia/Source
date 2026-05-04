//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   spothzx.c
   スポットライト投影ハザード
   
   2000/05/22 M.Sonoyama
   $Id: spothzx.c,v 1.1.1.3 2002/11/19 11:50:50 Yoshizawa1 Exp $
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
#include	"g_other.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

static	HZX_GROUP_ID	SpotHazard[ GM_MAX_MAPS ] = { -1 } ;
static	int	StartFlag = 0 ;

static	PL_PollingSet	PollingSet ;

/* hzx_group_no <--> shadow_hzx_id の１対１対応 */

/*----------------------------------------------------------------*/

/* 影位置計算 */
static	int		ExprShadowPos( work )
PlayerWork		*work ;
{
	FVECTOR		from, to, dir ;
	int			c, n_lights, i ;
	HZX_GROUP_ID	hzx_id, tmp ;
	GCT_ProjectionSpot_SpotParam	*spot ;

	n_lights = GC_ProjectionSpot_HitLights ;
	if ( StartFlag == 0 || n_lights == 0 ||
		 Status( PLAYER_STEALTH | PLAYER_CB_BOX ) ||
		 GM_N_PlayerShadowPos == GM_MAX_PLAYER_SHADOW_POS ) return 0 ;
	hzx_id = ( HZX_GROUP_ID )SpotHazard[ GV_GetNo( work->control.hzx_id ) ] ;
	if ( hzx_id == -1 ) return 0 ;
	
	tmp = HZX_CurrentGroupID ;
	HZX_CurrentGroupID = 0 ;
	if ( work->body.objs->flag & DG_FLAG_INVISIBLE0 ) {
		/* 主観のときはカメラ位置 */
		DG_COPY_VEC( &from, &work->subject_camera->position ) ;
	} else {
		ObjPos( work, HUMAN21_ATAMA, &from ) ;		
	}

	spot = GC_ProjectionSpot_SnakeCheckList ;
	for ( i = 0; i < n_lights; i ++ ) {
		if ( spot->flag == 1 ) {
			/* 平行光源 */
//			GV_LenVec3F( &spot->dir, &dir, 0.0F, spot->range * 2.0F ) ;
			GV_LenVec3F( &spot->dir, &dir, 0.0F, 10000.0F ) ;
			_sceVu0AddVector( &to, &from, &dir ) ;
		} else {
			_sceVu0SubVector( &dir, &from, &spot->pos ) ;
			/* 発見される影は、スポットから影響距離の半分までの間にある */
			GV_LenVec3F( &dir, &dir, 0.0F, spot->range * 2.0F ) ;
			_sceVu0AddVector( &to, &spot->pos, &dir ) ;
		}

		c = HZX_OnlineHazardCheck( hzx_id, &from, &to, HZX_CHK_ALL,
								   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;
#if 0
		{
			FVECTOR		v[ 2 ] ;

			DG_COPY_VEC( &v[ 0 ], &from ) ;
			DG_COPY_VEC( &v[ 1 ], &to ) ;
			NewLineView( v, 1, 232, 32, 32 ) ;
		}
#endif
		if ( c ) {
			HZX_GetOnlinePoint( &GM_PlayerShadowPos[ GM_N_PlayerShadowPos ] ) ;
			GM_PlayerShadowPos[ GM_N_PlayerShadowPos ].vy += 1.0F ;
#if 0
			{
				FVECTOR		v[ 2 ] ;

				DG_COPY_VEC( &v[ 0 ], &from ) ;
				DG_COPY_VEC( &v[ 1 ], &GM_PlayerShadowPos[ GM_N_PlayerShadowPos ] ) ;
				NewLineView( v, 1, 23, 232, 23 ) ;
			}
#endif
			if ( ++ GM_N_PlayerShadowPos == GM_MAX_PLAYER_SHADOW_POS ) break ;
		}
		spot ++ ;
	}
	HZX_CurrentGroupID = tmp ;
	return 0 ;
}

/*----------------------------------------------------------------*/

int		NewSpotDropHazard( void )
{
	HZX_GROUP_ID	hzx_id, shadow_id ;
	int				no ;

	/* 念のため */
	if ( StartFlag == 0 ) {
		int			i ;
		for ( i = 0; i < GM_MAX_MAPS; i ++ ) SpotHazard[ i ] = -1 ;
	}

	/* ポーリング二重登録防止 */
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
		SpotHazard[ no ] = shadow_id ;
	}
	StartFlag = 1 ;
	return 0 ;
}

