//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdray.c
	量産型ＲＡＹ

	2001/03/22 K.Takabe
	$Id: pdray.c,v 1.1.1.3 2002/11/19 11:51:26 Yoshizawa1 Exp $

*/
/*

chara	量産型ＲＡＹ設置[NewPDRaySet] $s:name \
	-pos $v:設置座標 \
	-dir $v:初期方向 \
	-resource $s:モデル $s:モーション \
	-end_proc $p:終了proc 
// 終了procは実際には死んだときではなく、ライフが0になった直後に
// 呼ばれるので注意。（デモにつなぐため）

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

#include "r_common.h"
#include "pdray.h"

/* ---------------------------------------------------------------- */
/* ＲＡＹの行動をセット */
void PDRAY_SetDemandActionBit( Work *work, int action_bit )
{
	work->demand_action_bit |= action_bit ;
}
/* ＲＡＹの行動をリセット */
void PDRAY_ResetDemandActionBit( Work *work, int action_bit )
{
	work->demand_action_bit &= ~action_bit ;
}
/* サーバーが監視するためのゲーム状態の取得 */
int PDRAY_GetGameCondition( Work *work )
{
	return ( work->game_condition );
}
/* ＲＡＹのステータス表示 */
void PDRAY_DisplayStatus( Work *work )
{
#ifdef DEBUG_MODE
	//DEBUG_Locate( 32, 360 + work->no * 18, 0 );
	//DEBUG_Printf("%2d: %-16s.%-16s.%-16s\n", work->no, work->think1_mark, work->think2_mark, work->think3_mark );
	//DEBUG_Printf("%2d: %-16s.%-16s.%-16s\n", work->no, work->think1_work.mark,
	//			 work->think2_work.mark, work->think3_work.mark );
	DEBUG_Printf("%2d: %-16s.%-16s.%02d\n", work->no, work->think1_work.mark,
				 work->think2_work.mark, work->current_motion_num[0] );
	//DEBUG_Printf("    %-16s, (%08x)%02d,%02d,%02d\n",
	//			 work->action_mark, work->demand_action_bit, work->action, work->next_action, work->true_action );
	DEBUG_Printf("    %-16s, (%08x)%02d,%02d,%02d %04x\n",
				 work->action_work.mark, work->demand_action_bit, work->action_work.mode, work->action_work.next_mode, work->true_action,
				 work->info.command );

	if ( GV_PadData[ 1 ].press & PAD_L2 ){
		printf("no %d, %s\n", work->no, work->disp_name );
		PDRAY_DumpProc( &work->think1_work, "think1" );
		PDRAY_DumpProc( &work->think2_work, "think2" );
		PDRAY_DumpProc( &work->action_work, "action" );
		PDRAY_DumpProc( &work->move_work, "move" );
	}
#endif
}


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;

	DEBUG_Color( 192, 192, 192, 128 );
	DEBUG_Locate( 32, 200, 0 );
	GTE_InitGTE();

	ctrl = &work->control ;
	body = &work->object ;

	/* チャフのチェック */
	work->chaff_freeze_count++ ;
	if ( ( ( GM_GameStatus & STATE_CHAFF ) == 0 ) ||
		( work->chaff_disable != 0 ) ||
		( ( work->chaff_freeze_count & 3 ) != 0 ) ){
		/* 通常処理 */
		work->chaff_flag = 0 ;

		/* 思考処理 */
		PDRAY_ThinkAct( work );

		/* 行動の選択 */
		PDRAY_SelectAction( work );

		/* 行動処理 */
		PDRAY_ActProc( work, &work->action_work );

		if ( work->active_flag ){
			GM_ActMotion( body );
		}

		/* チャフ軋みＳＥの呼び出しタイミング変更 */
		if ( ( work->chaff_freeze_count & 7 ) == 0 ){
			work->chaff_se_timing = RND(8) ;
		}
	} else {
		/* チャフダメージ */
		if ( work->chaff_flag == 0 ){
		}
		work->chaff_flag = 1 ;

		/* チャフ軋みＳＥの呼び出し（タイミングの設定は正常動作中に行われる） */
		if ( work->on_stage_flag ){
			if ( ( work->chaff_freeze_count & 7 ) == work->chaff_se_timing ){
				GM_SeSetMode( SD_E_GISIGI01, (FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3], GM_SEMODE_BOMB ) ;
			}
		}

		ctrl->step.vx = 0.0f ;
		ctrl->step.vz = 0.0f ;

#if 0
		/* ＩＫぶらし値＆補完時間決定 */
		for ( i = 0 ; i < 4 ; i++ ){
			float		t ;
			if ( work->confusion_offset_count[i] <= 0 ){
				/* 新規オフセットと補完時間決定 */
				work->confusion_offset_trg[i].vx = RND( 500 ) - 250 ;
				work->confusion_offset_trg[i].vy = RND( 500 ) - 250 ;
				work->confusion_offset_trg[i].vz = RND( 500 ) - 250 ;
				work->confusion_offset_count[i] = RND( 2 ) + 2 ;
			}
			/* オフセットの補間処理 */
			t = 1.0f / (float)work->confusion_offset_count[i] ;
			GTE_InterVector( &work->confusion_offset[i],
							&work->confusion_offset[i], &work->confusion_offset_trg[i], t );
			GTE_AddVector( &work->confusion_pos[i], &work->confusion_base_pos[i], &work->confusion_offset[i] );
			if ( i != 0 ){
				GTE_AddVector( &work->confusion_pos[i], &work->confusion_pos[i], &work->confusion_offset[0] );
			}
			work->confusion_pos[i].vw = 1.0f ;
			work->confusion_offset_count[i]-- ;
		}
#endif
	}

	if ( work->active_flag ){
		if ( !( work->flags & FLAG_JUMP ) ){
			/* ジャンプ中でなければ落下速度の設定及び当たり判定用高さの設定を行う */
			ctrl->step.vy = -5000.0f ;
			ctrl->height = body->height ;
			//ctrl->mov.vy = body->height ;
			//ctrl->skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
			ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;
		} else {
			/* 何もしない */
			ctrl->mov.vy += body->height ;
			ctrl->step.vy = 0.0f ;
			ctrl->grounded = 0 ;
			ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;
		}
		GM_ActControl( ctrl );
		if ( !( work->flags & FLAG_JUMP ) ){
			float	base_height, foot ;
			base_height = ( work->flags & FLAG_ON_STAGE ) ? ( PLAYER_LEVEL ) : ( 0 ) ;
			foot = ctrl->mov.vy - ctrl->height ;
			//if ( foot < base_height ){
				ctrl->mov.vy += base_height - foot ;
			//}
		}
		{/* Ｙを補正したマトリクスを腰に設定する */
			SVECTOR	rot ;
			DG_GetPos( &work->world );	/* 腰補正前のマトリクスを保存しておく */
			/* 補正方向のアニメーション */
//printf("a %d %d %d %d %d\n", work->no, ctrl->rot.vy, work->think_player_angle, work->adjust_rot_y, work->adjust_target_rot_y );
			if ( !work->chaff_flag ){
				work->adjust_rot_y = GV_NearExp8( work->adjust_rot_y, work->adjust_target_rot_y );
				work->adjust_target_rot_y = 0 ;
			}
			work->adjust_rot_y = GTE_REGULAR_ANGLE( work->adjust_rot_y );
//printf("b %d %d %d %d\n", work->no, work->think_player_angle, work->adjust_rot_y, work->adjust_target_rot_y );
			if ( work->adjust_rot_y > MAX_ADJUST_ANGLE ) work->adjust_rot_y = MAX_ADJUST_ANGLE ;
			if ( work->adjust_rot_y < -MAX_ADJUST_ANGLE ) work->adjust_rot_y = -MAX_ADJUST_ANGLE ;
			/* 腰回転を補正したマトリクスの作成 */
//printf("c %d %d %d %d\n", work->no, work->think_player_angle, work->adjust_rot_y, work->adjust_target_rot_y );
			rot = ctrl->rot ;
			rot.vy += work->adjust_rot_y ;
			DG_SetPos2( &ctrl->mov, &rot );
		}
		GM_ActObject2( body );

		/* ＬＯＤ制御 */
		body->objs->flag &= ~DG_FLAG_INVISIBLE ;
		if ( work->standby_lod_objs != NULL ){
			work->standby_lod_objs->flag |= DG_FLAG_INVISIBLE ;
		}
		if ( work->broken_lod_objs != NULL ){
			work->broken_lod_objs->flag |= DG_FLAG_INVISIBLE ;
		}
	} else {
		/* ＬＯＤ制御 */
		body->objs->flag |= DG_FLAG_INVISIBLE ;
		if ( work->life > 0 ){
			if ( work->standby_lod_objs != NULL ){
				work->standby_lod_objs->flag &= ~DG_FLAG_INVISIBLE ;
				work->standby_lod_objs->world = work->object.objs->world ;
			}
			if ( work->broken_lod_objs != NULL ){
				work->broken_lod_objs->flag |= DG_FLAG_INVISIBLE ;
			}
		} else {
			if ( work->standby_lod_objs != NULL ){
				work->standby_lod_objs->flag |= DG_FLAG_INVISIBLE ;
			}
			if ( work->broken_lod_objs != NULL ){
				work->broken_lod_objs->flag &= ~DG_FLAG_INVISIBLE ;
				work->broken_lod_objs->world = work->object.objs->world ;
			}
		}
	}

	if ( work->life <= 0 ){
		work->disable_head_ik_flag = 1 ;/* これはモーション切り替え時に戻すようにする */
	}

	GTE_InitGTE();

	/* ＩＫ関連処理の実行 */
	PDRAY_ActIk( work );

	/* モーション設定 */
	//SetMotion( body->objs, work->rots );

	/* あたり判定関連処理 */
	PDRAY_ActCollision( work );

	if ( work->active_flag ){
		/* ライトマトリクスの取得 */
		DG_GetLightMatrix( &ctrl->mov, work->light );
	}

	/* エフェクト関連処理実行 */
	PDRAY_ActEffect( work );


