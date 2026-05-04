//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_thk2.c
	量産型ＲＡＹ思考処理ルーチン

	2001/05/06 K.Takabe
	$Id: pdr_thk2.c,v 1.1.1.3 2002/11/19 11:51:26 Yoshizawa1 Exp $

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
#include "r_common.h"

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		各種定数群
	*/
#define BACK_RANGE		(13000)		/* 後退する距離 */
/* ---------------------------------------------------------------- */
//static int	range_data_list[4][4] = {
//	{3500,12000,-800,800},
//	{12000,18000,-450,450},
//	{18000,27000,-300,300},
//	{20000,40000,-120,120},
//};
#if 0
static int	range_data_list[4][4] = {
	{3500,12000,-800,800},
	{12000,18000,-500,500},
	{18000,27000,-500,500},
	{27000,60000,-250,250},
};
#endif
/* ---------------------------------------------------------------- */
/* ２点間の角度、距離、差分ベクトルを求める */
void PDRAY_GetPositionInfo2D( FVECTOR *_diff, int *_len, int *_angle, FVECTOR *from, FVECTOR *to, int rot_base )
{
	FVECTOR		tmp_vec ;
	float		lenlen, len, a ;
	int			angle ;

	GTE_SubVector( &tmp_vec, to, from );
	tmp_vec.vy = 0.0f ;
	lenlen = GTE_InnerProduct( &tmp_vec, &tmp_vec );
	len = DG_SQRT( lenlen );
	a = atan2f( tmp_vec.vx, tmp_vec.vz );
	angle = DG_FTOI( a * 2048 / (float)M_PI ) ;
	angle -= rot_base ;
	angle = REGULAR_ANGLE( angle );
	//angle &= 4095 ;
	//if ( angle & 2048 ) angle -= 4096 ;

	if ( _diff != NULL ) *_diff = tmp_vec ;
	if ( _len != NULL ) *_len = len ;
	if ( _angle != NULL ) *_angle = angle ;
}
#if 0
/* 円弧にて領域のチェックを行う */
static int CheckAreaArc( float len, int angle, float min_len, float max_len, int min_angle, int max_angle )
{
	if ( len < min_len ) return ( 0 );
	if ( len > max_len ) return ( 0 );
	if ( angle < min_angle ) return ( 0 );
	if ( angle > max_angle ) return ( 0 );
	return ( 1 );
}
#endif
/* ---------------------------------------------------------------- */
/* プレイヤー検索処理＆攻撃方法決定 */
static int Think_CancelCheck( Work *work )
{
	int			check_flag = 0 ;

	check_flag |= work->think_area_check[AREA_KICK] ;
	check_flag |= work->think_area_check[AREA_CUTTER] ;
	check_flag |= work->think_area_check[AREA_VALCAN] ;
	check_flag |= work->think_area_check[AREA_MISSILE] ;
	/* 攻撃範囲外にプレイヤーがいるので移動処理をキャンセルする */
	if ( check_flag ){
		return ( 1 );
	}
	return ( 0 );
}

