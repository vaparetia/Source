//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_move.c
	量産型ＲＡＹ移動処理ルーチン

	2001/04/04 K.Takabe
	$Id: pdr_move.c,v 1.1.1.3 2002/11/19 11:51:25 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

#include "pdray.h"


/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
	/*
		ローカル用算術補助関数
	*/
#if 0
/* 基準軸上での距離を求める（基準軸ベクトルは正規化されていること） */
static float GetVecWidth( FVECTOR *from, FVECTOR *to, FVECTOR *axis )
{
	float	len ;
	len  = ( to->vx - from->vx ) * axis->vx ;
	len += ( to->vy - from->vy ) * axis->vy ;
	len += ( to->vz - from->vz ) * axis->vz ;
	return ( len );
}
#endif

/* ユークリッド座標から極座標へ */
static void ConvPos_UtoP( float *length, float *angle, FVECTOR *org )
{
	*angle = atan2f( org->vx, org->vz );
	*length = DG_SQRT( org->vx * org->vx + org->vz * org->vz );
	
}
#if 0
/* 極座標からユークリッド座標へ */
static void ConvPos_PtoU( FVECTOR *res, float length, float angle )
{
	FVECTOR		tmp_vec ;
	GTE_SinCos( &tmp_vec, angle );
	res->vx = tmp_vec.vy * length ;
	res->vy = 0 ;
	res->vz = tmp_vec.vx * length ;
	
}
#endif
/* ---------------------------------------------------------------- */
/* 現在の足の座標をＩＫ基準位置として記録 */
void PDRAY_ForceRecordLegPosition( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	ctrl = &work->control ;
	body = &work->object ;
	work->right_leg_target_pos = *(FVECTOR*)body->objs->objs[ PDRAY_JOINT_RIGHT_ANKLE ].world.m[3] ;
	work->left_leg_target_pos = *(FVECTOR*)body->objs->objs[ PDRAY_JOINT_LEFT_ANKLE ].world.m[3] ;
}

/* 必要に応じて現在の足の座標をＩＫ基準位置として記録し、腰位置も補正 */
void PDRAY_RecordIKPosition( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	ankle_height ;

	ctrl = &work->control ;
	body = &work->object ;

	ankle_height = body->objs->objs[ PDRAY_JOINT_RIGHT_ANKLE ].world.m[3][1] ;	/* オリジナルの高さ取得 */
	switch ( work->right_leg_ik_mode ){
	  case IK_MODE_FREE:
		work->right_leg_target_pos = *(FVECTOR*)body->objs->objs[ PDRAY_JOINT_RIGHT_ANKLE ].world.m[3] ;
		work->right_leg_fall_speed = 0.0f ;
		break ;
	  case IK_MODE_FIX:
		/* 何もしない（以前のＩＫ制御座標を保持する） */
		/* 高さのみモーションの影響を受ける */
		work->right_leg_target_pos.vy = ankle_height ;
		work->right_leg_fall_speed = 0.0f ;
		break ;
	  case IK_MODE_CONTROL:
		/* 高さのみモーションの影響を受ける */
		work->right_leg_target_pos.vy = ankle_height ;
		work->right_leg_target_pos.vx = work->ik_target.vx ;
		work->right_leg_target_pos.vz = work->ik_target.vz ;
		work->right_leg_fall_speed = 0.0f ;
		break ;
	  case IK_MODE_FIXFALL:
		/* ＸＺは固定で、Ｙを自由落下させる */
		work->right_leg_fall_speed += ANKLE_ACCELERATE ;
		work->right_leg_target_pos.vy -= work->right_leg_fall_speed ;
		if ( work->right_leg_target_pos.vy < ankle_height ){
			work->right_leg_target_pos.vy = ankle_height ;
			work->right_leg_ik_mode = IK_MODE_FIX ;
		}
		break ;
	}
	ankle_height = body->objs->objs[ PDRAY_JOINT_RIGHT_ANKLE ].world.m[3][1] ;	/* オリジナルの高さ取得 */
	switch ( work->left_leg_ik_mode ){
	  case IK_MODE_FREE:
		work->left_leg_target_pos = *(FVECTOR*)body->objs->objs[ PDRAY_JOINT_LEFT_ANKLE ].world.m[3] ;
		work->left_leg_fall_speed = 0.0f ;
		break ;
	  case IK_MODE_FIX:
		/* 何もしない（以前のＩＫ制御座標を保持する） */
		/* 高さのみモーションの影響を受ける */
		work->left_leg_target_pos.vy = body->objs->objs[ PDRAY_JOINT_LEFT_ANKLE ].world.m[3][1] ;
		work->left_leg_fall_speed = 0.0f ;
		break ;
	  case IK_MODE_CONTROL:
		/* 高さのみモーションの影響を受ける */
		work->left_leg_target_pos.vy = body->objs->objs[ PDRAY_JOINT_LEFT_ANKLE ].world.m[3][1] ;
		work->left_leg_target_pos.vx = work->ik_target.vx ;
		work->left_leg_target_pos.vz = work->ik_target.vz ;
		work->left_leg_fall_speed = 0.0f ;
		break ;
	  case IK_MODE_FIXFALL:
		/* ＸＺは固定で、Ｙを自由落下させる */
		work->left_leg_fall_speed += ANKLE_ACCELERATE ;
		work->left_leg_target_pos.vy -= work->left_leg_fall_speed ;
		if ( work->left_leg_target_pos.vy < ankle_height ){
			work->left_leg_target_pos.vy = ankle_height ;
			work->left_leg_ik_mode = IK_MODE_FIX ;
		}
		break ;
	}
	//printf("%f %f\n", work->right_leg_target_pos.vy, work->left_leg_target_pos.vy );
	//AN_Test_Eye2( &work->right_leg_target_pos, 3 );
	//AN_Test_Eye2( &work->left_leg_target_pos, 3 );
	{/* 腰位置の補正処理 */
		FVECTOR		tmp_vec, org_offset ;
		/* モーションオリジナルの足の位置と腰位置から実際の座標を補正する */
		GTE_AddVector( &tmp_vec,
					  (FVECTOR*)body->objs->objs[ PDRAY_JOINT_RIGHT_ANKLE ].world.m[3],
					  (FVECTOR*)body->objs->objs[ PDRAY_JOINT_LEFT_ANKLE ].world.m[3] );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, 0.5f );
		GTE_SubVector( &org_offset, (FVECTOR*)body->objs->world.m[3], &tmp_vec );
		GTE_AddVector( &tmp_vec, &work->right_leg_target_pos, &work->left_leg_target_pos );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, 0.5f );
		/* Ｘ、Ｚのみ補正する */
		ctrl->mov.vx = tmp_vec.vx + org_offset.vx ;
		ctrl->mov.vz = tmp_vec.vz + org_offset.vz ;
		body->objs->world.m[3][0] = ctrl->mov.vx ;
		body->objs->world.m[3][2] = ctrl->mov.vz ;
	}
