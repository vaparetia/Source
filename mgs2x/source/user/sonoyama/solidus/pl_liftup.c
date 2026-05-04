//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_liftup.c
   ソリダス蛇手につかまれるプレイヤープラグイン

   2001/04/06	M.Sonoyama
   $Id: pl_liftup.c,v 1.1.1.3 2002/11/19 11:51:05 Yoshizawa1 Exp $
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

#include	"sol_common.h"

static	PL_PluginSet	PluginSet ;
static	TARGET			Offense ;
static	POWER_TARGET	PtOffense ;
static	int				RStickPress, RStickDir ;

static	int				Motion ;
static	int				LastStruggleTime ;

extern	void			PL_AddDeadAction( PL_DeadMode *this, PL_ACTION action, 
										  PL_DEADMODE_COND_FUNC cond ) ;
extern	void			PL_RemoveDeadAction( PL_DeadMode *this ) ;
static	PL_DeadMode		DeadModeSet ;

void	PL_VisibleO2Gage( void ) ;
void	PL_InvisibleO2Gage( void ) ;

extern	float	GM_O2RecoverValue ;

enum {
	Mlift_pose	=	0,
	Mlift_struggle,
	Mlift_escape_start,
	Mlift_escape_loop,
	Mlift_escape_end,
	Mlift_out,
	Mtrip_f,	
	Mtrip_b,
} ;

/*----------------------------------------------------------------*/

static	void	LiftupStart( PlayerWork *work, int time ) ;
//static	void	LiftupFlingGround( PlayerWork *work, int time ) ;

static	void	LiftupEscape( PlayerWork *work, int time ) ;

static	void	LiftupDamageCallback( PlayerWork *work, TARGET *off, TARGET *def ) ;

/*----------------------------------------------------------------*/

/* 右スティック入力方向 */
/* -1で入力なし */
#define	BLADE_RSTICK_MARGIN_F	(96.0F)
static	short	GetRStickDir( PlayerWork *work )
{
	FVECTOR		rstick_vec ;
	int			c ;

	if ( !PL_UseStickR( work ) ) return -1 ;
	c = 0 ;
	rstick_vec.vx = ( float )work->pad->right_dx - 128.0F ;
	rstick_vec.vz = ( float )work->pad->right_dy - 128.0F ;
	if ( DG_FABS( rstick_vec.vx ) < BLADE_RSTICK_MARGIN_F ) c |= 1 ;
	if ( DG_FABS( rstick_vec.vz ) < BLADE_RSTICK_MARGIN_F ) c |= 2 ;
	if ( c == 3 ) return -1 ;
	return ( GV_VecDir2( &rstick_vec ) & 4095 ) ;
}

static	void	CheckRStickPress( PlayerWork *work )
{
	int			dir ;

	RStickPress = 0 ;
	dir = GetRStickDir( work ) ;
	if ( dir < 0 ) {
		RStickDir = -1 ;
		return ;
	}
	if ( RStickDir < 0 || GV_DiffDirAbs( dir, RStickDir ) >= 768 ) {
		RStickPress = 1 ;
		RStickDir = dir ;
	}
}

static	void	LiftupDamageCallback( PlayerWork *work, TARGET *off, TARGET *def )
{
	printf( "liftup dmg callback!\n" ) ;
	if ( work->current_mar == Motion ) {
		PL_ReturnMotionArc( work ) ;
	}
	GM_ControlResetHeightMotion( &work->control ) ;
	if ( work->dmg_callback == LiftupDamageCallback ) {
		work->dmg_callback = NULL ;
	}
	PL_RemoveDeadAction( &DeadModeSet ) ;
}

/*----------------------------------------------------------------*/

/* 蛇手つかまれ中死亡チェック */
static	int		LiftupDeadConditionFunc( PlayerWork *work ) 
{
	//return ( GM_O2 <= 0 ) ? 1 : 0 ;
	return 1 ;	/* いつでも */
}

