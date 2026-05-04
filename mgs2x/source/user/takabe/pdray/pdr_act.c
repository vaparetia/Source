//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_act.c
	量産型ＲＡＹアクション処理ルーチン

	2001/04/09 K.Takabe
	$Id: pdr_act.c,v 1.1.1.3 2002/11/19 11:51:23 Yoshizawa1 Exp $

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

extern void *NewPDRayWeaponMissile( FMATRIX *pos, int type );

extern void *NewRAYBreathSmoke( FMATRIX *parent );
#define SIGNAL_RAYBREATHSMOKE			(0x78fe7700)	/* GV_StrCode("NewRAYBreathSmoke")<<8 */
#define SIGNAL_RAYBREATHSMOKE_STOP		(SIGNAL_RAYBREATHSMOKE|0x01)

/* ---------------------------------------------------------------- */
/* これらのステータスが立っていた場合には攻撃フェーズを終了する */
#define ATTACK_CANCEL_STATUS (RAY_STATUS_JUMP1|RAY_STATUS_JUMP2|RAY_STATUS_JUMP3|\
RAY_STATUS_STARTRUN|RAY_STATUS_MOVE1|RAY_STATUS_MOVE2|RAY_STATUS_MOVE3/*|RAY_STATUS_ROAR*/)
/* ダメージを受けた場合にはこれらのステータスをクリアする */
#define DAMAGE_RESET_STATUS	(\
/*RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3|RAY_STATUS_ATTACK4|*/\
RAY_STATUS_MOVE1|\
RAY_STATUS_MOVE2\
)

/* ---------------------------------------------------------------- */
static int Action_Sleep_Init( Work *work, PROC_WORK *proc_work );
static int Action_Idle_Init( Work *work, PROC_WORK *proc_work );
static int Action_Move1_Init( Work *work, PROC_WORK *proc_work );
static int Action_Move2_Init( Work *work, PROC_WORK *proc_work );
static int Action_Move3_Init( Work *work, PROC_WORK *proc_work );
static int Action_Move4_Init( Work *work, PROC_WORK *proc_work );
static int Action_Attack1_Init( Work *work, PROC_WORK *proc_work );
static int Action_Attack2_Init( Work *work, PROC_WORK *proc_work );
static int Action_Attack3_Init( Work *work, PROC_WORK *proc_work );
static int Action_Attack4_Init( Work *work, PROC_WORK *proc_work );
static int Action_Roar_Init( Work *work, PROC_WORK *proc_work );
static int Action_Damage_Init( Work *work, PROC_WORK *proc_work );
static int Action_Damage2_Init( Work *work, PROC_WORK *proc_work );
static int Action_Damage3_Init( Work *work, PROC_WORK *proc_work );
static int Action_Guard_Init( Work *work, PROC_WORK *proc_work );
static int Action_Confusion_Init( Work *work, PROC_WORK *proc_work );
static int Action_Jump1_Init( Work *work, PROC_WORK *proc_work );
static int Action_Jump2_Init( Work *work, PROC_WORK *proc_work );
static int Action_Break_Init( Work *work, PROC_WORK *proc_work );
static int Action_SAttack2_Init( Work *work, PROC_WORK *proc_work );
static int Action_Run_Init( Work *work, PROC_WORK *proc_work );
static int Action_Jump3_Init( Work *work, PROC_WORK *proc_work );
static int Action_Step_Init( Work *work, PROC_WORK *proc_work );
static int Action_Drink_Init( Work *work, PROC_WORK *proc_work );
static int Action_Teleport_Init( Work *work, PROC_WORK *proc_work );
PROC_CALLBACK	action_new_func_list[] = {
	Action_Sleep_Init,			/* ACTION_SLEEP */
	Action_Idle_Init,			/* ACTION_IDLE */
	Action_Move1_Init,			/* ACTION_MOVE1 */
	Action_Move2_Init,			/* ACTION_MOVE2 */
	Action_Move3_Init,			/* ACTION_MOVE3 */
	Action_Move4_Init,			/* ACTION_MOVE4 */
	Action_Attack1_Init,		/* ACTION_ATTACK1 */
	Action_Attack2_Init,		/* ACTION_ATTACK2 */
	Action_Attack3_Init,		/* ACTION_ATTACK3 */
	Action_Attack4_Init,		/* ACTION_ATTACK4 */
	Action_Roar_Init,			/* ACTION_ROAR */
	Action_Damage_Init,			/* ACTION_DAMAGE */
	Action_Jump1_Init,			/* ACTION_JAMP1 */
	Action_Jump2_Init,			/* ACTION_JAMP2 */
	Action_Sleep_Init,			/* ACTION_CHARGE1 */
	Action_Guard_Init,			/* ACTION_GUARD */
	Action_Confusion_Init,		/* ACTION_CONFUSION */
	Action_Sleep_Init,			/* ACTION_SATTACK1 */
	Action_SAttack2_Init,		/* ACTION_SATTACK2 */
	Action_Sleep_Init,			/* ACTION_SATTACK3 */
	Action_Sleep_Init,			/* ACTION_SATTACK4 */
	Action_Break_Init,			/* ACTION_BREAK */
	Action_Run_Init,			/* ACTION_RUN */
	Action_Jump3_Init,			/* ACTION_JUMP3 */
	Action_Damage2_Init,		/* ACTION_DAMAGE2 */
	Action_Damage3_Init,		/* ACTION_DAMAGE3 */
	Action_Step_Init,			/* ACTION_STEP */
	Action_Drink_Init,			/* ACTION_DRINK */
	Action_Teleport_Init,		/* ACTION_TELEPORT */
};

/* ---------------------------------------------------------------- */
struct _action_select_data{
	int		action_bit ;
	int		action_id ;
};
static struct _action_select_data	action_select_list[] = {
	{ RAY_STATUS_TELEPORT, ACTION_TELEPORT },
	{ RAY_STATUS_DAMAGE, ACTION_DAMAGE },
	{ RAY_STATUS_DAMAGE2, ACTION_DAMAGE2 },
	{ RAY_STATUS_DAMAGE3, ACTION_DAMAGE3 },
	{ RAY_STATUS_STEP, ACTION_STEP },
	{ RAY_STATUS_ATTACK1, ACTION_ATTACK1 },
	{ RAY_STATUS_ATTACK2, ACTION_ATTACK2 },
	{ RAY_STATUS_ATTACK3, ACTION_ATTACK3 },
	{ RAY_STATUS_ATTACK4, ACTION_ATTACK4 },
	{ RAY_STATUS_JUMP1, ACTION_JUMP1 },
	{ RAY_STATUS_JUMP2, ACTION_JUMP2 },
	{ RAY_STATUS_STARTRUN, ACTION_RUN },
	{ RAY_STATUS_JUMP3, ACTION_JUMP3 },
	//{ RAY_STATUS_CONFUSION, ACTION_CONFUSION },
	{ RAY_STATUS_MOVE1, ACTION_MOVE1 },
	{ RAY_STATUS_MOVE2, ACTION_MOVE2 },
	{ RAY_STATUS_MOVE3, ACTION_MOVE3 },
	{ RAY_STATUS_MOVE4, ACTION_MOVE4 },
	{ RAY_STATUS_DRINK, ACTION_DRINK },
	{ RAY_STATUS_ROAR, ACTION_ROAR },
	{ RAY_STATUS_SATTACK2, ACTION_SATTACK2 },
	{ RAY_STATUS_BREAK, ACTION_BREAK },
	{ RAY_STATUS_GUARD, ACTION_GUARD },
	{ RAY_STATUS_IDLE, ACTION_IDLE },
	{ -1, -1 },
};
/* ---------------------------------------------------------------- */
/* 要求アクション内から次のアクションの１つを選ぶ */
int PDRAY_SelectAction( Work *work )
{
	struct _action_select_data	*select ;
	int		i ;

	for ( i = 0, select = action_select_list ; ; i++, select++ ){
		if ( select->action_bit == -1 ) break ;
		if ( work->demand_action_bit & select->action_bit ){
			work->action_work.next_mode = work->action_work.mode ;	/* 常に最新の要求ビットを優先させる */
			if ( PDRAY_SetNextAction( work, select->action_id ) == 0 ){
				work->select_action_bit = select->action_bit ;
				return ( 0 );
			}
			return ( -1 );
		}
	}
	return ( 0 );
}
/* 移動アクション関連を初期化する */
void PDRAY_ResetAction( Work *work )
{
	PDRAY_InitProcWork( work, &work->action_work, action_new_func_list );
}
/* 次の移動アクションを設定 */
int PDRAY_SetNextAction( Work *work, int next_action )
{
	PDRAY_SetNextProc( &work->action_work, next_action, 0 );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* 停止中 */
static int Action_Sleep_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Sleep" );

	work->true_action = ACTION_SLEEP ;
	work->true_action_bit = RAY_STATUS_NONE ;

	switch ( proc_work->phase ){
	  case 0:
		PDRAY_SetMotion( work, MOTION_STANDBY, MOTION_FLAG_FORCE );
		break ;
	  case 1:
		if ( PDRAY_SetMotion( work, MOTION_STANDBY2IDLE, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		break ;
	  case 2:
		if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		break ;
	}

	return ( 0 );
}
static int Action_Sleep_Die( Work *work, PROC_WORK *proc_work )
{
	/* 切り替えには特にチェックはなし */
	if ( proc_work->phase == 0 ) proc_work->phase = 1 ;
	if ( proc_work->phase != 3 ) return ( -1 );
	return ( 0 );
}
static int Action_Sleep_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Sleep_Act, Action_Sleep_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 行動待機アクション */
static int Action_Idle_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Idle" );

	if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) != -1 ){
		work->true_action = ACTION_IDLE ;
		work->true_action_bit = RAY_STATUS_IDLE ;
	}

	return ( 0 );
}
static int Action_Idle_Die( Work *work, PROC_WORK *proc_work )
{
	/* 切り替えには特にチェックはなし */
	return ( 0 );
}
static int Action_Idle_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Idle_Act, Action_Idle_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* プレイヤーを正面に捕らえるように移動 */
static int Action_Move1_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Move1" );

	/* 移動処理キャンセル対応処理 */
	//if ( work->enter_attack_area_flag ){
	//	work->move_cancel_flag = 1 ;
	//	proc_work->phase = 2 ;
	//}

	/* チャフ禁止時＝足を上げている場合に移動先座標を更新する */
	if ( work->chaff_disable && work->move_work.mode == MOVE_FREE /* && work->move_work.count != 2*/ ){
		/* 移動先座標を求める */
		if ( !work->move1_stop_flag ){
			work->move_target = work->info.standby_pos ;
		} else {
			work->move_target = work->move_cursor_mov ;
		}
		/* 向く方向を設定 */
		work->dir_target = DG_ZeroVector ;
		{
			FVECTOR		tmp_vec ;
			float		len ;
			GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_target );
			len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
			GTE_ScaleVector( &tmp_vec, &tmp_vec, 8000.0f / len );
			GTE_AddVector( &work->dir_target, &work->move_target, &tmp_vec );
		}
		{
			FVECTOR		tmp_vec ;
			float		len ;
			GTE_SubVector( &tmp_vec, &work->move_cursor_mov, &work->move_target );
			tmp_vec.vy = 0.0f ;
			len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
			/*if ( len > 5000.0f ) work->move_work.count = 0 ;*/
		}
		//if ( work->info.command & RAY_COMMAND_ADJUST ){
		//	work->move_work.count = 0 ;
		//}
	}

	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetNextMoveAction( work, MOVE_FREE ) == -1 ) break ;
		proc_work->phase++ ;
		//work->true_action = ACTION_MOVE1 ;
		//work->true_action_bit = RAY_STATUS_MOVE1 ;
		break ;
	  case 1:/* 別のフェーズへ */
		if ( work->move_work.phase != -1 ){
			work->true_action = ACTION_MOVE1 ;
			work->true_action_bit = RAY_STATUS_MOVE1 ;
		}
		//if ( work->move_next_action != MOVE_FREE ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE1 );
		}
		break ;