#ifdef MOTION_BLUR
	for ( i = 0 ; i < BLUR_NUM ; i++ ){
		int		offset ;
		offset = BLUR_INTERVAL * i + BLUR_INTERVAL - 1 ;
		work->blur_objs[i]->world = work->blur_pos[ offset ];
		SetMotion( work->blur_objs[i], work->blur_rots[ offset ] );
	}
	for ( i = BLUR_NUM * BLUR_INTERVAL - 1 ; i > 0  ; i-- ){
		work->blur_pos[ i ] = work->blur_pos[ i - 1 ] ;
		DG_StartMemToSpr( SCRPAD_ADDR, work->blur_rots[ i - 1 ], PDRAY_MAX_JOINTS );
		DG_EndMemToSpr();
		DG_StartSprToMem( work->blur_rots[ i ], SCRPAD_ADDR, PDRAY_MAX_JOINTS );
		DG_EndSprToMem();
	}
	work->blur_pos[ 0 ] = body->objs->world ;
	DG_StartMemToSpr( SCRPAD_ADDR, work->abs_rots, PDRAY_MAX_JOINTS );
	DG_EndMemToSpr();
	DG_StartSprToMem( work->blur_rots[ 0 ], SCRPAD_ADDR, PDRAY_MAX_JOINTS );
	DG_EndSprToMem();
