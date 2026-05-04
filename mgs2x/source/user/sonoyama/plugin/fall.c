//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fall.c
   落下処理プラグイン

   2001/03/15 M.Sonoyama
   $Id: fall.c,v 1.1.1.3 2002/11/19 11:50:48 Yoshizawa1 Exp $
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

static	PL_PluginSet	PluginSet ;
static	PL_PollingSet	PollingSet ;

enum {
	FLstart,
	FLloop,
	FLend,
	MAX_FALL_MOTIONS
} ;

enum {
	FALL_FLAG_NONE		= 	0x0000,
	FALL_FLAG_NO_OVER	=	0x0001,
	FALL_FLAG_SHADOWOFF	=	0x0002,
	FALL_FLAG_START_SKIP =	0x0004,
	FALL_FLAG_NODEADSE	=	0x0008,
	FALL_FLAG_GOFAST =		0x0010,

	FALL_FLAG_FALLACT_NOW = 	0x8000,
} ;

static	int		Motions[ MAX_FALL_MOTIONS ] ;
static	int		StartProc = 0, EndProc = 0 ;
static	float	FallEndHeight ;
static	int		Flag ;

/*---------------------------------------------------------------*/

static	void	FallStart( PlayerWork *, int ) ;
static	void	FallLoop( PlayerWork *, int ) ;
static	void	FallEnd( PlayerWork *, int ) ;

/*---------------------------------------------------------------*/

static	void	FallStart( PlayerWork *work, int time )
{
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK ) ;
	if ( time == 0 ) {
		if ( StartProc > 0 ) GM_ExecProc( StartProc, NULL ) ;
		if ( !( Flag & FALL_FLAG_NO_OVER ) ) {
			GM_GameOverProcStart( &work->actor ) ;
			SetStatus( PLAYER_DEAD ) ;
		}
		PL_ChangeMotionArc( work, PL_Force->marfile ) ;
		PL_SetAction( work, Motions[ FLstart ], 0 ) ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		PL_LeaveSubject( work ) ;
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
	}
	if ( Flag & FALL_FLAG_SHADOWOFF ) work->shadow = 0 ;
	PL_LevelCheck( work ) ;
	work->control.step.vy = 0.0F ;
	if ( EndMotion( work ) || ( Flag & FALL_FLAG_START_SKIP ) ) {
		PL_SetMode( work, FallLoop ) ;
	}
}

static	void	FallLoop( PlayerWork *work, int time )
{
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK ) ;
	if ( time == 0 ) {
		PL_SetAction( work, Motions[ FLloop ], 0 ) ;
		//work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		work->control.grounded = 0 ;
		work->fdata = -ELUDE_FALL_SPEED_FIRST ;
		work->fv.vx = work->control.mov.vy ;
		if ( !( Flag & FALL_FLAG_NODEADSE ) ) {
#ifdef KP_XBOX	// 遮蔽計算切る
			GM_SeSetModeAddr( SD_V_PFALL01, &work->control.mov, GM_SEMODE_BOMB, GM_INVALID_ADDR ) ;
#else			
			GM_SeSetMode( SD_V_PFALL01, &work->control.mov, GM_SEMODE_BOMB ) ;
#endif			
		}
	}

	/* 1.5ｍ以上落ちたら床チェック再開 */
	if ( work->fv.vx - work->control.mov.vy > 1500.0F ) {
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
	} else {
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
	}

	if ( Flag & FALL_FLAG_SHADOWOFF ) work->shadow = 0 ;

	if ( work->control.mov.vy > GM_WaterLevel - 1000.0F ) work->fdata -= ELUDE_FALL_SPEED ;
	else												  work->fdata = -ELUDE_FALL_SPEED_FIRST ;

	work->control.step.vy = work->fdata ;

	/* ふみつけ */
	if ( work->fv.vx - work->control.mov.vy > 1500.0F ) {
		FVECTOR		size ;

		size.vx = size.vz = 150.0F ;
		size.vy = 150.0F ;
		GM_SetTarget( &work->offense, TARGET_OFFENSE | TARGET_POWER, 0,
					  ENEMY_SIDE, &size, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( &work->offense, WP_STAMP ) ;
		GM_MoveTarget3( &work->offense, PL_ObjWorld( work, HUMAN21_MIGI_KAKATO ) ) ;
		GM_PutTarget( &work->offense ) ;		
	}

	if ( work->data == 0 ) {
		if ( work->control.mov.vy < FallEndHeight ) {
			if ( EndProc > 0 ) GM_ExecProc( EndProc, NULL ) ;
			work->data = 1 ;
		}
		if ( work->control.grounded & 1 ) {
			PL_SetMode( work, FallEnd ) ;
		}
	} else if ( work->data == 1 ) {
		/* 1秒半ぐらいでゲームオーバーロゴ */
		if ( ++ work->data2 > DIRECT_TICK( 90 ) || ( Flag & FALL_FLAG_GOFAST ) ) {
			GM_GameOverProcEnd( &work->actor ) ;
		}
	}
}