/* ---------------------------------------------------------------- */
static int Think_Null_Init( Work *work, PROC_WORK *proc_work );
static int Think_JumpIn_Init( Work *work, PROC_WORK *proc_work );
static int Think_JumpOut_Init( Work *work, PROC_WORK *proc_work );
static int Think_Seek_Init( Work *work, PROC_WORK *proc_work );
static int Think_Pursue_Init( Work *work, PROC_WORK *proc_work );
static int Think_Attack_Init( Work *work, PROC_WORK *proc_work );
static int Think_Damage_Init( Work *work, PROC_WORK *proc_work );
static int Think_SA_StandBy_Init( Work *work, PROC_WORK *proc_work );
static int Think_SA_Execute_Init( Work *work, PROC_WORK *proc_work );
static int Think_OA_StandBy_Init( Work *work, PROC_WORK *proc_work );
static int Think_OA_Execute_Init( Work *work, PROC_WORK *proc_work );
static int Think_OA_Move_Init( Work *work, PROC_WORK *proc_work );
PROC_CALLBACK think2_new_func_list[] = {
	Think_Null_Init,		/* THINK2_NULL */
	Think_JumpIn_Init,		/* THINK2_JUMPIN */
	Think_JumpOut_Init,		/* THINK2_JUMPOUT */
	Think_Seek_Init,		/* THINK2_SEEK */
	Think_Pursue_Init,		/* THINK2_PURSUE */
	Think_Attack_Init,		/* THINK2_ATTACK */
	Think_Damage_Init,		/* THINK2_DAMAGE */
	Think_SA_StandBy_Init,	/* THINK2_SA_STANDBY */
	Think_SA_Execute_Init,	/* THINK2_SA_EXEC */
	Think_OA_StandBy_Init,	/* THINK2_OA_STANDBY */
	Think_OA_Execute_Init,	/* THINK2_OA_EXEC */
	Think_OA_Move_Init,		/* THINK2_OA_MOVE */
};
/* ---------------------------------------------------------------- */
/* 次の移動アクションを設定 */
int PDRAY_SetNextThink2( Work *work, int next_think2_mode )
{
	PDRAY_SetNextProc( &work->think2_work, next_think2_mode, 0 );
	return ( 0 );
}
void PDRAY_InitThink2( Work *work )
{
	PDRAY_InitProcWork( work, &work->think2_work, think2_new_func_list );
}

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static int Think_Null_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "Null" );
	return ( 0 );
}
static int Think_Null_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Null_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Null_Act, Think_Null_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜ステージ内乱入処理＞ */
static int Think_JumpIn_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "JumpIn" );

	switch ( proc_work->phase ){
	  case 0:
		{/* カメラ切り替えまでのウェイト処理 */
			int		diff ;
			diff = RAYSERVER_SetGameSignal( GAME_SIGNAL_GET_CAM_DIFF_ANGLE, 0 );
			if ( diff > -200 && diff < 200 ){
				proc_work->phase++ ;
				/* 初期アクション設定 */
				PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP1 );
				//PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR|RAY_STATUS_JUMP1 );
			}
		}
		break ;
	  case 1:
		if ( !( work->demand_action_bit & RAY_STATUS_JUMP1 ) ){
			/* ジャンプが終了したら攻撃フェーズへ */
			PDRAY_SetNextThink2( work, THINK2_SEEK );
			RAYSERVER_SetGameSignal( GAME_SIGNAL_START_IN_ATTACK, 0 );
			work->on_stage_flag = 1 ;
		}
		break ;
	}

	return ( 0 );
}
static int Think_JumpIn_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_JumpIn_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_JumpIn_Act, Think_JumpIn_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* カメラのロックを有効にする */
	RAYSERVER_SetGameSignal( GAME_SIGNAL_LOCKON_ENABLE, 0 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜ステージ外退場処理＞ */
static int Think_JumpOut_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "JumpOut" );

	switch ( proc_work->phase ){
	  case 0:/* ジャンプ先を向く */
		//PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE3 );
		//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
		if ( !( work->demand_action_bit & RAY_STATUS_MOVE3 ) ){
			//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
			PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2 );
			if ( work->life != 0 ){
				PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE1 );
			}
			proc_work->phase++ ;
		}
		break ;
	  case 1:/* ジャンプで降りる */
		if ( !( work->demand_action_bit & RAY_STATUS_JUMP2 ) ){
			//PDRAY_SetNextThink1( work, THINK1_STANDBY );
			PDRAY_SetNextProc( &work->think1_work, THINK1_STANDBY, 0 );
			RAYSERVER_SetGameSignal( GAME_SIGNAL_END_IN_ATTACK, 0 );
			work->on_stage_flag = 0 ;
		}
		break ;
	}

	return ( 0 );
}
static int Think_JumpOut_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_JumpOut_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_JumpOut_Act, Think_JumpOut_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* 初期アクション設定 */
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
	PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE3 );
	/* カメラのロックを解除する */
	RAYSERVER_SetGameSignal( GAME_SIGNAL_LOCKON_DISABLE, 0 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜検索処理＞ */
static int Think_Seek_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "Seek" );