#if 0
	  case 2:/* キャンセル処理 */
		if ( work->move_next_action != MOVE_STEP && work->move_next_action != MOVE_WALK ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE1 );
			work->move_cancel_flag = 0 ;/* キャンセル終了と自然終了が重なる可能性を考慮しておく */
		}
		break ;
#endif
	}
	work->move_cancel_flag = 0 ;

	/* 移動処理 */
	PDRAY_ActProc( work, &work->move_work );

	return ( 0 );
}
static int Action_Move1_Die( Work *work, PROC_WORK *proc_work )
{
	//if ( proc_work->phase >= 1 && proc_work->phase <= 3 ) return ( -1 );
	//if ( work->move_action == MOVE_FREE ) return ( -1 );
	//if ( work->move_work.mode == MOVE_FREE ) return ( -1 );
	/* 強制的にキャンセルされた場合を考慮し、移動カーソル座標を最終移動目的座標として設定 */
	work->move_target = work->move_cursor_mov ;
	/* 移動用Ｐｒｏｃは強制的に終了したことにする */
	//work->move_work.phase = 34 ;
	//work->move_work.mode = MOVE_IDLE ;
	/* 移動２ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE1 );
	return ( 0 );
}
static int Action_Move1_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Move1_Act, Action_Move1_Die );

	/* 移動先座標を求める */
	work->move_target = work->info.standby_pos ;
	/* 向く方向を設定 */
	work->dir_target = DG_ZeroVector ;
	{
		FVECTOR		tmp_vec ;
		float		len ;
		GTE_SubVector( &tmp_vec, &work->dir_target, &work->move_target );
		len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, 8000.0f / len );
		GTE_AddVector( &work->dir_target, &work->move_target, &tmp_vec );
	}
	/* 移動タイプの設定 */
	work->move_type = 1 ;

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* プレイヤーを正面に捕らえるように移動 */
static int Action_Move2_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Move2" );

	/* 移動処理キャンセル対応処理 */
	//if ( work->enter_attack_area_flag ){
	//	work->move_cancel_flag = 1 ;
	//	proc_work->phase = 2 ;
	//}

	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetNextMoveAction( work, MOVE_FREE ) == -1 ) break ;
		proc_work->phase++ ;
		//work->true_action = ACTION_MOVE2 ;
		//work->true_action_bit = RAY_STATUS_MOVE2 ;
		break ;
	  case 1:/* 別のフェーズへ */
		if ( work->move_work.phase != -1 ){
			work->true_action = ACTION_MOVE2 ;
			work->true_action_bit = RAY_STATUS_MOVE2 ;
		}
		//if ( work->move_next_action != MOVE_FREE ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE2 );
		}
		break ;

	  case 2:/* キャンセル処理 */
		//if ( work->move_next_action != MOVE_STEP && work->move_next_action != MOVE_WALK ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE2 );
			work->move_cancel_flag = 0 ;/* キャンセル終了と自然終了が重なる可能性を考慮しておく */
		}
		break ;
	}

	/* 移動処理 */
	PDRAY_ActProc( work, &work->move_work );

	return ( 0 );
}
static int Action_Move2_Die( Work *work, PROC_WORK *proc_work )
{
	//if ( proc_work->phase >= 1 && proc_work->phase <= 3 ) return ( -1 );
	//if ( work->move_action == MOVE_FREE ) return ( -1 );
	//if ( work->move_work.mode == MOVE_FREE ) return ( -1 );
	//if ( !( work->demand_action_bit & (RAY_STATUS_DAMAGE|RAY_STATUS_DAMAGE2|RAY_STATUS_DAMAGE3) ) ){
	//	if ( work->move_work.mode == MOVE_FREE ) return ( -1 );
	//}
	/* 強制的にキャンセルされた場合を考慮し、移動カーソル座標を最終移動目的座標として設定 */
	work->move_target = work->move_cursor_mov ;
	/* 移動用Ｐｒｏｃは強制的に終了したことにする */
	//work->move_work.phase = 34 ;
	//work->move_work.mode = MOVE_IDLE ;
	/* 移動２ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE2 );
	return ( 0 );
}
static int Action_Move2_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Move2_Act, Action_Move2_Die );

	/* 移動先座標を求める */
	work->move_target = work->stage_stand_pos ;
	/* 向く方向を設定 */
	work->dir_target = GM_PlayerPosition ;
	/* 移動タイプの設定 */
	work->move_type = 0 ;

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* ジャンプ前にジャンプ方向を向く */
static int Action_Move3_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Move3" );

	///* 移動処理キャンセル対応処理 */
	//if ( work->enter_attack_area_flag ){
	//	work->move_cancel_flag = 1 ;
	//	proc_work->phase = 2 ;
	//}


	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetNextMoveAction( work, MOVE_FREE ) == -1 ) break ;
		proc_work->phase++ ;
		//work->true_action = ACTION_MOVE3 ;
		//work->true_action_bit = RAY_STATUS_MOVE3 ;
		break ;
	  case 1:/* 別のフェーズへ */
		if ( work->move_work.phase != -1 ){
			work->true_action = ACTION_MOVE3 ;
			work->true_action_bit = RAY_STATUS_MOVE3 ;
		}
		//if ( work->move_next_action != MOVE_FREE ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE3 );
		}
		break ;
	  case 2:/* キャンセル処理 */
		//if ( work->move_next_action != MOVE_FREE ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE3 );
			work->move_cancel_flag = 0 ;/* キャンセル終了と自然終了が重なる可能性を考慮しておく */
		}
		break ;

	  case 6:/* 最初から移動処理を行わない */
		/* 移動１ステータスを消す */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE3 );
		work->move_cancel_flag = 0 ;/* キャンセル終了と自然終了が重なる可能性を考慮しておく */
		break ;
	}

	/* 移動処理 */
	PDRAY_ActProc( work, &work->move_work );

	return ( 0 );
}
static int Action_Move3_Die( Work *work, PROC_WORK *proc_work )
{
	//if ( proc_work->phase >= 1 && proc_work->phase <= 3 ) return ( -1 );
	//if ( work->move_action == MOVE_FREE ) return ( -1 );
	//if ( work->move_work.mode == MOVE_FREE ) return ( -1 );
	/* 強制的にキャンセルされた場合を考慮し、移動カーソル座標を最終移動目的座標として設定 */
	work->move_target = work->move_cursor_mov ;
	/* 移動用Ｐｒｏｃは強制的に終了したことにする */
	//work->move_work.phase = 34 ;
	//work->move_work.mode = MOVE_IDLE ;
	/* 移動２ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE3 );
	return ( 0 );
}
static int Action_Move3_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Move3_Act, Action_Move3_Die );

#if 0
	/* 移動先座標を求める */
	//work->move_target = work->control.mov ;
	work->move_target = work->info.standby_pos ;
	{
		FVECTOR		tmp_vec ;
		float		rlen ;
		tmp_vec = work->move_target ;
		tmp_vec.vy = 0 ;
		rlen = DG_RSQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, rlen * STAGE_OUT_RANGE );
		work->move_target = tmp_vec ;
	}
	/* 向く方向を設定 */
	//RAYSERVER_GetRayMovePosition( work, &work->dir_target );
	work->dir_target = work->info.standby_pos ;
#else
	/* 移動先座標を求める */
	work->move_target = work->control.mov ;
	{
		FVECTOR		tmp_vec ;
		float		rlen ;
		tmp_vec = work->move_target ;
		tmp_vec.vy = 0 ;
		rlen = DG_RSQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		GTE_ScaleVector( &tmp_vec, &tmp_vec, rlen * STAGE_OUT_RANGE );
		work->move_target = tmp_vec ;
	}
	/* 向く方向を設定 */
	GTE_ScaleVector( &work->dir_target, &work->move_target, 2.0f );
#endif

	/* 移動タイプの設定 */
	work->move_type = 0 ;

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* ふらふら歩き撤退 */
static int Action_Move4_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Move4" );

	///* 移動処理キャンセル対応処理 */
	//if ( work->enter_attack_area_flag ){
	//	work->move_cancel_flag = 1 ;
	//	proc_work->phase = 2 ;
	//}

#if 0
	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetNextMoveAction( work, MOVE_FREE2 ) == -1 ) break ;
		proc_work->phase++ ;
		//work->true_action = ACTION_MOVE4 ;
		//work->true_action_bit = RAY_STATUS_MOVE4 ;
		break ;
	  case 1:/* 別のフェーズへ */
		if ( work->move_work.phase != -1 ){
			work->true_action = ACTION_MOVE4 ;
			work->true_action_bit = RAY_STATUS_MOVE4 ;
		}
		//if ( work->move_next_action != MOVE_FREE2 ){
		if ( work->move_work.mode != MOVE_FREE2 ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE4 );
		}
		break ;
	  case 2:/* キャンセル処理 */
		//if ( work->move_next_action != MOVE_FREE2 ){
		if ( work->move_work.mode != MOVE_FREE2 ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE4 );
			work->move_cancel_flag = 0 ;/* キャンセル終了と自然終了が重なる可能性を考慮しておく */
		}
		break ;

	  case 6:/* 最初から移動処理を行わない */
		/* 移動１ステータスを消す */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE4 );
		work->move_cancel_flag = 0 ;/* キャンセル終了と自然終了が重なる可能性を考慮しておく */
		break ;
	}
#else
	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetNextMoveAction( work, MOVE_FREE ) == -1 ) break ;
		proc_work->phase++ ;
		work->true_action = ACTION_MOVE4 ;
		work->true_action_bit = RAY_STATUS_MOVE4 ;
		break ;
	  case 1:/* 別のフェーズへ */
		//if ( work->move_next_action != MOVE_FREE ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE4 );
		}
		break ;
	}
