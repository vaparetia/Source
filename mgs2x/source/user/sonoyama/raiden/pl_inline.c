//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_inline.c 
   ライデン／グローバルインライン関数（そうでないのもある）
   
   2000/03/23 M.Sonoyama
   $Id: pl_inline.c,v 1.1.1.3 2002/11/19 11:50:58 Yoshizawa1 Exp $
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

#include	"game.h"
#include	"gameheader.h"
#include	"raiden.h"
#include	"pl_inline.h"
//#include	"rai_arm.h"
#include	"pl_arm.h"

#define	Work		PlayerWork 
#define	ACTION		PL_ACTION
#define	OPERATE		PL_OPERATE
#define	MOTION_SET	PL_MOTION_SET

/* 整数ベクトル → 浮動小数点ベクトル */
SNC_EXTERN_INLINE	void	PL_IVtoFV( int *iv, float *fv )
{
    int		i ;

    for ( i = 0; i < 3; i ++ ) *( fv ++ ) = ( float )( *( iv ++ ) ) ;
}

/* ステータス設定 */
#if 0 /* yano 2002.03.11 以下同様 */
SNC_EXTERN_INLINE	void	PL_SetStatus( state )
long64			state ;
#else
SNC_EXTERN_INLINE	void	PL_SetStatus( long64 state )
#endif
{
	GM_SetPlayerStatus( state ) ;
}

SNC_EXTERN_INLINE	void	PL_UnsetStatus( long64 state )
{
	GM_ResetPlayerStatus( state ) ;
}

/* ステータスチェック */
SNC_EXTERN_INLINE	long64	PL_Status( long64 state )
{
	return GM_CheckPlayerStatus( state ) ;
}

/* フラグ設定 */
SNC_EXTERN_INLINE	void	PL_SetFlag( long64 state )
{
    GM_PlayerWork->flag |= state ;
}

SNC_EXTERN_INLINE	void	PL_UnsetFlag( long64 state )
{
    GM_PlayerWork->flag &= ~state ;
}

SNC_EXTERN_INLINE	void	PL_SetFlag2( long64 state )
{
    GM_PlayerWork->flag2 |= state ;
}

SNC_EXTERN_INLINE	void	PL_UnsetFlag2( long64 state )
{
    GM_PlayerWork->flag2 &= ~state ;
}

/* フラグチェック */
SNC_EXTERN_INLINE	long64	PL_Flag( long64 state )
{
    return ( ( GM_PlayerWork->flag ) & state ) ;
}

SNC_EXTERN_INLINE	long64	PL_Flag2( long64 state )
{
    return ( ( GM_PlayerWork->flag2 ) & state ) ;
}

;

/* 主観ウェイトの設定 */
void PL_SetVWait( PlayerWork *work, int time )
{
    work->vwait = time ;
}

/* モーション設定 */
void		PL_SetAction3( work, action, time, interp )
PlayerWork	*work ;
int			time ;
int			action, interp ;
{
    int		rc ;

	if ( Flag( FLAG_ACTION_MUST_CHANGE ) ) {
		work->motion1 = -1 ;
		UnsetFlag( FLAG_ACTION_MUST_CHANGE ) ;
	}
    if ( work->motion1 == action ) return ;
    GM_ConfigObjectAction( &( work->body ), 0, action,
						  time * NTSC_TIME_BASE, 0xfffff, interp * NTSC_TIME_BASE ) ;
	if ( interp == 0 ) work->body.m_ctrl->interp_flag = 0 ;
    work->motion1 = action ;
	work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL2 ;
	if ( work->motion2 == -1 ) {
		work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;		
	}
    if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
    }
    work->root_diff = DG_ZeroVector ;
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	/* 振動 */
	if ( !Flag( FLAG_MOTION_VIB_OFF ) ) GM_VctrlSetVibration( &work->vctrl, action, 0 ) ;
//    /* 腰補完なし */
//    if ( PL_Flag( FLAG_NO_WAIST_INTERP ) ) {
//		work->body.m_ctrl->interp_flag &= ~0x01 ;
//		PL_UnsetFlag( FLAG_NO_WAIST_INTERP ) ;
//    }
    if ( PL_Flag( FLAG_NO_WAIST_INTERP_READY ) ) {
		PL_UnsetFlag( FLAG_NO_WAIST_INTERP_READY ) ;
		PL_SetFlag( FLAG_NO_WAIST_INTERP ) ;
	}
}