static	void	FallEnd( PlayerWork *work, int time )
{
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK ) ;
	if ( time == 0 ) {
		/* 死んだ状態で落ちた */
		if ( ( Flag & FALL_FLAG_NO_OVER ) && Status( PLAYER_DEAD ) ) {
			/* 今は通常死にを出しとくが、エルード落下死亡がいいかも */
			PL_ChangeMotionArc( work, work->org_motion ) ;
			PL_SetAction( work, PL_MotionSet->shared[ Mdamage_out ], 6 ) ;
			Flag &= ~FALL_FLAG_NO_OVER ;
		} else {
			PL_SetAction( work, Motions[ FLend ], 0 ) ;
		}
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		work->body.m_ctrl->old_height = work->control.height ;
		work->body.m_ctrl->root_old_height = work->control.height ;
	}
	if ( Flag & FALL_FLAG_SHADOWOFF ) work->shadow = 0 ;
	if ( EndMotion( work ) ) {
		if ( EndProc > 0 ) GM_ExecProc( EndProc, NULL ) ;
		if ( !( Flag & FALL_FLAG_NO_OVER ) ) {
			PL_MotionSleep( work, 0 ) ;
			GM_GameOverProcEnd( &work->actor ) ;			
		} else {
			GM_ResetMenuStatus( MENU_MENU_OFF ) ;
			PL_ReturnMotionArc( work ) ;
			UnsetStatus( PLAYER_FORCE ) ;
			PL_UnsetInvincible( work ) ;
			PL_ResetAddress( work ) ;
			PL_SetMode( work, PL_StillMode[ STAND ] ) ;
			/* 落とし穴フラグを戻す */
			work->control.flr_flag &= ~HZX_FLOOR_PITFALL ;
			Flag &= ~FALL_FLAG_FALLACT_NOW ;
		}
	}
}

/*---------------------------------------------------------------*/

/* セットアップ */
static	int	SetFallAct( PlayerWork *work, GV_MSG *msg, int len )
{
	//int			dead_ok ;

	msg->message_len = len ;
	Flag = 0 ;

	/* エルード落下特殊 */
	if ( ( work->act_name == ELUDE_RETURN_MODE && PL_MotionTime( work ) > 73 ) ||
		 work->act_name == ELUDE_TOUCHDOWN ||
		 work->act_name == ELUDE_FALL ) {
		UnsetFlag( FLAG_FORCE | FLAG_MSG_SHADOW_OFF ) ;
		UnsetStatus( PLAYER_BEYOND ) ;
		PL_MotionActive( work, 0 ) ;
		if ( work->act_name == ELUDE_FALL ) {
			if ( ( PL_Beyond->flag & BY_FALL_DEAD ) ||
				 /*( work->hand_power.value <= 0 ) ||*/
				 ( GM_Vitality <= 0 ) ) {
				Flag |= FALL_FLAG_NODEADSE ;
			}
		}
	}

	/* 絶対に落下させる */
	//if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return 0 ;
	UnsetFlag( FLAG_FORCE_END ) ;

#if 0	/* 死亡時もＯＫ */
	if ( Status( PLAYER_DEAD ) ) {
		GV_ERROR( GV_ERROR_PL_MESG_FAIL ) ;
		return -1 ;
	}
#endif
	Motions[ FLstart ] = msg->message[ 1 ] ;
	Motions[ FLloop ] = msg->message[ 2 ] ;
	Motions[ FLend ] = msg->message[ 3 ] ;
	FallEndHeight = ( float )msg->message[ 4 ] ;
	Flag |= msg->message[ 5 ] ;
	StartProc = msg->message[ 6 ] ;
	EndProc = msg->message[ 7 ] ;

	PL_LeaveCaution( work ) ;
	PL_SetInvincible( work, 0 ) ;
	PL_LeaveSubject( work ) ;
	PL_ClearCaptureTarget( work ) ;
	PL_UnequipSpecials() ;
	UnsetFlag( FLAG_FORCE_END ) ;
	UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	SetStatus( PLAYER_FORCE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;

	/* 落とし穴床を通り抜けるように */
	work->control.flr_flag |= HZX_FLOOR_PITFALL ;

	work->trigger = TRIG_FALL ;	/* グレネード落とし */
	SetMode2( work, NULL ) ;
	SetMode( work, FallStart ) ;
	msg->message_len = 0 ;

	Flag |= FALL_FLAG_FALLACT_NOW ;

	return 1 ;
}

static	int		ExecMessageFall( PlayerWork *work )
{
	int			n_msg ;
	GV_MSG		*msg ;

	msg = work->message ;
	n_msg = work->n_msg ;
	while ( -- n_msg >= 0 ) {
		if ( msg->message[ 0 ] == PL_MSG_FALL ) {
			if ( SetFallAct( work, msg, msg->message_len ) ) return 1 ;
		}
		msg ++ ;
	}
	return 0 ;
}

static	int		ExecMessageFallNow( PlayerWork *work )
{
	int			n_msg ;
	GV_MSG		*msg ;

	n_msg = GV_ReceiveMessage( work->control.name, &msg ) ;
	while ( -- n_msg >= 0 ) {
		if ( msg->message[ 0 ] == PL_MSG_FALL ) {
			if ( SetFallAct( work, msg, msg->message_len ) ) return 1 ;
		}
		msg ++ ;
	}
	return 0 ;
}

/* ポーリング関数 */
static	int	PollingFuncFall( PlayerWork *work )
{
	/* 死んだときでも落とす為のものです */
	if ( Status( PLAYER_DEAD ) && !( Flag & FALL_FLAG_FALLACT_NOW ) ) {
		/* すでにＦａｌｌを受信しているかどうかチェック */
		if ( ExecMessageFall( work ) ) {
			printf( "dead fall 1\n" ) ;
			return 1 ;
		}
		/* このフレームにＦａｌｌが来ていないかチェック */
		if ( ExecMessageFallNow( work ) ) {
			printf( "dead fall 2\n" ) ;
			return 1 ;		
		}
	}
	return 0 ;
}

/* プラグイン登録 */
int	NewPluginFall( void )
{
    PL_AddPlugin( &PluginSet, PL_MSG_FALL, SetFallAct, NULL ) ;
	PL_AddPollingFunc( &PollingSet, PollingFuncFall ) ;
	StartProc = EndProc = 0 ;
	Flag = 0 ;
	return 0 ;
}

