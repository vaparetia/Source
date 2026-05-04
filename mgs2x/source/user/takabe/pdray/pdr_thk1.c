//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_thk1.c
	量産型ＲＡＹ思考処理ルーチン

	2001/05/06 K.Takabe
	$Id: pdr_thk1.c,v 1.1.1.3 2002/11/19 11:51:25 Yoshizawa1 Exp $

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
static int Think_None_Init( Work *work, PROC_WORK *proc_work );
static int Think_StandBy_Init( Work *work, PROC_WORK *proc_work );
static int Think_InAttack_Init( Work *work, PROC_WORK *proc_work );
static int Think_OutAttack_Init( Work *work, PROC_WORK *proc_work );
static int Think_SupportAttack_Init( Work *work, PROC_WORK *proc_work );
static int Think_Break_Init( Work *work, PROC_WORK *proc_work );
PROC_CALLBACK	think1_new_func_list[] = {
	NULL,						/* THINK1_NULL */
	Think_None_Init,			/* THINK1_NONE */
	Think_StandBy_Init,			/* THINK1_STANDBY */
	Think_InAttack_Init,		/* THINK1_INATTACK */
	Think_OutAttack_Init,		/* THINK1_OUTATTACK */
	Think_SupportAttack_Init,	/* THINK1_CONBINEATTACK */
	Think_Break_Init,			/* THINK1_BREAK */
};
/* ---------------------------------------------------------------- */
void PDRAY_InitThink1Work( Work *work )
{
	PDRAY_InitProcWork( work, &work->think1_work, think1_new_func_list );
	PDRAY_SetNextProc( &work->think1_work, THINK1_NONE, 0 );
}
/* ---------------------------------------------------------------- */
	/*
		ダミー思考＜ゲームスタート待ち＞
	*/
static int Think_None_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "None" );

	/* ゲーム状態更新 */
	work->game_condition = GAME_CONDITION_WAIT ;

	if ( work->info.command & RAY_COMMAND_ACTIVE ){
		PDRAY_SetNextProc( proc_work, THINK1_STANDBY, 0 );
#if 0
		if ( !( work->info.command & RAY_COMMAND_QUICKSTART ) ){
			PDRAY_SetDemandActionBit( work, 
									 RAY_STATUS_STARTRUN|RAY_STATUS_JUMP3|
									 RAY_STATUS_MOVE1|RAY_STATUS_ROAR|RAY_STATUS_GUARD );
		} else {
			PDRAY_SetDemandActionBit( work,  RAY_STATUS_TELEPORT|RAY_STATUS_ROAR|RAY_STATUS_GUARD );
			work->action_work.phase = 3 ;
		}
#else
		if ( work->no >= 3 ){
			PDRAY_SetDemandActionBit( work, 
									 RAY_STATUS_STARTRUN|RAY_STATUS_JUMP3|
									 RAY_STATUS_MOVE1|RAY_STATUS_ROAR|RAY_STATUS_GUARD );
		} else {
			//PDRAY_SetDemandActionBit( work,  RAY_STATUS_TELEPORT|RAY_STATUS_ROAR|RAY_STATUS_GUARD );
			PDRAY_SetDemandActionBit( work,  RAY_STATUS_TELEPORT|RAY_STATUS_GUARD );
			work->action_work.phase = 3 ;
		}
#endif
		/* 表示や当たり判定などを全て有効に設定 */
		work->active_flag = 1 ;
		work->number_model_handle = RAYSERVER_GetNumberModel( work->object.objs );
	}

	return ( 0 );
}
static int Think_None_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_None_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_None_Act, Think_None_Die );
	/* 第２思考モードの初期化 */
	PDRAY_SetNextThink2( work, THINK2_NULL );
	//PDRAY_SetNextProc( &work->think2_work, THINK2_NULL, 0 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜待機＞ */