#if 0
	/* ダメージチェック */
	if ( work->damaged_flag ){
		PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
		//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
		return ( 0 );
	}
#else
	/* ダメージチェック */
	if ( work->damaged_flag ){
		PDRAY_SetNextThink2( work, THINK2_PURSUE );
		return ( 0 );
	}
#endif

	/* 転ばした直後は何もしない */
	if ( work->player_tumble_wait_time ) return ( 0 );

	{
		CONTROL		*ctrl ;
		FVECTOR		tmp_vec, z_dir ;
		float		len ;
		int			angle ;

		ctrl = &work->control ;
		/* 前方向ベクトルをオブジェクトのマトリクスから取得 */
		z_dir = *(FVECTOR*)work->world.m[2] ;

		len = work->think_player_length ;
		angle = work->think_player_angle ;
		/* オブジェクトのマトリクスを使用してＺ軸に対する距離をチェック */
		//z_len = GTE_InnerProduct( &dir_vec, &z_dir );

		/* 直接攻撃が出来る場合 */
		if ( work->think_area_check[ AREA_KICK ] && work->disable_attack4_count == 0 ){
			/* キック攻撃 */
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK4 );
			PDRAY_SetNextThink2( work, THINK2_ATTACK );
			return ( 0 );
		} else if ( work->think_area_check[ AREA_CUTTER ] && work->disable_attack1_count == 0 ){
			/* 水圧カッター攻撃 */
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK1 );
			PDRAY_SetNextThink2( work, THINK2_ATTACK );
			return ( 0 );
		} else if ( work->think_area_check[ AREA_VALCAN ] && work->disable_attack2_count == 0 ){
			/* バルカン攻撃 */
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK2 );
			PDRAY_SetNextThink2( work, THINK2_ATTACK );
			return ( 0 );
		} else if ( work->think_area_check[ AREA_MISSILE ] && work->disable_attack3_count == 0 ){
			/* ミサイル攻撃 */
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );
			PDRAY_SetNextThink2( work, THINK2_ATTACK );
			return ( 0 );
		}
#if 0
		/* ターンだけすれば攻撃可能な場合 */
		if ( CheckAreaArc( len, angle, 3500, 400000, -1280, 1280 ) ){
			work->stage_stand_pos = work->control.mov ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE2 );
			PDRAY_SetNextThink2( work, THINK2_PURSUE );
//printf("%f %f %f\n", work->stage_stand_pos.vx, work->stage_stand_pos.vy, work->stage_stand_pos.vz );
			return ( 0 );
		}
		/* 後退すればいい場合 */
		GTE_ScaleVector( &tmp_vec, &z_dir, -BACK_RANGE );
		GTE_AddVector( &tmp_vec, &ctrl->mov, &tmp_vec );
		PDRAY_GetPositionInfo2D( &dir_vec, &ilen, &angle, &DG_ZeroVector, &tmp_vec, 0 );
		if ( ilen < 9000 ){
			work->stage_stand_pos = tmp_vec ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE2 );
			PDRAY_SetNextThink2( work, THINK2_PURSUE );
//printf("%f %f %f\n", work->stage_stand_pos.vx, work->stage_stand_pos.vy, work->stage_stand_pos.vz );
			return ( 0 ) ;
		}
		/* 前進及び旋回が必要な場合 */
		GTE_SubVector( &tmp_vec, &DG_ZeroVector, &GM_PlayerPosition );
		len = DG_RSQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, len );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, BACK_RANGE );	/* 原点からプレイヤーの逆方向に移動 */
		work->stage_stand_pos = tmp_vec ;
		PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE2 );
		PDRAY_SetNextThink2( work, THINK2_PURSUE );
//printf("%f %f %f\n", work->stage_stand_pos.vx, work->stage_stand_pos.vy, work->stage_stand_pos.vz );
		return ( 0 );
