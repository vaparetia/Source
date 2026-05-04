//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   beltply.c
   ベルトコンベア上ダンボールプレイヤープラグイン

   2001/01/15 M.Sonoyama
   $Id: beltply.c,v 1.1.1.3 2002/11/19 11:50:46 Yoshizawa1 Exp $
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
static	int				OnBelt = 0 ;
static	int				OnMap, OffMap ;
static	int				OnTrap1, OnTrap2 ;
static	int				OffTrap1, OffTrap2 ;
static	int				StartDir, EndDir ;
static	int				Proc ;
static	int				OffWaitTime ;
static	int				Restart ;

static	FVECTOR			OnCenter1, OnCenter2 ;
static	FVECTOR			OffCenter1, OffCenter2 ;

/*---------------------------------------------------------------*/

static	void	BeltConvActStart( PlayerWork *work, int time ) ;
static	void	BeltConvAct( PlayerWork *work, int time ) ;
static	void	BeltConvActEnd( PlayerWork *work, int time ) ;

/*---------------------------------------------------------------*/

enum {
	PROC_MODE_ON_START = 0,	
	PROC_MODE_ON_END,
	PROC_MODE_OFF_START,	
	PROC_MODE_OFF_END,
	PROC_MODE_ARRIVED,
	PROC_MODE_RESTART,
} ;

static	void	ExecProc( PlayerWork *work, int mode )
{
	GCL_ARGS	args ;
	int			buf[ 2 ] ;

	if ( Proc < 0 ) return ;
	args.argc = 2 ;
	args.argv = buf ;
	buf[ 0 ] = mode ;
	buf[ 1 ] = OffTrap2 ;
	GM_ExecProc( Proc, &args ) ;
}

/*---------------------------------------------------------------*/

