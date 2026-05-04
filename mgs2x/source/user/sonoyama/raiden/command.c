//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   command.c
   プレイヤー／シナリオリンク関数
   
   1999/12/15 M.Sonoyama
   $Id: command.c,v 1.3 2002/12/17 02:59:54 takaki Exp $
   */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

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
#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_inline.h"

/* ステータス取得 */
int	NewGetPlayerStatus( void )
{
	extern	long64	HZX_PlayerStateMask[] ;
    long64	p_mask, mask ;
    int		c ;

    p_mask = GM_CheckPlayerStatus( PLAYER_NORMAL_MASK ) ;
    if ( p_mask == 0 ) p_mask |= HZX_PLAYER_NORMAL ;
    mask = 0 ;
    if ( GCL_GetOption( 'f' ) != NULL ) {
		while( GCL_NextStr() != NULL ) {
			mask |= HZX_PlayerStateMask[ GCL_GetNextInt() ] ;
		}
    }
    c = ( mask & p_mask ) ? 1 : 0 ;
    return c ;
}

/* プレイヤーのいるマップの名前を返す */
int	NewGetPlayerMapName( void )
{
    MAP		*map ;
    if ( GM_PlayerControl == NULL ) return 0 ;
    map = GM_GetMap( GM_PlayerControl->map ) ;
    return map->name ;
}

/* エルード設定 */
int	NewSetEludeParam( void )
{
    BEYOND	*b ;

    b = PL_Beyond ;
	b->flag &= ~BY_RANGE_EXIST ;
    GCL_GetOption( 'v' ) ;
    b->still = GCL_GetNextInt() ;
    b->move1 = GCL_GetNextInt() ;
    b->move2 = GCL_GetNextInt() ;
    b->back = GCL_GetNextInt() ;
    b->object = GCL_GetNextInt() ;
    b->s_turn = GCL_GetNextInt() ;
    b->e_turn = GCL_GetNextInt() ;
    b->flag = GCL_GetNextInt() ;
    b->back2 = GCL_GetNextInt() ;
    b->wall_obj = GCL_GetNextInt() ;
    b->tired = GCL_GetNextInt() ;
    b->fall_start = GCL_GetNextInt() ;
    b->fall_loop = GCL_GetNextInt() ;
    b->fall_damage = GCL_GetNextInt() ;
    b->fall_out = GCL_GetNextInt() ;
    b->fall_safe = GCL_GetNextInt() ;
	b->damage = GCL_GetNextInt() ;
	
	if ( GCL_NextStr() != NULL ) b->up_start = GCL_GetNextInt() ;
	else						 b->up_start = 255 ;
	if ( GCL_NextStr() != NULL ) b->up_idle = GCL_GetNextInt() ;
	else						 b->up_idle = 255 ;
	if ( GCL_NextStr() != NULL ) b->up_end = GCL_GetNextInt() ;
	else						 b->up_end = 255 ;

	return 1 ;
}

/* エルード範囲設定 */
int		NewSetEludeRange( void )
{
    BEYOND	*b ;

    b = PL_Beyond ;
	if ( GCL_GetOption( 'x' ) != NULL ) {
		b->flag |= BY_X_RANGE_EXIST ;
		b->x_min = GCL_GetNextInt() ;
		b->x_max = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 'z' ) != NULL ) {
		b->flag |= BY_Z_RANGE_EXIST ;
		b->z_min = GCL_GetNextInt() ;
		b->z_max = GCL_GetNextInt() ;
	}
	return 1 ;
}

/* 無敵にする */
int		NewPlayerSetInvincible( void )
{
	GM_SetPlayerStatus( PLAYER_INVINCIBLE_SCN ) ;
	return 1 ;
}

/* 無敵を解除する */
int		NewPlayerResetInvincible( void )
{
	GM_ResetPlayerStatus (PLAYER_INVINCIBLE_SCN ) ;
	return 1 ;
}

/* 強制モーション終了プロック */
int		NewForceActEndProc( void )
{
	ASSERT( PL_Force != NULL ) ;
	GCL_GetOption( 'p' ) ;
	PL_Force->end_proc = GCL_GetNextInt() ;
	PL_Force->end_proc_motion = GCL_GetOptionValue( 'm', -1 ) ;
	if ( GCL_GetOption( 'a' ) != NULL ) {
		int			n = 0 ;

		while( GCL_NextStr() != NULL ) {
			ASSERT( n < 4 ) ;
			PL_Force->proc_args[ n ] = GCL_GetNextInt() ;
			n ++ ;
		}
	}
	return 1 ;
}

/* 強制モーションプロック */
int		NewForceActProc( void )
{
	ASSERT( PL_Force != NULL ) ;
	if ( GCL_GetOption( 'p' ) != NULL ) {
		PL_Force->proc = GCL_GetNextInt() ;
		PL_Force->proc_flag = FA_PROC_FLAG_PROC ;
	} else {
		PL_Force->proc = GCL_GetOptionValue( 'e', 0 ) ;
		PL_Force->proc_flag = FA_PROC_FLAG_BLOCK ;
	}
	ASSERT( PL_Force->proc != 0 ) ;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		PL_Force->proc_flag |= ( GCL_GetNextInt() & ( FA_PROC_FLAG_EVERY | FA_PROC_FLAG_END ) ) ;
	}
	return 1 ;
}