#else
		if ( len > 4000.0f ){/* ターンだけすれば攻撃可能な場合 */
			FVECTOR	x_vec, z_vec, a_vec ;
			int		dir ;
			x_vec.vx = 0 - ctrl->mov.vx ;
			z_vec.vx = 0 - ctrl->mov.vz ;
			x_vec.vy = GM_PlayerPosition.vx - ctrl->mov.vx ;
			z_vec.vy = GM_PlayerPosition.vz - ctrl->mov.vz ;
			MT_Atan2X4( &a_vec, &x_vec, &z_vec );
			dir = GTE_REGULAR_ANGLE( GTE_RAD2PS( a_vec.vx ) - GTE_RAD2PS( a_vec.vy ) ) ;
			if ( dir > -700 && dir < 700 ){
				//work->stage_stand_pos = work->control.mov ;
				PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE2 );
				PDRAY_SetNextThink2( work, THINK2_PURSUE );
				return ( 0 );
			}
		}
		/* 前進及び旋回が必要な場合 */
		GTE_SubVector( &tmp_vec, &DG_ZeroVector, &GM_PlayerPosition );
		tmp_vec.vy = 0 ;
		len = DG_RSQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, len * BACK_RANGE );/* 原点からプレイヤーの逆方向に移動 */
		work->stage_stand_pos = tmp_vec ;
		PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE2 );
		PDRAY_SetNextThink2( work, THINK2_PURSUE );
		return ( 0 );
#endif
		/* 特殊な移動が必要な場合 */
	}

	return ( 0 );
}
static int Think_Seek_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Seek_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Seek_Act, Think_Seek_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜追跡処理＞ */
static int Think_Pursue_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "Pursue" );

	/* プレイヤーを攻撃可能範囲に捕らえたかチェック */
	work->enter_attack_area_flag = Think_CancelCheck( work );
#if 0
	/* ダメージチェック */
	if ( work->damaged_flag ){
//		PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
		//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
//		return ( 0 );
	}
#else
	/* ダメージチェック */
	//if ( work->damaged_flag ){
	//	//PDRAY_SetNextThink2( work, THINK2_PURSUE );
	//	return ( 0 );
	//}
	if ( work->demand_action_bit & (RAY_STATUS_DAMAGE|RAY_STATUS_DAMAGE2|RAY_STATUS_DAMAGE3) ) return ( 0 );
#endif
	/* 移動終了チェック */
	if ( !( work->demand_action_bit & (RAY_STATUS_MOVE2|RAY_STATUS_STEP) ) ){
		PDRAY_SetNextThink2( work, THINK2_SEEK );
		return ( 0 );
	}

	return ( 0 );
}
static int Think_Pursue_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Pursue_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Pursue_Act, Think_Pursue_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜攻撃処理＞ */
static int Think_Attack_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "Attack" );

#if 0
	/* ダメージチェック */
	if ( work->damaged_flag ){
		PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
		//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
		return ( 0 );
	}
#else
	/* ダメージチェック */
	if ( work->damaged_flag ){
		PDRAY_SetNextThink2( work, THINK2_PURSUE );
 		//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
		return ( 0 );
	}
#endif
	/* 攻撃終了チェック */
	if ( !( work->demand_action_bit & RAY_STATUS_ATTACK ) ){
		PDRAY_SetNextThink2( work, THINK2_SEEK );
		return ( 0 );
	}

	return ( 0 );
}
static int Think_Attack_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Attack_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Attack_Act, Think_Attack_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜ダメージ処理＞ */
static int Think_Damage_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "Damage" );
	return ( 0 );
}
static int Think_Damage_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Damage_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Damage_Act, Think_Damage_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜サポート攻撃待機＞ */
static int Think_SA_StandBy_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "SAStandBy" );

	//if ( GV_PadData[1].press & PAD_A ) PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );

	//if ( !( work->demand_action_bit & RAY_STATUS_SATTACK2 ) ){
	//	/* ジャンプが終了したら攻撃フェーズへ */
	//	PDRAY_SetNextThink2( work, THINK2_SA_EXEC );
	//	work->think2_count = 0 ;
	//}

	return ( 0 );
}
static int Think_SA_StandBy_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_SA_StandBy_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_SA_StandBy_Act, Think_SA_StandBy_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* 初期アクション設定 */
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_SATTACK2 );
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜サポート攻撃実行＞ */
static int Think_SA_Execute_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "SAttack" );

	if ( !( work->demand_action_bit & RAY_STATUS_JUMP1 ) ){
		/* ジャンプが終了したら攻撃フェーズへ */
		PDRAY_SetNextThink2( work, THINK2_SEEK );
		work->on_stage_flag = 1 ;
		/* カメラのロックを有効にする */
		RAYSERVER_SetGameSignal( GAME_SIGNAL_LOCKON_ENABLE, 0 );
	}

	return ( 0 );
}
static int Think_SA_Execute_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_SA_Execute_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_SA_Execute_Act, Think_SA_Execute_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* 初期アクション設定 */
	PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP1 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜場外攻撃待機＞ */