/* ベルトコンベアスタート */
static	void	BeltConvActStart( PlayerWork *work, int time )
{
	HZX_BLOCK	*blk ;
	HZX_TRP		*trp ;
	FVECTOR		onc, offc, step, diff ;
	float		ip ;

	if ( time == 0 ) {
		HZX_FindTrap( GM_GetHzxGroupID( GM_GetMapID( OnMap ) ), OnTrap1, &blk, &trp ) ;
		ASSERT( blk != NULL && trp != NULL ) ;
		onc.vx = ( float )( blk->tx + ( trp->b1.vx + trp->b2.vx ) / 2 ) ;
		onc.vy = ( float )( blk->ty + ( trp->b1.vy + trp->b2.vy ) / 2 ) ;
		onc.vz = ( float )( blk->tz + ( trp->b1.vz + trp->b2.vz ) / 2 ) ;
		HZX_FindTrap( GM_GetHzxGroupID( GM_GetMapID( OnMap ) ), OnTrap2, &blk, &trp ) ;
		ASSERT( blk != NULL && trp != NULL ) ;
		offc.vx = ( float )( blk->tx + ( trp->b1.vx + trp->b2.vx ) / 2 ) ;
		offc.vy = ( float )( blk->ty + ( trp->b1.vy + trp->b2.vy ) / 2 ) ;
		offc.vz = ( float )( blk->tz + ( trp->b1.vz + trp->b2.vz ) / 2 ) ;
		_sceVu0SubVector( &step, &offc, &onc ) ;
		GV_LenVec3F( &step, &work->fv, 0.0F, 24.0F ) ;
		DG_COPY_VEC( &OnCenter1, &onc ) ;
		DG_COPY_VEC( &OnCenter2, &offc ) ;
#if 0
		if ( StartDir >= 0 ) {
			work->control.rot.vy = work->control.turn.vy = StartDir ;
			//work->control.turn.vy = StartDir ;
		}
#endif
		if ( Restart == 0 ) {
			work->control.rot.vy = work->control.turn.vy = GV_VecDir2( &step ) ;
			work->control.rot.vx = work->control.turn.vx = 0 ;
			work->control.rot.vz = work->control.turn.vz = 0 ;
			//PL_SetAction( work, PL_MotionSet->shared[ Mbox_idle ], 6 ) ;
			PL_SetAction( work, PL_MotionSet->shared[ Mbox_run ], 6 ) ;
		}
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		/* 終点の情報も作成しておく */
		HZX_FindTrap( GM_GetHzxGroupID( GM_GetMapID( OffMap ) ), OffTrap1, &blk, &trp ) ;
		ASSERT( blk != NULL && trp != NULL ) ;
		onc.vx = ( float )( blk->tx + ( trp->b1.vx + trp->b2.vx ) / 2 ) ;
		onc.vy = ( float )( blk->ty + ( trp->b1.vy + trp->b2.vy ) / 2 ) ;
		onc.vz = ( float )( blk->tz + ( trp->b1.vz + trp->b2.vz ) / 2 ) ;
		HZX_FindTrap( GM_GetHzxGroupID( GM_GetMapID( OffMap ) ), OffTrap2, &blk, &trp ) ;
		ASSERT( blk != NULL && trp != NULL ) ;
		offc.vx = ( float )( blk->tx + ( trp->b1.vx + trp->b2.vx ) / 2 ) ;
		offc.vy = ( float )( blk->ty + ( trp->b1.vy + trp->b2.vy ) / 2 ) ;
		offc.vz = ( float )( blk->tz + ( trp->b1.vz + trp->b2.vz ) / 2 ) ;		
		DG_COPY_VEC( &OffCenter1, &onc ) ;
		DG_COPY_VEC( &OffCenter2, &offc ) ;		

		if ( Restart == 0 ) {
			ExecProc( work, PROC_MODE_ON_START ) ;
		}
	}
	//SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_GRAVITY | FLAG_DONOT_CHECK_WATCH | 
			  //FLAG_NO_IK | FLAG_FORCE | FLAG_CANNOT_CHANGE ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH | 
			 FLAG_NO_IK | FLAG_FORCE | FLAG_CANNOT_CHANGE ) ;
	SetStatus( PLAYER_CB_BOX | PLAYER_WEAPON_INVISIBLE | PLAYER_FORCE ) ;
	//DG_COPY_VEC( &work->control.step, &work->fv ) ;
	_sceVu0SubVector( &diff, &OnCenter2, &work->control.mov ) ;
	ip = _sceVu0InnerProduct( &work->fv, &diff ) ;
	if ( Restart != 0 || ip <= 0.0F || time > 300 * 5 ) {	/* 保険。5秒 */
		if ( ip > 0.0F ) {
			/* 保険のとき */
			work->control.mov.vx = OnCenter2.vx ;
			work->control.mov.vz = OnCenter2.vz ;
			work->control.step.vx = 0.0F ;
			work->control.step.vz = 0.0F ;
		}
		if ( Restart == 0 ) {
			PL_SetAction( work, PL_MotionSet->shared[ Mbox_stop ], 6 ) ;
			work->control.turn.vy = StartDir ;
		}
		Restart = 0 ;
		SetMode( work, BeltConvAct ) ;
		ExecProc( work, PROC_MODE_ON_END ) ;
	}
}