#endif

	/* 攻撃禁止カウントの減算 */
	if ( ( work->disable_attack1_count -= TIME_BASE ) < 0 ) work->disable_attack1_count = 0 ;
	if ( ( work->disable_attack2_count -= TIME_BASE ) < 0 ) work->disable_attack2_count = 0 ;
	if ( ( work->disable_attack3_count -= TIME_BASE ) < 0 ) work->disable_attack3_count = 0 ;
	if ( ( work->disable_attack4_count -= TIME_BASE ) < 0 ) work->disable_attack4_count = 0 ;

	/* サーバーに返すパラメータを設定 */
	work->info.ray_life = work->life * 65536 / MAX_LIFE ;
	//work->info.status = work->status ;	/* 内容は保留 */
	work->info.n_water_tank = work->n_water_tank ;
	work->info.pos = ctrl->mov ;
	work->info.rot = ctrl->rot ;
	work->info.adjust_rot = work->adjust_rot_y ;
	work->info.condition = work->game_condition ;

	/* 赤外線関連対処 */
#if 0
	if ( GM_Item == IT_Thermal ){
		body->objs->flag |= DG_FLAG_FOGPARAM ;
		DG_SetFogParamObjs( body->objs, 0, 1000000 );
		work->standby_lod_objs->flag |= DG_FLAG_FOGPARAM ;
		DG_SetFogParamObjs( work->standby_lod_objs, 0, 1000000 );
	} else {
		body->objs->flag &= ~DG_FLAG_FOGPARAM ;
		work->standby_lod_objs->flag &= ~DG_FLAG_FOGPARAM ;
	}