/* 蛇手つかまれ最初 */
static	void	LiftupStart( PlayerWork *work, int time ) 
{
	FVECTOR		pmov, nmov, diff ;

	if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		if ( work->current_mar != Motion ) {
			PL_ChangeMotionArc( work, Motion ) ;
		}
		//PL_UnsetInvincible( work ) ;
		PL_LeaveSubject( work ) ;
		SetAction( work, Mlift_pose, 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;

		/* つかまれＳＥ */
		GM_SeSetMode( SD_E_HEBTKAM1, &work->control.mov, GM_SEMODE_BOMB ) ;

		GM_O2 = GM_O2Max ;
	}

	/* ダメージコールバック */
	/* ブレード装備時にポーリングで毎フレームセットされるので
	   それを上書き */
	work->dmg_callback = LiftupDamageCallback ;

	UnsetStatus( PLAYER_NO_BREATH ) ;
	SetFlag( FLAG_NO_GRAVITY | FLAG_NO_MOTION_STEP | FLAG_CANNOT_CHANGE |
			 FLAG_NO_IK | FLAG_SPECIAL_MODE | FLAG_SUBJECT_HORIZON_LIMIT ) ;
	SetFlag2( FLAG2_SUBJECT_MOVE ) ;
	if ( Status( PLAYER_WATCH ) ) GM_SetCameraQuick( 0 ) ;

	work->control.turn.vy = GV_VecDir2FromTo( &work->control.mov, &SOL_SolControl->mov ) ;

	PL_ObjPos( work, HUMAN21_KUBI, &pmov ) ;
	DG_COPY_VEC( &nmov, &pmov ) ;

//	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_FLING ) ) {
		DG_COPY_VEC( &nmov, &SOL_SnakeArmHangPos ) ;
//	} else {
//		GV_NearExp4VF( &nmov, &SOL_SnakeArmHangPos, 3 ) ;
//	}

	//_sceVu0SubVector( &work->control.step, &nmov, &pmov ) ;
	_sceVu0SubVector( &diff, &nmov, &pmov ) ;
	{
		int			i ;
		DG_OBJS		*objs ;

		objs = work->body.objs ;
		objs->world.m[ 3 ][ 0 ] += diff.vx ;
		objs->world.m[ 3 ][ 1 ] += diff.vy ;
		objs->world.m[ 3 ][ 2 ] += diff.vz ;
		for ( i = 0; i < 21; i ++ ) {
			objs->objs[ i ].world.m[ 3 ][ 0 ] += diff.vx ;
			objs->objs[ i ].world.m[ 3 ][ 1 ] += diff.vy ;
			objs->objs[ i ].world.m[ 3 ][ 2 ] += diff.vz ;
		}				
		_sceVu0AddVector( &nmov, &work->control.mov, &diff ) ;
		GM_ResetControlPosition( &work->control, &nmov ) ;
	}

	CheckRStickPress( work ) ;

	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL_ENABLE ) &&
		( ( work->pad->press != 0 ) || RStickPress ) ) {
		SetAction( work, Mlift_struggle, 6 ) ;
		LastStruggleTime = GV_Time ;
		if ( ++ work->data > 32 ) {
			SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL ) ;
			PL_SetInvincible( work, 0 ) ;
			ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ;
			PL_InvisibleO2Gage() ;
			GM_O2 = GM_O2Max ;
			SetMode( work, LiftupEscape ) ;
			return ;
		}
	} else if ( GV_Time - LastStruggleTime > 16 ) {
		SetAction( work, Mlift_pose, 6 ) ;			
	}

	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_FLING_GROUND ) ) {
		nmov.vy += 500.0F ;
		GM_ResetControlPosition( &work->control, &nmov ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		PL_RemoveDeadAction( &DeadModeSet ) ;
		PL_InvisibleO2Gage() ;
		GM_O2 = GM_O2Max ;
		{
			FVECTOR		force = { 128.0F, 0.0F, 0.0F } ;
			int			damage = 10 ;

			GM_SetTarget( &Offense, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, 
						  &DG_ZeroVector ) ;

			/* 難易度別 */
			switch( SOL_GameLevel ) {
			case GM_LEVEL_VERYEASY :
				damage -= 2 ;
				break ;
			case GM_LEVEL_EASY :
				damage -= 1 ;
				break ;
			case GM_LEVEL_HARD :
				damage += 1 ;
				break ;
			case GM_LEVEL_EXTREME :
				damage += 2 ;
				break ;
			case GM_LEVEL_NORMAL :
			default :
			  ;
			}
			if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
			if ( damage <= 0 ) damage = 1 ;		 

			GM_SetPowerTarget( &Offense, &PtOffense, POWER_EXPLODE, 255, 0, damage, &force ) ;
			GM_SetTargetWeaponType( &Offense, WP_C4BOMB ) ;
			GM_MoveTarget( &Offense, &SOL_SolControl->mov ) ;
			GM_TargetSetDirectAttack( &Offense, GM_PlayerTarget ) ;
			GM_PutTarget( &Offense ) ;
			
			NewSolidusDunkSmoke( &work->control.mov, 1200.0F ) ;
		}
//		SetMode( work, LiftupFlingGround ) ;
		return ;
	}

	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_END ) ) {
		PL_ReturnMotionArc( work ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		SetMode( work, PL_StillMode[ STAND ] ) ;
		PL_InvisibleO2Gage() ;
		GM_O2 = GM_O2Max ;
		return ;
	}

	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL_ENABLE ) ) {
		PL_UnsetInvincible( work ) ;
		PL_VisibleO2Gage() ;
		SetStatus( PLAYER_NO_BREATH ) ;
		//SetFlag( FLAG_CANNOT_USE_RATION ) ;
		GM_O2 -= GM_O2Max / 100 ;
		//GM_O2RecoverValue = 160.0F ;
	} else {
		PL_InvisibleO2Gage() ;
		GM_O2 = GM_O2Max ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
	}
}

