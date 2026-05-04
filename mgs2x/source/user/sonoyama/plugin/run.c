//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   run.c
   強制移動プラグイン

   2000/05/23 M.Sonoyama
   $Id: run.c,v 1.1.1.3 2002/11/19 11:50:50 Yoshizawa1 Exp $
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
static	FVECTOR			to ;
static	int				motion, near_time ;
static	int				edir, flag, end_proc, target_is_inzone ;
static	int				reverse = 0 ;

static	NAVIGATE		Navi ;
static	NAVITARGET		NaviTrg ;

enum {
	RM_WALK = 0,
	RM_RUN,
	RM_DASH,
	RM_GROUND,
	RM_GROUND_FAST,
	RM_CBBOX,
	RM_GROUND_BACK,
} ;

static	int	Motions[] = {
	Mwalk, Mrun, Mdash, Mcrouch_f, Mcrouch_f_fast, Mbox_run, Mcrouch_b
} ;

/*---------------------------------------------------------------*/

/* 到着チェック */
static	int		CheckArrived( PlayerWork *work )
{
	FVECTOR		diff ;

	if ( target_is_inzone == 1 ) {
		if ( GM_Navi( &Navi, &NaviTrg, 100 ) ) {
			return 1 ;
		}
	} else {
		if ( DG_FABS( to.vx - work->control.mov.vx ) < 100.0F &&
			 DG_FABS( to.vz - work->control.mov.vz ) < 100.0F ) {
			return 1 ;
		}
		_sceVu0SubVector( &diff, &to, &work->control.mov ) ;
		Navi.next_dir = GV_VecDir2( &diff ) ;
//		printf( "----\n" ) ;
//		DumpVec( &work->control.mov ) ;
//		DumpVec( &to ) ;
//		printf( "%d\n", Navi.next_dir ) ;
	}
    if ( near_time > TIME_BASE * 120 ) {	
		/* 保険。2秒以上近い状態が続いたら到着とみなす */
		return 1 ;
	}
	return 0 ;
}

/* 強制移動 */
static	void	ForceRunMoveAct( work, time )
PlayerWork		*work ;
int				time ;
{
	int				action, now, zn ;
	int				end_flag ;
	FVECTOR			diff ;
	PL_MOTION_SET	*ms ;

	end_flag = 0 ;
	ms = PL_MotionSet ;
	if ( time == 0 ) {
		PL_ChangeMotionArc( work, work->org_motion ) ;
		reverse = 0 ;
		if ( ( flag & FA_DIRECT_MOVE ) || 
 			 ( HZX_GetHzxIDbyZoneEx( 0, &to, &zn, 0 ) == 0 ) ) {
			target_is_inzone = 0 ;
			_sceVu0SubVector( &diff, &to, &work->control.mov ) ;
			Navi.next_dir = GV_VecDir2( &diff ) ;
printf( "---- run move 1\n" ) ;
		} else {
			target_is_inzone = 1 ;
			GM_SetNavi( &Navi, &work->control ) ;
			GM_SetNaviTargetFromPos( &NaviTrg, &to, 0 ) ;
printf( "---- run move 2\n" ) ;
		}

		/* あたりチェック関連 */
		if ( flag & FA_NO_CHECK_SEG ) {
			work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		}	
		
//		if ( flag & FA_NO_CHECK_FLR ) {
//			work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
//		}
		if ( flag & FA_NO_CHECK_TRP ) {
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
		}
		if ( motion == -1 ) {
			if ( ItemType( work ) & IT_TYPE_CBBOX ) {
				motion = RM_CBBOX ;
			} else if ( work->stance == GROUND ) {
				if ( work->motion1 == ms->change[ Mcrouch_b ] ) {
					motion = RM_GROUND_BACK ;
					reverse = 1 ;
				} else if ( work->motion1 == ms->change[ Mcrouch_f_fast ] ) {
					motion = RM_GROUND_FAST ;
				} else {
					motion = RM_GROUND ;
				}
			} else {
				now = work->motion1 ;
				if ( now == ms->change[ Mwalk ] || 
					 now == ms->change[ Mstair_walk ] ) motion = RM_WALK ;
				else if ( now == ms->change[ Mrun ] ||
						  now == ms->change[ Mstair_run ] ) motion = RM_RUN ;
				else if ( now == ms->change[ Mdash ] ) motion = RM_RUN ;
				else motion = RM_RUN ;
			}
		}
		if ( WeaponType( work ) & WP_TYPE_SUBJECT ) PL_NoWeapon( work ) ;
		if ( motion != RM_CBBOX ) {
			/* ダンボールでない */
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) PL_NoEquip( work ) ;
			action = ms->change[ Motions[ motion ] ] ;
		} else if ( !( ItemType( work ) & IT_TYPE_CBBOX ) ) {
			/* ダンボール指定されたが、ダンボールでない */
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) PL_NoEquip( work ) ;
			action = ms->change[ Motions[ RM_WALK ] ] ;
		} else {
			/* ダンボール */
			action = ms->shared[ Mbox_run ] ;
		}
		/* 補完ない */
		if ( flag & FA_NO_INTERP ) {
			SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
			SetAction( work, action, 0 ) ;			
		} else {
			SetAction( work, action, 6 ) ;
		}
		near_time = 0 ;
	}

	if ( motion == RM_CBBOX ) SetStatus( PLAYER_CB_BOX ) ;

	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE | FLAG_FORCE ) ;
	if ( flag & FA_NO_WEAPON ) SetStatus( PLAYER_WEAPON_INVISIBLE ) ;

	/* 階段 */
	if ( motion <= RM_DASH || motion == RM_CBBOX ) {
		if ( work->floor_atr & HZX_FLOOR_STEP ) {
			if ( motion == RM_WALK ) SetAction( work, ms->change[ Mstair_walk ], 6 ) ;
			else {
				SetAction( work, ms->change[ Mstair_run ], 6 ) ;
//               SetAction( work, ms->change[ Mrun ], 6 ) ;
//               MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 0.70F ) ;
			}
			if ( motion == RM_CBBOX ) SetStatus( PLAYER_CB_BOX_STAND ) ;
		} else {
			if ( motion == RM_CBBOX ) {
				SetAction( work, ms->shared[ Mbox_run ], 6 ) ;
			} else {
				SetAction( work, ms->change[ Motions[ motion ] ], 6 ) ;
			}
		}
	}

	/* 匍匐ＩＫ、イントルード */
	if ( motion == RM_GROUND || motion == RM_GROUND_FAST ||
		 motion == RM_GROUND_BACK ) {
		if ( PL_CheckIntrude( work ) ) {
			PL_IntoIntrude( work ) ;
		} else {
			PL_LeaveIntrude( work ) ;
			PL_GroundIK2( work, 300.0F ) ;
		}
	}

