//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dv_goggles.c
   潜水服ゴーグル主観

   2001/04/12	M.Sonoyama
   $Id: dv_goggles.c,v 1.1.1.3 2002/11/19 11:50:42 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

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
#include	"camera.h"

#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"

extern	void	*NewScrGoggles( int mode, int camera_num ) ;
extern	void	*NewScrGoggles_demo( int mode, int camera_num ) ;

typedef	struct	{
	GV_ACT			actor ;
	DG_OBJS			*body ;
	CONTROL			*pctrl ;
	OBJECT			*pbody ;
	int				map ;
	int				name ;
	int				flag ;
	float			rate ;
	int				last_se ;
	int				last_se_time ;
	int				on ;
} Work ;

/*----------------------------------------------------------------*/

static	inline	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	if ( n_msg ) {
		while( -- n_msg >= 0 ) {
			if ( msg->message[ 0 ] == 0 ) {
				work->flag = 0 ;	/* on */
			} else {
				work->flag = 1 ;	/* off */
			}
			msg ++ ;
		}
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GM_CameraSet	*cam ;
	PlayerWork		*pWork ;
	int				act ;

	CheckMessage( work ) ;

	if ( work->flag == 1 ) {
		//DG_InvisibleObjs( work->body ) ;
		NewScrGoggles( -1, 0 ) ;
		NewScrGoggles_demo( -1, 0 ) ;
		work->on = 0 ;
		return ;
	}
	
	cam = GM_GetCurrentCamera( 0 ) ;

	if ( GM_CheckGameStatus( STATE_DEMO ) ) {
		NewScrGoggles( -1, 0 ) ;
		NewScrGoggles_demo( -1, 0 ) ;
		work->on = 0 ;
	} else {
		if ( !( work->pbody->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
			/* プレイヤー表示中 */
			NewScrGoggles( -1, 0 ) ;
			NewScrGoggles_demo( -1, 0 ) ;
			//DG_InvisibleObjs( work->body ) ;
			work->last_se = -1 ;
			work->on = 0 ;
			return ;
		} else if ( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			/* 主観でない */
			NewScrGoggles( -1, 0 ) ;
			NewScrGoggles_demo( -1, 0 ) ;
			//DG_InvisibleObjs( work->body ) ;
			work->last_se = -1 ;
			work->on = 0 ;
			return ;
		} else {
			GM_CameraSet	*next ;
			
			next = GM_GetNextCamera( 0 ) ;
			if ( work->on == 0 ) {
				if ( GM_Camera->chanl[ 0 ].time > 0 &&
					 GV_VecLen3F2( &cam->position, &next->position ) > 240.0F ) {
					/* まだカメラが位置についてない */
					NewScrGoggles( -1, 0 ) ;
					NewScrGoggles_demo( -1, 0 ) ;
					//DG_InvisibleObjs( work->body ) ;
					work->last_se = -1 ;
					return ;				
				} else {
					work->on = 1 ;
				}
			} 
		}
	}
#if 0	
	{
		static	FVECTOR	shift = { 0.0F, 20.0F, 320.0F } ;
		static	float	scale = 1.0F ;
		FVECTOR			mov ;
		FMATRIX			world ;
#if 0
		if ( GV_PadData->status & PAD_A ) shift.vz += 10.0F ;
		else if ( GV_PadData->status & PAD_B ) shift.vz -= 10.0F ;
		if ( GV_PadData->status & PAD_X ) scale += 0.01F ;
		else if ( GV_PadData->status & PAD_Y ) scale -= 0.01F ;
		if ( scale < 0.0F ) scale = 0.0F ;
		MENU_Locate( 16,64,0 ) ;
		MENU_Printf( "%.2f\n", shift.vz ) ;
		MENU_Printf( "%.2f\n", scale ) ;
#endif
		DG_SetPos2( &cam->position, &cam->rotate ) ;
		DG_MovePos( &shift ) ;
		DG_GetPos( &world ) ;
		GV_MatToVec( &world, &mov ) ;
		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &cam->position, &mov,
								    HZX_CHK_ALL, 0, 0 ) ) {
			FVECTOR			diff ;

			HZX_GetOnlineVector( &diff ) ;
			_sceVu0ScaleVector( &diff, &diff, 0.85F ) ;
			_sceVu0AddVector( &mov, &cam->position, &diff ) ;
			GV_VecToMat( &mov, &world ) ;
		}
		_sceVu0ScaleVector( ( FVECTOR * )world.m[ 0 ], ( FVECTOR * )world.m[ 0 ], scale ) ;
		_sceVu0ScaleVector( ( FVECTOR * )world.m[ 1 ], ( FVECTOR * )world.m[ 1 ], scale ) ;
		_sceVu0ScaleVector( ( FVECTOR * )world.m[ 2 ], ( FVECTOR * )world.m[ 2 ], scale ) ;
		DG_COPY_MAT( &work->body->world, &world ) ;
	}