static int Think_OA_StandBy_Act( Work *work, PROC_WORK *proc_work )
{
	int		command ;

	SET_MARK( proc_work, "OAStandBy" );

	//if ( GV_PadData[1].press & PAD_A ) PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );

	//if ( !( work->demand_action_bit & RAY_STATUS_SATTACK2 ) ){
	//	/* ジャンプが終了したら攻撃フェーズへ */
	//	PDRAY_SetNextThink2( work, THINK2_OA_EXEC );
	//	work->think2_count = 0 ;
	//}
#if 0
	if ( GV_PadData[1].press & PAD_A ){/* 攻撃許可が下りれば攻撃方法を決定して実行へ */
		PDRAY_SetNextThink2( work, THINK2_OA_EXEC );
	}
#endif
	//command = RAYSERVER_GetRayCommand( work );
	command = work->info.command ;
	if ( command & RAY_COMMAND_ATTACK1 ){
		PDRAY_SetNextThink2( work, THINK2_OA_EXEC );
	}

	/* 場合によっては移動もいいかもね */

	return ( 0 );
}
static int Think_OA_StandBy_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_OA_StandBy_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_OA_StandBy_Act, Think_OA_StandBy_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* 初期アクション設定 */
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_SATTACK2 );
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜場外攻撃実行＞ */
static int Think_OA_Execute_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "OAExec" );

	proc_work->count += TIME_BASE ;
	switch ( proc_work->phase ){
	  case 0:
		{/* カメラ切り替えまでのウェイト処理 */
			int		diff ;
			diff = RAYSERVER_SetGameSignal( GAME_SIGNAL_GET_CAM_DIFF_ANGLE, 0 );
			//if ( diff > -200 && diff < 200 ){
#if 1
#if 0
				proc_work->phase++ ;
				/* 初期アクション設定 */
				if ( work->think_area_check[ AREA_CUTTER ] && work->disable_attack1_count == 0 ){
					/* 水圧カッター攻撃 */
					PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK1 );
				} else if ( work->think_area_check[ AREA_VALCAN ] && work->disable_attack2_count == 0 ){
					/* バルカン攻撃 */
					PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK2 );
				} else if ( work->think_area_check[ AREA_MISSILE ] && work->disable_attack3_count == 0 ){
					/* ミサイル攻撃 */
					PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );
				}