#if 0
void		PL_SetAction( work, action, interp )
PlayerWork	*work ;
int		action, interp ;
{
    int		rc ;

    if ( work->motion1 == action ) return ;
    GM_ConfigObjectAction( &( work->body ), 0, action,
						  0, 0xfffff, interp * NTSC_TIME_BASE ) ;
    work->motion1 = action ;
    if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
    }

    work->root_diff = DG_ZeroVector ;

    /* 腰補完なし */
    if ( PL_Flag( FLAG_NO_WAIST_INTERP ) ) {
		work->body.m_ctrl->interp_flag &= ~0x01 ;
		PL_UnsetFlag( FLAG_NO_WAIST_INTERP ) ;
    }
}
#endif

void		PL_SetAction2( work, action, interp, mask )
PlayerWork	*work ;
int		action, interp, mask ;
{
	//if ( work->motion2 == action ) return ;
	//mask = 0xffffffff ;
	if ( work->motion2 == -1 && action == -1 ) return ;

    work->motion2 = action ;    
    GM_ConfigObjectAction( &( work->body ), 1, action,
						  0, mask, interp * NTSC_TIME_BASE ) ;
	work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
    /* 腹にmerge_flag設定 */
	//    work->body.m_ctrl->merge_flag = 0x0000 ;
    if ( action >= 0 ) {
		work->body.m_ctrl->merge_flag = 0x0002 ;
    } else {
		if ( Flag( FLAG_MOTION_STEP_OVERRIDE ) ) {
			UnsetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;
			if ( mask & 1 ) {
				work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			}
		}
		work->body.m_ctrl->merge_flag = 0x0000 ;
		work->body.m_ctrl->use_move_layer = 0 ;
    }
	UnsetFlag( FLAG_RESET_MOTION2_AT_END ) ;
    if ( action >= 0 && ( mask & 1 ) ) work->root_diff = DG_ZeroVector ;
}

void		PL_SetAction2Check( PlayerWork *work, int action, int interp, int mask ) 
{
	if ( work->motion2 == action ) return ;
	PL_SetAction2( work, action, interp, mask ) ;
}

void		PL_SetAction2Ex( PlayerWork *work, int action, int time, float t, 
							 int interp, long64 mask )
{
	if ( work->motion2 == -1 && action == -1 ) return ;

    work->motion2 = action ;
	if ( time < 0 ) {
		GM_ConfigObjectAction( &( work->body ), 1, action,
							   0, mask, interp * NTSC_TIME_BASE ) ;
		time = ( int )( work->body.m_ctrl->mt3_ctrl[ 1 ].motion_total_time * t ) ;
	} else {
		time *= NTSC_TIME_BASE ;
	}
	GM_ConfigObjectAction( &( work->body ), 1, action,
						   time, mask, interp * NTSC_TIME_BASE ) ;
	work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
    /* 腹にmerge_flag設定 */
    if ( action >= 0 ) {
		work->body.m_ctrl->merge_flag = 0x0002 ;
    } else {
		if ( Flag( FLAG_MOTION_STEP_OVERRIDE ) ) {
			UnsetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;
			if ( mask & 1 ) {
				work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			}
		}
		work->body.m_ctrl->merge_flag = 0x0000 ;
		work->body.m_ctrl->use_move_layer = 0 ;
    }
	UnsetFlag( FLAG_RESET_MOTION2_AT_END ) ;

    if ( action >= 0 && ( mask & 1 ) ) work->root_diff = DG_ZeroVector ;
}

void		PL_SetAction4( PlayerWork *work, int action, float t, int interp )
{
    int		rc, time ;

    if ( work->motion1 == action ) return ;
    GM_ConfigObjectAction( &( work->body ), 0, action,
						  0, 0xfffff, interp * NTSC_TIME_BASE ) ;
    if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
    }
	time = ( int )( work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time * t ) ;
    GM_ConfigObjectAction( &( work->body ), 0, action,
						  time, 0xfffff, interp * NTSC_TIME_BASE ) ;
	if ( interp == 0 ) work->body.m_ctrl->interp_flag = 0 ;
    work->motion1 = action ;
	work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL2 ;
    work->root_diff = DG_ZeroVector ;
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
	/* 振動 */
	if ( !Flag( FLAG_MOTION_VIB_OFF ) ) GM_VctrlSetVibration( &work->vctrl, action, 0 ) ;
    /* 腰補完なし */