#endif

	/* 移動処理 */
	PDRAY_ActProc( work, &work->move_work );


	return ( 0 );
}
static int Action_Move4_Die( Work *work, PROC_WORK *proc_work )
{
	//if ( proc_work->phase >= 1 && proc_work->phase <= 3 ) return ( -1 );
	//if ( work->move_action == MOVE_FREE ) return ( -1 );
	//if ( work->move_work.mode == MOVE_FREE2 ) return ( -1 );
	/* 強制的にキャンセルされた場合を考慮し、移動カーソル座標を最終移動目的座標として設定 */
	work->move_target = work->move_cursor_mov ;
	/* 移動用Ｐｒｏｃは強制的に終了したことにする */
	//work->move_work.phase = 34 ;
	work->move_work.mode = MOVE_IDLE ;
	/* 移動２ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE4 );
	return ( 0 );
}
static int Action_Move4_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Move4_Act, Action_Move4_Die );
	/* 移動先座標を求める */
	//RAYSERVER_GetRayMovePosition( work, &work->move_target );
	//work->move_target = work->info.standby_pos ;
	work->move_target = work->control.mov ;
	GTE_ScaleVector( &work->move_target, &work->move_target, 3.0f );	/* （仮）立ち位置から３倍離れた距離に設定 */
	/* 向く方向を設定 */
	//work->dir_target = GM_PlayerPosition ;
	GTE_ScaleVector( &work->dir_target, &work->move_target, 2.0f );	/* （仮）立ち位置から３倍離れた距離に設定 */
	/* 移動タイプの設定 */
	work->move_type = 0 ;

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 口から水圧カッター */
static int Action_Attack1_Act( Work *work, PROC_WORK *proc_work )
{
	extern void *NewDummyWaterCutter( FMATRIX *parent );
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	t ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Attack1" );

	switch ( proc_work->phase ){
	  case 0:
		if ( PDRAY_SetMotion( work, MOTION_ATTACK1_START, 0 ) == -1 ) break ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
		work->true_action = ACTION_ATTACK1 ;
		work->true_action_bit = RAY_STATUS_ATTACK1 ;
		proc_work->phase++ ;
		/* 首ＩＫ補間パラメータ設定 */
		work->head_look_start_pos = work->head_look_pos ;
	  case 1:/* 攻撃準備フェーズ */
		//t = (float)( proc_work->count - ATTACK1_PHASE0_TIME ) / ( ATTACK1_PHASE1_TIME - ATTACK1_PHASE0_TIME );
		t = MOTION_PLAY_RATE( body, 0 );

		MT_HermiteLerpVec( &work->head_look_pos, &work->head_look_start_pos, &work->head_look_end_pos,
						  &DG_ZeroVector, &DG_ZeroVector, t );

		/* 腰の方向を補正する */
		work->adjust_target_rot_y = work->think_player_angle ;
		work->attack_angle = work->adjust_target_rot_y ;

		/* ＳＥ */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], 335 ) ){
			GM_SeSetMode( SD_E_LASOPN01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_HEAD].world.m[3], GM_SEMODE_BOMB ) ;
		}

		{/* 攻撃座標の設定 */
			FVECTOR		tmp_vec ;
			float		near_len, far_len ;
			if ( work->on_stage_flag ){
				near_len = ATTACK1_NEAR_LENGTH ;
				far_len = ATTACK1_FAR_LENGTH ;
				if ( RAYSERVER_GameLevel >= 3 ){/* ノーマル以上は直接プレイヤーを狙う */
					near_len = work->think_player_length ;
				}
			} else {
				near_len = work->think_player_length - ( ATTACK1_FAR_LENGTH - ATTACK1_NEAR_LENGTH ) / 2 ;
				if ( near_len < ATTACK1_NEAR_LENGTH ) near_len = ATTACK1_NEAR_LENGTH ;
				far_len = near_len + ( ATTACK1_FAR_LENGTH - ATTACK1_NEAR_LENGTH ) ;
			}
			GTE_ZeroVector( &tmp_vec );
			GTE_LoadMatrix( &body->objs->world );
			tmp_vec.vz = near_len ;
			GTE_PutVector1( &work->attack_start_pos, &tmp_vec );	/* 攻撃開始座標 */
			tmp_vec.vz = far_len ;
			GTE_PutVector1( &work->attack_end_pos, &tmp_vec );		/* 攻撃終了座標 */
			work->attack_start_pos.vy = PLAYER_LEVEL ;				/* 座標をプレイヤーと同じ座標に設定 */
			work->attack_end_pos.vy = PLAYER_LEVEL ;				/* 座標をプレイヤーと同じ座標に設定 */
			GTE_ZeroVector( &work->attack_start_speed );
			GTE_ZeroVector( &work->attack_end_speed );
		}

		/* 頭ＩＫ設定 */
		work->head_look_end_pos = work->attack_start_pos ;
		work->head_look_flag = 1 ;			/* 頭ＩＫを操作するモードに変更 */

		proc_work->count += TIME_BASE ;
		if ( PDRAY_SetMotion( work, MOTION_ATTACK1_LOOP, 0 ) != -1 ){
			proc_work->phase++ ;
			//work->true_action = ACTION_ATTACK1 ;
			//work->true_action_bit = RAY_STATUS_ATTACK1 ;
			/* エフェクト起動 */
			work->attack_effect_work = NewDummyWaterCutter( &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
			work->attack_count = 0 ;
			proc_work->count = 0 ;
			work->n_water_tank -= 1 ;
		}
		break ;
	  case 2:/* 攻撃フェーズ */
		/* モーション設定 */
		t = (float)( proc_work->count - ATTACK1_PHASE1_TIME ) / ( ATTACK1_PHASE2_TIME - ATTACK1_PHASE1_TIME );
		//t = (float)proc_work->count / ATTACK1_PHASE2_TIME ;
		t = DG_MAX( t, 0.0f );
		t = DG_MIN( t, 1.0f );
		//printf("*** %d/%d %f\n", work->action_count, ATTACK1_PHASE2_TIME, t );

		MT_HermiteLerpVec( &work->attack_pos, &work->attack_start_pos, &work->attack_end_pos,
						  &work->attack_start_speed, &work->attack_end_speed, t );
		work->head_look_pos = work->attack_pos ;

		//AN_Test_Eye2( &work->attack_pos, 3 );

		/* 腰の方向を補正する */
		//work->adjust_target_rot_y = work->think_player_angle ;
		work->adjust_target_rot_y = work->attack_angle ;

		/* キャンセルチェック */
		if ( work->demand_action_bit & ( ATTACK_CANCEL_STATUS ) ){
			proc_work->count = ATTACK1_PHASE3_TIME ;
		}
		proc_work->count += TIME_BASE ;
#if 0
		if ( proc_work->count > ATTACK1_PHASE1_TIME ){
			/* エフェクト起動 */
			if ( work->attack_effect_work == NULL ){
				work->attack_effect_work = NewDummyWaterCutter( &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
			}
		}
#endif
		if ( proc_work->count > ATTACK1_PHASE2_TIME ){
			/* エフェクト解除 */
			if ( work->attack_effect_work != NULL ){
				GV_DestroyOtherActor( work->attack_effect_work );
				work->attack_effect_work = NULL ;
			}
		}
		if ( proc_work->count > ATTACK1_PHASE3_TIME ){
			/* エフェクト解除 */
			if ( work->attack_effect_work != NULL ){
				GV_DestroyOtherActor( work->attack_effect_work );
				work->attack_effect_work = NULL ;
			}
			/* 首ＩＫ補間パラメータ設定 */
			work->head_look_start_pos = work->head_look_pos ;
			/* モーション設定 */
			if ( PDRAY_SetMotion( work, MOTION_ATTACK1_END, MOTION_FLAG_FORCE ) == -1 ) break ;
			proc_work->phase++ ;
		}
		break ;
	  case 3:/* 別のフェーズへ */
		t = (float)( proc_work->count - ATTACK1_PHASE2_TIME ) / ( ATTACK1_PHASE3_TIME - ATTACK1_PHASE2_TIME );

		MT_HermiteLerpVec( &work->head_look_pos, &work->head_look_start_pos, &GM_PlayerPosition,
						  &DG_ZeroVector, &DG_ZeroVector, t );

		/* ＳＥ */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], 3 ) ){
			GM_SeSetMode( SD_E_LASCLS01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_HEAD].world.m[3], GM_SEMODE_BOMB ) ;
		}

		proc_work->count += TIME_BASE ;
		work->head_look_flag = 0 ;		/* 頭ＩＫを通常に戻す */
		/* 攻撃１ステータスを消す */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK1 );
		break ;
	}

	return ( 0 );
}
static int Action_Attack1_Die( Work *work, PROC_WORK *proc_work )
{
	/* エフェクト解除 */
	if ( work->attack_effect_work != NULL ){
		GV_DestroyOtherActor( work->attack_effect_work );
		work->attack_effect_work = NULL ;
	}
	work->head_look_flag = 0 ;		/* 頭ＩＫを通常に戻す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK1 );
	return ( 0 );
}
static int Action_Attack1_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Attack1_Act, Action_Attack1_Die );
	/* 攻撃中止カウンタの設定 */
	work->disable_attack1_count = DISABLE_ATTACK1_COUNT ;
	/* 叫びチェック */
	if ( work->on_stage_flag ){
		if ( ++work->n_attack_count >= ATTACK_ROAR_COUNT ){
			work->n_attack_count = 0 ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
		}
	}
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 手からバルカン */
static int Action_Attack2_Act( Work *work, PROC_WORK *proc_work )
{
	extern void	*NewRayValcanBullet( FMATRIX *world, u_int type, u_int side,
						   u_int size, u_int damage, u_int length, u_int speed, int weapon );
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	t ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Attack2" );

	switch ( proc_work->phase ){
	  case 0:
		/* モーション設定 */
		if ( PDRAY_SetMotion( work, MOTION_ATTACK2_START, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		work->true_action = ACTION_ATTACK2 ;
		work->true_action_bit = RAY_STATUS_ATTACK2 ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
		/* 右腕ＩＫの有効化 */
		work->right_arm_ik_mode = 1 ;
	  case 1:/* 攻撃準備フェーズ */
		//t = (float)( proc_work->count - ATTACK1_PHASE0_TIME ) / ( ATTACK1_PHASE1_TIME - ATTACK1_PHASE0_TIME );
		//t = body->m_ctrl->mt3_ctrl[ 0 ].play_time / body->m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		//if ( body->m_ctrl->mt3_ctrl[ 0 ].loop != 0 ) t = 1.0f ;
		t = MOTION_PLAY_RATE( body, 0 );

		//MT_HermiteLerpVec( &work->head_look_pos, &work->head_look_start_pos, &work->head_look_end_pos,
		//				  &DG_ZeroVector, &DG_ZeroVector, t );

		/* 腰の方向を補正する */
		work->adjust_target_rot_y = work->think_player_angle + 450 ;

		/* ＳＥ */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], 5 ) ){
			GM_SeSetMode( SD_E_ARMSWI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_HAND].world.m[3], GM_SEMODE_BOMB ) ;
		}

		{/* 攻撃座標計算 */
			FVECTOR		tmp_vec ;
			FMATRIX		tmp_mat ;
			GTE_ZeroVector( &tmp_vec );
			//GTE_LoadMatrix( &body->objs->world );
			GTE_MakeRotate( &tmp_mat, 0, 1, 0, GTE_PS2RAD( -450 ) );
			GTE_MulMatrix( &tmp_mat, &body->objs->world, &tmp_mat );
			GTE_LoadMatrix( &tmp_mat );
			tmp_vec.vz = work->think_player_length ;
			tmp_vec.vx = ATTACK2_WIDTH ;
			GTE_PutVector1( &work->attack_start_pos, &tmp_vec );
			tmp_vec.vx = -ATTACK2_WIDTH ;
			GTE_PutVector1( &work->attack_end_pos, &tmp_vec );
			work->attack_start_pos.vy = PLAYER_LEVEL + 500 ;
			work->attack_end_pos.vy = PLAYER_LEVEL + 500 ;
			work->attack_pos = work->attack_start_pos ;
			GTE_ZeroVector( &work->attack_start_speed );
			GTE_ZeroVector( &work->attack_end_speed );
		}

		proc_work->count += TIME_BASE ;
		if ( PDRAY_SetMotion( work, MOTION_ATTACK2_LOOP, MOTION_FLAG_NOINTERP ) != -1 ){
			proc_work->phase++ ;
			proc_work->count = 0 ;
			work->attack_count = 0 ;
			work->attack_phase = 0 ;
		}
		break ;
	  case 2:/* 攻撃フェーズ */
		work->attack_count += TIME_BASE ;
		if ( work->attack_count > 15 ){
			//static FVECTOR	offset_vec[3] = {{ -1000, -500, -200, 1 },{ -1000, -500, 0, 1 },{ -1000, -500, 200, 1 }};
			static FVECTOR	offset_vec[3] = {{ -1700, -500, -200, 1 },{ -1700, -500, 0, 1 },{ -1700, -500, 200, 1 }};
			FMATRIX		shot_mat, rot_mat ;
			shot_mat = body->objs->objs[PDRAY_JOINT_RIGHT_HAND].world ;
			GTE_ApplyMatrix( (FVECTOR*)shot_mat.m[3], &shot_mat, &offset_vec[work->attack_phase] );
			GTE_MakeRotate( &rot_mat, 0.0f, 0.0f, 1.0f, DEG2RAD(-90) );
			GTE_MulMatrix( &shot_mat, &shot_mat, &rot_mat );
			{/* 発射方向乱数散らし */
				int		angle ;
				FVECTOR	axis ;
				FMATRIX	error_mat ;
				axis.vx = RND( 256 ) - 128 ;
				axis.vy = RND( 256 ) - 128 ;
				axis.vz = RND( 256 ) - 128 ;
				angle = 6 ;
				GTE_Normalize( &axis, &axis );
				GTE_MakeRotateAxis( &error_mat, &axis, GTE_PS2RAD( angle ) );
				GTE_MulMatrix( &shot_mat, &shot_mat, &error_mat );
			}
			NewRayValcanBullet( &shot_mat, BUL_TYPE_VISIBLE|BUL_TYPE_SPARK, PLAYER_SIDE, 100, 16, 40000, 1800, WP_ShotGun_Far );

			work->attack_count -= 15 ;
			if ( ++(work->attack_phase) >= 3 ) work->attack_phase = 0 ;
			/* ＳＥ */
			GM_SeSetMode( SD_E_ARMGUN01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_HAND].world.m[3], GM_SEMODE_BOMB ) ;
		}

		//t = (float)( proc_work->count - ATTACK2_PHASE1_TIME ) / ( ATTACK2_PHASE2_TIME - ATTACK2_PHASE1_TIME );
		t = (float)proc_work->count / ATTACK2_PHASE2_TIME ;

		MT_HermiteLerpVec( &work->attack_pos, &work->attack_start_pos, &work->attack_end_pos,
						  &work->attack_start_speed, &work->attack_end_speed, t );
		//work->head_look_pos = work->attack_pos ;

		//AN_Test_Eye2( &work->attack_pos, 3 );

		/* 腰の方向を補正する */
		work->adjust_target_rot_y = work->think_player_angle + 450 ;

		proc_work->count += TIME_BASE ;
		/* キャンセルステータスによる攻撃キャンセルチェック */	
		if ( work->demand_action_bit & ( ATTACK_CANCEL_STATUS ) ){
			proc_work->count = ATTACK2_PHASE2_TIME + 1 ;
		}
		/* ステージ内攻撃時の攻撃エリア外判定による攻撃キャンセルチェック */	
		if ( work->on_stage_flag && ( work->think_area_check[AREA_VALCAN_OVER_RIGHT] == 0 ) ){
			proc_work->count = ATTACK2_PHASE2_TIME + 1 ;
		}
		if ( proc_work->count > ATTACK2_PHASE2_TIME ){
			/* モーション設定 */
			if ( PDRAY_SetMotion( work, MOTION_ATTACK2_END, MOTION_FLAG_FORCE ) == -1 ) break ;
			proc_work->phase++ ;
			/* 首ＩＫ補間パラメータ設定 */
			work->head_look_start_pos = work->head_look_pos ;
			/* 右腕ＩＫの無効化 */
			work->right_arm_ik_mode = 2 ;

			/* ＳＥ */
			GM_SeSetMode( SD_E_ARMSWI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_HAND].world.m[3], GM_SEMODE_BOMB ) ;
		}
		break ;
	  case 3:/* 別のフェーズへ */
		t = (float)( proc_work->count - ATTACK1_PHASE2_TIME ) / ( ATTACK1_PHASE3_TIME - ATTACK1_PHASE2_TIME );

		//MT_HermiteLerpVec( &work->head_look_pos, &work->head_look_start_pos, &GM_PlayerPosition,
		//				  &DG_ZeroVector, &DG_ZeroVector, t );

		proc_work->count += TIME_BASE ;
		work->head_look_flag = 0 ;		/* 頭ＩＫを通常に戻す */
		/* 攻撃２ステータスを消す */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK2 );
		break ;

	  case 16:
		/* モーション設定 */
		if ( PDRAY_SetMotion( work, MOTION_ATTACK2_START, MOTION_FLAG_BODY_REVERS|MOTION_FLAG_LEG_REVERS ) == -1 ) break ;
		proc_work->phase++ ;
		work->true_action = ACTION_ATTACK2 ;
		work->true_action_bit = RAY_STATUS_ATTACK2 ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
		/* 左腕ＩＫの有効化 */
		work->left_arm_ik_mode = 1 ;
	  case 17:/* 攻撃準備フェーズ */
		//t = (float)( work->action_count - ATTACK1_PHASE0_TIME ) / ( ATTACK1_PHASE1_TIME - ATTACK1_PHASE0_TIME );
		//t = body->m_ctrl->mt3_ctrl[ 0 ].play_time / body->m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		//if ( body->m_ctrl->mt3_ctrl[ 0 ].loop != 0 ) t = 1.0f ;
		t = MOTION_PLAY_RATE( body, 0 );

		//MT_HermiteLerpVec( &work->head_look_pos, &work->head_look_start_pos, &work->head_look_end_pos,
		//				  &DG_ZeroVector, &DG_ZeroVector, t );

		/* 腰の方向を補正する */
		work->adjust_target_rot_y = work->think_player_angle - 450 ;

		/* ＳＥ */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], 5 ) ){
			GM_SeSetMode( SD_E_ARMSWI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_HAND].world.m[3], GM_SEMODE_BOMB ) ;
		}

		{/* 攻撃座標計算 */
			FVECTOR		tmp_vec ;
			FMATRIX		tmp_mat ;
			GTE_ZeroVector( &tmp_vec );
			//GTE_LoadMatrix( &body->objs->world );
			GTE_MakeRotate( &tmp_mat, 0, 1, 0, GTE_PS2RAD( 450 ) );
			GTE_MulMatrix( &tmp_mat, &body->objs->world, &tmp_mat );
			GTE_LoadMatrix( &tmp_mat );
			tmp_vec.vz = work->think_player_length ;
			tmp_vec.vx = -ATTACK2_WIDTH ;
			GTE_PutVector1( &work->attack_start_pos, &tmp_vec );
			tmp_vec.vx = ATTACK2_WIDTH ;
			GTE_PutVector1( &work->attack_end_pos, &tmp_vec );
			work->attack_start_pos.vy = PLAYER_LEVEL + 500 ;
			work->attack_end_pos.vy = PLAYER_LEVEL + 500 ;
			work->attack_pos = work->attack_start_pos ;
			GTE_ZeroVector( &work->attack_start_speed );
			GTE_ZeroVector( &work->attack_end_speed );
		}

		proc_work->count += TIME_BASE ;
		if ( PDRAY_SetMotion( work, MOTION_ATTACK2_LOOP, MOTION_FLAG_BODY_REVERS|MOTION_FLAG_LEG_REVERS|MOTION_FLAG_NOINTERP ) != -1 ){
			proc_work->phase++ ;
			proc_work->count = 0 ;
			work->attack_count = 0 ;
			work->attack_phase = 0 ;
		}
		break ;
	  case 18:/* 攻撃フェーズ */
		work->attack_count += TIME_BASE ;
		if ( work->attack_count > 15 ){
			//static FVECTOR	offset_vec[3] = {{ 1000, -500, -200, 1 },{ 1000, -500, 0, 1 },{ 1000, -500, 200, 1 }};
			static FVECTOR	offset_vec[3] = {{ 1700, -500, -200, 1 },{ 1700, -500, 0, 1 },{ 1700, -500, 200, 1 }};
			FMATRIX		shot_mat, rot_mat ;
			shot_mat = body->objs->objs[PDRAY_JOINT_LEFT_HAND].world ;
			GTE_ApplyMatrix( (FVECTOR*)shot_mat.m[3], &shot_mat, &offset_vec[work->attack_phase] );
			GTE_MakeRotate( &rot_mat, 0.0f, 0.0f, 1.0f, DEG2RAD(90) );
			GTE_MulMatrix( &shot_mat, &shot_mat, &rot_mat );
			{/* 発射方向乱数散らし */
				int		angle ;
				FVECTOR	axis ;
				FMATRIX	error_mat ;
				axis.vx = RND( 256 ) - 128 ;
				axis.vy = RND( 256 ) - 128 ;
				axis.vz = RND( 256 ) - 128 ;
				angle = 6 ;
				GTE_Normalize( &axis, &axis );
				GTE_MakeRotateAxis( &error_mat, &axis, GTE_PS2RAD( angle ) );
				GTE_MulMatrix( &shot_mat, &shot_mat, &error_mat );
			}
			NewRayValcanBullet( &shot_mat, BUL_TYPE_VISIBLE|BUL_TYPE_SPARK, PLAYER_SIDE, 100, 16, 40000, 1800, WP_ShotGun_Far );
			//work->attack_count -= 50 ;
			work->attack_count -= 15 ;
			if ( ++(work->attack_phase) >= 3 ) work->attack_phase = 0 ;
			/* ＳＥ */
			GM_SeSetMode( SD_E_ARMGUN01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_HAND].world.m[3], GM_SEMODE_BOMB ) ;
		}

		//t = (float)( proc_work->count - ATTACK1_PHASE1_TIME ) / ( ATTACK1_PHASE2_TIME - ATTACK1_PHASE1_TIME );
		t = (float)proc_work->count / ATTACK2_PHASE2_TIME ;

		MT_HermiteLerpVec( &work->attack_pos, &work->attack_start_pos, &work->attack_end_pos,
						  &work->attack_start_speed, &work->attack_end_speed, t );

		//AN_Test_Eye2( &work->attack_pos, 3 );

		/* 腰の方向を補正する */
		work->adjust_target_rot_y = work->think_player_angle - 450 ;

		proc_work->count += TIME_BASE ;
		/* キャンセルステータスによる攻撃キャンセルチェック */	
		if ( work->demand_action_bit & ( ATTACK_CANCEL_STATUS ) ){
			proc_work->count = ATTACK2_PHASE2_TIME + 1 ;
		}
		/* ステージ内攻撃時の攻撃エリア外判定による攻撃キャンセルチェック */	
		if ( work->on_stage_flag && ( work->think_area_check[AREA_VALCAN_OVER_LEFT] == 0 ) ){
			proc_work->count = ATTACK2_PHASE2_TIME + 1 ;
		}
		if ( proc_work->count > ATTACK2_PHASE2_TIME ){
			/* モーション設定 */
			if ( PDRAY_SetMotion( work, MOTION_ATTACK2_END, MOTION_FLAG_BODY_REVERS|MOTION_FLAG_LEG_REVERS|MOTION_FLAG_FORCE ) == -1 ) break ;
			proc_work->phase++ ;
			/* 首ＩＫ補間パラメータ設定 */
			work->head_look_start_pos = work->head_look_pos ;
			/* 左腕ＩＫの無効化 */
			work->left_arm_ik_mode = 2 ;

			/* ＳＥ */
			GM_SeSetMode( SD_E_ARMSWI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_HAND].world.m[3], GM_SEMODE_BOMB ) ;
		}
		break ;
	  case 19:/* 別のフェーズへ */
		t = (float)( proc_work->count - ATTACK1_PHASE2_TIME ) / ( ATTACK1_PHASE3_TIME - ATTACK1_PHASE2_TIME );

		proc_work->count += TIME_BASE ;
		work->head_look_flag = 0 ;		/* 頭ＩＫを通常に戻す */
		/* 攻撃２ステータスを消す */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK2 );
		break ;

	}

	return ( 0 );
}
static int Action_Attack2_Die( Work *work, PROC_WORK *proc_work )
{
	work->head_look_flag = 0 ;		/* 頭ＩＫを通常に戻す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK2 );
	/* 右腕ＩＫの無効化 */
	if ( work->right_arm_ik_mode ) work->right_arm_ik_mode = 2 ;
	/* 左腕ＩＫの無効化 */
	if ( work->left_arm_ik_mode ) work->left_arm_ik_mode = 2 ;

	return ( 0 );
}
static int Action_Attack2_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Attack2_Act, Action_Attack2_Die );

	/* 首ＩＫ補間パラメータ設定 */
	work->head_look_start_pos = work->head_look_pos ;
	work->head_look_end_pos = work->attack_start_pos ;
	work->head_look_flag = 0 ;			/* 頭ＩＫを操作するモードに変更 */

	if ( work->think_player_angle > 0 ){
		proc_work->phase = 16 ;	/* 左腕を使用する */
	}

	/* 攻撃中止カウンタの設定 */
	work->disable_attack2_count = DISABLE_ATTACK2_COUNT ;

	/* 叫びチェック */
	if ( work->on_stage_flag ){
		if ( ++work->n_attack_count >= ATTACK_ROAR_COUNT ){
			work->n_attack_count = 0 ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
		}
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 足からミサイル */
static int Action_Attack3_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	//float	t ;
	int			attack_flag = 0 ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Attack3" );

	switch ( proc_work->phase ){
	  case 0:/* 攻撃準備フェーズ */
		/* モーション設定 */
		if ( PDRAY_SetMotion( work, MOTION_ATTACK3_START, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		work->true_action = ACTION_ATTACK3 ;
		work->true_action_bit = RAY_STATUS_ATTACK3 ;
	  case 1:
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
		proc_work->count += TIME_BASE ;
		if ( PDRAY_SetMotion( work, MOTION_ATTACK3_LOOP, 0 ) != -1 ){
			proc_work->phase++ ;
			work->attack_count = 0 ;
			work->attack_end_count = 0 ;
		}
		break ;
	  case 2:/* 攻撃フェーズ */

		work->attack_count += TIME_BASE ;
		if ( work->on_stage_flag == 0 ){
			if ( PDRAY_ATTACK_TIMING(150) ){
				attack_flag = 1 ;
			} else if ( PDRAY_ATTACK_TIMING(250) ){
				attack_flag = 2 ;
			} else if ( PDRAY_ATTACK_TIMING(350) ){
				attack_flag = 1 ;
			} else if ( PDRAY_ATTACK_TIMING(450) ){
				attack_flag = 2 ;
			}
			if ( attack_flag ){
				static FVECTOR	r_offset = {-1000,3000,0,1};
				static FVECTOR	l_offset = {1000,3000,0,1};
				FMATRIX		mat, x_rot_mat ;

				GTE_UnitMatrix( &x_rot_mat );
				GTE_LoadMatrix( &x_rot_mat );
				GTE_RotateX( GTE_PS2RAD( -1024 ) );
				GTE_StoreMatrix( &x_rot_mat );

				if ( attack_flag == 1 ){
					GTE_LoadMatrix( &body->objs->objs[ 2 ].world );
					GTE_Translate( &r_offset );
					GTE_StoreMatrix( &mat );
					GTE_MulMatrix( &mat, &mat, &x_rot_mat );
					NewPDRayWeaponMissile( &mat, 0|2 );
					/* ＳＥ */
					GM_SeSetMode( SD_E_LEGMISF1, (FVECTOR*)mat.m[3], GM_SEMODE_BOMB ) ;
				} else {
					GTE_LoadMatrix( &body->objs->objs[ 2 ].world );
					GTE_Translate( &l_offset );
					GTE_StoreMatrix( &mat );
					GTE_MulMatrix( &mat, &mat, &x_rot_mat );
					NewPDRayWeaponMissile( &mat, 1|2 );
					/* ＳＥ */
					GM_SeSetMode( SD_E_LEGMISF1, (FVECTOR*)mat.m[3], GM_SEMODE_BOMB ) ;
				}
			}
			
			if ( PDRAY_ATTACK_TIMING(600) ){
				PDRAY_SetMotion( work, MOTION_ATTACK3_END, MOTION_FLAG_FORCE );
				proc_work->phase++ ;
			}
		} else {
			if ( PDRAY_ATTACK_TIMING(5) ){
				static FVECTOR	r_offset = {-1000,0,8000,1};
				static FVECTOR	l_offset = {1000,0,8000,1};
				FMATRIX		mat, x_rot_mat ;

				GTE_UnitMatrix( &x_rot_mat );
				GTE_LoadMatrix( &x_rot_mat );
				GTE_RotateX( GTE_PS2RAD( -400 ) );
				GTE_StoreMatrix( &x_rot_mat );

				GTE_LoadMatrix( &body->objs->objs[ 22 ].world );
				GTE_Translate( &r_offset );
				GTE_StoreMatrix( &mat );
				GTE_MulMatrix( &mat, &mat, &x_rot_mat );
				NewPDRayWeaponMissile( &mat, 0 );
				/* ＳＥ */
				GM_SeSetMode( SD_E_LEGMISF1, (FVECTOR*)mat.m[3], GM_SEMODE_BOMB ) ;

				GTE_LoadMatrix( &body->objs->objs[ 28 ].world );
				GTE_Translate( &l_offset );
				GTE_StoreMatrix( &mat );
				GTE_MulMatrix( &mat, &mat, &x_rot_mat );
				NewPDRayWeaponMissile( &mat, 1 );
				/* ＳＥ */
				GM_SeSetMode( SD_E_LEGMISF1, (FVECTOR*)mat.m[3], GM_SEMODE_BOMB ) ;

			}
			
			if ( PDRAY_SetMotion( work, MOTION_ATTACK3_END, 0 ) != -1 ){
				proc_work->phase++ ;
			}
		}

		break ;
	  case 3:/* 別のフェーズへ */
		proc_work->count += TIME_BASE ;
		work->head_look_flag = 0 ;		/* 頭ＩＫを通常に戻す */
		/* 攻撃３ステータスを消す */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK3 );
		break ;
	}

	return ( 0 );
}
static int Action_Attack3_Die( Work *work, PROC_WORK *proc_work )
{
	/* 攻撃３ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK3 );
	return ( 0 );
}
static int Action_Attack3_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Attack3_Act, Action_Attack3_Die );
	/* 攻撃中止カウンタの設定 */
	work->disable_attack3_count = DISABLE_ATTACK3_COUNT ;
	/* 叫びチェック */
	if ( work->on_stage_flag ){
		if ( ++work->n_attack_count >= ATTACK_ROAR_COUNT ){
			work->n_attack_count = 0 ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
		}
	}

#if 0
	/* ベリーイージー難易度修正 */
	if ( ( RAYSERVER_GameLevel <= 1 ) && ( work->on_stage_flag == 0 ) ){
		proc_work->phase = 3 ;
	}
#endif

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 踏み潰し攻撃 */
static int Action_Attack4_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	t, play_time ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Attack4" );

	/* 現在のモーション再生時間を取得 */
	play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;

	/* 使用する足を決定 */
	if ( ( proc_work->phase == 0 ) || ( proc_work->phase == 16 ) ){
		/* モーション発動待ち中は常に最新のプレイヤー方向で変更する */
		if ( work->think_player_angle > 0 ){
			proc_work->phase = 16 ;
		} else {
			proc_work->phase = 0 ;
		}
	}

	switch ( proc_work->phase ){
	  case 0:/* 足上げる */
		/* モーション設定 */
		if ( PDRAY_SetMotion( work, MOTION_KICK, MOTION_FLAG_AGAIN ) == -1 ) break ;
		proc_work->phase++ ;
		work->true_action = ACTION_ATTACK4 ;
		work->true_action_bit = RAY_STATUS_ATTACK4 ;
		work->chaff_disable = 1 ;
		/* 上からの見下ろしカメラに変更 */
		RAYSERVER_SetGameSignal( GAME_SIGNAL_CHANGE_CAMERA, 1 );

		/* 右足駆動開始 */
		{
			/* 求めた座標から右足の目標座標を計算 */
			work->attack_end_pos = work->right_leg_target_pos ;	/* 現在の足の位置を記録 */
			work->ik_target = work->right_leg_target_pos ;
			work->ik_target_from = work->right_leg_target_pos ;
			work->ik_target_to = GM_PlayerPosition ;
			{/* 踏みつけ位置の距離チェック */
				FVECTOR		tmp_vec ;
				float		len ;
				GTE_SubVector( &tmp_vec, &work->ik_target_to, &work->ik_target_from );
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len > ATTACK4_RANGE ){
					GTE_ScaleVector( &tmp_vec, &tmp_vec, ATTACK4_RANGE / len );
					GTE_AddVector( &work->ik_target_to, &work->ik_target_from, &tmp_vec ) ;
				}
			}
			GTE_ZeroVector( &work->ik_target_from_speed );
			GTE_ZeroVector( &work->ik_target_to_speed );
			//GTE_SubVector( &work->ik_target_to_speed, &work->ik_target_to, &work->ik_target_from );
		}
	  case 1:/* プレイヤー座標に対して足を下ろす */
		t = ( play_time - ATTACK4_PHASE0_TIME ) / ( ATTACK4_PHASE1_TIME - ATTACK4_PHASE0_TIME ) ;
		if ( t > 1.0f ) t = 1.0f ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );

		/* ＳＥ */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], 100 ) ){
			GM_SeSetMode( SD_E_LEGSWI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		}

		/* フェーズ移行チェック */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], ATTACK4_PHASE1_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			///* 攻撃判定の変わりに汎用爆発を呼び出す */
			//NewBlast( &work->ik_target, PLAYER_SIDE, 1000, 2000, 64, 1, WP_Stamp );
			{/* 地震発生チェック */
				FVECTOR		tmp_vec ;
				float		len ;
				work->earthquake_pos = work->ik_target ;
				work->earthquake_pos.vy = PLAYER_LEVEL ;
				GTE_SubVector( &tmp_vec, &work->earthquake_pos, &GM_PlayerPosition );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len < EARTHQUAKE_RANGE1 ){
					work->earthquake_flag = 1 ;
				}
				work->earthquake_effect_flag = 1 ;
				/* ＳＥ */
				GM_SeSetMode( SD_E_R_FTAT01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}
		break ;
	  case 2:/* モーション終了待ち */
		if ( PDRAY_SetMotion( work, MOTION_WALK, MOTION_FLAG_AGAIN ) != -1 ){
			proc_work->phase++ ;
			work->chaff_disable = 1 ;
			SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
			work->ik_target_from = work->ik_target_to ;
			work->ik_target_to = work->attack_end_pos ;
		}
		break ;
	  case 3:/* 元の位置に足を戻す */
		t = ( play_time - WALK_PHASE0_TIME ) / ( WALK_PHASE2_TIME - WALK_PHASE0_TIME ) ;
		if ( t > 1.0f ) t = 1.0f ;
		SET_LEG_IK_MODE( IK_MODE_CONTROL, IK_MODE_FIX );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );

		proc_work->count += TIME_BASE ;
		/* フェーズ移行チェック */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], WALK_PHASE2_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			work->ik_target = work->ik_target_to ;
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			///PDRAY_UpdateActionStatus( work );
			/* カメラを元に戻す */
			RAYSERVER_SetGameSignal( GAME_SIGNAL_CHANGE_CAMERA, 0 );
			/* 攻撃４ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK4 );

			{/* ＳＥ */
				int se_code[4] = {SD_E_R_FTFR01,SD_E_R_FTFR02,SD_E_R_FTFR03,SD_E_R_FTNR01};
				int	se_type ;
				se_type = work->no % 3 ;
				if ( work->on_stage_flag ) se_type = 3 ;
				GM_SeSetMode( se_code[ se_type ], (FVECTOR*)body->objs->objs[PDRAY_JOINT_RIGHT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}
		break ;
	  case 4:
		if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) != -1 ){
			///PDRAY_UpdateActionStatus( work );
		}
		break ;


	  case 16:/* 足上げる */
		/* モーション設定 */
		if ( PDRAY_SetMotion( work, MOTION_KICK, MOTION_FLAG_AGAIN|MOTION_FLAG_LEG_REVERS ) == -1 ) break ;
		proc_work->phase++ ;
		work->true_action = ACTION_ATTACK4 ;
		work->true_action_bit = RAY_STATUS_ATTACK4 ;
		work->chaff_disable = 1 ;
		/* 上からの見下ろしカメラに変更 */
		RAYSERVER_SetGameSignal( GAME_SIGNAL_CHANGE_CAMERA, 1 );

		/* 左足駆動開始 */
		{
			/* 求めた座標から右足の目標座標を計算 */
			work->attack_end_pos = work->left_leg_target_pos ;	/* 現在の足の位置を記録 */
			work->ik_target = work->left_leg_target_pos ;
			work->ik_target_from = work->left_leg_target_pos ;
			work->ik_target_to = GM_PlayerPosition ;
			{/* 踏みつけ位置の距離チェック */
				FVECTOR		tmp_vec ;
				float		len ;
				GTE_SubVector( &tmp_vec, &work->ik_target_to, &work->ik_target_from );
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len > ATTACK4_RANGE ){
					GTE_ScaleVector( &tmp_vec, &tmp_vec, ATTACK4_RANGE / len );
					GTE_AddVector( &work->ik_target_to, &work->ik_target_from, &tmp_vec ) ;
				}
			}
			GTE_ZeroVector( &work->ik_target_from_speed );
			GTE_ZeroVector( &work->ik_target_to_speed );
			//GTE_SubVector( &work->ik_target_to_speed, &work->ik_target_to, &work->ik_target_from );
		}
	  case 17:/* プレイヤー座標に対して足を下ろす */
		t = ( play_time - ATTACK4_PHASE0_TIME ) / ( ATTACK4_PHASE1_TIME - ATTACK4_PHASE0_TIME ) ;
		if ( t > 1.0f ) t = 1.0f ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_CONTROL );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );

		/* ＳＥ */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], 100 ) ){
			GM_SeSetMode( SD_E_LEGSWI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		}

		/* フェーズ移行チェック */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], ATTACK4_PHASE1_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			///* 攻撃判定の変わりに汎用爆発を呼び出す */
			//NewBlast( &work->ik_target, PLAYER_SIDE, 1000, 2000, 64, 1, WP_Stamp );
			{/* 地震発生チェック */
				FVECTOR		tmp_vec ;
				float		len ;
				work->earthquake_pos = work->ik_target ;
				work->earthquake_pos.vy = PLAYER_LEVEL ;
				GTE_SubVector( &tmp_vec, &work->earthquake_pos, &GM_PlayerPosition );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len < EARTHQUAKE_RANGE1 ){
					work->earthquake_flag = 1 ;
				}
				work->earthquake_effect_flag = 1 ;
				/* ＳＥ */
				GM_SeSetMode( SD_E_R_FTAT01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}
		break ;
	  case 18:/* モーション終了待ち */
		if ( PDRAY_SetMotion( work, MOTION_WALK, MOTION_FLAG_AGAIN|MOTION_FLAG_LEG_REVERS ) != -1 ){
			proc_work->phase++ ;
			work->chaff_disable = 1 ;
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_CONTROL );
			work->ik_target_from = work->ik_target_to ;
			work->ik_target_to = work->attack_end_pos ;
		}
		break ;
	  case 19:/* 元の位置に足を戻す */
		t = ( play_time - WALK_PHASE0_TIME ) / ( WALK_PHASE2_TIME - WALK_PHASE0_TIME ) ;
		if ( t > 1.0f ) t = 1.0f ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_CONTROL );
		/* ＩＫ制御点計算 */
		MT_HermiteLerpVec( &work->ik_target, &work->ik_target_from, &work->ik_target_to,
						  &work->ik_target_from_speed, &work->ik_target_to_speed, t );

		proc_work->count += TIME_BASE ;
		/* フェーズ移行チェック */
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], WALK_PHASE2_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			work->ik_target = work->ik_target_to ;
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			///PDRAY_UpdateActionStatus( work );
			/* カメラを元に戻す */
			RAYSERVER_SetGameSignal( GAME_SIGNAL_CHANGE_CAMERA, 0 );
			/* 攻撃４ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK4 );

			{/* ＳＥ */
				int se_code[4] = {SD_E_R_FTFL01,SD_E_R_FTFL02,SD_E_R_FTFL03,SD_E_R_FTNL01};
				int	se_type ;
				se_type = work->no % 3 ;
				if ( work->on_stage_flag ) se_type = 3 ;
				GM_SeSetMode( se_code[ se_type ], (FVECTOR*)body->objs->objs[PDRAY_JOINT_LEFT_TOE].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}
		break ;
	  case 20:
		if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) != -1 ){
			///PDRAY_UpdateActionStatus( work );
		}
		break ;
	}

	return ( 0 );
}
static int Action_Attack4_Die( Work *work, PROC_WORK *proc_work )
{
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK4 );
	/* カメラを元に戻す */
	RAYSERVER_SetGameSignal( GAME_SIGNAL_CHANGE_CAMERA, 0 );
	return ( 0 );
}
static int Action_Attack4_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Attack4_Act, Action_Attack4_Die );
	/* 攻撃中止カウンタの設定 */
	work->disable_attack4_count = DISABLE_ATTACK4_COUNT ;
	/* 叫びチェック */
	if ( work->on_stage_flag ){
		if ( ++work->n_attack_count >= ATTACK_ROAR_COUNT ){
			work->n_attack_count = 0 ;
			PDRAY_SetDemandActionBit( work, RAY_STATUS_ROAR );
		}
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 叫びアクション */
static int Action_Roar_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Roar" );

	switch ( proc_work->phase ){
	  case 0:
		if ( PDRAY_SetMotion( work, MOTION_ROAR, 0 ) == -1 ) break ;
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_ROAR );
		///PDRAY_UpdateActionStatus( work );
		work->true_action = ACTION_ROAR ;
		work->true_action_bit = RAY_STATUS_ROAR ;
		proc_work->phase++ ;

		/* ＳＥ */
		if ( work->on_stage_flag ){
			GM_SeSetMode( SD_E_GROWL002, (FVECTOR*)body->objs->objs[PDRAY_JOINT_HEAD].world.m[3], GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_GROWL003, (FVECTOR*)body->objs->objs[PDRAY_JOINT_HEAD].world.m[3], GM_SEMODE_BOMB ) ;
		}
		break ;

	  case 1:
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], ROAR_BREATH_TIME0 ) ){
			proc_work->phase++ ;
			{/* エフェクト起動 */
				extern void *NewSmokeBlurEffect( FMATRIX *world, int start_speed, int end_speed,
												int start_size, int end_size, int spot_size, int spot_angle,
												int n_prims, int interval, int color, int flag );
				work->smoke_blur_work_ptr = NewRAYBreathSmoke( &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
				GV_SetActorChild( work, work->smoke_blur_work_ptr );
				GM_SeSetMode( SD_E_RBRESS01,
							 (FVECTOR*)body->objs->objs[ PDRAY_JOINT_HEAD ].world.m[3], GM_SEMODE_BOMB );
#if 0
				work->smoke_blur_work_ptr =
				  NewSmokeBlurEffect( &body->objs->objs[ PDRAY_JOINT_HEAD ].world,
									 200, 60, 1200, 2000,
									 200, 256,
									 32, 1, 0x00e1b1b1, 0x0000 );
#endif
			}

		}
		break ;

	  case 2:
		if ( MT_CheckMotionTime( &body->m_ctrl->mt3_ctrl[ 0 ], ROAR_BREATH_TIME1 ) ){
			proc_work->phase++ ;
			/* エフェクト終了 */
			if ( work->smoke_blur_work_ptr != NULL ){
				//GV_DestroyActor( work->smoke_blur_work_ptr );
				GV_CallChildSignalFunc( work, SIGNAL_RAYBREATHSMOKE_STOP, 0 );
				work->smoke_blur_work_ptr = NULL ;
			}
		}
		break ;

	  case 3:
		if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) == -1 ) break ;
		proc_work->phase++ ;
			
		break ;
	}

	return ( 0 );
}
static int Action_Roar_Die( Work *work, PROC_WORK *proc_work )
{
	if ( !( work->demand_action_bit & (RAY_STATUS_DAMAGE|RAY_STATUS_DAMAGE2|RAY_STATUS_DAMAGE3) ) ){
		/* ダメージ以外は終了するまで次に移行しない */
		if ( proc_work->phase < 3 ) return ( -1 ) ;
	}
	/* エフェクト終了 */
	if ( work->smoke_blur_work_ptr != NULL ){
		//GV_DestroyActor( work->smoke_blur_work_ptr );
		GV_CallChildSignalFunc( work, SIGNAL_RAYBREATHSMOKE_STOP, 0 );
		work->smoke_blur_work_ptr = NULL ;
	}
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_ROAR );
	return ( 0 );
}
static int Action_Roar_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Roar_Act, Action_Roar_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 通常ダメージ */
static int Action_Damage_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Damage" );

	if ( PDRAY_SetMotion( work, MOTION_DAMAGE, MOTION_FLAG_FORCE ) != -1 ){
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		/* 念のため現在の足の状態を記録 */
		PDRAY_ForceRecordLegPosition( work );
		SET_LEG_IK_MODE( IK_MODE_FIXFALL, IK_MODE_FIXFALL );
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_DAMAGE );
		PDRAY_ResetDemandActionBit( work, DAMAGE_RESET_STATUS );
		work->true_action = ACTION_DAMAGE ;
		work->true_action_bit = RAY_STATUS_DAMAGE ;
		PDRAY_SetDemandActionBit( work, RAY_STATUS_STEP );
		/* 共有ステータスとして通知する */
		if ( work->on_stage_flag ){
			RAYSERVER_SetGameSignal( GAME_SIGNAL_NOTICE, CSTATUS_DAMAGE1 );
		} else {
			RAYSERVER_SetGameSignal( GAME_SIGNAL_NOTICE, CSTATUS_DAMAGE2 );
		}
	}
	return ( 0 );
}
static int Action_Damage_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_Damage_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Damage_Act, Action_Damage_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 右足ダメージ怯み */
static int Action_Damage2_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Damage2" );

	if ( PDRAY_SetMotion( work, MOTION_LEG_DAMAGE, MOTION_FLAG_FORCE ) != -1 ){
		/* 念のため現在の足の状態を記録 */
		PDRAY_ForceRecordLegPosition( work );
		SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FIXFALL );
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_DAMAGE2 );
		//PDRAY_ResetDemandActionBit( work, DAMAGE_RESET_STATUS );
		work->true_action = ACTION_DAMAGE2 ;
		work->true_action_bit = RAY_STATUS_DAMAGE2 ;
		/* 共有ステータスとして通知する */
		if ( work->on_stage_flag ){
			RAYSERVER_SetGameSignal( GAME_SIGNAL_NOTICE, CSTATUS_SDAMAGE1 );
		} else {
			RAYSERVER_SetGameSignal( GAME_SIGNAL_NOTICE, CSTATUS_SDAMAGE2 );
		}
	}

	return ( 0 );
}
static int Action_Damage2_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_Damage2_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Damage2_Act, Action_Damage2_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 左足ダメージ怯み */
static int Action_Damage3_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Damage3" );

	if ( PDRAY_SetMotion( work, MOTION_LEG_DAMAGE, MOTION_FLAG_BODY_REVERS|MOTION_FLAG_LEG_REVERS|MOTION_FLAG_FORCE ) != -1 ){
		/* 念のため現在の足の状態を記録 */
		PDRAY_ForceRecordLegPosition( work );
		SET_LEG_IK_MODE( IK_MODE_FIXFALL, IK_MODE_FREE );
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_DAMAGE3 );
		//PDRAY_ResetDemandActionBit( work, DAMAGE_RESET_STATUS );
		work->true_action = ACTION_DAMAGE3 ;
		work->true_action_bit = RAY_STATUS_DAMAGE3 ;
		/* 共有ステータスとして通知する */
		if ( work->on_stage_flag ){
			RAYSERVER_SetGameSignal( GAME_SIGNAL_NOTICE, CSTATUS_SDAMAGE1 );
		} else {
			RAYSERVER_SetGameSignal( GAME_SIGNAL_NOTICE, CSTATUS_SDAMAGE2 );
		}
		return ( 0 );
	}

	return ( 0 );
}
static int Action_Damage3_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_Damage3_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Damage3_Act, Action_Damage3_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 防御体勢 */
static int Action_Guard_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Guard" );

   // BP_WARNING - This is always false
	if ( GM_GameStatus & 0 ){
		work->head_look_flag = 1 ;
		work->head_look_pos = GM_StingerPosition ;
	} else {
		work->head_look_flag = 0 ;
	}

	switch ( proc_work->phase ){
	  case 0:
		if ( PDRAY_SetMotion( work, MOTION_IDLE, 0 ) == -1 ) break ;
		SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
		work->true_action = ACTION_GUARD ;
		work->true_action_bit = RAY_STATUS_GUARD ;
#if 0
		/* 自分が場外にいるときにステージ上のＲＡＹがダメージを受けたら叫ぶ */
		if ( work->on_stage_flag == 0 ){
			if ( 0 );
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_ROAR );
		}