/* 強制モーションキャンセル */
int		NewForceActCancel( void )
{
	ASSERT( PL_Force != NULL ) ;
	if ( !PL_CheckMode( GM_PlayerWork, "ForceAct" ) ) return 0 ;
	PL_Force->flag |= FA_CANCEL ;
	return 1 ;
}

/* 強制モーションループ設定 */
int		PL_COM_SetForceActLoop( void )
{
	PL_Force->loop = GCL_GetOptionValue( 'l', 0 ) ;
	return 1 ;
}

/* 死体歩き */
int		NewCorpseWalk( void )
{
	GM_SetPlayerStatus( PLAYER_ON_CORPSE ) ;
	return 1 ;
}

/* モーション振動止める */
int		NewPlayerMotionVibOff( void )
{
	if ( GM_PlayerWork != NULL ) {
		PL_SetFlag( FLAG_MOTION_VIB_OFF ) ;
		GM_VctrlStopVibration( &GM_PlayerWork->vctrl ) ;
	}
	return 1 ;
}

/* 主観武器、装備サイト表示／非表示 */
int		PL_COM_SightOnOff( void )
{
	int		which ;

	which = GCL_GetOptionValue( 's', 0 ) ;
	if ( which == GM_STRCODE_OFF ) GM_SetSightStatus( SGT_Invisible ) ;
	else						   GM_ResetSightStatus( SGT_Invisible ) ;
	return 0 ;
}

/* シナリオで前武器を変更 */
int		PL_COM_ChangePrevWeaponScn( void )
{
	GM_WeaponPrevChangedScn = GCL_GetOptionValue( 'n', 0 ) ;
	return 0 ;
}

/* シナリオで前装備を変更 */
int		PL_COM_ChangePrevItemScn( void )
{
	GM_ItemPrevChangedScn = GCL_GetOptionValue( 'n', 0 ) ;
	return 0 ;
}

/* カメラランダム振動 */
int		PL_COM_VibCamera( void )
{
	int		level ;

	level = GCL_GetOptionValue( 'l', 60 ) ;
	PL_DamageCamera( level, I64(0) ) ;
	return 0 ;
}

/* ミサイル移動範囲設定 */
int		PL_COM_SetMissileBounding( void )
{
	GM_MissileBoundMin.vx = ( float )GCL_GetNextInt() ;
	GM_MissileBoundMin.vy = ( float )GCL_GetNextInt() ;
	GM_MissileBoundMin.vz = ( float )GCL_GetNextInt() ;
	GM_MissileBoundMax.vx = ( float )GCL_GetNextInt() ;
	GM_MissileBoundMax.vy = ( float )GCL_GetNextInt() ;
	GM_MissileBoundMax.vz = ( float )GCL_GetNextInt() ;
	return 0 ;
}

/* プレイヤー可視／不可視シナリオ版 */
int		PL_COM_InvisiblePlayer( void )
{
	PL_InvisiblePlayer() ;
	return 0 ;
}

int		PL_COM_VisiblePlayer( void )
{
	PL_InvisiblePlayer() ;
	return 0 ;
}


/* プレイヤーの主観移動モードにする */
int		PL_COM_SetSubjectMove( void )
{
    if ( GCL_GetOption( 'm' ) ) {
	extern int PL_SubjectMove ;

	PL_SubjectMove = GCL_GetNextInt() ;
    }

    /* 主観移動モード角度 */
    if ( GCL_GetOption( 'a' ) ) {
	extern float PL_SubjectAngle    ;

	PL_SubjectAngle = GCL_GetNextInt()/100.0f ;
    }

#ifdef KP_WINDOWS
	if ( PL_SubjectMove ) {
		GV_SetWindowsInputConfigulationMode(TRUE) ;	// 主観移動操作モード

		PL_SubjectMove = 3 ;
	} else {
		GV_SetWindowsInputConfigulationMode(FALSE) ; // 通常移動視点モード
	}
#endif

    return 0 ;
}

/* プレイヤーの描画を指定できるようにする */
int		PL_COM_SetObjectFlag( void )
{
    if ( GCL_GetOption( 'l' ) )
	  PL_ObjectFlag = DG_FLAG_LATTERDRAW ;

    return 0 ;
}

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT			actor ;
	int				id1, id2 ;
} EXC_Work ;

static	void	EXC_Die( EXC_Work *work )
{
	GV_ExchangeCache( work->id1, work->id2 ) ;
}

int		PL_COM_ExchangeCache( void )
{
	EXC_Work	*work ;
	int			id1, id2 ;

	if ( GCL_GetOption( 'e' ) != NULL ) {
		id1 = GV_CacheID( GCL_GetNextInt(), 'e' ) ;
		id2 = GV_CacheID( GCL_GetNextInt(), 'e' ) ;
	} else if ( GCL_GetOption( 'k' ) != NULL ) {
		id1 = GV_CacheID( GCL_GetNextInt(), 'k' ) ;
		id2 = GV_CacheID( GCL_GetNextInt(), 'k' ) ;
	} else {	
		return 0 ;
	}

	work = ( EXC_Work * )GV_NewActor( GV_ACTOR_USER, sizeof( EXC_Work ) ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, NULL, EXC_Die ) ;
	work->id1 = id1 ;
	work->id2 = id2 ;
	
	GV_ExchangeCache( id1, id2 ) ;

	return 0 ;
}