//    if ( PL_Flag( FLAG_NO_WAIST_INTERP ) ) {
//		work->body.m_ctrl->interp_flag &= ~0x01 ;
//		PL_UnsetFlag( FLAG_NO_WAIST_INTERP ) ;
//    }
}

/* 主観腕モーションセット */
void		PL_SetArmAction( work, action )
PlayerWork	*work ;
int		action ;
{
	if ( work->stance == GROUND ) action += AM_CROUCH_SHIFT ;
	if ( work->arm_motion == action ) return ;
	work->arm_motion = action ;
	work->arm_start_time = 0 ;
	work->arm_interp = 6 ;
}

void	PL_SetArmAction3( PlayerWork *work, int action, int time, int interp )
{
	if ( work->arm_motion == action && 
		!( work->arm_trigger & ARM_MOTION_SET_OVER ) ) return ;
	work->arm_motion = action ;
	work->arm_start_time = time ;
	work->arm_interp = interp ;
}

void	PL_SetArmAction4( PlayerWork *work, int action, float t, int interp )
{
	int			time ;

	if ( work->arm_motion == action && 
		!( work->arm_trigger & ARM_MOTION_SET_OVER ) ) return ;
    GM_ConfigObjectAction( GM_PlayerArmBody, 0, action,
						  0, 0xfffff, interp * NTSC_TIME_BASE ) ;	
	time = ( int )( GM_PlayerArmBody->m_ctrl->mt3_ctrl[ 0 ].motion_total_time * t ) ;
	work->arm_motion = action ;
	work->arm_start_time = time ;
	work->arm_interp = interp ;
}

/* アクション設定 */
void		PL_SetMode( work, action )
PlayerWork	*work ;
PL_ACTION	action ;
{	
	PL_ACTION	new_action ;

	new_action = action ;

	/* 通常静止状態をのっとる */
	if ( Flag( FLAG_RETURNMODE_SET ) ) {
		if ( PL_ReturnModes[ STAND ] != NULL && action == PL_StillMode[ STAND ] ) {
			new_action = PL_ReturnModes[ STAND ] ;
			PL_ReturnModes[ STAND ] = NULL ;
		} else if ( PL_ReturnModes[ SQUAT ] != NULL && action == PL_StillMode[ SQUAT ] ) {
			new_action = PL_ReturnModes[ SQUAT ] ;
			PL_ReturnModes[ SQUAT ] = NULL ;
		} else if ( PL_ReturnModes[ GROUND ] != NULL && action == PL_StillMode[ GROUND ] ) {
			new_action = PL_ReturnModes[ GROUND ] ;
			PL_ReturnModes[ GROUND ] = NULL ;
		}
	}

    work->time = work->ftime_count = 0 ;
    work->action = new_action ;
	work->act_name = 0 ;
	if ( !PL_SubjectMove ) {
#ifndef CAMERA_TURN
		if ( !PL_Status( PLAYER_GROUND | PLAYER_WATCH ) ) work->control.turn.vx = 0 ;
#else
		if ( !PL_Status( PLAYER_GROUND | PLAYER_WATCH ) ) {
			work->g_rot = 0 ;
			work->camdir.vx = 0 ;
		}
#endif
	}
    work->control.turn.vz = 0 ;
    work->data = work->data2 = work->counter = 0 ;
#if 0
	/* 絶対無敵時に変更されてないかチェック */
	if ( GM_CheckPlayerStatus( PLAYER_INVINCIBLE ) &&
		 work->invincible_time == 0 ) {
		printf( "warning : player : invincible mode change!\n" ) ;
	}
#endif
	/* 武器カメラＯＦＦ */
    if ( work->vwait2 <= 0 ) work->vwait2 = 4 ;
	/* 腕モーションデフォルト */
	if ( work->action2 == NULL ) PL_SetArmAction( work, AMstand ) ;
}

void		PL_SetMode2( work, action )
PlayerWork	*work ;
PL_ACTION	action ;
{
    work->time2 = work->ftime2_count = 0 ;
    work->action2 = action ;
    work->data3 = work->data4 = 0 ;
    if ( action == NULL ) {
		if ( work->motion2 >= 0 && !Flag( FLAG_RESET_MOTION2_AT_END ) ) {
			PL_SetAction2( work, -1, 6, work->body.m_ctrl->mt3_ctrl[ 1 ].mask ) ;
		}
		PL_UnsetStatus( PLAYER_HOLD ) ;
//		if ( work->weapon_camera->on ) work->vwait2 = 4 ;
		work->vwait2 = 4 ;
    }
}