#if 0
	if ( work->no == 0 ){
		printf("left %f %f %f\n",
			   work->left_leg_target_pos.vx, work->left_leg_target_pos.vy, work->left_leg_target_pos.vz );
		printf("right %f %f %f\n",
			   work->right_leg_target_pos.vx, work->right_leg_target_pos.vy, work->right_leg_target_pos.vz );
		printf("mov %f %f %f\n", 
			   ctrl->mov.vx, ctrl->mov.vy, ctrl->mov.vz );
	}
#endif
}
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* 量産型ＲＡＹ専用：モーションセット関数 */
int PDRAY_SetMotion( Work *work, int motion_num, int flag )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	int		motion_flag ;
	float	scale = 1.0f ;

	ctrl = &work->control ;
	body = &work->object ;
	motion_flag = flag & ~(MOTION_FLAG_SUBMASK) ;

	if ( !( flag & MOTION_FLAG_AGAIN ) ){
		/* 連続モーション設定を防止する */
		if ( work->current_motion_num[0] == motion_num &&
			work->current_motion_flag[0] == motion_flag ){
			return ( 0 );/* 既に再生中 */
		}
	}

	if ( work->current_motion_num[0] != MOTION_IDLE ){
		if ( !( flag & MOTION_FLAG_FORCE ) ){
			/* モーションの切り替えをチェックする */
			if ( !MT_CHECK_END( body->m_ctrl, 0 ) ){
				return ( -1 );/* タイミングが合わなくて失敗 */
			}
		}
	}

	if ( !( flag & MOTION_FLAG_NOINTERP ) ){
		GM_ConfigObjectAction( body, 0, motion_num, 0, ~0, MOTION_INTERP_TIME );
	} else {
		/* 補間なしでモーション切り替え（ＩＫで不具合が出る場合に使用する） */
		GM_ConfigObjectAction( body, 0, motion_num, 0, ~0, 0 );
	}
	/* 体のの左右反転 */
	if ( motion_flag & MOTION_FLAG_BODY_REVERS ){
		body->m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
	} else {
		body->m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
	}
	/* 足の左右反転 */
	if ( motion_flag & MOTION_FLAG_LEG_REVERS ){
		body->m_ctrl->flag |= MT_FLAG_REVERSAL2 ;
	} else {
		body->m_ctrl->flag &= ~MT_FLAG_REVERSAL2 ;
	}
	//MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * 1.4f );
#if 0
	if ( motion_num == MOTION_WALK ){
		MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * 2.0f );
	} else {
		MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * 1.0f );
	}
#endif
	//MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * work->motion_speed );
	//work->motion_speed = 1.0f ;
	scale = 1.0f + RAYSERVER_GameUpLevel * 0.1f ;
	switch ( motion_num ){
	  case MOTION_ATTACK1_START:
	  case MOTION_ATTACK1_END:
		MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * 1.5f * scale );
		break ;
	  default:
		MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * 1.0f * scale );
		break ;
	}

	/* モーション情報の記録 */
	work->current_motion_num[0] = motion_num ;
	work->current_motion_flag[0] = motion_flag ;

	{/* モーション補間用にＩＫ計算済みモーションをモーションワークにコピー */
		int		i ;
		for ( i = 0 ; i < PDRAY_MAX_JOINTS ; i++ ){
			body->m_ctrl->old_abs_rots[ i ] = work->abs_rots[ i ] ;
		}
	}
	/* 首ＩＫ禁止状態の解除 */
	work->disable_head_ik_flag = 0 ;

	return ( 1 );/* モーションセット完了 */
}
/* 量産型ＲＡＹ専用：モーションセット関数 */
int PDRAY_SetMotionUpper( Work *work, int motion_num, int flag )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	int		motion_flag ;

	ctrl = &work->control ;
	body = &work->object ;
	motion_flag = flag & ~(MOTION_FLAG_SUBMASK) ;

	if ( !( flag & MOTION_FLAG_AGAIN ) ){
		/* 連続モーション設定を防止する */
		if ( work->current_motion_num[1] == motion_num &&
			work->current_motion_flag[1] == motion_flag ){
			return ( 0 );/* 既に再生中 */
		}
	}

	if ( !( flag & MOTION_FLAG_FORCE ) ){
		/* モーションの切り替えをチェックする */
		if ( !MT_CHECK_END( body->m_ctrl, 0 ) ){
			return ( -1 );/* タイミングが合わなくて失敗 */
		}
	}

	if ( !( flag & MOTION_FLAG_NOINTERP ) ){
		GM_ConfigObjectAction( body, 1, motion_num, 0, PDRAY_JOINT_MASK_UPPER, MOTION_INTERP_TIME );
	} else {
		/* 補間なしでモーション切り替え（ＩＫで不具合が出る場合に使用する） */
		GM_ConfigObjectAction( body, 1, motion_num, 0, PDRAY_JOINT_MASK_UPPER, 0 );
	}
	/* 体のの左右反転 */
	if ( motion_flag & MOTION_FLAG_BODY_REVERS ){
		body->m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
	} else {
		body->m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
	}
	/* 足の左右反転 */
	if ( motion_flag & MOTION_FLAG_LEG_REVERS ){
		body->m_ctrl->flag |= MT_FLAG_REVERSAL2 ;
	} else {
		body->m_ctrl->flag &= ~MT_FLAG_REVERSAL2 ;
	}
	//MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * 1.4f );
	//MT_SetMotionSpeed( body->m_ctrl, TIME_BASE * work->motion_speed );
	//work->motion_speed = 1.0f ;

	/* モーション情報の記録 */
	work->current_motion_num[0] = motion_num ;
	work->current_motion_flag[0] = motion_flag ;

	{/* モーション補間用にＩＫ計算済みモーションをモーションワークにコピー */
		int		i ;
		for ( i = 0 ; i < PDRAY_MAX_JOINTS ; i++ ){
			body->m_ctrl->old_abs_rots[ i ] = work->abs_rots[ i ] ;
		}
	}

	return ( 1 );/* モーションセット完了 */
}
/* 切り替えチェック機能付きモーションセット関数 */
int PDRAY_SetMotionCheck( Work *work, int motion_num, int flag )
{
	OBJECT	*body = &work->object ;

	/* モーションループ時のみモーションの切り替えが行えるようにする */
	if ( MT_CHECK_LAST1( body->m_ctrl, 0 ) ){	/* 実際には終了１フレーム前なので注意 */
		PDRAY_SetMotion( work, motion_num, flag );
		return ( 0 );
	}
	return ( -1 );
}
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
//static int Move_Idle_Init( Work *work, PROC_WORK *proc_work );
//static int Move_WalkReady_Init( Work *work, PROC_WORK *proc_work );
static int Move_Free_Init( Work *work, PROC_WORK *proc_work );
static int Move_Free2_Init( Work *work, PROC_WORK *proc_work );
PROC_CALLBACK	move_new_func_list[] = {
	NULL,					/* MOVE_IDLE */
	NULL,					/* MOVE_WALK_READY */
	NULL,					/* MOVE_WALK */
	NULL,					/* MOVE_STEP */
	NULL,					/* MOVE_WALK2 */
	NULL,					/* MOVE_STEP2 */
	Move_Free_Init,			/* MOVE_FREE */
	Move_Free2_Init,		/* MOVE_FREE2 */
};