/* ベルトコンベア上 */
static	void	BeltConvAct( PlayerWork *work, int time )
{
	FVECTOR		npos, ppos ;
	FVECTOR		diff, step ;
	float		ip ;

	if ( time == 0 ) {
		PL_SetAction( work, PL_MotionSet->shared[ Mbox_idle ], 6 ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
		/* このときだけ主観カメラ移行を早く */
		GM_SetCameraInterpMode( work->subject_camera, GM_CAM_INTERP_QUICK,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	}

	SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_IK | FLAG_NO_GRAVITY | FLAG_FORCE |
			 FLAG_CANNOT_CHANGE ) ;
	SetFlag2( FLAG2_SUBJECT_MOVE ) ;
	SetStatus( PLAYER_CB_BOX | PLAYER_WEAPON_INVISIBLE | PLAYER_FORCE ) ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurnCB( work ) ;
	}

	DG_COPY_VEC( &ppos, &work->control.mov ) ;
	ppos.vy -= work->control.height ;
	if ( PL_ExecMoveFloorFunc( &npos, work->control.hzx_id, &ppos,
							   &work->control.evt, work->control.name ) ) {
		work->control.step.vx += npos.vx - work->control.mov.vx ;
		work->control.step.vz += npos.vz - work->control.mov.vz ;
		work->control.step.vy = npos.vy - ppos.vy ; /* += でない点に注意 */
#if 0
printf( "%f %f %f : %f %f %f : %x\n",
	     work->control.step.vx, work->control.step.vy, work->control.step.vz,
	     work->control.mov.vx, work->control.mov.vy, work->control.mov.vz, work->control.map ) ;
#endif
		/* 終点チェック */
		if ( GM_CheckInsideTrap( &work->control, &OffTrap2, 1, 0 ) ) {
			_sceVu0SubVector( &diff, &OffCenter2, &npos ) ; 
			_sceVu0SubVector( &step, &npos, &ppos ) ;
			ip = _sceVu0InnerProduct( &step, &diff ) ;
			if ( ip <= 0.0F ) {
				work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
				SetMode( work, BeltConvActEnd ) ;
				/* 主観カメラ移行を戻す */
				GM_SetCameraInterpMode( work->subject_camera, GM_CAM_INTERP_INTO_SUBJECT,
									    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
			}
		}
	} else {
		//printf( "warning : outside beltconv trap!!!!!\n" ) ;
	}
	GM_SetCameraQuick( 0 ) ;
}

/* ベルトコンベア終了 */
static	void	BeltConvActEnd( PlayerWork *work, int time )
{
	FVECTOR		step, diff ;
	float		ip ;

	if ( time == 0 ) {
		_sceVu0SubVector( &step, &OffCenter1, &OffCenter2 ) ;
		GV_LenVec3F( &step, &work->fv, 0.0F, 24.0F ) ;
		PL_SetAction( work, PL_MotionSet->shared[ Mbox_idle ], 6 ) ;
		PL_LeaveSubject( work ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		if ( OffWaitTime > 0 ) ExecProc( work, PROC_MODE_ARRIVED ) ;
	}
	//SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_GRAVITY | FLAG_DONOT_CHECK_WATCH | 
			  //FLAG_NO_IK | FLAG_FORCE | FLAG_CANNOT_CHANGE ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH | 
			 FLAG_NO_IK | FLAG_FORCE | FLAG_CANNOT_CHANGE ) ;
	SetStatus( PLAYER_CB_BOX | PLAYER_WEAPON_INVISIBLE | PLAYER_FORCE ) ;

	if ( work->data == 0 ) {
		if ( OffWaitTime < 0 ||
			( work->pad->dir != -1 && 
			 GV_DiffDirAbs( work->pad->dir, GV_VecDir2( &work->fv ) ) < 256 ) ) {
			ExecProc( work, PROC_MODE_OFF_START ) ;
			PL_SetAction( work, PL_MotionSet->shared[ Mbox_run ], 6 ) ;
			//work->control.rot.vy = work->control.turn.vy = GV_VecDir2( &work->fv ) ;
			work->control.turn.vy = GV_VecDir2( &work->fv ) ;
			work->control.rot.vx = work->control.turn.vx = 0 ;
			work->control.rot.vz = work->control.turn.vz = 0 ;		
			work->data = 1 ;
			work->data2 = 0 ;
		} else {
			/* 降りパッド ＯＲ 再稼動待ち */
			if (  -- OffWaitTime <= 0 ) {
				ExecProc( work, PROC_MODE_RESTART ) ;
				work->data = 2 ;
			}
			return ;
		}
	}
	if ( work->data == 2 ) return ;

	if ( GV_DiffDirAbs( work->control.turn.vy, work->control.rot.vy ) > 128 ) {
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		return ;
	}
	//DG_COPY_VEC( &work->control.step, &work->fv ) ;
	_sceVu0SubVector( &diff, &OffCenter1, &work->control.mov ) ;
	ip = _sceVu0InnerProduct( &work->fv, &diff ) ;
	if ( ip <= 0.0F || ++ work->data2 > 300 / TIME_BASE * 5 ) {	/* 保険。5秒 */
		if ( ip > 0.0F ) {
			/* 保険のとき */
			work->control.mov.vx = OffCenter1.vx ;
			work->control.mov.vz = OffCenter1.vz ;
			work->control.step.vx = 0.0F ;
			work->control.step.vz = 0.0F ;
		}
		PL_SetAction( work, PL_MotionSet->shared[ Mbox_stop ], 6 ) ;		
		if ( EndDir >= 0 ) {
//			work->control.rot.vy = work->control.turn.vy = EndDir ;
			work->control.turn.vy = EndDir ;
		}
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;		
		UnsetFlag( FLAG_FORCE ) ;
		UnsetStatus( PLAYER_FORCE ) ;
		PL_UnsetInvincible( work ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF | MENU_RADAR_OFF ) ;
		SetMode( work, PL_CB_BoxStill ) ;
		ExecProc( work, PROC_MODE_OFF_END ) ;
		Proc = -1 ;
		OnBelt = 0 ;
	}
}

/*---------------------------------------------------------------*/

static	int		ExistGrenade( void )
{
	GM_BOMB		*this, *list ;

	list = GM_BombList.next ;
	while( list != NULL ) {
		this = list ;
		list = list->next ;
		if ( this->weapon == WP_Grenade ) {	
			return 1 ;
		}
	}
	return 0 ;
}

/* セットアップ関数 */
static	int		SetBeltConvAct( work, msg, len )
PlayerWork		*work ;
GV_MSG			*msg ;
int				len ;
{
	int			beltend = 0 ;
	
	if ( work->action == BeltConvActEnd && work->data == 2 ) beltend = 1 ;
	/* 以下のときは無効 */
	if ( OnBelt != 0 && !beltend ) return -1 ;
	if ( !Status( PLAYER_CB_BOX ) ) return -1 ;
	if ( !( PL_ItemType( work ) & IT_TYPE_CBBOX ) ) return -1 ;
	if ( Status( PLAYER_DAMAGED | PLAYER_DEAD | PLAYER_DOWNED | 
				 PLAYER_FORCE ) ) {
		if ( !beltend ) return -1 ;
	}
	/* 未爆発のグレネードがあるときは無効 */
	if ( ExistGrenade() ) return -1 ;

	if ( !beltend ) {
		PL_LeaveCaution( work ) ;
		PL_SetInvincible( work, 0 ) ; /* ベルト上は無敵 */
		PL_LeaveSubject( work ) ;
		PL_ClearCaptureTarget( work ) ;
		UnsetFlag( FLAG_FORCE_END ) ;
		SetFlag( FLAG_FORCE ) ;
		SetStatus( PLAYER_FORCE ) ;
		SetMode2( work, NULL ) ;
	} else {
		Restart = 1 ;
	}
	SetMode( work, BeltConvActStart ) ;
	OnBelt = 1 ;
	OnMap = msg->message[ 1 ] ;
	OnTrap1 = msg->message[ 2 ] ;
	OnTrap2 = msg->message[ 3 ] ;
	StartDir = msg->message[ 4 ] ;
	OffMap = msg->message[ 5 ] ;
	OffTrap1 = msg->message[ 6 ] ;
	OffTrap2 = msg->message[ 7 ] ;
	EndDir = msg->message[ 8 ] ;
	OffWaitTime = PL_GetMessageValue( msg, 9, -1 ) ;

	SetFlag( FLAG_CANNOT_CHANGE ) ;
	GM_SetMenuStatus( MENU_MENU_OFF | MENU_RADAR_OFF ) ;	/* メニュー禁止 */
	msg->message_len = 0 ;
	return 1 ;
}

/*---------------------------------------------------------------*/

/* プラグイン登録 */
int	NewPluginBeltConveyer( void )
{
    PL_AddPlugin( &PluginSet, PL_MSG_BELTCONV, SetBeltConvAct, NULL ) ;
	OnBelt = 0 ;
	Restart = 0 ;
	Proc = -1 ;
    return 0 ;
}

/* 開始プロック、終了プロックの登録 */
/* 一回降りるとクリアされる */
int	NewSetPlayerBeltConveyerProc( void )
{
	Proc = GCL_GetOptionValue( 'p', -1 ) ;
	return 0 ;
}

/* ベルトコンベア上にプレイヤーがいるかチェック */
int	PL_COM_CheckPlayerOnBelt( void )
{
	return OnBelt ;
}