#if 0
/* 蛇手たたきつけられ */
static	void	LiftupFlingGround( PlayerWork *work, int time ) 
{
	GM_VitalityAdjust -= 10 ;
	SetStatus( PLAYER_DAMAGED ) ;
	work->force.vx = 0.0F ;
	work->force.vz = 0.0F ;
	NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
	NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
	SetMode( work, PL_Blow ) ;
}
#endif

/* 蛇手から逃げる */
static	void	LiftupEscape( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mlift_escape_start, 6 ) ;
		SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
		work->data2 = 0 ;
		work->control.grounded &= ~1 ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		PL_LeaveSubject( work ) ;
	}	
	
	switch( work->data ) {
	case 0 :
		SetFlag( FLAG_NO_GRAVITY | FLAG_CANNOT_CHANGE |
				 FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT |
				 FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK | FLAG_SPECIAL_MODE ) ;
		work->data2 -= 6 ;
		work->control.step.vy = ( float )work->data2 ;
		if ( EndMotion( work ) ) {
			if ( work->control.grounded & 1 ) {
				SetAction( work, Mlift_escape_end, 6 ) ;
				SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
				work->data = 2 ;
			} else {
				SetAction( work, Mlift_escape_loop, 6 ) ;
				SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
				work->data = 1 ;
			}
		}
		break ;
	case 1 :
		SetFlag( FLAG_NO_GRAVITY | FLAG_CANNOT_CHANGE |
				 FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT |
				 FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK | FLAG_SPECIAL_MODE ) ;
		work->data2 -= 8 ;
		work->control.step.vy = ( float )work->data2 ;
		if ( work->control.grounded & 1 ) {
			SetAction( work, Mlift_escape_end, 6 ) ;
			SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
			work->data = 2 ;
		}
		break ;
	case 2 :
		SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_SPECIAL_MODE ) ;
		if ( EndMotion( work ) ) {
			PL_SetInvincible( work, DIRECT_TICK( 8 ) ) ;			
			GM_ResetMenuStatus( MENU_MENU_OFF ) ;
			PL_ReturnMotionArc( work ) ;
			if ( work->dmg_callback == LiftupDamageCallback ) {
				work->dmg_callback = NULL ;
			}
			PL_RemoveDeadAction( &DeadModeSet ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;			
		}
	}
}