/* ---------------------------------------------------------------- */
static int MoveCursor( Work *work, float move_speed, int turn_speed )
{
	/* 移動用カーソルの移動処理 */
	FVECTOR		tmp_vec ;
	float		len ;
	int			angle, tmp_angle, end_flag = 0 ;

	GTE_SubVector( &tmp_vec, &work->move_target, &work->move_cursor_mov );
	tmp_vec.vy = 0.0f ;
	len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
	if ( len > move_speed ){
		/* 目標座標に向かって移動する処理 */
		GTE_ScaleVector( &tmp_vec, &tmp_vec, move_speed / len );
		GTE_AddVector( &work->move_cursor_mov, &work->move_cursor_mov, &tmp_vec );
		/* 角度を目標座標に向ける */
#if 0
		angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
		angle = GTE_REGULAR_ANGLE( angle );
		tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
		if ( tmp_angle > -1024 && tmp_angle < 1024 ){
			if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
			if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
		} else {
			tmp_angle = GTE_REGULAR_ANGLE( tmp_angle - 2048 );
			if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
			if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
				
		}
#else
		/* 角度を目標座標に向ける */
		GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_cursor_mov );
		angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
		angle = GTE_REGULAR_ANGLE( angle );
		tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
		if ( /*tmp_angle == 0*/ tmp_angle <= turn_speed && tmp_angle >= -turn_speed ){
			//end_flag = 1 ;
		}
		if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
		if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
#endif
		work->move_cursor_rot.vy += tmp_angle ;
	} else {
		/* 目標座標から注目座標方向に向く */
		GTE_AddVector( &work->move_cursor_mov, &work->move_cursor_mov, &tmp_vec );
		/* 角度を目標座標に向ける */
		GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_cursor_mov );
		angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
		angle = GTE_REGULAR_ANGLE( angle );
		tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
		if ( /*tmp_angle == 0*/ tmp_angle <= turn_speed && tmp_angle >= -turn_speed ){
			end_flag = 1 ;
		}
		if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
		if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
		work->move_cursor_rot.vy += tmp_angle ;
	}
	work->move_cursor_rot.vy = GTE_REGULAR_ANGLE( work->move_cursor_rot.vy );
	return ( end_flag );
}
/* 極座標で移動を行う */
static int MoveCursor2( Work *work, float move_speed, int turn_speed )
{
	/* 移動用カーソルの移動処理 */
	FVECTOR		tmp_vec ;
	float		len, goal_len ;
	int			angle, tmp_angle, end_flag = 0 ;

	GTE_SubVector( &tmp_vec, &work->move_target, &work->move_cursor_mov );
	tmp_vec.vy = 0.0f ;
	len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
	if ( len > move_speed ){
		/* 目標座標に向かって移動する処理 */
		goal_len = len ;
#if 0
		GTE_ScaleVector( &tmp_vec, &tmp_vec, move_speed / len );
#else
		/* 極座標に沿って移動（円を描くように） */
		{/* なんだかよくわからなくなってきた・・・ */
			FMATRIX		mat ;
			FVECTOR		vec ;
			float now_length, now_angle, trg_length, trg_angle, diff_length, diff_angle ;
			float vr_r_length ;/* 円移動による仮想的な移動軌跡距離 */
			ConvPos_UtoP( &now_length, &now_angle, &work->move_cursor_mov );
			ConvPos_UtoP( &trg_length, &trg_angle, &work->move_target );
			diff_angle = trg_angle - now_angle ;
			if ( diff_angle > GTE_PI ) diff_angle -= 2 * GTE_PI ;
			if ( diff_angle < -GTE_PI ) diff_angle += 2 * GTE_PI ;
			diff_length = trg_length - now_length ;
			vr_r_length = ( now_length * diff_angle + trg_length * diff_angle ) * 0.5f ;
			vec.vz = diff_length ;
			vec.vy = 0 ;
			vec.vx = vr_r_length ;
			vec.vw = 0 ;
			GTE_UnitMatrix( &mat );
			_sceVu0RotMatrixY( &mat, &mat, now_angle );
			GTE_ApplyMatrix( &tmp_vec, &mat, &vec );
			
			tmp_vec.vy = 0.0f ;
			len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
			GTE_ScaleVector( &tmp_vec, &tmp_vec, move_speed / len );
		}
#endif
		GTE_AddVector( &work->move_cursor_mov, &work->move_cursor_mov, &tmp_vec );
		/* 角度を目標座標に向ける */
#if 0
		angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
		angle = GTE_REGULAR_ANGLE( angle );
		tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
		if ( tmp_angle > -1024 && tmp_angle < 1024 ){
			if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
			if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
		} else {
			tmp_angle = GTE_REGULAR_ANGLE( tmp_angle - 2048 );
			if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
			if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
				
		}
#else
		/* 角度を目標座標に向ける */
		if ( goal_len < 10000.0f ){
			/* 目標が近い場合は注目座標を向くように */
			GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_cursor_mov );
		} else {
			/* 目標が遠い場合には進行方向を向くように */
		}
		angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
		angle = GTE_REGULAR_ANGLE( angle );
		tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
		//if ( /*tmp_angle == 0*/ tmp_angle <= turn_speed && tmp_angle >= -turn_speed ){
		//	end_flag = 1 ;
		//}
		if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
		if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
#endif
		work->move_cursor_rot.vy += tmp_angle ;
	} else {
		/* 目標座標から注目座標方向に向く */
		GTE_AddVector( &work->move_cursor_mov, &work->move_cursor_mov, &tmp_vec );
		/* 角度を目標座標に向ける */
		GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_cursor_mov );
		angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
		angle = GTE_REGULAR_ANGLE( angle );
		tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
		if ( /*tmp_angle == 0*/ tmp_angle <= turn_speed && tmp_angle >= -turn_speed ){
			end_flag = 1 ;
		}
		if ( tmp_angle > turn_speed ) tmp_angle = turn_speed ;
		if ( tmp_angle < -turn_speed ) tmp_angle = -turn_speed ;
		work->move_cursor_rot.vy += tmp_angle ;
	}
	work->move_cursor_rot.vy = GTE_REGULAR_ANGLE( work->move_cursor_rot.vy );
	return ( end_flag );
}
/* ---------------------------------------------------------------- */
/* 移動アクション関連を初期化する */
void PDRAY_ResetMoveAction( Work *work )
{
	PDRAY_InitProcWork( work, &work->move_work, move_new_func_list );
}
/* 次の移動アクションを設定 */
int PDRAY_SetNextMoveAction( Work *work, int next_move_action )
{
#if 0
	/* 既に別のものが設定されていた場合は失敗 */
	if ( work->move_action != work->move_next_action ) return ( -1 );
	/* 既に同じものが設定されている場合は何もしない */
	if ( work->move_action == next_move_action ) return ( 0 );
	/* 途中に別の移動アクションが必要な場合のチェック */
	if ( work->move_action == MOVE_IDLE && next_move_action != MOVE_WALK_READY ) next_move_action = MOVE_WALK_READY ;
	if ( work->move_action != MOVE_WALK_READY && next_move_action == MOVE_IDLE ) next_move_action = MOVE_WALK_READY ;
#endif
	/* 次の移動アクションを設定する */
	PDRAY_SetNextProc( &work->move_work, next_move_action, 0 );
	/* 同じアクションの場合がほとんどなので強制的に初期化が行われるように細工を行う */
	work->move_work.mode = -1 ;
	work->move_work.next_mode = next_move_action ;
	work->move_work.die_func = NULL ;
	return ( 0 );
}