static int Think_StandBy_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "STANDBY" );

	/* 破壊撤退判定処理 */
	if ( work->life == 0 ){
		/* ゲーム状態更新 */
		work->game_condition = GAME_CONDITION_ESCAPE ;
		PDRAY_SetNextProc( proc_work, THINK1_BREAK, 0 );
		return (0);
	}

	if ( work->demand_action_bit & RAY_STATUS_MOVE1 ){
		/* ゲーム状態更新（移動処理が終わるまで待機状態にしておく） */
		work->game_condition = GAME_CONDITION_WAIT ;
	} else {
		/* ゲーム状態更新 */
		work->game_condition = GAME_CONDITION_STANDBY ;
	}

	{
		int		command ;
		command = work->info.command ;
		if ( !( work->true_action_bit & RAY_STATUS_MOVE1 ) ){
			if ( command & RAY_COMMAND_ATTACK1 ){
				PDRAY_SetNextProc( proc_work, THINK1_OUTATTACK, 0 );
			}
		}
		if ( command & RAY_COMMAND_ATTACK2 ){
			if ( !( work->demand_action_bit & RAY_STATUS_MOVE1 ) ){
				PDRAY_SetNextProc( proc_work, THINK1_INATTACK, 0 );
				/* ステージ内攻撃に移る際に他のアクションを一度消す */
				PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|
										   /*RAY_STATUS_MOVE1|*//* これはやばいかも */
										   RAY_STATUS_ROAR|RAY_STATUS_DRINK|RAY_STATUS_STEP);
			} else {
				PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|
										   /*RAY_STATUS_MOVE1|*//* これはやばいかも */
										   RAY_STATUS_ROAR|RAY_STATUS_DRINK|RAY_STATUS_STEP);
				/* 移動をキャンセルして速やかにステージ内攻撃フェーズに移行させる */
				/* MOVE1は普通にはキャンセルさせることが出来ないので専用の停止フラグを使用する */
				//printf("move stop!!\n");
				if ( work->true_action_bit & RAY_STATUS_MOVE1 ){
					work->move1_stop_flag = 1 ;/* action_move1内でクリアされる */
				}
				return ( 0 );
			}
		}
		if ( command & RAY_COMMAND_ADJUST ){
			PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE1 );
			work->idle_time = 0 ;
		}
		if ( command & RAY_COMMAND_ROAR ){
			printf("set ray roar!\n");
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
			work->idle_time = 0 ;
		}
		//if ( work->on_stage_flag ){
		//	if ( RAYSERVER_CommonStatus & CSTATUS_DAMAGE2 ){
		//		PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
		//	}
		//} else {
			if ( RAYSERVER_CommonStatus & CSTATUS_DAMAGE1 ){
				PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
				work->idle_time = 0 ;
			}
		//}
		if ( work->idle_time > IDLE_MAX_TIME ){
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR|RAY_STATUS_DRINK );
			work->idle_time = 0 ;
		}
	}

	if ( work->true_action_bit & RAY_STATUS_GUARD ){
		work->idle_time += TIME_BASE ;
	} else {
		work->idle_time = 0 ;
	}

	return ( 0 );
}
static int Think_StandBy_Die( Work *work, PROC_WORK *proc_work )
{
	work->idle_time = 0 ;
	return ( 0 );
}
static int Think_StandBy_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_StandBy_Act, Think_StandBy_Die );
	/* 第２思考モードの初期化 */
	PDRAY_SetNextThink2( work, THINK2_NULL );
	//PDRAY_SetNextProc( &work->think2_work, THINK2_NULL, 0 );
	work->idle_time = 0 ;
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜ステージ内攻撃＞ */
static int Think_InAttack_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "IN_ATTACK" );

	/* 破壊撤退判定処理 */
	if ( work->life <= 0 ){
		/* ゲーム状態更新 */
		work->game_condition = GAME_CONDITION_ESCAPE ;
		//PDRAY_SetNextProc( proc_work, THINK1_STANDBY, 0 );
		/* 攻撃系のステータスは消しておく */
		PDRAY_ResetDemandActionBit( work,
								   RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|RAY_STATUS_ATTACK4);
		PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
		return (0);
	}

	/* ゲーム状態更新 */
	work->game_condition = GAME_CONDITION_INATTACK ;

	{
		int		command ;
		command = work->info.command ;
		if ( !( command & RAY_COMMAND_ATTACK2 ) ){
			//PDRAY_SetNextProc( proc_work, THINK1_STANDBY, 0 );
			PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
		}
	}

	//proc_work->count += TIME_BASE ;
	//if ( proc_work->count > INATTACK_TIME ){
	//	PDRAY_SetNextThink2( work, THINK2_JUMPOUT );
	//}

	return ( 0 );
}
static int Think_InAttack_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_InAttack_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_InAttack_Act, Think_InAttack_Die );
	/* 第２思考モードの初期化 */
	PDRAY_SetNextThink2( work, THINK2_JUMPIN );
	//PDRAY_SetNextProc( &work->think2_work, THINK2_JUMPIN, 0 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 思考処理＜ステージ外攻撃＞ */
