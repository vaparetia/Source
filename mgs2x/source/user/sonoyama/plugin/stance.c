//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   stance.c
   強制姿勢制御プラグイン

   2000/05/11 M.Sonoyama
   $Id: stance.c,v 1.1.1.3 2002/11/19 11:50:50 Yoshizawa1 Exp $
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
static	int				stance ;
static	int				sdir, edir, flag ;

/*---------------------------------------------------------------*/

/* 強制姿勢制御実体 */
static	void	ForceStanceAct( work, time )
PlayerWork		*work ;
int				time ;
{
	int				st ;
	PL_MOTION_SET	*ms ;

	ms = PL_MotionSet ;
	if ( time == 0 ) {
		if ( sdir != -1 ) {
			work->control.turn.vy = work->control.rot.vy = sdir ;
		}
		if ( WeaponType( work ) & WP_TYPE_SUBJECT ) PL_NoWeapon( work ) ;
		if ( ItemType( work ) & IT_TYPE_ABNORMAL ) PL_NoEquip( work ) ;
		PL_ChangeMotionArc( work, work->org_motion ) ;
	}

	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_FORCE ) ;
	if ( flag & FA_NO_WEAPON ) SetStatus( PLAYER_WEAPON_INVISIBLE ) ;

	st = work->stance ;
	switch( stance ) {
	case 0 :		/* 立ちへ */
		if ( work->data == 0 ) {
			if ( st == STAND || st == SQUAT ) {
//				work->stance = STAND ;
				goto stance_act_end ;
			} else {
				SetAction( work, ms->change[ Mcrouch_stand ], 6 ) ;
				work->data = 1 ;
			}
		} else {
			if ( EndMotion( work ) ) {
//				work->stance = STAND ;
				goto stance_act_end ;
			}
		}
		break ;
	case 1 :		/* しゃがみへ */
		if ( work->data == 0 ) {
			if ( st == STAND || st == SQUAT ) {
//				work->stance = SQUAT ;
				goto stance_act_end ;
			} else {
				SetAction( work, ms->change[ Mcrouch_stand ], 6 ) ;
				work->data = 1 ;
			}
		} else {
			if ( EndMotion( work ) ) {
//				work->stance = SQUAT ;
				goto stance_act_end ;
			}
		}
		break ;
	case 2 :		/* 匍匐へ */
		if ( work->data == 0 ) {
			if ( st == STAND || st == SQUAT ) {
				SetAction( work, ms->change[ Msquat_crouch ], 6 ) ;
				work->data = 1 ;
			} else {
				goto stance_act_end ;
			}
		} else {
			if ( EndMotion( work ) ) {
//				work->stance = GROUND ;
				goto stance_act_end ;
			}
		}
		break ;
	default :
		ASSERT( 0 ) ;
	}
	PL_ExecForceActProc( work, FA_PROCMODE_STANCE, time, 0 ) ;
	return ;
stance_act_end :
	PL_UnsetInvincible( work ) ;
	if ( edir != -1 ) {
		work->control.turn.vy = edir ;
	}
//	UnsetStatus( PLAYER_FORCE ) ;
	PL_EndForceAct( work, FA_USE_DEFAULT, -1 ) ;
	GM_ResetMenuStatus( MENU_MENU_OFF ) ;
//	SetMode( work, PL_StillMode[ work->stance ] ) ;
	SetMode( work, PL_StillMode[ stance ] ) ;
	PL_ExecForceActProc( work, FA_PROCMODE_STANCE, 1, 1 ) ;
}

/*---------------------------------------------------------------*/

/* セットアップ関数 */
static	int	SetStanceAct( work, msg, len )
PlayerWork		*work ;
GV_MSG			*msg ;
int				len ;
{
	
	msg->message_len = len ;
	if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return 0 ;
	if ( Status( PLAYER_DEAD ) ) {
		GV_ERROR( GV_ERROR_PL_MESG_FAIL ) ;
		UnsetStatus( PLAYER_FORCE ) ;
		return -1 ;
	}
	msg->message_len = 0 ;
	stance = msg->message[ 1 ] ;
	sdir = msg->message[ 2 ] ;
	edir = msg->message[ 3 ] ;
	flag = msg->message[ 4 ] ; 
	PL_LeaveCaution( work ) ;
	PL_SetInvincible( work, 0 ) ;
	PL_LeaveSubject( work ) ;
	PL_ClearCaptureTarget( work ) ;
	PL_UnequipSpecials() ;
	GM_ClearTargetDamage( &( work->def ) ) ;
	UnsetFlag( FLAG_FORCE_END ) ;
	UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE ) ;
	SetStatus( PLAYER_FORCE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	SetMode2( work, NULL ) ;
	SetMode( work, ForceStanceAct ) ;
	return 1 ;
}

/*---------------------------------------------------------------*/

/* プラグイン登録 */
int	NewPluginStance( void )
{
    PL_AddPlugin( &PluginSet, PL_MSG_STANCE, SetStanceAct, NULL ) ;
    return 0 ;
}