/* ---------------------------------------------------------------- */

/* 新方式自由移動ルーチン */
static int Move_Free_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	t, play_time ;
	int		end_flag, exec_flag ;
	FMATRIX	cursor_mat ;

	ctrl = &work->control ;
	body = &work->object ;

	end_flag = 0 ;	/* デフォルトで移動未完状態 */
	exec_flag = 1 ;	/* デフォルトで実行許可 */
	if ( proc_work->phase == -1 ) exec_flag = 0 ;

	/* まだモーション再生が行えてない場合には実行を行わない */
	if ( proc_work->phase == -1 ){
		exec_flag = 0 ;
		work->move_cursor_mov = ctrl->mov ;
		work->move_cursor_rot = ctrl->rot ;
	}
	/* キャンセルフラグが立っている場合には実行権をなくして移動完了フラグを立てる */
	/* （＝現在のカーソル位置で立ち止まる） */
	if ( work->move_cancel_flag ){
		exec_flag = 0 ;
		end_flag = 1 ;
	}

	if ( exec_flag ){
		if ( work->move_type == 0 ){
			//end_flag = MoveCursor( work, MOVE_SPEED, TURN_SPEED );
			end_flag = MoveCursor( work, work->move_speed, work->turn_speed );
		} else {
			//end_flag = MoveCursor2( work, MOVE_SPEED, TURN_SPEED );
			end_flag = MoveCursor2( work, work->move_speed, work->turn_speed );
		}
	}

	/* カーソル位置でのマトリクス生成 */
	DG_SetPos2( &work->move_cursor_mov, &work->move_cursor_rot );
	DG_GetPos( &cursor_mat );

#if 0
	{/* デバッグ用カーソル位置での中心、両足位置表示 */
		FVECTOR	tmp_vec ;
		tmp_vec = work->move_cursor_mov ;
		tmp_vec.vy = PLAYER_LEVEL ;
		AN_Test_Eye2( &tmp_vec, 3 );
		GTE_LoadMatrix( &cursor_mat );
		GTE_PutVector1( &tmp_vec, &work->ik_base_right_leg_offset );
		tmp_vec.vy = PLAYER_LEVEL ;
		AN_Test_Eye2( &tmp_vec, 3 );
		GTE_PutVector1( &tmp_vec, &work->ik_base_left_leg_offset );
		tmp_vec.vy = PLAYER_LEVEL ;
		AN_Test_Eye2( &tmp_vec, 3 );
	}
