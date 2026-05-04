//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   rai_nude_ik.c
   はだかライデンの手ＩＫ
   
   2001/07/26	M.Sonoyama
   $Id: rai_nude_ik.c,v 1.1.1.3 2002/11/19 11:51:01 Yoshizawa1 Exp $
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
#include	"utl_dma.h"
#include	"pl_define.h"
#include	"pl_work.h"

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT			actor ;
	SVECTOR			rot_adj ;
} Work ;

static	void	Act( Work *work )
{
	SVECTOR		adj	= { 1, 1, 196 }, radj ;
	FVECTOR		quat ;

	if ( GM_PlayerWork == NULL ) return ;
	if ( GM_PlayerWork->act_name == SQUAT_CAUTION_LEFT &&
		 GM_CheckPlayerStatus( PLAYER_SQUAT ) &&
		 GM_CheckPlayerStatus( PLAYER_CAUTION ) &&
		 GM_CheckPlayerStatus( PLAYER_MOVE ) &&
		 GM_CheckPlayerStatusEX( I64(0), PLAYER2_KNOCK_WALL ) ) {
		GV_NearExp4PV( &work->rot_adj, &adj, 3 ) ;
	} else {
		GV_NearExp4PV( &work->rot_adj, &DG_ZeroSVector, 3 ) ;
	}
	radj = work->rot_adj ;
	if ( radj.vz != 0 ) {
		GM_RotToQuat( &radj, &quat ) ;
		DG_COPY_VEC( &GM_PlayerWork->body.m_ctrl->adjust[ HUMAN21_HIDARI_UDE2 ], &quat ) ;
		radj.vz *= -1 ;
		GM_RotToQuat( &radj, &quat ) ;
		DG_COPY_VEC( &GM_PlayerWork->body.m_ctrl->adjust[ HUMAN21_HIDARI_TE ], &quat ) ;
		GM_PlayerWork->body.m_ctrl->adjust_flag |= 1 << HUMAN21_HIDARI_UDE2 ;
		GM_PlayerWork->body.m_ctrl->adjust_flag |= 1 << HUMAN21_HIDARI_TE ;
	}
}

static	void	Die( Work *work )
{

}

/*----------------------------------------------------------------*/

/* はだかモード設定 */
int		PL_COM_SetNudeMode( void )
{
	Work		*work ;

	//ASSERT( GM_PlayerWork != NULL ) ; /* プレイヤー起動チェック */
	GM_SetPlayerStatusEX( I64(0), PLAYER2_NUDE ) ;

	/* ＩＫコントロールの起動 */
	work = GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
						   sizeof( Work ), PLAYER_CHECK_ACTOR_PRIO ) ;
	if ( work == NULL ) ASSERT( 0 ) ;
	GV_SetActor( &work->actor, Act, Die ) ;
	return 0 ;
}