#else
				if ( ( work->demand_action_bit & RAY_STATUS_ATTACK ) == 0 ){
					switch ( work->attack_type_cycle ){
					  case 0:
						if ( work->disable_attack1_count ) work->attack_type_cycle-- ;/* タイプ変更を行わない */
						if ( work->disable_attack1_count ){
							PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
							break ;
						}
						if ( work->think_area_check[ AREA_CUTTER ] && work->disable_attack1_count == 0 ){
							/* 水圧カッター攻撃 */
							if ( work->n_water_tank <= 0 ){
								PDRAY_SetDemandActionBit( work, RAY_STATUS_DRINK );
							} else {
								PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK1 );
							}
							proc_work->phase++ ;
						}
						break ;
					  case 1:
						if ( work->disable_attack2_count ) work->attack_type_cycle-- ;/* タイプ変更を行わない */
						if ( work->disable_attack2_count ){
							PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
							break ;
						}
						if ( work->think_area_check[ AREA_VALCAN ] && work->disable_attack2_count == 0 ){
						/* 水圧カッター攻撃 */
							PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK2 );
							proc_work->phase++ ;
						}
						break ;
					  case 2:
						if ( work->disable_attack3_count ) work->attack_type_cycle-- ;/* タイプ変更を行わない */
						if ( work->disable_attack3_count ){
							PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
							break ;
						}
						if ( work->think_area_check[ AREA_MISSILE ] && work->disable_attack3_count == 0 ){
						/* 水圧カッター攻撃 */
							PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );
							proc_work->phase++ ;
						}
						break ;
					  case 3:
						break ;
					}
					if ( ++work->attack_type_cycle > 3 ) work->attack_type_cycle = 0 ;
					//proc_work->phase++ ;
				}
#endif
#else
				{/* 実験 */
					int		command ;
					//command = RAYSERVER_GetRayCommand( work );
					command = work->info.command ;
					if ( command & RAY_COMMAND_DEBUG_A3 ){
						PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK3 );
					} else if ( command & RAY_COMMAND_DEBUG_A2 ){
						PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK2 );
					} else if ( command & RAY_COMMAND_DEBUG_A1 ){
						PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK1 );
					}
				}
#endif
				//PDRAY_SetDemandActionBit( work, RAY_STATUS_ATTACK2 );
				//PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR|RAY_STATUS_ATTACK1 );
			//}
		}
		break ;
	  case 1:
#if 0
#if 0
		if ( !( work->demand_action_bit & RAY_STATUS_ATTACK ) ){
			/* 攻撃が終了したら攻撃フェーズへ */
			PDRAY_SetNextThink2( work, THINK2_OA_STANDBY );
			PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE1 );
		}
#endif
#if 0
		if ( !( work->demand_action_bit & RAY_STATUS_MOVE1 ) ){
			/* 攻撃が終了したら攻撃フェーズへ */
			PDRAY_SetNextThink2( work, THINK2_OA_STANDBY );
		}
#endif
		if ( proc_work->count > ( 300 * 4 ) ){
			/* 攻撃が終了したら攻撃フェーズへ */
			PDRAY_SetNextThink2( work, THINK2_OA_STANDBY );
		}
#endif
		break ;
	}

	{/* 攻撃終了判定 */
		int		command ;
		//command = RAYSERVER_GetRayCommand( work );
		command = work->info.command ;
		if ( !( command & RAY_COMMAND_ATTACK1 ) ){
			PDRAY_SetNextThink2( work, THINK2_OA_STANDBY );
		}
	}

	/* ダメージチェック */
	if ( work->damaged_flag ){
		//PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
		//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP2|RAY_STATUS_MOVE1 );
		return ( 0 );
	}

	return ( 0 );
}
static int Think_OA_Execute_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_OA_Execute_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_OA_Execute_Act, Think_OA_Execute_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* カメラのロックを有効にする */
	//RAYSERVER_SetGameSignal( GAME_SIGNAL_LOCKON_ENABLE, 0 );
	/* 初期アクション設定 */
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP1 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜場外攻撃時移動＞ */
static int Think_OA_Move_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "OAMove" );

	if ( !( work->demand_action_bit & RAY_STATUS_JUMP1 ) ){
		/* ジャンプが終了したら攻撃フェーズへ */
		PDRAY_SetNextThink2( work, THINK2_SEEK );
		work->on_stage_flag = 1 ;
		/* カメラのロックを有効にする */
		//RAYSERVER_SetGameSignal( GAME_SIGNAL_LOCKON_ENABLE, 0 );
	}

	return ( 0 );
}
static int Think_OA_Move_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_OA_Move_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_OA_Move_Act, Think_OA_Move_Die );
	/* 第３思考モードの初期化 */
	PDRAY_SetNextThink3( work, THINK3_NULL );
	/* 初期アクション設定 */
	//PDRAY_SetDemandActionBit( work, RAY_STATUS_JUMP1 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