#endif

	/* 現在のモーション再生時間を取得 */
	play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;

	switch ( proc_work->phase ){
	  case -1:
		/* モーション発行待ち */
		//if ( PDRAY_SetMotion( work, MOTION_WALK_READY, 0 ) == -1 ) break ;
		//if ( PDRAY_SetMotion( work, MOTION_IDLE2WALK, 0 ) == -1 ) break ;
		if ( PDRAY_SetMotion( work, work->motion_idle2walk, 0 ) == -1 ) break ;
		/* 初期化処理 */
		if ( work->last_move_leg == 0 ){
			proc_work->phase = 16 ;
		} else {
			proc_work->phase = 0 ;
		}
		/* 移動カーソル初期化 */
		ctrl = &work->control ;
		body = &work->object ;
		work->move_cursor_mov = ctrl->mov ;
		work->move_cursor_rot = ctrl->rot ;
		work->r_leg_angle_to = ctrl->rot.vy ;
		work->l_leg_angle_to = ctrl->rot.vy ;
		work->r_leg_angle_from = ctrl->rot.vy ;
		work->l_leg_angle_from = ctrl->rot.vy ;
#if 0
		{/* 一歩歩く分の時間分だけ移動用カーソルを予め移動させておく */
			float	scale ;
			scale = (float)WALK_PHASE3_TIME / TIME_BASE / 2 ;
			if ( work->move_type == 0 ){
				end_flag = MoveCursor( work, MOVE_SPEED * scale, TURN_SPEED * scale );
			} else {
				end_flag = MoveCursor2( work, MOVE_SPEED * scale, TURN_SPEED * scale );
			}
		}
#endif
		break ;

		/* 右側モーション再生 */
	  case 0:
		/* モーション発行 */
		//if ( PDRAY_SetMotion( work, MOTION_WALK, 0 ) == -1 ) break ;
		if ( PDRAY_SetMotion( work, work->motion_walk, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		work->chaff_disable = 1 ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		work->last_move_leg = 0 ;	/* 右足を動かした */

		/* 右足駆動開始 */
		{/* 現在のカーソル位置における足位置を目標座標に設定する */
			GTE_LoadMatrix( &cursor_mat );
#if 1
			GTE_PutVector1( &work->ik_target_to, &work->ik_base_right_leg_offset );
			if ( ( work->life <= 0 ) && ( end_flag == 0 ) ){
				FVECTOR	tmp_vec ;
				tmp_vec = work->ik_base_right_leg_offset ;
				tmp_vec.vx *= 0.6f ;
				GTE_PutVector1( &work->ik_target_to, &tmp_vec );
			}
#else
			{
				FVECTOR	tmp_vec ;
				float	len ;
				GTE_SubVector( &tmp_vec, &work->move_cursor_mov, &work->move_last_check_mov_l );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				len = DG_MIN( len, 8000.0f );
				tmp_vec = work->ik_base_right_leg_offset ;
				tmp_vec.vx *= 1.0 - len / 16000.0f ;
				GTE_PutVector1( &work->ik_target_to, &tmp_vec );
			}
#endif

			work->ik_target = work->right_leg_target_pos ;
			work->ik_target_from = work->right_leg_target_pos ;
			GTE_ZeroVector( &work->ik_target_from_speed );
			GTE_ZeroVector( &work->ik_target_to_speed );

			/* 終了チェック用に記録する */
			work->move_last_check_mov_r = work->move_cursor_mov ;
			work->move_last_check_rot_r = work->move_cursor_rot ;
		}
		{
			work->r_leg_angle_to = work->move_cursor_rot.vy ;
			work->r_leg_angle_diff = GTE_REGULAR_ANGLE( work->r_leg_angle_to - work->r_leg_angle_from );
		}
		if ( end_flag ) proc_work->count++ ;

	  case 1:/* 右足下ろす */
		ACTION_MARK("Move1-move-r");
		//t = ( play_time - WALK_PHASE0_TIME ) / ( WALK_PHASE2_TIME - WALK_PHASE0_TIME ) ;
		t = ( play_time - 0 ) / ( work->walk_phase2_time - 0 ) ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );
		{/* 腰方向計算 */
			int		r_leg_angle, l_leg_angle ;
			r_leg_angle = work->r_leg_angle_from + DG_FTOI( t * work->r_leg_angle_diff ) ;
			l_leg_angle = work->l_leg_angle_from ;
			ctrl->rot.vy = GTE_REGULAR_ANGLE( l_leg_angle + GTE_REGULAR_ANGLE( r_leg_angle - l_leg_angle ) / 2 ) ;
			ctrl->turn.vy = ctrl->rot.vy ;
		}
		/* フェーズ移行チェック */
		//if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], WALK_PHASE2_TIME ) ){
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], work->walk_phase2_time ) ){
			work->chaff_disable = 0 ;
			/* ＩＫ制御点更新 */
			work->ik_target = work->ik_target_to ;
			/* 足角度更新 */
			work->r_leg_angle_from = work->r_leg_angle_to ;
			/* 次フェーズの設定 */
			proc_work->phase = 16 ;
			/* 移動終了チェック */
#if 0
			if ( proc_work->count >= 2 ){
				proc_work->phase = 32 ;
			}
#else
			if ( ( work->move_last_check_mov_r.vx == work->move_last_check_mov_l.vx ) &&
				( work->move_last_check_mov_r.vz == work->move_last_check_mov_l.vz ) &&
				( work->move_last_check_rot_r.vy == work->move_last_check_rot_l.vy ) ){
				proc_work->phase = 32 ;
			}
#endif

			{/* ＳＥ */
				int se_code[4] = {SD_E_R_FTFR01,SD_E_R_FTFR02,SD_E_R_FTFR03,SD_E_R_FTNR01};
				int	se_type ;
				se_type = work->no % 3 ;
				if ( work->on_stage_flag ) se_type = 3 ;
				GM_SeSetMode( se_code[ se_type ], (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}

			/* 着地エフェクト */
			if ( work->on_stage_flag ){
				extern void *NewFogWave( FMATRIX *world, int size, int speed, int height );
				FMATRIX		mat ;
				GTE_UnitMatrix( &mat );
				mat.m[3][0] = body->objs->objs[ PDRAY_JOINT_RIGHT_TOE ].world.m[3][0] ;
				mat.m[3][2] = body->objs->objs[ PDRAY_JOINT_RIGHT_TOE ].world.m[3][2] ;
				mat.m[3][1] = PLAYER_LEVEL ;
				NewFogWave( &mat, 300, 150, 300 );
			}
		}
		break ;

		/* 左側モーション再生 */
	  case 16:
		/* モーション発行 */
		//if ( PDRAY_SetMotion( work, MOTION_WALK, MOTION_FLAG_BODY_REVERS|MOTION_FLAG_LEG_REVERS ) == -1 ) break ;
		if ( PDRAY_SetMotion( work, work->motion_walk, MOTION_FLAG_BODY_REVERS|MOTION_FLAG_LEG_REVERS ) == -1 ) break ;
		proc_work->phase++ ;
		work->chaff_disable = 1 ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		work->last_move_leg = 1 ;	/* 左足を動かした */

		/* 左足駆動開始 */
		{
			GTE_LoadMatrix( &cursor_mat );
#if 1
			GTE_PutVector1( &work->ik_target_to, &work->ik_base_left_leg_offset );
			if ( ( work->life <= 0 ) && ( end_flag == 0 ) ){
				FVECTOR	tmp_vec ;
				tmp_vec = work->ik_base_left_leg_offset ;
				tmp_vec.vx *= 0.6f ;
				GTE_PutVector1( &work->ik_target_to, &tmp_vec );
			}
#else
			{
				FVECTOR	tmp_vec ;
				float	len ;
				GTE_SubVector( &tmp_vec, &work->move_cursor_mov, &work->move_last_check_mov_r );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				len = DG_MIN( len, 8000.0f );
				tmp_vec = work->ik_base_left_leg_offset ;
				tmp_vec.vx *= 1.0 - len / 16000.0f ;
				GTE_PutVector1( &work->ik_target_to, &tmp_vec );
			}
#endif

			work->ik_target = work->left_leg_target_pos ;
			work->ik_target_from = work->left_leg_target_pos ;
			GTE_ZeroVector( &work->ik_target_from_speed );
			GTE_ZeroVector( &work->ik_target_to_speed );

			/* 終了チェック用に記録する */
			work->move_last_check_mov_l = work->move_cursor_mov ;
			work->move_last_check_rot_l = work->move_cursor_rot ;
		}
		{
			work->l_leg_angle_to = work->move_cursor_rot.vy ;
			work->l_leg_angle_diff = GTE_REGULAR_ANGLE( work->l_leg_angle_to - work->l_leg_angle_from );
		}
		if ( end_flag ) proc_work->count++ ;

	  case 17:/* 左足下ろす */
		ACTION_MARK("Move1-move-l");
		//t = ( play_time - WALK_PHASE0_TIME ) / ( WALK_PHASE2_TIME - WALK_PHASE0_TIME ) ;
		t = ( play_time - 0 ) / ( work->walk_phase2_time - 0 ) ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_CONTROL );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );
		{/* 腰方向計算 */
			int		r_leg_angle, l_leg_angle ;
			l_leg_angle = work->l_leg_angle_from + DG_FTOI( t * work->l_leg_angle_diff ) ;
			r_leg_angle = work->r_leg_angle_from ;
			ctrl->rot.vy = GTE_REGULAR_ANGLE( l_leg_angle + GTE_REGULAR_ANGLE( r_leg_angle - l_leg_angle ) / 2 ) ;
			ctrl->turn.vy = ctrl->rot.vy ;
		}
		/* フェーズ移行チェック */
		//if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], WALK_PHASE2_TIME ) ){
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], work->walk_phase2_time ) ){
			work->chaff_disable = 0 ;
			/* ＩＫ制御点更新 */
			work->ik_target = work->ik_target_to ;
			/* 足角度更新 */
			work->l_leg_angle_from = work->l_leg_angle_to ;
			/* 次フェーズの設定 */
			proc_work->phase = 0 ;
			/* 移動終了チェック */
#if 0
			if ( proc_work->count >= 2 ){
				proc_work->phase = 32 ;
			}
#else
			if ( ( work->move_last_check_mov_r.vx == work->move_last_check_mov_l.vx ) &&
				( work->move_last_check_mov_r.vz == work->move_last_check_mov_l.vz ) &&
				( work->move_last_check_rot_r.vy == work->move_last_check_rot_l.vy ) ){
				proc_work->phase = 32 ;
			}
#endif

			{/* ＳＥ */
				int se_code[4] = {SD_E_R_FTFL01,SD_E_R_FTFL02,SD_E_R_FTFL03,SD_E_R_FTNL01};
				int	se_type ;
				se_type = work->no % 3 ;
				if ( work->on_stage_flag ) se_type = 3 ;
				GM_SeSetMode( se_code[ se_type ], (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}

			/* 着地エフェクト */
			if ( work->on_stage_flag ){
				extern void *NewFogWave( FMATRIX *world, int size, int speed, int height );
				FMATRIX		mat ;
				GTE_UnitMatrix( &mat );
				mat.m[3][0] = body->objs->objs[ PDRAY_JOINT_LEFT_TOE ].world.m[3][0] ;
				mat.m[3][2] = body->objs->objs[ PDRAY_JOINT_LEFT_TOE ].world.m[3][2] ;
				mat.m[3][1] = PLAYER_LEVEL ;
				NewFogWave( &mat, 300, 150, 300 );
			}
		}
		break ;

	  case 32:
		//if ( PDRAY_SetMotion( work, MOTION_WALK_READY, 0 ) == -1 ) break ;
		//if ( PDRAY_SetMotion( work, MOTION_WALK2IDLE, 0 ) == -1 ) break ;
		if ( PDRAY_SetMotion( work, work->motion_walk2idle, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		//PDRAY_SetNextMoveAction( work, MOVE_WALK_READY );
		work->move_work.mode = MOVE_IDLE ;
		break ;
	  case 33:
		//if ( PDRAY_SetMotion( work, MOTION_WALK_READY, 0 ) == -1 ) break ;
		if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) == -1 ) break ;
		work->move_work.mode = MOVE_IDLE ;
		return ( -1 );

	}
	return ( 0 );
}

