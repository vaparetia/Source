//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   jump.c
   自動ジャンププラグイン

   2000/07/14 M.Sonoyama
   $Id: jump.c,v 1.1.1.3 2002/11/19 11:50:48 Yoshizawa1 Exp $
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

static	PL_PollingSet	PollingSet ;

enum {
	JMup = 0,
	JMdown_s,
	JMdown_l,
	JMdown_e,
	JMup_b,
	MAX_JUMP_MOTIONS
} ;

enum {
	JFLAG_NONE = 			0x0000,
	JFLAG_CHECK_SEGMENT	=	0x0001,
} ;

static	int		Motions[ MAX_JUMP_MOTIONS ] ;
static	int		Trap, Proc, Flag, First = 0 ;
static	short	Dir ;
static	float	ClimbHeight, ClimbHeightW ;

static  float   JumpingStepX = 0.0f ; /* 主観移動のときの進む方向 */
static  float   JumpingStepZ = 0.0f ; /* 主観移動のときの進む方向 */

#define	PAD_JUMPUP	PL_PAD_ACTION

/*---------------------------------------------------------------*/

static	void	AutoJumpUp( PlayerWork *, int ) ;
static	void	AutoJumpDown( PlayerWork *, int ) ;

static	void	ExecProc( v1, v2, v3 )
int				v1, v2, v3 ;
{
	GCL_ARGS	arg ;
	int			buf[ 3 ] ;

	if ( Proc == 0 ) return ;
	arg.argv = buf ;
	arg.argc = 3 ;
	buf[ 0 ] = v1 ;
	buf[ 1 ] = v2 ;
	buf[ 2 ] = v3 ;
	GM_ExecProc( Proc, &arg ) ;
}

/* 角度調整 */
static	void	DirAdjust( PlayerWork *work )
{
	int			d1, d2 ;

	if ( Dir < 0 ) return ;
	d1 = GV_DiffDirAbs( work->control.rot.vy, Dir ) ;
	d2 = GV_DiffDirAbs( work->control.rot.vy, Dir + 2048 ) ;
	if ( d1 < d2 ) work->control.rot.vy = Dir ;
	else		   work->control.rot.vy = Dir + 2048 ;
	PL_StopTurn( work ) ;
}

/* 登り */
static	void	AutoJumpUp( PlayerWork *work, int time )
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK |
			 FLAG_NO_GRAVITY | FLAG_CANNOT_ATTACK | FLAG_NO_TOUCH_DAMAGE |
			 FLAG_NO_CAUTION | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_JUMP ) ;

	if ( time == 0 ) {
		PL_ChangeMotionArc( work, PL_Force->marfile ) ;
		if ( GM_CheckPlayerStatus( PLAYER_CAUTION ) ) {
			work->control.rot.vy = work->control.turn.vy ;
			PL_SetAction( work, Motions[ JMup_b ], 6 ) ;
		} else {
			PL_StopTurn( work ) ;
			PL_SetAction( work, Motions[ JMup ], 6 ) ;
		}

		DirAdjust( work ) ;

		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		//work->control.seg_flag |= HZX_SEG_NO_ENEMY_EYES ;
		SetStatus( PLAYER_FORCE ) ;
		PL_LeaveSubject( work ) ;
		PL_LeaveCaution( work ) ;
		PL_ChangeStance( work, STAND ) ;
		PL_SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
	}
	PL_LevelCheck( work ) ;
	work->control.step.vy = 0.0F ;

	if ( EndMotion( work ) ) {
		PL_Force->e_turn = -1 ;
		PL_Force->flag = 0 ;
		SetFlag( FLAG_FORCE_END ) ;
		work->control.skip_flag &= ~CTRL_SKIP_HZX ;
		//work->control.seg_flag &= ~HZX_SEG_NO_ENEMY_EYES ;
		SetMode( work, PL_StillMode[ STAND ] ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		PL_UnsetInvincible( work ) ;
		ExecProc( 1, 0, 0 ) ;
	}
}