#endif
	if ( GM_CheckGameStatus( STATE_SCN_DEMO | STATE_PRG_DEMO ) ) {
		NewScrGoggles( -1, 0 ) ;
		NewScrGoggles_demo( 0, 0 ) ;
	} else {
		NewScrGoggles( 0, 0 ) ;
		NewScrGoggles_demo( -1, 0 ) ;
	}
	//DG_VisibleObjs( work->body ) ;
	//GM_GroupObjs( work->body, GM_CurrentStageMap ) ;

	if ( GM_CheckGameStatus( STATE_DEMO ) ) {
		work->last_se = -1 ;
		return ;
	}

	/* 呼吸音 */
	pWork = GM_PlayerWork ;
	if ( pWork == NULL ) {
		work->last_se = -1 ;
		return ;
	}
	act = pWork->act_name ;

	if ( pWork->ftime < 60 ) return ;

	if ( act == STAND_STILL || 
		 act == SQUAT_STILL ||
		 act == GROUND_STILL ||
		 act == STAND_CAUTION_STILL ||
		 act == SQUAT_CAUTION_STILL ||
		 act == INTRUDE_STILL ) {
		float			rate ;
		MOTION_CONTROL	*m_ctrl ;

		m_ctrl = GM_PlayerBody->m_ctrl ;
		rate = m_ctrl->mt3_ctrl[ 0 ].play_time 
				/ m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		
		if ( GV_Time - work->last_se_time < 48 ) return ;

		if ( work->rate < 0.10F && rate >= 0.10F && work->last_se != 0 ) {
			GM_SeSetMode( SD_I_MASK02, &cam->position, GM_SEMODE_BOMB ) ;
			work->last_se = 0 ;
			work->last_se_time = GV_Time ;
		} else if ( work->rate < 0.55F && rate >= 0.55F && work->last_se != 1 ) {
			GM_SeSetMode( SD_I_MASK01, &cam->position, GM_SEMODE_BOMB ) ;
			if ( cam->position.vy < GM_WaterLevel ) {
				GM_SeSetMode( SD_P_INWBRES1, &cam->position, GM_SEMODE_BOMB ) ;
			}
			work->last_se = 1 ;
			work->last_se_time = GV_Time ;
		}
		work->rate = rate ;
	} else {
		if ( ( GV_Time % DIRECT_TICK( 120 ) ) == 0 && work->last_se != 0 ) {
			GM_SeSetMode( SD_I_MASK02, &cam->position, GM_SEMODE_BOMB ) ;
			work->last_se = 0 ;
			work->last_se_time = GV_Time ;
		} else if ( ( GV_Time % DIRECT_TICK( 120 ) ) == DIRECT_TICK( 60 ) && work->last_se != 1 ) {
			GM_SeSetMode( SD_I_MASK01, &cam->position, GM_SEMODE_BOMB ) ;
			if ( cam->position.vy < GM_WaterLevel ) {
				GM_SeSetMode( SD_P_INWBRES1, &cam->position, GM_SEMODE_BOMB ) ;
			}
			work->last_se = 1 ;
			work->last_se_time = GV_Time ;
		}
		work->rate = 0.0F ;
	} 
}

static	void	Die( Work *work ) 
{
#if 0
	DG_FreeObjs( work->body ) ;
	DG_DequeueObjs( work->body ) ;
#endif
	NewScrGoggles( -1, 0 ) ;
	NewScrGoggles_demo( -1, 0 ) ;
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_DIVE_GOGGLES ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work, int name, int where )
{
#if 0
	DG_OBJS		*body ;
	DG_DEF		*def ;

	def = ( DG_DEF * )GV_GetCache( GV_CacheID( GCL_GetOptionValue( 'm', 0 ), 'k' ) ) ;
	ASSERT( def != NULL ) ;
	body = work->body = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_NOFOG, 0 ) ;
	ASSERT( body != NULL ) ;
	DG_QueueObjs( body ) ;
	DG_InvisibleObjs( body ) ;
#endif
	if ( GM_CheckGameStatus( STATE_DEMO ) ) {
		work->pctrl = NULL ;
		work->pbody = NULL ;
	} else {
		work->pctrl = GM_PlayerControl ;
		work->pbody = GM_PlayerBody ;
	}

	work->name = name ;
	work->map = where ;
	work->last_se = -1 ;
	work->last_se_time = GV_Time ;

	NewScrGoggles( 0, 0 ) ;
	NewScrGoggles_demo( 0, 0 ) ;
	NewScrGoggles( 1, 0 ) ;
	NewScrGoggles_demo( 1, 0 ) ;

	GM_SetPlayerStatusEX( I64(0), PLAYER2_DIVE_GOGGLES ) ;
	return 0 ;
}

/*----------------------------------------------------------------*/

void		*NewDivingGoggles( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