static int Move_Free_Die( Work *work, PROC_WORK *proc_work )
{
	if ( proc_work->phase < 33 ) return ( -1 );
	return ( 0 );
}

static int Move_Free_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Move_Free_Act, Move_Free_Die );

	proc_work->phase = -1 ;
	/* 終了チェック位置の初期化 */
	work->move_last_check_mov_r.vx = 10000000000.0f ;
	work->move_last_check_mov_r.vz = 10000000000.0f ;
	work->move_last_check_mov_l.vx = -10000000000.0f ;
	work->move_last_check_mov_l.vz = -10000000000.0f ;
	/* ダメージによるモーション変化 */
	if ( work->life > 0 ){
		work->move_speed = MOVE_SPEED ;
		work->turn_speed = TURN_SPEED ;
		work->motion_idle2walk = MOTION_IDLE2WALK ;
		work->motion_walk = MOTION_WALK ;
		work->motion_walk2idle = MOTION_WALK2IDLE ;
		work->walk_phase2_time = WALK_PHASE2_TIME ;
	} else {
		work->move_speed = MOVE_SPEED2 ;
		work->turn_speed = TURN_SPEED2 ;
		work->motion_idle2walk = MOTION_IDLE2WALK ;
		work->motion_walk = MOTION_BREAK_WALK ;
		work->motion_walk2idle = MOTION_WALK2IDLE ;
		work->walk_phase2_time = WALK2_PHASE2_TIME ;
	}

	return ( 0 );
}

