//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   superblow.c
   強制吹っ飛ばし（ハリアー戦専用）

   2001/07/10	M.Sonoyama
   $Id: superblow.c,v 1.1.1.3 2002/11/19 11:50:51 Yoshizawa1 Exp $
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
#include	"../raiden/motion.h"

#include "BP_Misc.h"

static	PL_PluginSet	PluginSet ;
static	int				Damage ;
static	FVECTOR			Force ;
static	int				SuperBlowFlag = 0 ;

static	void  	SuperBlow( PlayerWork *work, int time ) ;

extern	u_char	PL_DamageVib2H[] ;
extern	u_char	PL_DamageVib2L[] ;

/*----------------------------------------------------------------*/

static	void	SuperBlow( PlayerWork *work, int time )
{
    int			sink, dir ;
    SVECTOR		rot ;

	if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, work->org_motion ) ;
		PL_SetAction( work, none_damage_near, 0 ) ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / 2.5F ) ;
		SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_WAIST_INTERP | FLAG_HZX_HEIGHT_QUICK ) ;
		work->control.step.vy = 0.0F ;
		PL_StopTurn( work ) ;
		GM_SeSetMode( SD_V_PDMG01, &work->control.mov, GM_SEMODE_BOMB ) ;
		NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
	}
	SetStatus( PLAYER_FORCE | PLAYER_MENU_DISABLE ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH |
			 FLAG_NO_MOTION_STEP | FLAG_NO_GRAVITY | FLAG_NO_IK | 
			 FLAG_NO_TOUCH_DAMAGE | FLAG_SPECIAL_MODE ) ;

	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	PL_GroundIK2( work, 500.0F ) ;

    switch( work->data ) {
    case 0 :
		/* めり込みチェック */
		rot.vx = 0 ;
		rot.vy = work->control.rot.vy ;
		rot.vz = 0 ;
		sink = PL_AvoidSink( work, 200.0F, 0.0F ) ;
		if ( ( sink & 4 ) || ( ( sink & 3 ) == 3 ) ) {
			PL_StopTurn( work ) ;
			work->data = 1 ;
			goto search_dir_superblow ;
		}
		break ;
    case 1 :
search_dir_superblow :
		dir = PL_CheckGroundEnableDir( work ) ;
		if ( dir != -1 ) {
			work->control.turn.vy = dir ;
			work->control.interp = 8 ;
			work->data = 2 ;
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		} else {
			SetFlag( FLAG_NO_STEP | FLAG_NO_MOTION_STEP ) ;
		}
		break ;
    case 2 :
		if ( work->control.interp == 0 ) work->data = 3 ;
		break ;
    case 3 :
		sink = PL_AvoidSink( work, 200.0F, 0.0F ) ;
		if ( ( sink & 4 ) || ( ( sink & 3 ) == 3 ) ) {
			PL_StopTurn( work ) ;
			goto search_dir_superblow ;
		} 
    }

	Force.vy = GV_NearExp16F( Force.vy, 0.0F ) ;

	if ( !( work->control.grounded & 1 ) ) {
		work->control.step.vx = Force.vx ;
		work->control.step.vz = Force.vz ;
	} else {
		Force.vx = 0.0F ;
		Force.vz = 0.0F ;
		work->control.step.vx = Force.vx ;
		work->control.step.vz = Force.vz ;
	}

	if ( !( work->control.grounded & 1 ) ) {

      if ( BP_IsPAL()==TRUE )
		   work->control.step.vy = 1.20F * ( Force.vy - 8.0F * work->ftime ) ;
      else
		   work->control.step.vy = Force.vy - 8.0F * work->ftime ;

   } else {
		work->control.step.vy = 0.0F ;
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / 1.5F ) ;
	}

	if ( EndMotion( work ) ) {
		work->down_dir = DOWN_FACE ;
		SetStatus( PLAYER_DOWNED | PLAYER_GROUND ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		UnsetStatus( PLAYER_DAMAGED | PLAYER_FORCE ) ;
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
		SuperBlowFlag = 0 ;
		SetMode( work, PL_Down ) ;
    }
}

/*----------------------------------------------------------------*/


/* セットアップ */
static	int		SetSuperBlowAct( PlayerWork *work, GV_MSG *msg, int len )
{
	msg->message_len = len ;
	/* 死亡時以外は全部ＯＫ */
	if ( SuperBlowFlag != 0 || GM_IsGameOver() ) return -1 ;
	Damage = msg->message[ 1 ] ;
	Force.vx = msg->message[ 2 ] ;
	Force.vy = msg->message[ 3 ] ;
	Force.vz = msg->message[ 4 ] ;
	/* 死なない */
	work->power.vital -= Damage ;
	if ( work->power.vital <= 0 ) work->power.vital = 1 ;

	/*エルード中は、grounded を ０にする */
	if ( Status( PLAYER_BEYOND ) ) {
		work->control.grounded = 0 ;
		/* グリップゲージＯＦＦ */
		GM_InvisibleGage( &work->hand_power ) ;
	}

	UnsetFlag( FLAG_FORCE | FLAG_FORCE_END | FLAG_MSG_SHADOW_OFF ) ;
	UnsetStatus( PLAYER_BEYOND | PLAYER_PAD_OFF | PLAYER_DAMAGED | PLAYER_DOWNED ) ;
	PL_LeaveCaution( work ) ;
	PL_SetInvincible( work, 0 ) ;
	PL_LeaveSubject( work ) ;
	PL_ClearCaptureTarget( work ) ;
	PL_UnequipSpecials() ;
	PL_ClearSpecialCallback() ;
	SetStatus( PLAYER_FORCE | PLAYER_DAMAGED | PLAYER_MENU_DISABLE ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_SPECIAL_MODE ) ;
	work->control.skip_flag &= ~( CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	SetMode2( work, NULL ) ;
	work->trigger = TRIG_FALL ;	/* グレネード落とし */

	SetMode( work, SuperBlow ) ;
	msg->message_len = 0 ;
	SuperBlowFlag = 1 ;
	return 1 ;
}


/* プラグイン登録 */
int	NewPluginSuperBlow( void )
{
    PL_AddPlugin( &PluginSet, PL_MSG_SUPERBLOW, SetSuperBlowAct, NULL ) ;
	SuperBlowFlag = 0 ;
	return 0 ;
}