/* 名前付きアクション設定 */
void	PL_SetModeName( work, action, name )
PlayerWork	*work ;
PL_ACTION	action ;
char		*name ;
{
	PL_SetMode( work, action ) ;
	work->act_name = GV_StrCode( name ) ;
}

void	PL_ModeName( PlayerWork *work, char *name )
{
	work->act_name = GV_StrCode( name ) ;
}

int		PL_CheckMode( PlayerWork *work, char *name )
{
	return ( work->act_name == GV_StrCode( name ) ) ;
}

/* フレーム数 */
int	PL_FrameTime( time )
int		time ;
{
    return ( time / ONE_CLOCK ) ;
}

/* 回転をとめる */
void	PL_StopTurn( work )
PlayerWork	*work ;
{
    work->control.turn.vy = work->control.rot.vy ;
}

/* マトリクスの平行移動成分をベクトルにいれる */
void	PL_MatToVec( w, v )
FMATRIX		*w ;
FVECTOR		*v ;
{
//    v->vx = w->m[ 3 ][ 0 ] ;
//    v->vy = w->m[ 3 ][ 1 ] ;
//    v->vz = w->m[ 3 ][ 2 ] ;
      GV_MatToVec( w, v ) ;
}

/* ビハインド覗き用パラメータ初期化 */
void	PL_InitBehindPeepCamera( work )
PlayerWork	*work ;
{
    work->behindcam_add = DG_ZeroVector ;
    work->behindtrg_add = DG_ZeroVector ;
}

/* 敵チェック部位設定 */
void	PL_AddFindObj( obj )
int		obj ;
{
    GM_PlayerFindObj |= GV_GetBit( obj ) ;
#ifdef DEBUG_MODE
//	if ( GM_PlayerDebugMode == GM_PDM_FINDPOSVIEW ) {
	if ( PlayerDebugMenuStatus & PDMS_FINDPOSVIEW ) {
		FVECTOR			pos ;
		SVECTOR			col = { 32, 232, 32 } ;
		PL_ObjPos( GM_PlayerWork, obj, &pos ) ;
		PosBox( &pos, 150.0F, &col ) ;
	}
#endif
}

/* アドレスを再計算 */
void	PL_ResetAddress( work )
PlayerWork	*work ;
{
    FVECTOR	mov ;
    CONTROL	*ctrl ;

    ctrl = &( work->control ) ;
    mov = ctrl->mov ;
    mov.vy = ctrl->levels[ 0 ] ;
    ctrl->addr = HZX_GetAddress( ctrl->hzx_id, &mov, ctrl->addr ) ;
}

float	PL_ObjHeight( work, n ) 
PlayerWork	*work ;
int		n ;
{
    return work->body.objs->objs[ n ].world.m[ 3 ][ 1 ] ;
}

FMATRIX	*PL_ObjWorld( work, n )
PlayerWork		*work ;
int				n ;
{
	return &( work->body.objs->objs[ n ].world ) ;
}

int	PL_GetMessageValue( msg, n, def )
GV_MSG		*msg ;
int		n ;
int		def ;
{
    if ( msg->message_len >= n + 1 ) return msg->message[ n ] ;
    return def ;
}

/* 武器タイプ */
int	PL_WeaponType( work )
PlayerWork		*work ;
{
    WeaponSet	*wp ;
    if ( ( wp = ( WeaponSet * )( work->wp_set ) ) == NULL ) return 0 ;
    return wp->type ;
}

/* アイテムタイプ */
int	PL_ItemType( work )
PlayerWork		*work ;
{
    ItemSet	*it ;
    if ( ( it = ( ItemSet * )( work->it_set ) ) == NULL ) return 0 ;
    return it->type ;
}

/* モーション経過時間 */
int	PL_MotionTime( work )
PlayerWork	*work ;
{
    return ( int )( work->body.m_ctrl->mt3_ctrl->play_time / 
				   ( float )NTSC_TIME_BASE ) ;
//			work->body.m_ctrl->mt3_ctrl->motion_time_base ) ;
}