/* ---------------------------------------------------------------- */
/* 新方式自由移動ルーチン */
static int Move_Free2_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	t, play_time ;
	int		end_flag = 0 ;
	FMATRIX	cursor_mat ;

	ctrl = &work->control ;
	body = &work->object ;

	/* まだ歩きがスタートしていなければカーソル位置を初期化する */
	if ( work->current_motion_num[ 0 ] == MOTION_BREAK_WALK && work->move_cancel_flag == 0 ){
	{/* 移動用カーソルの移動処理 */
		FVECTOR		tmp_vec ;
		float		len ;
		int			angle, tmp_angle ;
		GTE_SubVector( &tmp_vec, &work->move_target, &work->move_cursor_mov );
		tmp_vec.vy = 0.0f ;
		len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		if ( len > MOVE_SPEED ){
			/* 目標座標に向かって移動する処理 */
			GTE_ScaleVector( &tmp_vec, &tmp_vec, MOVE_SPEED2 / len );
			GTE_AddVector( &work->move_cursor_mov, &work->move_cursor_mov, &tmp_vec );
			/* 角度を目標座標に向ける */
			angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
			angle = GTE_REGULAR_ANGLE( angle );
			tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
			if ( tmp_angle > -1024 && tmp_angle < 1024 ){
				if ( tmp_angle > TURN_SPEED2 ) tmp_angle = TURN_SPEED2 ;
				if ( tmp_angle < -TURN_SPEED2 ) tmp_angle = -TURN_SPEED2 ;
				work->move_cursor_rot.vy += tmp_angle ;
			} else {
				tmp_angle = GTE_REGULAR_ANGLE( tmp_angle - 2048 );
				if ( tmp_angle > TURN_SPEED2 ) tmp_angle = TURN_SPEED2 ;
				if ( tmp_angle < -TURN_SPEED2 ) tmp_angle = -TURN_SPEED2 ;
				work->move_cursor_rot.vy += tmp_angle ;
			}
		} else {
			GTE_AddVector( &work->move_cursor_mov, &work->move_cursor_mov, &tmp_vec );
			//end_flag = 1 ;
			/* 角度を目標座標に向ける */
			GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_cursor_mov );
			angle = DG_FTOI( 2048.0f * atan2f( tmp_vec.vx, tmp_vec.vz ) / GTE_PI );
			angle = GTE_REGULAR_ANGLE( angle );
			tmp_angle = GTE_REGULAR_ANGLE( angle - work->move_cursor_rot.vy );
			if ( tmp_angle > TURN_SPEED2 ) tmp_angle = TURN_SPEED2 ;
			if ( tmp_angle < -TURN_SPEED2 ) tmp_angle = -TURN_SPEED2 ;
			work->move_cursor_rot.vy += tmp_angle ;
			if ( tmp_angle == 0 ){
				end_flag = 1 ;
			}
		}
		work->move_cursor_rot.vy = GTE_REGULAR_ANGLE( work->move_cursor_rot.vy );
	}
	}
	if ( work->move_cancel_flag ) end_flag = 1 ;

	/* カーソル位置でのマトリクス生成 */
	DG_SetPos2( &work->move_cursor_mov, &work->move_cursor_rot );
	DG_GetPos( &cursor_mat );

#if 0
	{
		FVECTOR	tmp_vec ;
		tmp_vec = work->move_cursor_mov ;
		tmp_vec.vy = PLAYER_LEVEL ;
		AN_Test_Eye2( &tmp_vec, 3 );
		GTE_LoadMatrix( &cursor_mat );
		GTE_PutVector1( &tmp_vec, &work->ik_base_right_leg_offset );
		tmp_vec.vy = PLAYER_LEVEL ;
		AN_Test_Eye2( &tmp_vec, 3 );
		GTE_PutVector1( &tmp_vec, &work->ik_base_left_leg_offset );
		tmp_vec.vy = PLAYER_LEVEL ;
		AN_Test_Eye2( &tmp_vec, 3 );
	}