#endif
#if 0
		/* スティンガーが飛んできたらはたき落とす */
		if ( PDRAY_CheckNearStinger( work ) != -1 ){
			/* モーション設定 */
			//proc_work->phase = 1 ;
			//PDRAY_SetMotion( work, MOTION_SLAP, MOTION_FLAG_FORCE );	/* 強制的にモーションを発動 */
			work->adjust_target_rot_y = 400 ;
		}
#endif
		break ;
	  case 1:
		proc_work->count += TIME_BASE ;
		if ( proc_work->count > 150 ){
			proc_work->phase = 0 ;
			//work->force_stg_break_flag = 1 ;
		}
		break ;
	}

	return ( 0 );
}
static int Action_Guard_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_Guard_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Guard_Act, Action_Guard_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 混乱（現在未使用） */
static int Action_Confusion_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Confusion" );

	switch ( proc_work->phase ){
	  case 0:
		if ( PDRAY_SetMotion( work, MOTION_CONFUSION, MOTION_FLAG_FORCE ) != -1 ){
			SET_LEG_IK_MODE( IK_MODE_FIXFALL, IK_MODE_FIXFALL );
			work->true_action = ACTION_CONFUSION ;
			work->true_action_bit = RAY_STATUS_CONFUSION ;
			proc_work->phase++ ;
		}
		break ;
	  case 1:
		break ;
	}
	
	return ( 0 );
}
static int Action_Confusion_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_Confusion_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Confusion_Act, Action_Confusion_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* ジャンプ */
static int Action_Jump1_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	play_time, t ;
	FVECTOR		tmp_vec ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Jump1" );

	switch ( proc_work->phase ){
	  case 0:
		/* モーション発動までウェイトを入れる */
		if ( PDRAY_SetMotion( work, MOTION_JUMP, MOTION_FLAG_AGAIN ) == 1 ){
			proc_work->phase++ ;
			work->true_action = ACTION_JUMP1 ;
			work->true_action_bit = RAY_STATUS_JUMP1 ;
		}
		break ;
	  case 1:/* ジャンプ前のため */
		/* ジャンプ開始チェック */
		play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;
		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, JUMP_PHASE0_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 1 ;

			work->jump_start_pos = ctrl->mov ;
			work->jump_start_pos.vy = 0.0f ;
			work->jump_end_pos.vx = 0 ;
			work->jump_end_pos.vy = PLAYER_LEVEL ;
			work->jump_end_pos.vz = 0 ;
			{
				FVECTOR		tmp_vec ;
				float		rlen ;
				GTE_ZeroVector( &tmp_vec );
				GTE_SubVector( &tmp_vec, &ctrl->mov, &tmp_vec );
				tmp_vec.vy = 0 ;
				rlen = DG_RSQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				GTE_ScaleVector( &tmp_vec, &tmp_vec, rlen * STAGE_IN_RANGE );
				work->jump_end_pos = tmp_vec ;
				work->jump_end_pos.vy = PLAYER_LEVEL ;
				work->stage_stand_pos = work->jump_end_pos ;
				work->move_target = work->jump_end_pos ;
			}
			GTE_SubVector( &work->jump_offset, &work->jump_end_pos, &work->jump_start_pos );
			work->jump_offset.vy = 4000.0f ;

			SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );

			/* ＳＥ */
			GM_SeSetMode( SD_E_R_FTFJ01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;

			{/* 足水しぶきエフェクト起動 */
				extern void *NewRayJumpSplash( FMATRIX *parent );
				NewRayJumpSplash( &body->objs->objs[PDRAY_JOINT_RIGHT_TOE].world );
				NewRayJumpSplash( &body->objs->objs[PDRAY_JOINT_LEFT_TOE].world );
			}
		}
		break ;
	  case 2:/* ジャンプ中 */
		play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;
		t = ( play_time - JUMP_PHASE0_TIME ) / ( JUMP_PHASE1_TIME - JUMP_PHASE0_TIME );
		if ( t > 1.0f ) t = 1.0f ;
		
		/* 先にジャンプ終了チェック */
		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, JUMP_PHASE1_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			work->flags &= ~FLAG_JUMP ;
			ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_JUMP1 );
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			{/* 地震発生チェック */
				FVECTOR		tmp_vec ;
				float		len ;
				GTE_AddVector( &work->earthquake_pos, &work->right_leg_target_pos, &work->left_leg_target_pos );
				GTE_ScaleVector( &work->earthquake_pos, &work->earthquake_pos, 0.5f );
				work->earthquake_pos.vy = PLAYER_LEVEL ;
				GTE_SubVector( &tmp_vec, &work->earthquake_pos, &GM_PlayerPosition );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len < EARTHQUAKE_RANGE2 ){
					work->earthquake_flag = 2 ;
				}
				work->earthquake_effect_flag = 2 ;
			}

			/* ステージ上に乗っかった */
			work->flags |= FLAG_ON_STAGE ;

			/* ＳＥ */
			GM_SeSetMode( SD_E_R_FTNG01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		} else {
			work->flags |= FLAG_JUMP ;
			GTE_ScaleVector( &tmp_vec, &work->jump_offset, t );
			GTE_AddVector( &ctrl->mov, &work->jump_start_pos, &tmp_vec );
		}
		break ;
	  case 3:
		break ;
	}

	return ( 0 );
}
static int Action_Jump1_Die( Work *work, PROC_WORK *proc_work )
{
	/* ジャンプは何があってもキャンセルできない */
	if ( proc_work->phase != 3 && proc_work->phase != 0 ) return ( -1 );
	return ( 0 );
}
static int Action_Jump1_Init( Work *work, PROC_WORK *proc_work )
{
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_MOVE1 );
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_DRINK|RAY_STATUS_ROAR );
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_ATTACK1|RAY_STATUS_ATTACK2|RAY_STATUS_ATTACK3 );
	PROC_SET_CALLBACK( proc_work, Action_Jump1_Act, Action_Jump1_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* ジャンプ */
static int Action_Jump2_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	play_time, t ;
	FVECTOR		tmp_vec ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Jump2" );

	switch ( proc_work->phase ){
	  case 0:
		/* モーション発動までウェイトを入れる */
		if ( PDRAY_SetMotion( work, MOTION_JUMP, MOTION_FLAG_AGAIN ) == 1 ){
			proc_work->phase++ ;
			work->true_action = ACTION_JUMP2 ;
			work->true_action_bit = RAY_STATUS_JUMP2 ;
		}
		break ;
	  case 1:/* ジャンプ前のため */
		/* ジャンプ開始チェック */
		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, JUMP_PHASE0_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 1 ;

			work->jump_start_pos = ctrl->mov ;
			work->jump_start_pos.vy = PLAYER_LEVEL ;
			//RAYSERVER_GetRayMovePosition( work, &work->jump_end_pos );
			//work->jump_end_pos = work->info.standby_pos ;
			{
				float	len ;
				tmp_vec = work->jump_start_pos ;
				tmp_vec.vy = 0.0f ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				GTE_ScaleVector( &work->jump_end_pos, &tmp_vec, JUMP_OUT_RANGE/len );
			}
			GTE_SubVector( &work->jump_offset, &work->jump_end_pos, &work->jump_start_pos );
			work->jump_offset.vy = -PLAYER_LEVEL ;
			work->move_target = work->jump_end_pos ;

			SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );

			/* ＳＥ */
			GM_SeSetMode( SD_E_R_FTNJ01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		}
		break ;
	  case 2:/* ジャンプ中 */
		play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;
		t = ( play_time - JUMP_PHASE0_TIME ) / ( JUMP_PHASE1_TIME - JUMP_PHASE0_TIME );
		if ( t > 1.0f ) t = 1.0f ;

		/* 先にジャンプ終了チェック */
		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, JUMP_PHASE1_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			work->flags &= ~FLAG_JUMP ;
			ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_JUMP2 );
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			{/* 地震発生チェック */
				GTE_AddVector( &work->earthquake_pos, &work->right_leg_target_pos, &work->left_leg_target_pos );
				GTE_ScaleVector( &work->earthquake_pos, &work->earthquake_pos, 0.5f );
				work->earthquake_pos.vy = 0 ;
				work->earthquake_effect_flag = 2 ;
			}

			/* ステージから降りた */
			work->flags &= ~FLAG_ON_STAGE ;

			if ( work->life <= 0 ){
				PDRAY_SetDemandActionBit( work, RAY_STATUS_DAMAGE2 );
			}

			/* ＳＥ */
			GM_SeSetMode( SD_E_R_FTFG01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		} else {
			work->flags |= FLAG_JUMP ;
			GTE_ScaleVector( &tmp_vec, &work->jump_offset, t );
			GTE_AddVector( &ctrl->mov, &work->jump_start_pos, &tmp_vec );
		}
		break ;
	  case 3:
		break ;
	}

	return ( 0 );
}
static int Action_Jump2_Die( Work *work, PROC_WORK *proc_work )
{
	/* ジャンプは何があってもキャンセルできない */
	if ( proc_work->phase != 3 ) return ( -1 );
	return ( 0 );
}
static int Action_Jump2_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Jump2_Act, Action_Jump2_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 破壊崩れこみ */
static int Action_Break_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Break" );

	switch ( proc_work->phase ){
	  case 0:
		if ( PDRAY_SetMotion( work, MOTION_BREAK, 0 ) == -1 ) break ;
		SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		proc_work->phase++ ;
		work->true_action = ACTION_BREAK ;
		work->true_action_bit = RAY_STATUS_BREAK ;
		break ;
	  case 1:
		if ( PDRAY_SetMotion( work, MOTION_BREAK_PAUSE, 0 ) == -1 ) break ;
		SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		proc_work->phase++ ;
		/* 表示や当たり判定などを全て有効に設定 */
		work->active_flag = 0 ;
		//RAYSERVER_FreeNumberModel( work->number_model_handle );
		break ;
	  case 2:
		break ;
	}

	return ( 0 );
}
static int Action_Break_Die( Work *work, PROC_WORK *proc_work )
{
	return ( -1 );	/* 復帰不可 */
}
static int Action_Break_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Break_Act, Action_Break_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 手からバルカン（サポート攻撃版） */
static int Action_SAttack2_Act( Work *work, PROC_WORK *proc_work )
{
	extern void	*NewRayValcanBullet( FMATRIX *world, u_int type, u_int side,
						   u_int size, u_int damage, u_int length, u_int speed, int weapon );
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "SAttack2" );


	return ( 0 );
}
static int Action_SAttack2_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_SAttack2_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_SAttack2_Act, Action_SAttack2_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 目的地点まで移動 */
static int Action_Run_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float		len ;
	FVECTOR		tmp_vec ;
	short		rot_y ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Run" );

	/* 移動先座標を求める */
	//RAYSERVER_GetRayMovePosition( work, &work->move_target );
	work->move_target = work->info.standby_pos ;
	GTE_SubVector( &tmp_vec, &work->move_target, &ctrl->mov );
	len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
	{
		FVECTOR		x_vec, y_vec, angle_vec ;
		x_vec.vx = tmp_vec.vz ;
		y_vec.vx = tmp_vec.vx ;
		MT_Atan2X4( &angle_vec, &y_vec, &x_vec );
		rot_y = angle_vec.vx * 2048.0f / (float)M_PI ;
		ctrl->turn.vy = rot_y ;
	}

	switch ( proc_work->phase ){
	  case 0:
		if ( PDRAY_SetMotion( work, MOTION_RUN_START, 0 ) == -1 ) break ;
		SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );
		proc_work->phase++ ;
		work->true_action = ACTION_RUN ;
		work->true_action_bit = RAY_STATUS_STARTRUN ;
		break ;
	  case 1:
		if ( PDRAY_SetMotion( work, MOTION_RUN, 0 ) == -1 ) break ;
		proc_work->phase++ ;
		break ;
	  case 2:
		if ( len < ( RUN_WIDTH * 2 + RUN_STOP_RANGE ) ){
			if ( PDRAY_SetMotion( work, MOTION_RUN_STOP, 0 ) == -1 ) break ;
			proc_work->phase++ ;
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_STARTRUN );
		}
		break ;
	  case 3:
		break ;
	}

	return ( 0 );
}
static int Action_Run_Die( Work *work, PROC_WORK *proc_work )
{
	/* 走りが終了するまで切り替え禁止 */
	if ( proc_work->phase != 3 ) return ( -1 );
	return ( 0 );
}
static int Action_Run_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Run_Act, Action_Run_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
static int Action_Jump3_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	float	play_time, t ;
	FVECTOR		tmp_vec ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Jump3" );

	switch ( proc_work->phase ){
	  case 0:
		/* モーション発動までウェイトを入れる */
		if ( PDRAY_SetMotion( work, MOTION_JUMP, MOTION_FLAG_AGAIN ) == 1 ){
			proc_work->phase++ ;
			work->true_action = ACTION_JUMP3 ;
			work->true_action_bit = RAY_STATUS_JUMP3 ;
		}
		break ;
	  case 1:/* ジャンプ前のため */
		/* ジャンプ開始チェック */
		play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;
		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, JUMP_PHASE0_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 1 ;

			work->jump_start_pos = ctrl->mov ;
			//RAYSERVER_GetRayMovePosition( work, &work->jump_end_pos );
			work->jump_end_pos = work->info.standby_pos ;
			work->jump_start_pos.vy = 0.0f ;
			work->jump_end_pos.vy = 0.0f ;
			GTE_SubVector( &work->jump_offset, &work->jump_end_pos, &work->jump_start_pos );

			SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );

			/* ＳＥ */
			GM_SeSetMode( SD_E_R_FTFJ01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		}
		break ;
	  case 2:/* ジャンプ中 */
		play_time = body->m_ctrl->mt3_ctrl[ 0 ].play_time ;
		t = ( play_time - JUMP_PHASE0_TIME ) / ( JUMP_PHASE1_TIME - JUMP_PHASE0_TIME );
		if ( t > 1.0f ) t = 1.0f ;
		
		/* 先にジャンプ終了チェック */
		if ( MT_CHECK_MOTION_TIME( body->m_ctrl, 0, JUMP_PHASE1_TIME ) ){
			proc_work->phase++ ;
			work->chaff_disable = 0 ;
			work->flags &= ~FLAG_JUMP ;
			ctrl->skip_flag |= CTRL_RESET_HZX_BASE ;
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_JUMP3 );
			SET_LEG_IK_MODE( IK_MODE_FIX, IK_MODE_FIX );
			{/* 地震発生チェック */
				GTE_AddVector( &work->earthquake_pos, &work->right_leg_target_pos, &work->left_leg_target_pos );
				GTE_ScaleVector( &work->earthquake_pos, &work->earthquake_pos, 0.5f );
				work->earthquake_pos.vy = 0 ;
				work->earthquake_effect_flag = 2 ;
			}

			/* ＳＥ */
			GM_SeSetMode( SD_E_R_FTFG01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
		} else {
			work->flags |= FLAG_JUMP ;
			GTE_ScaleVector( &tmp_vec, &work->jump_offset, t );
			GTE_AddVector( &ctrl->mov, &work->jump_start_pos, &tmp_vec );
		}
		break ;
	  case 3:
		break ;
	}

	return ( 0 );
}
static int Action_Jump3_Die( Work *work, PROC_WORK *proc_work )
{
	/* ジャンプは何があってもキャンセルできない */
	if ( proc_work->phase != 3 ) return ( -1 );
	return ( 0 );
}
static int Action_Jump3_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Jump3_Act, Action_Jump3_Die );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* プレイヤーを正面に捕らえるように移動 */
static int Action_Step_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Step" );

	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetNextMoveAction( work, MOVE_FREE ) == -1 ) break ;
		proc_work->phase++ ;
		//work->true_action = ACTION_STEP ;
		//work->true_action_bit = RAY_STATUS_STEP ;
		break ;
	  case 1:/* 別のフェーズへ */
		if ( work->move_work.phase != -1 ){
			work->true_action = ACTION_STEP ;
			work->true_action_bit = RAY_STATUS_STEP ;
		}
		//if ( work->move_next_action != MOVE_FREE ){
		if ( work->move_work.mode != MOVE_FREE ){
			/* 移動１ステータスを消す */
			PDRAY_ResetDemandActionBit( work, RAY_STATUS_STEP );
			/* 無敵時間が残っている場合には消す */
			work->invinsible_time = 0 ;
		}
		break ;
	}

	/* 移動処理 */
	PDRAY_ActProc( work, &work->move_work );

	return ( 0 );
}
static int Action_Step_Die( Work *work, PROC_WORK *proc_work )
{
	/* ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_STEP );
	return ( 0 );
}
static int Action_Step_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Step_Act, Action_Step_Die );

	if ( work->on_stage_flag ){
		/* 移動先座標を求める */
		//work->move_target = work->stage_stand_pos ;
		/* 向く方向を設定 */
		work->dir_target = GM_PlayerPosition ;
		/* 移動タイプの設定 */
		work->move_type = 0 ;
	} else {
		/* 移動先座標を求める */
		//RAYSERVER_GetRayMovePosition( work, &work->move_target );
		//work->move_target = work->info.standby_pos ;
		/* 向く方向を設定 */
		work->dir_target = DG_ZeroVector ;
		/* 移動タイプの設定 */
		work->move_type = 1 ;
	}

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 水のみ */
static int Action_Drink_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Drink" );

	switch ( proc_work->phase ){
	  case 0:/* 移動先を向く */
		if ( PDRAY_SetMotion( work, MOTION_DRINK_START, 0 ) == -1 ) break ;
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		proc_work->phase++ ;
		work->true_action = ACTION_DRINK ;
		work->true_action_bit = RAY_STATUS_DRINK ;
		break ;
	  case 1:/* 別のフェーズへ */
		if ( PDRAY_SetMotion( work, MOTION_DRINK_LOOP, 0 ) == -1 ) break ;
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		proc_work->phase++ ;
		proc_work->count = 0 ;
		work->n_water_tank += 6 ;
		break ;
	  case 2:
		proc_work->count += TIME_BASE ;
		if ( proc_work->count > DRINK_TIME ){
			proc_work->phase++ ;
		}
		break ;
	  case 3:/* 別のフェーズへ */
		if ( PDRAY_SetMotion( work, MOTION_DRINK_END, 0 ) == -1 ) break ;
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
		proc_work->phase++ ;
		PDRAY_ResetDemandActionBit( work, RAY_STATUS_DRINK );
		break ;
	}

	return ( 0 );
}
static int Action_Drink_Die( Work *work, PROC_WORK *proc_work )
{
	if ( !( work->demand_action_bit & (RAY_STATUS_DAMAGE|RAY_STATUS_DAMAGE2|RAY_STATUS_DAMAGE3) ) ){
		/* ダメージ以外は終了するまで次に移行しない */
		if ( proc_work->phase <= 3 ){
			return ( -1 );
		}
	}
	/* ステータスを消す */
	PDRAY_ResetDemandActionBit( work, RAY_STATUS_DRINK );
	return ( 0 );
}
static int Action_Drink_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Drink_Act, Action_Drink_Die );

	return ( 0 );
}
/* ---------------------------------------------------------------- */
/* 場外での自分の立ち位置へ瞬間移動 */
static int Action_Teleport_Act( Work *work, PROC_WORK *proc_work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	SET_MARK( proc_work, "Teleport" );

	PDRAY_ResetDemandActionBit( work, RAY_STATUS_TELEPORT );

	return ( 0 );
}
static int Action_Teleport_Die( Work *work, PROC_WORK *proc_work )
{
	return ( 0 );
}
static int Action_Teleport_Init( Work *work, PROC_WORK *proc_work )
{
	PROC_SET_CALLBACK( proc_work, Action_Teleport_Act, Action_Teleport_Die );

	/* 移動先座標を求める */
	work->move_target = work->info.standby_pos ;
	work->control.mov = work->move_target ;
	PDRAY_SetMotion( work, MOTION_IDLE, MOTION_FLAG_FORCE|MOTION_FLAG_AGAIN|MOTION_FLAG_NOINTERP );
	GM_ConfigObjectAction( &work->object, 0, MOTION_IDLE, 300 - work->no * 100 + 10, ~0, 0 );
	SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );

	return ( 0 );
}