/* 下り */
static	void	AutoJumpDown( PlayerWork *work, int time )
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK |
			 FLAG_NO_GRAVITY | FLAG_CANNOT_ATTACK | FLAG_NO_TOUCH_DAMAGE |
			 FLAG_NO_CAUTION | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_JUMP ) ;

	if ( time == 0 ) {
		PL_LeaveSubject( work ) ;
		PL_LeaveCaution( work ) ;
		PL_ChangeMotionArc( work, PL_Force->marfile ) ;
//		PL_StopTurn( work ) ;
		PL_SetAction( work, Motions[ JMdown_s ], 6 ) ;
//		work->control.skip_flag |= CTRL_SKIP_HZX ;
		SetStatus( PLAYER_FORCE ) ;

		DirAdjust( work ) ;
	}

	switch( work->data ) {
	case 0 :	/* 降り開始 */
		PL_LevelCheck( work ) ;
		work->control.step.vy = 0.0F ;
		if ( EndMotion( work ) ) {
			PL_SetAction( work, Motions[ JMdown_l ], 6 ) ;
			work->control.skip_flag &= ~CTRL_SKIP_HZX ;
			work->data = 1 ;
		}
		break ;
	case 1 :	/* 降りループ */
		work->control.step.vy -= 32.0F ;
		if ( work->control.grounded & 1 ) {
			FVECTOR		size ;
			/* ふみつけ */
			if ( Status( PLAYER_SNAKE ) ) {
				size.vx = size.vy = size.vz = 240.0F ;
			} else {
				GV_SetVec3( &size, 320.0F, 500.0F, 320.0F ) ;
			}
			GM_SetTarget( &work->offense, TARGET_OFFENSE | TARGET_POWER, 0,
						  ENEMY_SIDE, &size, &DG_ZeroVector ) ;
			GM_SetTargetWeaponType( &work->offense, WP_KICK ) ;
			GM_MoveTarget3( &work->offense, PL_ObjWorld( work, HUMAN21_MIGI_KAKATO ) ) ;
			GM_PutTarget( &work->offense ) ;
			PL_SetAction( work, Motions[ JMdown_e ], 6 ) ;			
			work->data = 2 ;
		}
		break ;
	case 2 :	/* 降りた津 */
		UnsetFlag( FLAG_NO_GRAVITY ) ;
		if ( EndMotion( work ) ) {
			PL_Force->e_turn = -1 ;
			PL_Force->flag = 0 ;
			SetFlag( FLAG_FORCE_END ) ;
			work->control.skip_flag &= ~CTRL_SKIP_HZX ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
			GM_ResetMenuStatus( MENU_MENU_OFF ) ;
			PL_UnsetInvincible( work ) ;
			ExecProc( 3, 0, 0 ) ;
		}
	}

	/* 主観移動のときにここに来る場合がある　ステップは独自計算 */
	if ( PL_SubjectMove != 0 ) {
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		work->control.step.vx = JumpingStepX ;
		work->control.step.vz = JumpingStepZ ;
	}
}

/*---------------------------------------------------------------*/