#endif

}
/* ---------------------------------------------------------------- */
	/*
		終了
	*/
static void Die( Work *work )
{
	PDRAY_EndEffect( work );
	PDRAY_EndCollision( work );
	PDRAY_EndIk( work );
	GM_FreeObject( &work->object );
	GM_FreeControl( &work->control );
	if ( work->standby_lod_objs != NULL ){
		DG_DequeueObjs( work->standby_lod_objs );
		DG_FreeObjs( work->standby_lod_objs );
	}
	if ( work->broken_lod_objs != NULL ){
		DG_DequeueObjs( work->broken_lod_objs );
		DG_FreeObjs( work->broken_lod_objs );
	}
#ifdef MOTION_BLUR
	{
		int		i ;
		for ( i = 0 ; i < BLUR_NUM ; i++ ){
			DG_DequeueObjs ( work->blur_objs[ i ] );
			DG_FreeObjs( work->blur_objs[ i ] );
		}
	}
#endif
}
/* ---------------------------------------------------------------- */
	/*
		初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			model_id, motion_id ;
	FVECTOR		pos ;
	SVECTOR		rot ;

	work->name = name ;
	work->map = where ;

	work->life_max = work->life = MAX_LIFE ;

	if ( GCL_GetOption( 'r' ) != NULL ){
		model_id = GCL_GetNextInt();
		motion_id = GCL_GetNextInt();
	} else {
		printf("%s:no resource\n", __FILE__ );
		return ( -1 );
	}
	if ( GCL_GetOption( 'p' ) != NULL ){
		pos.vx = GCL_GetNextInt();
		pos.vy = GCL_GetNextInt();
		pos.vz = GCL_GetNextInt();
	}
	if ( GCL_GetOption( 'd' ) != NULL ){
		rot.vx = GCL_GetNextInt();
		rot.vy = GCL_GetNextInt();
		rot.vz = GCL_GetNextInt();
	}
	if ( GCL_GetOption( 'e' ) != NULL ){
		work->proc_id = GCL_GetNextInt();
	}

	ctrl = &work->control ;
	body = &work->object ;

	GM_InitControlEx( ctrl, name, where, CTRL_FLAG );	/* コントロール初期化 */
	GM_ConfigControlPosition( ctrl, &pos, &rot );		/* 位置初期化 */
	GM_ConfigControlObject( ctrl, body );				/* オブジェクトとの関連付け */
	GM_ConfigControlHazard( ctrl, 1000, 1000, 1000 );	/* 当たり関連初期化（今は仮！！） */
	GM_ConfigControlHzxHeight( ctrl, 1000, 1000 );		/* 当たり判定逆さ初期化（今は仮！！） */
	GM_ConfigControlHzxCheckFlag( ctrl, 0, 0 );			/* （今は仮！！） */

	GM_InitObject( body, model_id, BODY_FLAG );			/* オブジェクト初期化 */
	GM_ConfigObjectLight( body, work->light );			/* ライトマトリクス設定 */
	GM_ConfigObjectStep( body, &ctrl->step );			/* モーション反映用にステップ変数設定 */
	GM_ConfigObjectMotion( body, 2, motion_id, MT_FLAG_PDRAY );		/* モーション割り当て */
	GM_ConfigObjectAction( body, 0, MOTION_STANDBY, 0, ~0, 0 );

	GM_ActMotion( body );
	ctrl->height = body->height ;
	GM_ActControl( ctrl );
	GM_ActObject2( body );
	DG_PutObjs( body->objs );

	{/* ＬＯＤ用モデル生成 */
		DG_DEF	*def ;
		DG_OBJS	*objs ;
		def = GV_GetCache( GV_CacheID( STANDBY_MODEL_NAME, 'k' ) );
		if ( def != NULL ){
			work->standby_lod_objs = objs = DG_MakeObjs( def, MODEL_FLAG|DG_FLAG_IRREACTION, 0 );
			DG_QueueObjs( objs );
			objs->flag |= DG_FLAG_INVISIBLE ;
		}
		def = GV_GetCache( GV_CacheID( BROKEN_MODEL_NAME, 'k' ) );
		if ( def != NULL ){
			work->broken_lod_objs = objs = DG_MakeObjs( def, MODEL_FLAG, 0 );
			DG_QueueObjs( objs );
			objs->flag |= DG_FLAG_INVISIBLE ;
		}
	}


	/* 基準設置位置マトリクス初期化 */
	work->world = body->objs->world ;
	work->world.m[3][1] = 0.0f ;

	/* サーバーに対して自分を登録する */
	work->no = RAYSERVER_AddRayControl( work, &work->info, ctrl );

	/* ＩＫ関連初期化 */
	PDRAY_InitIk( work );

	PDRAY_ResetAction( work );
	PDRAY_SetNextAction( work, ACTION_SLEEP );
	///PDRAY_UpdateActionStatus( work );
	PDRAY_ResetMoveAction( work ) ;
	PDRAY_SetNextMoveAction( work, MOVE_IDLE );
	//PDRAY_UpdateMoveActionStatus( work );

	/* あたり判定初期化 */
	PDRAY_InitCollision( work );

	/* 思考処理の初期化 */
	PDRAY_InitThink( work );

	work->game_condition = GAME_CONDITION_WAIT ;

	/* エフェクト関連初期化 */
	//PDRAY_InitEffect( work );

	/* その他パラメータ設定 */
	work->n_water_tank = 3 ;		/* 初期水圧カッター使用回数を３に設定 */

#ifdef MOTION_BLUR
	{
		int		i ;
		for ( i = 0 ; i < BLUR_NUM ; i++ ){
			work->blur_objs[ i ] = DG_MakeObjs( body->objs->def, body->objs->flag | DG_FLAG_SEMITRANS, 0 );
			work->blur_objs[ i ]->flag &= ~DG_FLAG_MULTITEX ;
			DG_QueueObjs ( work->blur_objs[ i ] );
		}
		body->objs->flag &= ~DG_FLAG_MULTITEX ;
	}
#endif

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewPDRaySet( int name, int where )
{
	Work *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 1 );
	if ( work != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ){
			GV_DestroyActor( work );
			return ( NULL );
		}
	}
	return ( work );
}