/* つかまれ中死亡 */
static	void	LiftupDead( PlayerWork *work, int time )
{
	if ( time == 0 ) {
		if ( work->current_mar != Motion ) {
			PL_ChangeMotionArc( work, Motion ) ;
		}
		SetAction( work, Mlift_escape_start, 6 ) ;
		SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
		work->data2 = 0 ;
		work->control.grounded &= ~1 ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		PL_LeaveSubject( work ) ;
		GM_SeSetMode( SD_V_POUT0001, &work->control.mov, GM_SEMODE_BOMB ) ;
	}	
	
	switch( work->data ) {
	case 0 :
		SetFlag( FLAG_NO_GRAVITY | FLAG_CANNOT_CHANGE |
				 FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT |
				 FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK | FLAG_SPECIAL_MODE ) ;
		work->data2 -= 6 ;
		work->control.step.vy = ( float )work->data2 ;
		if ( EndMotion( work ) ) {
			if ( work->control.grounded & 1 ) {
				SetAction( work, Mlift_out, 6 ) ;
				SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
				work->data = 2 ;
			} else {
				SetAction( work, Mlift_escape_loop, 6 ) ;
				SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
				work->data = 1 ;
			}
		}
		break ;
	case 1 :
		SetFlag( FLAG_NO_GRAVITY | FLAG_CANNOT_CHANGE |
				 FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT |
				 FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK | FLAG_SPECIAL_MODE ) ;
		work->data2 -= 8 ;
		work->control.step.vy = ( float )work->data2 ;
		if ( work->control.grounded & 1 ) {
			SetAction( work, Mlift_out, 6 ) ;
			SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
			work->data = 2 ;
		}
		break ;
	case 2 :
		PL_GroundIK2( work, 500.0F ) ;
		PL_AvoidSink( work, 200.0F, 0.0F ) ;
		SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_SPECIAL_MODE ) ;
		if ( EndMotion( work ) ) {
			PL_MotionSleep( work, 0 ) ;
			GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
			GM_GameOverProcEnd( &work->actor ) ;
			work->data = 3 ;
		} 
		break ;
	case 3 :
	  ;
	}
}

/*----------------------------------------------------------------*/

/* セットアップ関数 */
static	int	SetLiftupAct( PlayerWork *work, GV_MSG *msg, int len )
{
	if ( GM_CheckPlayerStatus( PLAYER_DEAD ) ) {
		SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL ) ;
		ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ;
		return -1 ;
	}
	if ( GM_CheckPlayerStatus( PLAYER_BEYOND | PLAYER_FORCE ) ) {
		/* エルード中はだめ */
		return -1 ;
	}
	PL_LeaveSubject( work ) ;
	PL_LeaveCaution( work ) ;
	PL_ClearCaptureTarget( work ) ;
	PL_UnequipSpecials() ;
	PL_SetInvincible( work, 0 ) ;
	SetMode2( work, NULL ) ;
	UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED | PLAYER_SQUAT |
				 PLAYER_GROUND ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_SPECIAL_MODE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	RStickPress = 0 ;
	RStickDir = -1 ;
	LastStruggleTime = 0 ;
	work->dmg_callback = LiftupDamageCallback ;
	SetMode( work, LiftupStart ) ;

	PL_AddDeadAction( &DeadModeSet, LiftupDead, LiftupDeadConditionFunc ) ;

	return 1 ;
}

/* プラグイン登録 */
int		PL_PluginLiftup( void )
{
	PL_AddPlugin( &PluginSet, PL_MSG_LIFTUP, SetLiftupAct, NULL ) ;
	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
		Motion = GV_StrCode( "sna_solidus" ) ;
	} else {
		Motion = GV_StrCode( "rai_solidus" ) ;
	}
	return 0 ;
}

int		PL_LiftupNow( void )
{
	if ( GM_PlayerWork != NULL && GM_PlayerWork->action == LiftupStart ) return 1 ;
	return 0 ;
}