/* 登りチェック */
static	int	CheckAutoJumpUp( PlayerWork *work )
{
	float		level0, levels[ 2 ] ;
	int			flag, chk ;
	FVECTOR		front, mov, chk_mov ;
	FVECTOR		hchkpos1, hchkpos2 ;
	SVECTOR		rot ;
	CONTROL		*ctrl ;
	
	ctrl = &work->control ;
	/* 地に足がついていない */
	if ( !( ctrl->grounded & 1 ) ) return 0 ;
	rot = ctrl->rot ;
	if ( GM_CheckPlayerStatus( PLAYER_CAUTION ) ) rot.vy = ctrl->turn.vy + 2048 ;
	chk = Flag & JFLAG_CHECK_SEGMENT ;
	level0 = work->control.levels[ 0 ] ;
	DG_COPY_VEC( &mov, &ctrl->mov ) ;
	DG_COPY_VEC( &chk_mov, &ctrl->mov ) ;
	DG_COPY_VEC( &hchkpos1, &ctrl->mov ) ; hchkpos1.vy = level0 + 1252.0F ;
	chk_mov.vy = mov.vy + ClimbHeight ;
//	mov.vy = level0 + 750.0F ;
	/* 750前をチェック */
	GV_GetShiftPos( &front, &mov, &rot, 0.0F, ClimbHeight, 750.0F ) ;
	if ( chk && HZX_OnlineHazardCheck( ctrl->hzx_id, &chk_mov, &front, HZX_CHK_ALL,
									   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;

	/* ジャンプ前の床から1252の位置でオンラインチェック */
	DG_COPY_VEC( &hchkpos2, &front ) ; hchkpos2.vy = hchkpos1.vy ;
	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &hchkpos1, &hchkpos2, HZX_CHK_ALL,
							    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;

	flag = HZX_LevelHazardCheck( HZX_CurrentGroupID, &front, HZX_CHK_F_FLOOR,
								 HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( !( flag & 1 ) ) return 0 ;


	HZX_GetLevelHeight( levels ) ;
	if ( ( levels[ 0 ] > level0 + ClimbHeight + ClimbHeightW ) ||
		 ( levels[ 0 ] < level0 + ClimbHeight - ClimbHeightW ) ) return 0 ;
//printf( "1 : %f %f\n", level0, levels[ 0 ] ) ;
	/* 第2点チェック */
	GV_GetShiftPos( &front, &mov, &rot, 350.0F, ClimbHeight, 750.0F ) ;
	if ( chk && HZX_OnlineHazardCheck( ctrl->hzx_id, &chk_mov, &front, HZX_CHK_ALL,
									   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;
	/* ジャンプ前の床から1252の位置でオンラインチェック */
	DG_COPY_VEC( &hchkpos2, &front ) ; hchkpos2.vy = hchkpos1.vy ;
	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &hchkpos1, &hchkpos2, HZX_CHK_ALL,
							    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;
	flag = HZX_LevelHazardCheck( HZX_CurrentGroupID, &front, HZX_CHK_F_FLOOR,
								 HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( !( flag & 1 ) ) return 0 ;
	HZX_GetLevelHeight( levels ) ;
	if ( ( levels[ 0 ] > level0 + ClimbHeight + ClimbHeightW ) ||
		 ( levels[ 0 ] < level0 + ClimbHeight - ClimbHeightW ) ) return 0 ;
	/* 第3点チェック */
	GV_GetShiftPos( &front, &mov, &rot, -350.0F, ClimbHeight, 750.0F ) ;
	if ( chk && HZX_OnlineHazardCheck( ctrl->hzx_id, &chk_mov, &front, HZX_CHK_ALL,
									   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;

	/* ジャンプ前の床から1252の位置でオンラインチェック */
	DG_COPY_VEC( &hchkpos2, &front ) ; hchkpos2.vy = hchkpos1.vy ;
	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &hchkpos1, &hchkpos2, HZX_CHK_ALL,
							    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;

	flag = HZX_LevelHazardCheck( HZX_CurrentGroupID, &front, HZX_CHK_F_FLOOR,
								 HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( !( flag & 1 ) ) return 0 ;
	HZX_GetLevelHeight( levels ) ;
	if ( ( levels[ 0 ] > level0 + ClimbHeight + ClimbHeightW ) ||
		 ( levels[ 0 ] < level0 + ClimbHeight - ClimbHeightW ) ) return 0 ;

   // Arm fix:
   // in w20a, don't allow raiden to jump onto the conveyor belt
   // fixes MGSTWO-3298
   if (!strcmp(GM_GetArea(), "w20a")
      && ctrl->mov.vx > 56000.0f && ctrl->mov.vx < 57300.0f
      && ctrl->mov.vz > -94000.0f && ctrl->mov.vz < 93000.0f)
   {
      return 0;
   }

	return 1 ;
}

/* 降りチェック */
static	int	CheckAutoJumpDown( PlayerWork *work )
{
	float		level0, levels[ 2 ] ;
	int			flag, chk ;
	FVECTOR		front, mov ;
	SVECTOR		rot ;
	CONTROL		*ctrl ;
	extern int PL_SubjectMove ;

	if ( work->act_name == STAND_STILL ||
		 work->act_name == STAND_CAUTION_STILL ||
		 work->act_name == SQUAT_CAUTION_STILL ) {
		return 0 ;
	}
	ctrl = &work->control ;
	/* 地に足がついていない */
	if ( !( ctrl->grounded & 1 ) ) return 0 ;
	chk = Flag & JFLAG_CHECK_SEGMENT ;
	level0 = work->control.levels[ 0 ] ;
	DG_COPY_VEC( &mov, &ctrl->mov ) ;
	mov.vy = level0 + 750.0F ;
	/* 250前をチェック */
	rot.vx = rot.vz = 0 ; rot.vy = work->pad->dir ;
	if ( PL_SubjectMove ) {
		int   ang ;
		float rad ;
		FVECTOR		step ;
		/* 主観移動の時は実際の移動を見る */
		if ( DG_FABS( work->control.step.vx ) > 0.01f ||
			 DG_FABS( work->control.step.vz ) > 0.01f ) {
			DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
			DG_RotVector( &work->control.step, &step, 1 ) ;
			rot.vy = GV_VecDir2( &step ) ;
		}
		ang = rot.vy & 4095 ;
		ang -= ang>2048 ? 4096 : 0 ;
		rad = ang/2048.0f*M_PI ;
		JumpingStepX = 40.0f*sinf( rad ) ;
		JumpingStepZ = 40.0f*cosf( rad ) ;
	} else {
		FVECTOR		step = {0.0f,0.0f,0.0f,1.0f};/*yano 2002.03.28 初期化追加*/

		step.vx = work->body.m_ctrl->mt3_ctrl[ work->body.m_ctrl->use_move_layer ].root->step.vx ;
		step.vz = work->body.m_ctrl->mt3_ctrl[ work->body.m_ctrl->use_move_layer ].root->step.vz ;
		if ( DG_FABS( step.vx ) > 0.10F || DG_FABS( step.vz ) > 0.10F ) {
			DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
			DG_RotVector( &step, &step, 1 ) ;
			rot.vy = GV_VecDir2( &step ) ;
		}
	}
	GV_GetShiftPos( &front, &mov, &rot, 0.0F, 0.0F, 250.0F ) ;
	if ( chk && HZX_OnlineHazardCheck( ctrl->hzx_id, &mov, &front, HZX_CHK_ALL,
									  HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return 0 ;
	flag = HZX_LevelHazardCheck( HZX_CurrentGroupID, &front, HZX_CHK_F_FLOOR,
								HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	if ( !( flag & 1 ) ) return 0 ;
	HZX_GetLevelHeight( levels ) ;
	if ( levels[ 0 ] > level0 - 750.0F ) return 0 ;

	/* 主観移動の時は方向回転はなし */
	if ( PL_SubjectMove ) return 1 ;

	if ( work->act_name == STAND_CAUTION_LEFT ||
 		 work->act_name == STAND_CAUTION_RIGHT ) {
		work->control.turn.vy = work->control.rot.vy 
			= GM_PlayerCautionDir ;
	} else {
		work->control.turn.vy = work->control.rot.vy 
		    = rot.vy ;
	}
	return 1 ;
}

/*---------------------------------------------------------------*/

/* モードチェック */
static	int	CheckMode( int act )
{
	static int	Modes[] = { 
		STAND_STILL, STAND_RUN, STAND_CAUTION_STILL,
		SQUAT_CAUTION_STILL, STAND_CAUTION_LEFT, STAND_CAUTION_RIGHT,
		BLADE_SLASH, COMBO, PULL_BODY, ENEMY_HANG
	} ;
	int		i ;						
	
	for ( i = 0; i < sizeof( Modes ) / sizeof( int ); i ++ ) {
		if ( act == Modes[ i ] ) return 1 ;
	}
	return 0 ;
}

/* ポーリング関数 */
static	int	CheckAutoJump( PlayerWork *work )
{
	if ( First == 0 ) return 0 ;
	if ( Status( PLAYER_FORCE | PLAYER_DAMAGED | PLAYER_DEAD | PLAYER_DOWNED |
				 PLAYER_LOCKER | PLAYER_INTRUDE | PLAYER_GROUND |
				 PLAYER_BEYOND | PLAYER_CB_BOX | PLAYER_LADDER | PLAYER_WATCH |
				 /*PLAYER_ENEMY_HANG |*/ PLAYER_PAD_OFF | PLAYER_BEHIND_ATTACK |
				 PLAYER_IN_THE_WATER | PLAYER_CROSS ) ) return 0 ;
	if ( Trap != 0 && !GM_CheckInsideTrap( &work->control, &Trap, 1, 0 ) ) {
		return 0 ;
	}

	if ( !CheckMode( work->act_name ) ) return 0 ;
	/* 登る */
	if ( ( work->pad->press & PAD_JUMPUP ) &&
		work->act_name != ENEMY_HANG && 
		work->act_name != PULL_BODY && 
		work->act_name != COMBO && 
		 ( CheckAutoJumpUp( work ) ) ) {
		ExecProc( 0, 0, 0 ) ;
		PL_ClearCaptureTarget( work ) ;
		PL_UnequipSpecials() ;
		PL_SetInvincible( work, 0 ) ;
		SetMode( work, AutoJumpUp ) ;
		SetMode2( work, NULL ) ;
		return 1 ;
	}
	/* 降りる */
	if ( ( work->pad->dir >= 0 || 
		  work->act_name == COMBO || 
		  work->act_name == ENEMY_HANG ||
		  work->act_name == PULL_BODY ) &&
		( CheckAutoJumpDown( work ) ) ) {
		ExecProc( 2, 0, 0 ) ;
		PL_ClearCaptureTarget( work ) ;
		PL_LeaveSubject( work ) ;
		PL_UnequipSpecials() ;
		PL_SetInvincible( work, 0 ) ;
		SetMode( work, AutoJumpDown ) ;
		SetMode2( work, NULL ) ;
		return 1 ;
	}
	return 0 ;
}

/*---------------------------------------------------------------*/

/* プラグイン登録 */
int	NewPluginAutoJump( void )
{
    PL_AddPollingFunc( &PollingSet, CheckAutoJump ) ;
	Proc = 0 ;
	Trap = 0 ;
	First = 0 ;
	return 0 ;
}

int NewAutoJumpSetting( void )
{
	int			i ;

	GCL_GetOption( 'm' ) ;
	i = 0 ;
	while( GCL_NextStr() != NULL ) {
		if ( i >= MAX_JUMP_MOTIONS ) break ;
		Motions[ i ] = GCL_GetNextInt() ;
		i ++ ;
	}
	GCL_GetOption( 'c' ) ;
	ClimbHeight = ( float )GCL_GetNextInt() ;
	ClimbHeightW = ( float )GCL_GetNextInt() ;
	Trap = GCL_GetOptionValue( 't', 0 ) ;
	Proc = GCL_GetOptionValue( 'p', 0 ) ;
	Flag = GCL_GetOptionValue( 'f', 0 ) ;
	Dir = GCL_GetOptionValue( 'd', -1 ) ;
	First = 1 ;
    return 0 ;
}

int NewSetAutoJumpDir( void )
{
	Dir = GCL_GetOptionValue( 'd', -1 ) ;
	return 0 ;
}