//printf( "%x %x\n", Navi.ctrl->addr, NaviTrg.addr ) ;
	if ( CheckArrived( work ) ) {
		/* 到着 */
		end_flag = 1 ;
		to.vy = work->control.mov.vy ;
		GM_ResetControlPosition( &work->control, &to ) ;
		if ( edir != -1 ) work->control.turn.vy = edir ;
		SetFlag( FLAG_NO_STEP ) ;
		PL_UnsetInvincible( work ) ;
//		UnsetStatus( PLAYER_FORCE ) ;
		PL_EndForceAct( work, FA_USE_DEFAULT, -1 ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		if ( motion < RM_GROUND ) {
			SetMode( work, PL_StillMode[ STAND ] ) ;
		} else if ( motion < RM_CBBOX ) {
			if ( PL_CheckIntrude( work ) ) {
				SetMode( work, PL_StillMode[ INTRUDE ] ) ;
			} else {
				SetMode( work, PL_StillMode[ GROUND ] ) ;
			}
		} else {
			SetMode( work, PL_CB_BoxStop ) ;
		}
//		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;	
		/* あたりチェック再開 */
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		if ( !( flag & FA_NO_RECHECK_TRP ) ) {
			work->control.skip_flag &= ~CTRL_SKIP_TRAP ;
		}		
		if ( flag & FA_STAGESTART ) {
			DG_COPY_VEC( &GM_PlayerFirstPosition, &work->control.mov ) ;
		}
		if ( end_proc > 0 ) {
			GM_ExecProc( end_proc, NULL ) ;
			end_proc = 0 ;
		}
	} else {
		/* まだ */
		if ( DG_FABS( to.vx - work->control.mov.vx ) > 750.0F ||
			 DG_FABS( to.vz - work->control.mov.vz ) > 750.0F ) {
			if ( target_is_inzone ) {
				work->control.turn.vy = Navi.next_dir ;
				if ( reverse ) work->control.turn.vy += 2048 ;
			} else {
				work->control.turn.vy = work->control.rot.vy = Navi.next_dir ;
				if ( reverse ) {
					work->control.turn.vy += 2048 ;
					work->control.rot.vy += 2048 ;
				}
			}
		} else {
			/* 近い場合は一気にターンする */
			work->control.turn.vy = work->control.rot.vy = Navi.next_dir ;
			if ( reverse ) {
				work->control.turn.vy += 2048 ;
				work->control.rot.vy += 2048 ;
			}
			near_time += TIME_BASE ;	/* 保険 */
		}
	}
	PL_ExecForceActProc( work, FA_PROCMODE_RUN, time, end_flag ) ;
}

/*---------------------------------------------------------------*/

/* セットアップ関数 */
static	int	SetForceRunMoveAct( work, msg, len )
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
	to.vx = ( float )msg->message[ 1 ] ;
	to.vy = ( float )msg->message[ 2 ] ;
	to.vz = ( float )msg->message[ 3 ] ;
	motion = msg->message[ 4 ] ;
	/* ダッシュは廃止 */
	if ( motion == RM_DASH ) motion = RM_RUN ;
	edir = msg->message[ 5 ] ;
	flag = msg->message[ 6 ] ; 
	end_proc = PL_GetMessageValue( msg, 7, 0 ) ;
	PL_LeaveCaution( work ) ;
	PL_SetInvincible( work, 0 ) ;
	PL_LeaveSubject( work ) ;
	PL_ClearCaptureTarget( work ) ;	
	UnsetFlag( FLAG_FORCE_END ) ;
	UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;
	SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE ) ;
	SetStatus( PLAYER_FORCE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
	SetMode2( work, NULL ) ;
	SetMode( work, ForceRunMoveAct ) ;
	msg->message_len = 0 ;
	return 1 ;
}

/*---------------------------------------------------------------*/

/* プラグイン登録 */
int	NewPluginForceRunMove( void )
{
    PL_AddPlugin( &PluginSet, PL_MSG_RUN, SetForceRunMoveAct, NULL ) ;
    return 0 ;
}