static int Think_OutAttack_Act( Work *work, PROC_WORK *proc_work )
{

	SET_MARK( proc_work, "OUT_ATTACK" );

	/* 破壊撤退判定処理 */
	if ( work->life == 0 ){
		/* ゲーム状態更新 */
		work->game_condition = GAME_CONDITION_ESCAPE ;
		PDRAY_SetNextProc( proc_work, THINK1_BREAK, 0 );
		return ( 0 );
	}

	/* ゲーム状態更新 */
	work->game_condition = GAME_CONDITION_OUTATTACK ;

	{
		int		command ;
		//command = RAYSERVER_GetRayCommand( work );
		command = work->info.command ;
		if ( !( command & RAY_COMMAND_ATTACK1 ) ){
			PDRAY_SetNextProc( proc_work, THINK1_STANDBY, 0 );
		}
	}

	return ( 0 );
}
static int Think_OutAttack_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_OutAttack_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_OutAttack_Act, Think_OutAttack_Die );
	/* 第２思考モードの初期化 */
	//PDRAY_SetNextThink2( work, THINK2_OA_STANDBY );
	PDRAY_SetNextThink2( work, THINK2_OA_EXEC );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
static int Think_SupportAttack_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "Support" );

	/* ゲーム状態更新 */
	work->game_condition = GAME_CONDITION_SUPPORTATTACK ;

	if ( !( work->info.command & RAY_COMMAND_ATTACK3 ) ){
		PDRAY_SetNextProc( proc_work, THINK1_STANDBY, 0 );
	}

	return ( 0 );
}
static int Think_SupportAttack_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_SupportAttack_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_SupportAttack_Act, Think_SupportAttack_Die );
	/* 第２思考モードの初期化 */
	PDRAY_SetNextThink2( work, THINK2_NULL );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
static int Think_Break_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "BREAK" );

	if ( !( work->demand_action_bit & RAY_STATUS_MOVE4 ) ){
		PDRAY_SetDemandActionBit( work, RAY_STATUS_BREAK );
		/* ゲーム状態更新 */
		work->game_condition = GAME_CONDITION_BREAK ;
	} else {
		/* ゲーム状態更新 */
		//work->game_condition = GAME_CONDITION_ESCAPE ;
		work->game_condition = GAME_CONDITION_BREAK ;
	}

	return ( 0 );
}
static int Think_Break_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Break_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Break_Act, Think_Break_Die );
	/* 第２思考モードの初期化 */
	PDRAY_SetNextThink2( work, THINK2_NULL );
	/* アクション設定 */
	PDRAY_SetDemandActionBit( work, RAY_STATUS_MOVE4|RAY_STATUS_BREAK );
	RAYSERVER_FreeNumberModel( work->number_model_handle );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
#if 0
/* ---------------------------------------------------------------- */
static int Think_Break_Act( Work *work, PROC_WORK *proc_work )
{
	SET_MARK( proc_work, "BREAK" );
	return ( 0 );
}
static int Think_Break_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Think_Break_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Think_Break_Act, Think_Break_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
#endif