#endif

	/* 現在のモーション再生時間を取得 */
	play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;

	switch ( proc_work->phase ){
		/* 右側モーション再生 */
	  case 0:
		/* モーション発行 */
		if ( PDRAY_SetMotion( work, MOTION_BREAK_WALK, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		work->chaff_disable = 1 ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		work->last_move_leg = 0 ;	/* 右足を動かした */

		/* 右足駆動開始 */
		{/* 現在のカーソル位置における足位置を目標座標に設定する */
			GTE_LoadMatrix( &cursor_mat );
			GTE_PutVector1( &work->ik_target_to, &work->ik_base_right_leg_offset );

			work->ik_target = work->right_leg_target_pos ;
			work->ik_target_from = work->right_leg_target_pos ;
			GTE_ZeroVector( &work->ik_target_from_speed );
			GTE_ZeroVector( &work->ik_target_to_speed );
		}
		{
			work->r_leg_angle_to = work->move_cursor_rot.vy ;
			work->r_leg_angle_diff = GTE_REGULAR_ANGLE( work->r_leg_angle_to - work->r_leg_angle_from );
		}
		if ( end_flag ) proc_work->count++ ;

	  case 1:/* 右足下ろす */
		ACTION_MARK("Move1-move-r");
		t = ( play_time - WALK2_PHASE0_TIME ) / ( WALK2_PHASE2_TIME - WALK2_PHASE0_TIME ) ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );
		{/* 腰方向計算 */
			int		r_leg_angle, l_leg_angle ;
			r_leg_angle = work->r_leg_angle_from + DG_FTOI( t * work->r_leg_angle_diff ) ;
			l_leg_angle = work->l_leg_angle_from ;
			ctrl->rot.vy = GTE_REGULAR_ANGLE( l_leg_angle + GTE_REGULAR_ANGLE( r_leg_angle - l_leg_angle ) / 2 ) ;
			ctrl->turn.vy = ctrl->rot.vy ;
		}
		/* フェーズ移行チェック */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], WALK2_PHASE2_TIME ) ){
			work->chaff_disable = 0 ;
			/* ＩＫ制御点更新 */
			work->ik_target = work->ik_target_to ;
			/* 足角度更新 */
			work->r_leg_angle_from = work->r_leg_angle_to ;
			/* 次フェーズの設定 */
			proc_work->phase = 16 ;
			/* 移動終了チェック */
			if ( proc_work->count >= 2 ){
				proc_work->phase = 32 ;
			}

			{/* ＳＥ */
				int se_code[4] = {SD_E_R_FTFR01,SD_E_R_FTFR02,SD_E_R_FTFR03,SD_E_R_FTNR01};
				int	se_type ;
				se_type = work->no % 3 ;
				if ( work->on_stage_flag ) se_type = 3 ;
				GM_SeSetMode( se_code[ se_type ], (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}
		break ;

		/* 左側モーション再生 */
	  case 16:
		/* モーション発行 */
		if ( PDRAY_SetMotion( work, MOTION_BREAK_WALK, MOTION_FLAG_LEG_REVERS ) == -1 ) break ;
		proc_work->phase++ ;
		work->chaff_disable = 1 ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		work->last_move_leg = 1 ;	/* 左足を動かした */

		/* 左足駆動開始 */
		{
			GTE_LoadMatrix( &cursor_mat );
			GTE_PutVector1( &work->ik_target_to, &work->ik_base_left_leg_offset );

			work->ik_target = work->left_leg_target_pos ;
			work->ik_target_from = work->left_leg_target_pos ;
			GTE_ZeroVector( &work->ik_target_from_speed );
			GTE_ZeroVector( &work->ik_target_to_speed );
		}
		{
			work->l_leg_angle_to = work->move_cursor_rot.vy ;
			work->l_leg_angle_diff = GTE_REGULAR_ANGLE( work->l_leg_angle_to - work->l_leg_angle_from );
		}
		if ( end_flag ) proc_work->count++ ;

	  case 17:/* 左足下ろす */
		ACTION_MARK("Move1-move-l");
		t = ( play_time - WALK2_PHASE0_TIME ) / ( WALK2_PHASE2_TIME - WALK2_PHASE0_TIME ) ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_CONTROL );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );
		{/* 腰方向計算 */
			int		r_leg_angle, l_leg_angle ;
			l_leg_angle = work->l_leg_angle_from + DG_FTOI( t * work->l_leg_angle_diff ) ;
			r_leg_angle = work->r_leg_angle_from ;
			ctrl->rot.vy = GTE_REGULAR_ANGLE( l_leg_angle + GTE_REGULAR_ANGLE( r_leg_angle - l_leg_angle ) / 2 ) ;
			ctrl->turn.vy = ctrl->rot.vy ;
		}
		/* フェーズ移行チェック */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], WALK2_PHASE2_TIME ) ){
			work->chaff_disable = 0 ;
			/* ＩＫ制御点更新 */
			work->ik_target = work->ik_target_to ;
			/* 足角度更新 */
			work->l_leg_angle_from = work->l_leg_angle_to ;
			/* 次フェーズの設定 */
			proc_work->phase = 0 ;
			/* 移動終了チェック */
			if ( proc_work->count >= 2 ){
				proc_work->phase = 32 ;
			}

			{/* ＳＥ */
				int se_code[4] = {SD_E_R_FTFL01,SD_E_R_FTFL02,SD_E_R_FTFL03,SD_E_R_FTNL01};
				int	se_type ;
				se_type = work->no % 3 ;
				if ( work->on_stage_flag ) se_type = 3 ;
				GM_SeSetMode( se_code[ se_type ], (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}
		break ;

	  case 32:
		if ( PDRAY_SetMotion( work, MOTION_WALK_READY, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		PDRAY_SetNextMoveAction( work, MOVE_WALK_READY );
		work->move_work.mode = MOVE_WALK_READY ;
		break ;
	  case 33:
		return ( -1 );
	}

#if 0
	/* アクションの変更が行われたとき */
	if ( work->move_cancel_flag ){
		if ( proc_work->count < 2 ){
			/* 足の動きの停止を確認して他の移動アクションへ移行 */
			work->move_cursor_mov = ctrl->mov ;
			work->move_cursor_rot = ctrl->rot ;
		}
	}
#endif
	return ( 0 );
}

static int Move_Free2_Die( Work *work, PROC_WORK *proc_work )
{
	if ( proc_work->phase != 33 ){
		return ( -1 );
	}
	return ( 0 );
}

static int Move_Free2_Init( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	PROC_SET_CALLBACK( proc_work, Move_Free2_Act, Move_Free2_Die );

	/* 初期化処理 */
	if ( work->last_move_leg == 0 ){
		proc_work->phase = 16 ;
	} else {
		proc_work->phase = 0 ;
	}
	/* 移動カーソル初期化 */
	ctrl = &work->control ;
	body = &work->object ;
	work->move_cursor_mov = ctrl->mov ;
	work->move_cursor_rot = ctrl->rot ;
	work->r_leg_angle_to = ctrl->rot.vy ;
	work->l_leg_angle_to = ctrl->rot.vy ;
	work->r_leg_angle_from = ctrl->rot.vy ;
	work->l_leg_angle_from = ctrl->rot.vy ;

	return ( 0 );
}

/* ---------------------------------------------------------------- */