int	PL_MotionTime2( work )
PlayerWork	*work ;
{
    return ( int )( work->body.m_ctrl->mt3_ctrl[ 1 ].play_time / 
				   ( float )NTSC_TIME_BASE ) ;
//			 work->body.m_ctrl->mt3_ctrl[ 1 ].motion_time_base ) ;
}

int	PL_CheckMotionTime( PlayerWork *work, int time )
{
	return MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, time * NTSC_TIME_BASE ) ;
}

int	PL_CheckMotionTime2( PlayerWork *work, int time )
{
	return MT_CHECK_MOTION_TIME( work->body.m_ctrl, 1, time * NTSC_TIME_BASE ) ;
}

float	PL_CheckMotionRate( PlayerWork *work )
{
	float		t ;

	t = work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
		/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
	return t ;
}

/* オブジェの位置 */
void	PL_ObjPos( work, n, pos )
PlayerWork		*work ;
int				n ;
FVECTOR			*pos ;
{
	GV_MatToVec( &work->body.objs->objs[ n ].world, pos ) ;
}

void	PL_ArmObjPos( int n, FVECTOR *pos )
{
	ASSERT( GM_PlayerArmBody != NULL ) ;
	GV_MatToVec( &GM_PlayerArmBody->objs->objs[ n ].world, pos ) ;
}

/* 可視チェック */
int			PL_CheckInvisible( work )
PlayerWork			*work ;
{
	return ( work->body.objs->flag & ( DG_FLAG_INVISIBLE0 << work->chanl ) ) ;
}

/* モーションファイルチェンジ */
void			PL_ChangeMotionArc( work, mar )
PlayerWork			*work ;
int					mar ;
{
	if ( mar == work->current_mar ) return ;
	work->body.m_ctrl->motion_arc = MT_GetMotionArchives( mar ) ;
	MT_FreeSequence( work->body.m_ctrl->sar_ctrl ) ;
	work->body.m_ctrl->sar_ctrl = MT_InitSequence( work->body.m_ctrl->n_layer, mar, 0 ) ;
	ASSERT( work->body.m_ctrl->sar_ctrl == NULL ||
		    work->body.m_ctrl->sar_ctrl->header != NULL ) ;	
	work->motion1 = -1 ;
	work->current_mar = mar ;
	GM_VctrlStopVibration( &work->vctrl ) ;
	work->vctrl.def = GM_GetVibArc( mar ) ;
}

/* モーションファイル戻す設定 */
void	PL_ReturnMotionArc( PlayerWork *work )
{
	PL_Force->e_turn = -1 ;
	PL_Force->flag = 0 ;
	SetFlag( FLAG_RETURN_MOTION_ARC ) ;
}

/* モーション停止 */
void	PL_MotionSleep( PlayerWork *work, int layer )
{
	work->body.m_ctrl->mt3_ctrl[ layer ].flag |= MT3_SLEEP ;
}

/* モーション再開 */
void	PL_MotionActive( PlayerWork *work, int layer )
{
	work->body.m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_SLEEP ;
}

/* 死亡時フラグセット */
void	PL_SetDeadFlag( PlayerWork *work )
{
	if ( GM_IsGameOver() || Status( PLAYER_DEAD ) ) {
		GM_SetMenuStatus( MENU_WEAPON_OFF | MENU_ITEM_OFF | MENU_GAGE_OFF | MENU_RADAR_OFF |
						  MENU_SUBWIN_OFF ) ;
		SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
		SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
				 FLAG_DONOT_EXEC_MESSAGE | 
				 FLAG_DONOT_RECEIVE_MESSAGE | FLAG_NO_HEADTOENEMY ) ;
		if ( Status( PLAYER_DEAD ) ) {
			SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK ) ;
		}
	}
}

/* 左スティック使用？ */
int	PL_UseStickL( PlayerWork *work )
{
	return ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) ;
}

/* 左スティック使用？ */
int	PL_UseStickR( PlayerWork *work )
{
	return ( work->pad->analog_input & GV_PAD_ANALOG_R_USE ) ;
}

/* 強制モーション以外での強制終了 */
void	PL_EndForceAct( PlayerWork *work, int flag, int dir )
{
	work->flag |= FLAG_FORCE_END ;
	if ( flag >= 0 ) ForceWork.flag = flag ;
	if ( dir >= -1 ) ForceWork.e_turn = dir ;
}
