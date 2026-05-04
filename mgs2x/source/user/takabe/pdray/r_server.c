//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	r_server.c
	量産型ＲＡＹ用ＡＩサーバー

	2001/04/09 K.Takabe
	$Id: r_server.c,v 1.1.1.3 2002/11/19 11:51:27 Yoshizawa1 Exp $

*/
/*

chara	ＲＡＹサーバー設置[NewPDRayServerSet] $s:name \
	-num $w:基準破壊数 \
	-proc $p:終了Proc \
	-flag $w:フラグ
// 量産型ＲＡＹ設置よりも先に起動してください
// 終了Procは基準破壊数に相当するダメージをＲＡＹに与えた場合に呼ばれる

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
#include	"camera.h"

#include	"../other/vec_util.h"
#include	"r_common.h"

#ifndef PSX2
#undef DEBUG_MODE
#endif

#define MAX_LIFE			(1024)		/* ＲＡＹの最大ライフ *//* 本来はpdray.h内に記述 */

/* ---------------------------------------------------------------- */
#define MAX_RAYS	(32)

#define CIRCLE_RANGE		(35000.0f)	/* ＲＡＹ立ち位置円半径 */
//#define MAX_CIRCLE_POINTS	(36)		/* ＲＡＹ立ち位置点数 */
//#define NEAR_CIRCLE_STEP	(5)			/* ＲＡＹの最小立ち位置間隔 */
#define MAX_CIRCLE_POINTS	(6)		/* ＲＡＹ立ち位置点数 */
#define NEAR_CIRCLE_STEP	(1)			/* ＲＡＹの最小立ち位置間隔 */

#define AI_CHANGE_INTERVAL	(100)		/* ＡＩ管理切り替え間隔 */

#define MAX_CAMERA_TURN	(45)

#ifdef DEBUG_MODE
#define GAME_MARK( _s )	{work->game_mark = _s ;}
#else
#define GAME_MARK( _s )
#endif

//#define RND( _n )	( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 15 )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )

#ifndef ABS
#define ABS( _n )	( ( (_n) < 0 ) ? -(_n) : (_n) )
#endif

/* ---------------------------------------------------------------- */
/* ＲＡＹコントロール用ワーク（３体分） */
typedef struct {
	int			status ;		/* 現在の状態 */
	int			command ;		/* 実行要求コマンド */
	int			pos_angle ;		/* ＲＡＹの立ち位置（極座標） */
	int			pos_length ;	/* ＲＡＹの立ち位置（極座標） */
	int			list_num ;		/* 関連付けを行ったＲＡＹリスト番号 */
	RAY_INFO	*info ;
} RAY_CONTROL ;

/* サーバーが管理するＲＡＹのリスト構造体 */
typedef struct _ray_list {
	void		*work ;			/* ＲＡＹワーク */
	RAY_INFO	*info ;			/* 情報交換ワーク */
	CONTROL		*ctrl ;			/* コントロール */
	int			no ;			/* ＩＤ番号 */
	int			flag ;			/* 使用フラグ */
} RAY_LIST ;

/* ---------------------------------------------------------------- */
typedef struct _work{
	GV_ACT_EX		actor ;
	int				name ;
	int				map ;
	int				hzx_id ;

	/* ゲーム進行管理 */
	int				game_num ;					/* 実行中ゲームフェーズ */
	int				game_count ;				/* ゲームカウント */
	int				game_phase ;				/* ゲームフェーズ */
	int				game_phase_state ;			/* フェーズ内の実行状態 */
	int				game_phase_count ;			/* ゲームフェーズ内でのカウント */
	int				game_phase_end_count ;
	int				game_ai_change_time ;		/* ＡＩ切り替え時間 */
	int				game_ai_target_num ;		/* ＡＩ対象番号 */
	/* フラグ関連 */
	int				on_select_in_attack ;		/* ステージ内攻撃ＲＡＹ選択フラグ */
	int				demand_next_phase_flag ;	/* フェーズ進行要求フラグ */
	int				demand_next_step_flag ;		/* ステップ進行要求フラグ */
	int				demand_out_attack_flag ;	/* ステージ外攻撃要求フラグ */

	/* ロックオン処理 */
	int				lock_on_ray_num ;			/* ロックオンしているＲＡＹ番号 */
	int				enable_lockon ;				/* カメラロックオン有効化 */
	SVECTOR			camera_rotate ;				/* カメラ角度 */
	GM_CameraSet	*camera ;					/* カメラ制御用 */
	FVECTOR			cam_player_tpos ;			/* カメラ用プレイヤー位置 */
	FVECTOR			cam_player_pos ;			/* 実際のカメラ用プレイヤー位置 */
	FVECTOR			cam_pdray_tpos ;			/* カメラ用ＲＡＹ位置 */
	FVECTOR			cam_pdray_pos ;				/* 実際のカメラ用ＲＡＹ位置 */
	int				cam_diff_angle ;			/* 現在のカメラとＲＡＹの角度差 */
	int				camera_mode ;				/* カメラモード */

	/* プレイヤー監視関連 */
	int				record_count ;			/* 記録回数 */
	FVECTOR			old_player_pos ;		/* 前回のプレイヤー座標 */
	FVECTOR			player_speed ;			/* プレイヤーの移動スピード（１０フレーム置きの更新） */

	/* プレイヤー攻撃監視関連 */
	int				old_weapon_alive_flag ;	/* 前回のスティンガーミサイル生存フラグ */
	FVECTOR			old_stinger_pos ;		/* 前回のスティンガーミサイル位置（速度算出用） */
	FVECTOR			stinger_speed ;			/* スティンガー速度 */

	/* ミサイル警告音関連 */
	float			missile_near_len ;		/* ミサイル最近距離 */
	int				missile_count ;			/* 効果音カウンタ */

	/* ＲＡＹ管理 */
	int				n_rays ;
	RAY_LIST		ray_lists[ MAX_RAYS ];

	int				last_in_attack_num ;		/* 最後にステージ内攻撃を行ったキャラ番号 */
	int				current_max_rays ;			/* 現在動いているＲＡＹ数 */
	int				current_n_rays ;			/* 現在待機中ＲＡＹ数 */
	int				use_ray_count ;

	RAY_CONTROL		ray_control[3] ;
	int				attacker ;					/* ステージ内攻撃中ＲＡＹ番号 */
	int				supporter[2] ;				/* 補助ＲＡＹ番号 */

	int				next_common_status ;		/* 次のフレームに設定する共通ステータス保持用 */

	DG_OBJS			*number_parts_r[6] ;		/* ナンバリング用モデル右足 */
	DG_OBJS			*number_parts_l[6] ;		/* ナンバリング用モデル左足 */
//#ifdef PSX2
	DG_TEX_MOVEREPLACE	*number_replace[6] ;	/* テクスチャ入れ替え */
//#endif
	int				number_free_buffer[6] ;		/* モデル空リストバッファ */
	int				number_count ;				/* ナンバー使用回数 */

	/* シナリオ連携 */
	int				end_proc ;					/* 終了プロック */
	int				mark_num ;					/* 破壊目標数 */
	int				total_life ;				/* 破壊目標数から求めたクリアまでの総耐久値 */

	/* デバッグ用パッド入力 */
	unsigned int	status ;
	unsigned int	press ;
	unsigned int	release ;
	unsigned int	toggle ;
	unsigned int	old_status ;

	char			*game_mark ;

} Work ;

static Work	*work_ptr = NULL;
int		RAYSERVER_CommonStatus ;

int		RAYSERVER_GameLevel ;			/* ゲームレベル（1,2,3,4,5） */
int		RAYSERVER_GameLevelOffset ;		/* ゲームレベルオフセット（-2,-1,0,1,2） */
int		RAYSERVER_GameDownLevel ;		/* ゲーム難易度低下レベル（2,1,0,0,0） */
int		RAYSERVER_GameUpLevel ;			/* ゲーム難易度上昇レベル（0,0,0,1,2） */



/* ---------------------------------------------------------------- */
//#define FLOOR_FLAG	(HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_IK)
#define FLOOR_FLAG	(HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_BULLETHOLE)

#define PHASE_PASSAGE()	{work->game_phase_count += TIME_BASE ;}
#ifdef __GNUC__
#define PHASE_TIME_CHECK( _n ) \
({\
	int _f = 0 ; \
	if ( work->game_phase_count >= (_n) && work->game_phase_end_count < (_n) ){\
		work->game_phase_end_count = work->game_phase_count ;\
		_f = 1 ;\
	}\
	(_f);\
})
#else
#define PHASE_TIME_CHECK( _n ) _phase_time_check( work, _n )
static inline int _phase_time_check( Work *work, int _n )
{
	int _f = 0 ; 
	if ( work->game_phase_count >= (_n) && work->game_phase_end_count < (_n) ){
		work->game_phase_end_count = work->game_phase_count ;
		_f = 1 ;
	}
	return (_f);
}
#endif
#define CHANGE_PHASE( _n )	\
{\
   work->game_phase = _n ;\
   work->game_phase_count = 0 ;\
   work->game_phase_end_count = 0 ;\
}
#define RESET_CHECK_TIME()	\
{\
   work->game_phase_count = 0 ;\
   work->game_phase_end_count = 0 ;\
}


/* ---------------------------------------------------------------- */
extern void PDRAY_SetDemandActionBit( Work *work, int action_bit );
extern void PDRAY_ResetDemandActionBit( Work *work, int action_bit );
extern int PDRAY_GetGameCondition( Work *work );
extern void PDRAY_DisplayStatus( Work *work );
/* ---------------------------------------------------------------- */
#ifdef DEBUG_MODE
//void GM_AddDebugMenu( GM_DEBUG_MENU *menu );
static int	PDRAY_StatusDisplayFlag ;
static int	PDRAY_CollisionEditFlag ;
static GM_DEBUG_MENU debug_statusdisplay = {
  class:	"PDRAY",
  menu:		"STATUS DISP",
  max:		2,
  items:	( char *[] ){ "ON", "OFF" },
  values:	( int [] ){ 1, 0 },
  target:	&PDRAY_StatusDisplayFlag,
  type:		GM_DEBUG_MENU_FLAG,
};
static GM_DEBUG_MENU debug_collisionedit = {
  class:	"PDRAY",
  menu:		"COLLISION EDIT",
  max:		2,
  items:	( char *[] ){ "ON", "OFF" },
  values:	( int [] ){ 1, 0 },
  target:	&PDRAY_CollisionEditFlag,
  type:		GM_DEBUG_MENU_FLAG,
};
static void DebugAct( GV_ACT_EX *debug_work )
{
	int		i ;
	RAY_CONTROL	*r_ctrl ;
	if ( work_ptr == NULL ) return ;
	DEBUG_Locate( 32, 280, 0 );
	if ( PDRAY_StatusDisplayFlag ){
		Work *work = work_ptr ;
		DEBUG_Printf( "%s %d \n", work->game_mark, work->game_num );
		DEBUG_Printf( "%d %d %d\n", work->current_max_rays, work->current_n_rays, work->last_in_attack_num );

		for ( i = 0 ; i < 3 ; i++ ){
			//r_ctrl = GetRayControl( work, i );
			r_ctrl = &work->ray_control[ i ];
			if ( r_ctrl->list_num != -1 ){
				PDRAY_DisplayStatus( work->ray_lists[ r_ctrl->list_num ].work );
			}
		}
	}
	DEBUG_Locate( 32, 200, 0 );
	if ( PDRAY_CollisionEditFlag ){
		extern void PDRAY_CollisionEdit( Work *work );
		Work *work = work_ptr ;
		for ( i = 0 ; i < 3 ; i++ ){
			//r_ctrl = GetRayControl( work, i );
			r_ctrl = &work->ray_control[ i ];
			if ( r_ctrl->list_num != -1 ){
				PDRAY_CollisionEdit( work->ray_lists[ r_ctrl->list_num ].work );
				break ;
			}
		}
	}
}
void *NewRServerDebugActor( void )
{
	GV_ACT_EX *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, sizeof(GV_ACT_EX), 0 );
	if ( work != NULL ){
		GV_SetActor( work, DebugAct, NULL );
		GV_ActorEX( work );
		GM_AddDebugMenu( &debug_statusdisplay );
		GM_AddDebugMenu( &debug_collisionedit );
	}
	return ( work );
}
#endif
/* ---------------------------------------------------------------- */
enum {/* これは現段階ではあまり意味がないので作り直す必要あり */
	RAY_STATUS_NONE			= 0x00000000,		/* 何もなし */
	RAY_STATUS_ACTIVE		= 0x00000001,		/* 起動済み */
	RAY_STATUS_FIRST		= 0x00000002,		/* 起動済み */
};
enum {
	GAME_PHASE_START,		/* 起動フェーズ */
	GAME_PHASE_STANDBY,		/* 攻撃開始待ち */
	GAME_PHASE_ATTACK1,		/* ３体連続攻撃 */
	GAME_PHASE_ATTACK2,		/* １体ステージ上に上って攻撃 */
	GAME_PHASE_ATTACK3,		/* １体ステージ上に上って攻撃 */
	GAME_PHASE_ATTACK4,		/* １体ステージ上に上って攻撃 */
	GAME_PHASE_POSITIONING,	/* ３体とも攻撃位置変更 */
	GAME_PHASE_END,			/*  */
};

static int game_phase_pattern_list [] = {
	GAME_PHASE_START,
	GAME_PHASE_STANDBY,
	GAME_PHASE_ATTACK3,
	GAME_PHASE_ATTACK4,
	GAME_PHASE_ATTACK4,
	//GAME_PHASE_ATTACK2,
	-1,2
};
/* ---------------------------------------------------------------- */
#define T_SEC	(300)
static int attack_table1[][6] = {
	/* time     ray1                 ray2                 ray3               cam_mode cam_num */
	{ T_SEC*0 , RAY_COMMAND_NULL   , RAY_COMMAND_NULL   , RAY_COMMAND_NULL   , 0, -1 },
	{ T_SEC*1 , RAY_COMMAND_ATTACK1, RAY_COMMAND_NULL   , RAY_COMMAND_NULL   , 0, -1 },
	{ T_SEC*5 , RAY_COMMAND_NULL   , RAY_COMMAND_ATTACK1, RAY_COMMAND_NULL   , 0, -1 },
	{ T_SEC*9 , RAY_COMMAND_NULL   , RAY_COMMAND_NULL   , RAY_COMMAND_ATTACK1, 0, -1 },
	{ T_SEC*13, RAY_COMMAND_NULL   , RAY_COMMAND_NULL   , RAY_COMMAND_NULL   , 0, -1 },
	{ -1, 0, 0, 0, 0, -1 },
};

/* ---------------------------------------------------------------- */
static RAY_CONTROL* GetRayControl( Work *work, int num )
{
	RAY_CONTROL		*r_ctrl ;
	RAY_LIST		*r_list ;
	int				i ;

	r_ctrl = &work->ray_control[ num ] ;
	if ( r_ctrl->list_num == -1 ){
		/* 管理用ＲＡＹが割り当てられてない場合にまだＲＡＹが存在すればそれを割り当てる */
		for ( i = 0 ; i < work->n_rays ; i++ ){/* とりあえず全検索 */
			r_list = &work->ray_lists[ i ] ;
			if ( r_list->flag == 0 ){
				r_list->flag = 1 ;
				r_ctrl->list_num = i ;
				r_ctrl->info = r_list->info ;
				r_ctrl->status &= ~RAY_STATUS_FIRST ;
				break ;
			}
		}
		//if ( i == work->n_rays ) return ( NULL );
	}
	return ( &work->ray_control[ num ] );
}
/* ---------------------------------------------------------------- */
static void ExecProc( int proc_id, int param0 )
{
	GCL_ARGS	arg ;
	int			data[4] ;

	if ( proc_id == 0 ) return ;
	arg.argc = 1 ;
	arg.argv = data ;
	data[0] = param0 ;
	GCL_ExecProc( proc_id, &arg );
}
/* ---------------------------------------------------------------- */
static void ChangeGamePhase( Work *work )
{
	while ( 1 ){
		work->game_phase = game_phase_pattern_list[ work->game_num++ ] ;
		if ( work->game_phase == -1 ){
			work->game_num = game_phase_pattern_list[ work->game_num ] ;
			continue ;
		}
		break ;
	}
	work->game_phase_count = 0 ;
	work->game_phase_end_count = 0 ;
	work->game_phase_state = -1 ;
}
/* ---------------------------------------------------------------- */
	/*
		ＲＡＹ位置の極座標管理ユーティリティ
	*/
/* 極座標をユークリッド座標に変換 */
static void CalcPosition( FVECTOR *pos, int length, int angle )
{
	FVECTOR	tmp_vec ;
	GTE_SinCos( &tmp_vec, GTE_PS2RAD( angle ) );
	pos->vx = tmp_vec.vx ;
	pos->vy = 0 ;
	pos->vz = tmp_vec.vy ;
	GTE_ScaleVector( pos, pos, (float)length );
}
#if 0
/* ＲＡＹの目標座標をセット */
static void SetRayPosition( RAY_CONTROL *r_ctrl, FVECTOR *pos )
{
	FVECTOR		tmp_vec ;
	float		len, angle ;
	tmp_vec = *pos ;
	tmp_vec.vy = 0 ;
	len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
	angle = atan2f( tmp_vec.vx, tmp_vec.vz ) ;
	r_ctrl->pos_length = DG_FTOI( len ) ;
	r_ctrl->pos_angle = GTE_RAD2PS( angle );
}
#endif
/* ---------------------------------------------------------------- */
/* 新起動用座標設定処理 */
void SetRayNewPosition( Work *work, int num )
{
	RAY_CONTROL	*r_ctrl, *other_r_ctrl ;
	FVECTOR		pos, other_pos, tmp_vec ;
	float		len ;
	int		i, phase = 0 ;

#if 1
	r_ctrl = &work->ray_control[ num ];
	CalcPosition( &pos, r_ctrl->pos_length, r_ctrl->pos_angle );
	for ( i = 0 ; i < 3 ; i++ ){
		if ( i == num ) continue ;
		other_r_ctrl = &work->ray_control[ i ] ;
		CalcPosition( &other_pos, other_r_ctrl->pos_length, other_r_ctrl->pos_angle );
		GTE_SubVector( &tmp_vec, &pos, &other_pos );
		len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		printf("%d,%d: %d %d %d %d, %f\n", num, i, r_ctrl->pos_length, r_ctrl->pos_angle, other_r_ctrl->pos_length, other_r_ctrl->pos_angle, len );
		if ( len < 5000.0f ){
			printf("%s: new ray position near (%d)!!\n", __FILE__, phase );
			/* 近すぎるので場所を変更する */
			switch ( phase ){
			  case 0:
				r_ctrl->pos_angle = 2048 - 150 ;
				r_ctrl->pos_length = CIRCLE_RANGE + 10000 ;
				break ;
			  case 1:
				r_ctrl->pos_angle = 2048 + 150 ;
				r_ctrl->pos_length = CIRCLE_RANGE + 10000 ;
				break ;
			  case 2:
				r_ctrl->pos_angle = 2048 ;
				r_ctrl->pos_length = CIRCLE_RANGE + 20000 ;
				break ;
			}
			CalcPosition( &pos, r_ctrl->pos_length, r_ctrl->pos_angle );
			i = -1 ;
			phase++ ;
		}
	}
#else
	r_ctrl = &work->ray_control[ num ];
	switch ( r_ctrl->list_num % 3 ){
	  case 0:
		r_ctrl->pos_angle = 2048 ;
		r_ctrl->pos_length = CIRCLE_RANGE ;
		break ;
	  case 1:
		r_ctrl->pos_angle = 2048 - 300 ;
		r_ctrl->pos_length = CIRCLE_RANGE ;
		break ;
	  case 2:
		r_ctrl->pos_angle = 2048 + 300 ;
		r_ctrl->pos_length = CIRCLE_RANGE ;
		break ;
	}
#endif
}
/* ---------------------------------------------------------------- */
	/*
		起動開始フェーズ
	*/
static void ActPhase_Start( Work *work )
{
	RAY_CONTROL		*r_ctrl ;

	GAME_MARK( "start" );
	PHASE_PASSAGE();
#if 0
	if ( PHASE_TIME_CHECK( 50 ) ){
		r_ctrl = GetRayControl( work, 0 );
		if ( ( r_ctrl->list_num != -1 ) && ( ( r_ctrl->status & RAY_STATUS_ACTIVE ) == 0 ) ){
			r_ctrl->pos_angle = 2048 ;
			r_ctrl->pos_length = CIRCLE_RANGE ;
			r_ctrl->status = RAY_STATUS_ACTIVE ;
		}
	} else if ( PHASE_TIME_CHECK( 100 ) ){
		r_ctrl = GetRayControl( work, 1 );
		if ( ( r_ctrl->list_num != -1 ) && ( ( r_ctrl->status & RAY_STATUS_ACTIVE ) == 0 ) ){
			r_ctrl->pos_angle = 2048 - 300 ;
			r_ctrl->pos_length = CIRCLE_RANGE + 10000 ;
			r_ctrl->status = RAY_STATUS_ACTIVE ;
		}
	} else if ( PHASE_TIME_CHECK( 150 ) ){
		r_ctrl = GetRayControl( work, 2 );
		if ( ( r_ctrl->list_num != -1 ) && ( ( r_ctrl->status & RAY_STATUS_ACTIVE ) == 0 ) ){
			r_ctrl->pos_angle = 2048 + 300 ;
			r_ctrl->pos_length = CIRCLE_RANGE + 10000 ;
			r_ctrl->status = RAY_STATUS_ACTIVE ;
		}
	} else if ( PHASE_TIME_CHECK( 600 ) ){
		ChangeGamePhase( work );
	}
#else
	r_ctrl = GetRayControl( work, 0 );
	if ( ( r_ctrl->list_num != -1 ) && ( ( r_ctrl->status & RAY_STATUS_ACTIVE ) == 0 ) ){
		r_ctrl->pos_angle = 2048 ;
		r_ctrl->pos_length = CIRCLE_RANGE ;
		r_ctrl->status = RAY_STATUS_ACTIVE|RAY_STATUS_FIRST ;
	}
	r_ctrl = GetRayControl( work, 1 );
	if ( ( r_ctrl->list_num != -1 ) && ( ( r_ctrl->status & RAY_STATUS_ACTIVE ) == 0 ) ){
		r_ctrl->pos_angle = 2048 - 300 ;
		r_ctrl->pos_length = CIRCLE_RANGE + 10000 ;
		r_ctrl->status = RAY_STATUS_ACTIVE|RAY_STATUS_FIRST ;
	}
	r_ctrl = GetRayControl( work, 2 );
	if ( ( r_ctrl->list_num != -1 ) && ( ( r_ctrl->status & RAY_STATUS_ACTIVE ) == 0 ) ){
		r_ctrl->pos_angle = 2048 + 300 ;
		r_ctrl->pos_length = CIRCLE_RANGE + 10000 ;
		r_ctrl->status = RAY_STATUS_ACTIVE|RAY_STATUS_FIRST ;
	}
	ChangeGamePhase( work );
#endif
}
	/*
		スタンバイフェーズ
	*/
static void ActPhase_Standby( Work *work )
{
	RAY_CONTROL		*r_ctrl ;
	//int				i, flag ;

	GAME_MARK( "standby" );
	PHASE_PASSAGE();

#if 0
	flag = -1 ;
	for ( i = 0 ; i < 3 ; i++ ){
		//r_ctrl = GetRayControl( work, i );
		r_ctrl = &work->ray_control[ i ];
		if ( r_ctrl->list_num != -1 ){
			flag &= PDRAY_GetGameCondition( work->ray_lists[ r_ctrl->list_num ].work ) == GAME_CONDITION_STANDBY ;
		}
	}
	if ( flag != 0 ){
		ChangeGamePhase( work );
		work->last_in_attack_num = -1 ;
	}
#else
	if ( PHASE_TIME_CHECK( 50 ) ){
		r_ctrl = GetRayControl( work, 0 );
		if ( r_ctrl->list_num != -1 ){
			r_ctrl->command = RAY_COMMAND_ATTACK1|RAY_COMMAND_ROAR ;
		}
	} else if ( PHASE_TIME_CHECK( 450 ) ){
		r_ctrl = GetRayControl( work, 1 );
		if ( r_ctrl->list_num != -1 ){
			r_ctrl->command = RAY_COMMAND_ATTACK1|RAY_COMMAND_ROAR ;
		}
	} else if ( PHASE_TIME_CHECK( 850 ) ){
		r_ctrl = GetRayControl( work, 2 );
		if ( r_ctrl->list_num != -1 ){
			r_ctrl->command = RAY_COMMAND_ATTACK1|RAY_COMMAND_ROAR ;
		}
	} else if ( PHASE_TIME_CHECK( 900 ) ){
		ChangeGamePhase( work );
	}
#endif
}
#if 0
	/*
		攻撃１フェーズ（順番に攻撃）
	*/
static void ActPhase_Attack1( Work *work )
{
	RAY_CONTROL		*r_ctrl ;
	int				i ;

	GAME_MARK( "attack1" );
	PHASE_PASSAGE();

	/* フェーズ初期化 */
	if ( work->game_phase_state == -1 ){
		work->game_phase_state = 0 ;
		//work->on_select_in_attack = 1 ;
	}

	switch ( work->game_phase_state ){
	  case 0:/* ステージに上がる１体を決定する */
		{/*  */
			int		flag, condition ;
			int		in_attack_num ;
			if ( work->current_n_rays < 1 ) break ;

			/* １体をステージ内攻撃に */
			do {
				int		condition ;
				in_attack_num = RND(3) ;
				//r_ctrl = GetRayControl( work, in_attack_num );
				r_ctrl = &work->ray_control[ work->in_attack_num ];
				if ( r_ctrl != NULL ){
					condition = PDRAY_GetGameCondition( r_ctrl->work );
					if ( ( condition == GAME_CONDITION_ESCAPE ) || ( condition == GAME_CONDITION_BREAK ) ){
						r_ctrl = NULL ;
						continue ;
					}
					if ( work->current_n_rays > 1 ){
						if ( work->last_in_attack_num == in_attack_num ){
							r_ctrl = NULL ;
						}
					}
				}
			} while ( r_ctrl == NULL );
			//r_ctrl->job_num = RAY_JOB_ATTACK1 ;
			{/* ポジション位置を変更 */
				int		new_pos ;
				ResetStayPoint( r_ctrl->pos_num );
				do {
					new_pos = RND( MAX_CIRCLE_POINTS );
				} while ( CheckStayPoint( new_pos ) != 0 );
				r_ctrl->pos_num = new_pos ;
				r_ctrl->target_pos_num = new_pos ;
				SetStayPoint( r_ctrl->pos_num, r_ctrl->no );
			}
			work->lock_on_ray_num = in_attack_num ;
			//work->enable_lockon = 1 ;
			/* それ以外をステージ外攻撃に設定 */
			for ( i = 0 ; i < 3 ; i++ ){
				if ( i == in_attack_num ) continue ;
				//r_ctrl = GetRayControl( work, i );
				r_ctrl = &work->ray_control[ i ];
				if ( r_ctrl != NULL ){
					//r_ctrl->job_num = RAY_JOB_ATTACK2 ;
					condition = PDRAY_GetGameCondition( r_ctrl->work );
				}
			}
			work->last_in_attack_num = in_attack_num ;
			work->game_phase_state++ ;
		}
		break ;
	  case 1:
		if ( work->on_select_in_attack ){/* ステージ内攻撃を行うＲＡＹ選定フラグが立っていば次のフェーズへ */
			work->on_select_in_attack = 0 ;
			for ( i = 0 ; i < 3 ; i++ ){
				//r_ctrl = GetRayControl( work, i );
				r_ctrl = &work->ray_control[ i ];
				if ( r_ctrl != NULL ){
					//r_ctrl->job_num = RAY_JOB_STANDBY ;
				}
			}
			ChangeGamePhase( work );
		}
		break ;
	}
}
	/*
		攻撃２フェーズ（全員一斉に攻撃）
	*/
static void ActPhase_Attack2( Work *work )
{
	RAY_CONTROL		*r_ctrl ;

	GAME_MARK( "attack2" );
	PHASE_PASSAGE();
	if ( PHASE_TIME_CHECK( 1500 ) ){
		//r_ctrl = GetRayControl( work, 0 );
		r_ctrl = &work->ray_control[ 0 ];
		if ( ( r_ctrl != NULL ) && ( ( r_ctrl->status & RAY_STATUS_ATTACK ) == 0 ) ){
			PDRAY_SetDemandActionBit( r_ctrl->work, PDRAY_SelectAttack( r_ctrl->work ) );
		}
		//r_ctrl = GetRayControl( work, 1 );
		r_ctrl = &work->ray_control[ 1 ];
		if ( ( r_ctrl != NULL ) && ( ( r_ctrl->status & RAY_STATUS_ATTACK ) == 0 ) ){
			PDRAY_SetDemandActionBit( r_ctrl->work, PDRAY_SelectAttack( r_ctrl->work ) );
		}
		//r_ctrl = GetRayControl( work, 2 );
		r_ctrl = &work->ray_control[ 2 ];
		if ( ( r_ctrl != NULL ) && ( ( r_ctrl->status & RAY_STATUS_ATTACK ) == 0 ) ){
			PDRAY_SetDemandActionBit( r_ctrl->work, PDRAY_SelectAttack( r_ctrl->work ) );
		}
	} else if ( PHASE_TIME_CHECK( 3000 ) ){
		//CHANGE_PHASE( GAME_PHASE_STANDBY );
		ChangeGamePhase( work );
	}
}
#endif
	/*
		攻撃３フェーズ（場外から３対同時攻撃）
	*/
static void ActPhase_Attack3( Work *work )
{
	RAY_CONTROL		*r_ctrl ;
	int				i ;

	GAME_MARK( "attack3" );
	PHASE_PASSAGE();

	/* フェーズ初期化 */
	if ( work->game_phase_state == -1 ){
		work->game_phase_state = 0 ;
		//work->on_select_in_attack = 1 ;
		work->demand_next_step_flag = 0 ;
		work->game_phase_state = 1 ;
	}

	/* 攻撃パターンループチェック */
	if ( attack_table1[ work->game_phase_state ][ 0 ] == -1 ){
		work->game_phase_state = 1 ;
		RESET_CHECK_TIME();
	}
	/* 攻撃パターンテーブルによるコマンド発行処理 */
	if ( PHASE_TIME_CHECK( attack_table1[ work->game_phase_state ][ 0 ] ) ){
		work->ray_control[ 0 ].command |= attack_table1[ work->game_phase_state ][ 1 ] ;
		work->ray_control[ 1 ].command |= attack_table1[ work->game_phase_state ][ 2 ] ;
		work->ray_control[ 2 ].command |= attack_table1[ work->game_phase_state ][ 3 ] ;
		work->game_phase_state++ ;
		work->camera_mode = attack_table1[ work->game_phase_state ][ 4 ] ;
		work->lock_on_ray_num = attack_table1[ work->game_phase_state ][ 5 ] ;
		work->game_phase_count += RAYSERVER_GameUpLevel * 200 ;
	} else {
		work->ray_control[ 0 ].command |= attack_table1[ work->game_phase_state-1 ][ 1 ] ;
		work->ray_control[ 1 ].command |= attack_table1[ work->game_phase_state-1 ][ 2 ] ;
		work->ray_control[ 2 ].command |= attack_table1[ work->game_phase_state-1 ][ 3 ] ;
		work->camera_mode = attack_table1[ work->game_phase_state-1 ][ 4 ] ;
		work->lock_on_ray_num = attack_table1[ work->game_phase_state-1 ][ 5 ] ;
	}

	{/* 次フェーズ移行チェック */
		int		life[3], min_life = 65536 ;
		for ( i = 0 ; i < 3 ; i++ ){
			//r_ctrl = GetRayControl( work, i );
			r_ctrl = &work->ray_control[ i ];
			if ( r_ctrl->list_num != -1 ){
				life[i] = r_ctrl->info->ray_life ;
			} else {
				life[i] = 0 ;
			}
			if ( min_life > life[i] ) min_life = life[i] ;
		}
		/* ３体のＲＡＹのライフ合計が半分を切ったら次フェーズへ */
		if ( ( life[ 0 ] + life[ 1 ] + life[ 2 ] ) < ( 65536*3/2 ) ){
		}
		/* もしくは単体のＲＡＹのライフが１/３を切ったら次フェーズへ */
		if ( min_life < ( 65536/3 ) ){
			ChangeGamePhase( work );
		}
	}

}

	/*
		攻撃４フェーズ（１体ステージ内へ、のこりは観察）
	*/
static void ActPhase_Attack4( Work *work )
{
	RAY_CONTROL		*r_ctrl ;
	int				i ;

	GAME_MARK( "attack4" );
	PHASE_PASSAGE();

	/* フェーズ初期化 */
	if ( work->game_phase_state == -1 ){
		work->on_select_in_attack = 1 ;
		work->camera_mode = 0 ;
	}
	if ( work->on_select_in_attack ){
		int		life, min_life_num = 0, min_life = 65537, min_list_num = 99999 ;

		work->game_phase_state = 0 ;
		/* 一番ライフの少ないＲＡＹをステージ上に乗せる */
		work->attacker = 0 ;
		work->supporter[ 0 ] = 1 ;
		work->supporter[ 1 ] = 2 ;
		for ( i = 0 ; i < 3 ; i++ ){
			//r_ctrl = GetRayControl( work, i );
			r_ctrl = &work->ray_control[ i ];
			if ( r_ctrl->list_num != -1 ){
				life = r_ctrl->info->ray_life ;
				if ( life > 0 ){
					if ( ( min_life > life ) || ( ( min_life == life ) && ( min_list_num > r_ctrl->list_num ) ) ){
						min_life_num = i ;
						min_life = life ;
						min_list_num = r_ctrl->list_num ;
						if ( work->attacker != i ){
							if ( work->supporter[ 0 ] == i ){
								work->supporter[ 0 ] = work->attacker ;
							} else {
								work->supporter[ 1 ] = work->attacker ;
							}
						}
						work->attacker = i ;
					}
				}
			}
		}
		work->last_in_attack_num = min_life_num ;
		work->lock_on_ray_num = min_life_num ;
		if ( work->ray_control[0].list_num != -1 && work->ray_control[0].info->condition == GAME_CONDITION_STANDBY ){
			work->ray_control[0].pos_length = CIRCLE_RANGE ;
		}
		if ( work->ray_control[1].list_num != -1 && work->ray_control[1].info->condition == GAME_CONDITION_STANDBY ){
			work->ray_control[1].pos_length = CIRCLE_RANGE ;
		}
		if ( work->ray_control[2].list_num != -1 && work->ray_control[2].info->condition == GAME_CONDITION_STANDBY ){
			work->ray_control[2].pos_length = CIRCLE_RANGE ;
		}

		{/* ２体のサポートＲＡＹに対して守備範囲の決定 */
			int		dir1, dir2 ;
			//r_ctrl = GetRayControl( work, work->supporter[ 0 ] );
			r_ctrl = &work->ray_control[ work->supporter[ 0 ] ];
			dir1 = r_ctrl->pos_angle ;
			//r_ctrl = GetRayControl( work, work->supporter[ 1 ] );
			r_ctrl = &work->ray_control[ work->supporter[ 1 ] ];
			dir2 = r_ctrl->pos_angle ;
			if ( ABS( GTE_REGULAR_ANGLE( dir1 - 1024 ) ) > ABS( GTE_REGULAR_ANGLE( dir2 - 1024 ) ) ){
				int		tmp ;
				tmp = work->supporter[ 0 ];
				work->supporter[ 0 ] = work->supporter[ 1 ] ;
				work->supporter[ 1 ] = tmp ;
			}
			/*
			   work->supporter[0] は右半分をカバー
			   work->supporter[1] は左半分をカバーするように設定する
			*/
		}
		work->on_select_in_attack = 0 ;

		/* ステージ内に入ったＲＡＹは死ぬまで戦いつづけるので死亡時の死に場所を決める */
		//r_ctrl = GetRayControl( work, work->attacker );
		//r_ctrl->pos_length = 50000 ;
		//r_ctrl->pos_angle = GTE_REGULAR_ANGLE( RND(4096) ) ;
		/* 死に場所はＲＡＹがステージに乗ってから決定する */
	}

	/* 検討事項 */
	/* 水圧カッターを３発発射したらＲＡＹを交代させるか？ */
	/* 場外のＲＡＹをうまくカメラに入るように移動させるか？ */
	/* スタンバイ中のＲＡＹの向く方法もサーバーで管理するほうがいいか？ */

	/* ステージ内攻撃ＲＡＹ行動処理（ＡＩによる自動戦闘） */
	work->ray_control[ work->attacker ].command = RAY_COMMAND_ATTACK2 ;

	/* ステージ外ＲＡＹ行動処理 */
	work->ray_control[ work->supporter[0] ].command |= RAY_COMMAND_NULL ;
	work->ray_control[ work->supporter[1] ].command |= RAY_COMMAND_NULL ;
#if 0
	{/* 試しに画面外に消えたならカメラと同じ方向に移動してみる */
		RAY_CONTROL		*r_ctrl1, *r_ctrl2 ;
		int		diff_angle1, diff_angle2, dir1, dir2, max_angle1, max_angle2, center_angle ;

		dir1 = GTE_REGULAR_ANGLE( work->camera_rotate.vy - 400 );
		dir2 = GTE_REGULAR_ANGLE( work->camera_rotate.vy + 400 );
		//r_ctrl1 = GetRayControl( work, work->supporter[0] ) ;
		//r_ctrl2 = GetRayControl( work, work->supporter[1] ) ;
		r_ctrl1 = &work->ray_control[ work->supporter[ 0 ] ];
		r_ctrl2 = &work->ray_control[ work->supporter[ 1 ] ];
		center_angle = GTE_REGULAR_ANGLE( 
										 GTE_REGULAR_ANGLE( r_ctrl2->pos_angle - r_ctrl1->pos_angle ) / 2 +
										 r_ctrl1->pos_angle );
		if ( ABS( GTE_REGULAR_ANGLE( center_angle - work->camera_rotate.vy ) ) > 300 ){
			/* 一番移動量の多いＲＡＹ同士の移動量が少ないパターンを選択 */
			diff_angle1 = ABS( GTE_REGULAR_ANGLE( r_ctrl1->pos_angle - dir1 ) );
			diff_angle2 = ABS( GTE_REGULAR_ANGLE( r_ctrl2->pos_angle - dir2 ) );
			max_angle1 = ( diff_angle1 > diff_angle2 ) ? diff_angle1 : diff_angle2 ;
			diff_angle1 = ABS( GTE_REGULAR_ANGLE( r_ctrl1->pos_angle - dir2 ) );
			diff_angle2 = ABS( GTE_REGULAR_ANGLE( r_ctrl2->pos_angle - dir1 ) );
			max_angle2 = ( diff_angle1 > diff_angle2 ) ? diff_angle1 : diff_angle2 ;
			if ( max_angle1 < max_angle2 ){
				r_ctrl1->pos_angle = dir1 ;
				r_ctrl2->pos_angle = dir2 ;
			} else {
				r_ctrl1->pos_angle = dir2 ;
				r_ctrl2->pos_angle = dir1 ;
			}
			//r_ctrl1->command |= RAY_COMMAND_ADJUST ;
			//r_ctrl2->command |= RAY_COMMAND_ADJUST ;
			{/* 移動範囲に制限をつける */
				int		diff ;
				diff = GTE_REGULAR_ANGLE( r_ctrl1->pos_angle - (1024) );
				if ( diff < -700 ) diff = -700 ;
				if ( diff > 700 ) diff = 700 ;
				r_ctrl1->pos_angle = 1024 + diff ;

				diff = GTE_REGULAR_ANGLE( r_ctrl2->pos_angle - (-1024) );
				if ( diff < -700 ) diff = -700 ;
				if ( diff > 700 ) diff = 700 ;
				r_ctrl2->pos_angle = -1024 + diff ;
			}
		}
	}
#else
	{/* このへんもっと改良が必要！！ */
		/* 正反対側のＲＡＹの挙動も作成する */
		int		diff_angle, base_angle, dir1, dir2 ;
		int		back_num, front_num ;

		if ( GTE_REGULAR_ANGLE( work->camera_rotate.vy ) > 0 ){
			//r_ctrl = GetRayControl( work, work->supporter[ 0 ] );
			r_ctrl = &work->ray_control[ work->supporter[ 0 ] ];
			back_num = work->supporter[ 0 ] ;
			front_num = work->supporter[ 1 ] ;
			base_angle = 1024 ;
		} else {
			//r_ctrl = GetRayControl( work, work->supporter[ 1 ] );
			r_ctrl = &work->ray_control[ work->supporter[ 1 ] ];
			back_num = work->supporter[ 1 ] ;
			front_num = work->supporter[ 0 ] ;
			base_angle = -1024 ;
		}
		/* 後方攻撃ＲＡＹ挙動設定 */
		r_ctrl = &work->ray_control[ back_num ];
		if ( r_ctrl->list_num != -1 && r_ctrl->info->condition != GAME_CONDITION_WAIT ){
			diff_angle = r_ctrl->pos_angle - base_angle ;
			if ( ABS( GTE_REGULAR_ANGLE( r_ctrl->pos_angle - work->camera_rotate.vy ) ) > 300 ){
#if 0
				dir1 = GTE_REGULAR_ANGLE( work->camera_rotate.vy - 300 );
				dir2 = GTE_REGULAR_ANGLE( work->camera_rotate.vy + 300 );

				diff_angle = GTE_REGULAR_ANGLE( dir1 - base_angle );
				if ( diff_angle < -700 ) diff_angle = -700 ;
				if ( diff_angle > 700 ) diff_angle = 700 ;
				dir1 = base_angle + diff_angle ;

				diff_angle = GTE_REGULAR_ANGLE( dir2 - base_angle );
				if ( diff_angle < -700 ) diff_angle = -700 ;
				if ( diff_angle > 700 ) diff_angle = 700 ;
				dir2 = base_angle + diff_angle ;

				if ( ABS( dir1 - r_ctrl->pos_angle ) < ABS( dir2 - r_ctrl->pos_angle ) ){
					r_ctrl->pos_angle = dir1 ;
				} else {
					r_ctrl->pos_angle = dir2 ;
				}
#else
				diff_angle = GTE_REGULAR_ANGLE( work->camera_rotate.vy - base_angle );
				if ( diff_angle < -700 ){
					diff_angle = -700 ;
					r_ctrl->command |= RAY_COMMAND_ADJUST ;
				}
				if ( diff_angle > 700 ){
					diff_angle = 700 ;
					r_ctrl->command |= RAY_COMMAND_ADJUST ;
				}
				r_ctrl->pos_angle = base_angle + diff_angle ;
#endif
			}
		}
		/* 前方ＲＡＹ挙動設定 */
		r_ctrl = &work->ray_control[ front_num ];
		if ( r_ctrl->list_num != -1 && r_ctrl->info->condition != GAME_CONDITION_WAIT ){
			base_angle = GTE_REGULAR_ANGLE( base_angle - 2048 );	/* ここでベース角度を反転しておく */
			dir1 = GTE_REGULAR_ANGLE( work->camera_rotate.vy - 1024 ) ;
			dir2 = GTE_REGULAR_ANGLE( work->camera_rotate.vy + 1024 ) ;
			/* ベースから近いほうを選択 */
			if ( ABS( GTE_REGULAR_ANGLE( base_angle - dir1 ) ) > ABS( GTE_REGULAR_ANGLE( base_angle - dir2 ) ) ){
				dir1 = dir2 ;
			}
			/* ある一定上差がある場合に移動を開始する */
#if 0
			if ( ABS( GTE_REGULAR_ANGLE( r_ctrl->pos_angle - dir1 ) ) > 300 ){
				/* 移動範囲に制限を設ける（ベース角度から一定角度以内） */
				diff_angle = GTE_REGULAR_ANGLE( dir1 - base_angle );
				if ( diff_angle < -700 ){
					diff_angle = -700 ;
					r_ctrl->command |= RAY_COMMAND_ADJUST ;
				}
				if ( diff_angle > 700 ){
					diff_angle = 700 ;
					r_ctrl->command |= RAY_COMMAND_ADJUST ;
				}
				r_ctrl->pos_angle = base_angle + diff_angle ;
			}
#else
			/* 移動範囲に制限を設ける（ベース角度から一定角度以内） */
			diff_angle = GTE_REGULAR_ANGLE( dir1 - base_angle );
			if ( diff_angle < -700 ){
				diff_angle = -700 ;
			}
			if ( diff_angle > 700 ){
				diff_angle = 700 ;
			}
			dir1 = base_angle + diff_angle ;
			if ( ABS( GTE_REGULAR_ANGLE( r_ctrl->pos_angle - dir1 ) ) > 300 ){
				r_ctrl->pos_angle = dir1 ;
				r_ctrl->command |= RAY_COMMAND_ADJUST ;
			}
#endif
#if 0
			if ( work->press & USB_KBM_D1 ){
				r_ctrl->command |= RAY_COMMAND_ROAR ;
				printf("command ray roar!\n");
			}
#endif
		}
		/* 但しスティンガーに狙われた場合などは何らかの動作を行いたい！！ */
	}
#endif

	/* 現在の座標と移動目的座標が離れている場合には位置補正を要求する */
	for ( i = 0 ; i < 3 ; i++ ){
		RAY_INFO	*info ;
		if ( i != work->game_ai_target_num ) continue ;
		//r_ctrl = GetRayControl( work, i );
		r_ctrl = &work->ray_control[ i ];
		if ( r_ctrl->list_num != -1 ){
			FVECTOR		tmp_vec ;
			float		len ;
			info = r_ctrl->info ;
			CalcPosition( &info->standby_pos, r_ctrl->pos_length, r_ctrl->pos_angle );
			GTE_SubVector( &tmp_vec, &info->standby_pos, &info->pos );
			tmp_vec.vy = 0 ;
			len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
			if ( len > 8000.0f ){
				r_ctrl->command |= RAY_COMMAND_ADJUST ;
			}
		}
	}

	/* 場外待機ＲＡＹによる攻撃開始判定 */
	if ( work->game_ai_target_num != work->attacker ){
		//r_ctrl = GetRayControl( work, work->game_ai_target_num );
		r_ctrl = &work->ray_control[ work->game_ai_target_num ];
		if ( r_ctrl->list_num != -1 ){
			//printf("ray %d : %d - %d = %d\n", r_ctrl->list_num,
			//	   r_ctrl->pos_angle, work->camera_rotate.vy,
			//	   GTE_REGULAR_ANGLE( r_ctrl->pos_angle - work->camera_rotate.vy ) );
			if ( ABS( GTE_REGULAR_ANGLE( r_ctrl->pos_angle - work->camera_rotate.vy ) ) < 300 ){
				/* 攻撃タイミングをどうする？ */
				r_ctrl->command |= RAY_COMMAND_ATTACK1 ;
#ifdef DEBUG_MODE
				//if ( work->status & USB_KBM_C1 ) r_ctrl->command |= RAY_COMMAND_ATTACK1 ;
#endif
			}
		}
	}

	/* カメラモードの設定 */
	//work->camera_mode = 0 ;


	{/* 次フェーズ移行チェック */
	}
	

}

#if 0
	/*
		攻撃５フェーズ（１体ステージ内へ、のこりは補助）
	*/
static void ActPhase_Attack5( Work *work )
{
	RAY_CONTROL		*r_ctrl ;
	int				i, index ;

	GAME_MARK( "attack4" );
	PHASE_PASSAGE();

	/* フェーズ初期化 */
	if ( work->game_phase_state == -1 ){
		int		life, min_life_num = 0, min_life = 65537, l_num, r_num ;
		work->game_phase_state = 0 ;
		/* 一番ライフの少ないＲＡＹをステージ上に乗せる */
		for ( i = 0 ; i < 3 ; i++ ){
			//r_ctrl = GetRayControl( work, i );
			r_ctrl = &work->ray_control[ i ];
			if ( r_ctrl != NULL ){
				life = PDRAY_GetRestLife( r_ctrl->work );
				if ( min_life > life ){
					min_life_num = i ;
					min_life = life ;
				}
			}
		}
		work->lock_on_ray_num = min_life_num ;
		work->game_phase_state = 1 ;
	}

	/* 攻撃パターンループチェック */
	if ( attack_table1[ work->game_phase_state ][ 0 ] == -1 ){
		work->game_phase_state = 1 ;
		RESET_CHECK_TIME();
	}
	/* 攻撃パターンテーブルによるコマンド発行処理 */
	if ( PHASE_TIME_CHECK( attack_table1[ work->game_phase_state ][ 0 ] ) ){
		work->game_command[ 0 ] |= attack_table1[ work->game_phase_state ][ 1 ] ;
		work->game_command[ 1 ] |= attack_table1[ work->game_phase_state ][ 2 ] ;
		work->game_command[ 2 ] |= attack_table1[ work->game_phase_state ][ 3 ] ;
		work->game_phase_state++ ;
		work->camera_mode = attack_table1[ work->game_phase_state ][ 4 ] ;
		//work->lock_on_ray_num = attack_table1[ work->game_phase_state ][ 5 ] ;
	} else {
		work->game_command[ 0 ] |= attack_table1[ work->game_phase_state-1 ][ 1 ] ;
		work->game_command[ 1 ] |= attack_table1[ work->game_phase_state-1 ][ 2 ] ;
		work->game_command[ 2 ] |= attack_table1[ work->game_phase_state-1 ][ 3 ] ;
		work->camera_mode = attack_table1[ work->game_phase_state-1 ][ 4 ] ;
		//work->lock_on_ray_num = attack_table1[ work->game_phase_state-1 ][ 5 ] ;
	}
	work->game_command[ work->lock_on_ray_num ] = RAY_COMMAND_ATTACK2 ;
	work->camera_mode = 0 ;


	{/* 次フェーズ移行チェック */
	}
	
	/* コマンド割り当て */
	for ( i = 0 ; i < 3 ; i++ ){
		//r_ctrl = GetRayControl( work, i );
		r_ctrl = &work->ray_control[ i ];
		if ( r_ctrl != NULL ){
			r_ctrl->command = work->game_command[i] ;
		}
	}

}
#endif
#if 0
	/*
		デバッグ用
	*/
static void ActPhase_Debug( Work *work )
{
	RAY_CONTROL		*r_ctrl ;
	int				i, index ;

	GAME_MARK( "Debug" );
	PHASE_PASSAGE();

	/* フェーズ初期化 */
	if ( work->game_phase_state == -1 ){
		work->game_phase_state = 0 ;
	}

#ifdef DEBUG_MODE
#if 0
	{
		static int	old_pad, toggle ;
		int		pad, press, release ;
		pad = GetUsbPad();
		press = pad & ~old_pad ;
		release = ~pad & old_pad ;
		toggle = toggle ^ press ;
		//printf("%08x %08x %08x %08x %08x\n", pad, old_pad, press, release, toggle );
		if ( pad & USB_KBM_C1 )		work->game_command[0] |= RAY_COMMAND_DEBUG_A1 ;
		if ( toggle & USB_KBM_CD1 )	work->game_command[0] |= RAY_COMMAND_ATTACK2 ;
		if ( pad & USB_KBM_D1 )		work->game_command[0] |= RAY_COMMAND_DEBUG_A2 ;
		if ( pad & USB_KBM_DE1 )	work->game_command[0] |= RAY_COMMAND_ATTACK1 ;
		if ( pad & USB_KBM_E1 )		work->game_command[0] |= RAY_COMMAND_DEBUG_A3 ;

		if ( pad & USB_KBM_F1 )		work->game_command[1] |= RAY_COMMAND_DEBUG_A1 ;
		if ( toggle & USB_KBM_FG1 )	work->game_command[1] |= RAY_COMMAND_ATTACK2 ;
		if ( pad & USB_KBM_G1 )		work->game_command[1] |= RAY_COMMAND_DEBUG_A2 ;
		if ( pad & USB_KBM_GA1 )	work->game_command[1] |= RAY_COMMAND_ATTACK1 ;
		if ( pad & USB_KBM_A1 )		work->game_command[1] |= RAY_COMMAND_DEBUG_A3 ;

		if ( pad & USB_KBM_C2 )		work->game_command[2] |= RAY_COMMAND_DEBUG_A1 ;
		if ( toggle & USB_KBM_CD2 )	work->game_command[2] |= RAY_COMMAND_ATTACK2 ;
		if ( pad & USB_KBM_D2 )		work->game_command[2] |= RAY_COMMAND_DEBUG_A2 ;
		if ( pad & USB_KBM_DE2 )	work->game_command[2] |= RAY_COMMAND_ATTACK1 ;
		if ( pad & USB_KBM_E2 )		work->game_command[2] |= RAY_COMMAND_DEBUG_A3 ;
		if ( press & USB_KBM_HU ){
			work->camera_mode++ ;
		}
		if ( press & USB_KBM_HD ){
			work->camera_mode-- ;
		}
		if ( press & USB_KBM_ST ){
			if ( ++work->lock_on_ray_num > 2 ) work->lock_on_ray_num = -1 ;
		}
		if ( press & USB_KBM_SL ){
			if ( --work->lock_on_ray_num < -1 ) work->lock_on_ray_num = 2 ;
		}
		if ( press & USB_KBM_F2 ){
			//r_ctrl = GetRayControl( work, 2 );
			r_ctrl = &work->ray_control[ 2 ];
			if ( r_ctrl != NULL ){
				r_ctrl->trg_angle -= 300 ;
				//SetRayPosition( RAY_CONTROL *r_ctrl, FVECTOR *pos );
			}
		}
		if ( press & USB_KBM_FG2 ){
			//r_ctrl = GetRayControl( work, 2 );
			r_ctrl = &work->ray_control[ 2 ];
			if ( r_ctrl != NULL ){
				r_ctrl->trg_angle += 300 ;
				//SetRayPosition( RAY_CONTROL *r_ctrl, FVECTOR *pos );
			}
		}
		old_pad = pad ;
	}

	/* コマンド割り当て */
	for ( i = 0 ; i < 3 ; i++ ){
		//r_ctrl = GetRayControl( work, i );
		r_ctrl = &work->ray_control[ i ];
		if ( r_ctrl != NULL ){
			r_ctrl->command = work->game_command[i] ;
		}
	}
#endif
#endif
}
#endif
/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
	int		i ;
	RAY_CONTROL		*r_ctrl ;

	/* コマンド内容の初期化 */
	work->ray_control[ 0 ].command = 0 ;
	work->ray_control[ 1 ].command = 0 ;
	work->ray_control[ 2 ].command = 0 ;

	{/* プレイヤー監視処理 */
		FVECTOR		tmp_vec ;
		if ( work->record_count == 0 ){
			GTE_SubVector( &tmp_vec, &GM_PlayerPosition, &work->old_player_pos );
			GTE_ScaleVector( &work->player_speed, &tmp_vec, 1.0f / 10.0f );
			work->old_player_pos = GM_PlayerPosition ;
			work->record_count = 10 ;
		}
		work->record_count-- ;
	}

	{/* プレイヤー攻撃監視処理 */
		if ( GM_WeaponAlive & WP_ALIVE_STINGER ){
			if ( !( work->old_weapon_alive_flag & WP_ALIVE_STINGER ) ){
				work->old_stinger_pos = GM_StingerPosition ;
			}
		}
		GTE_SubVector( &work->stinger_speed, &GM_StingerPosition, &work->old_stinger_pos );
		work->old_weapon_alive_flag = GM_WeaponAlive ;
		work->old_stinger_pos = GM_StingerPosition ;
	}

	/* 全滅判定 */
	if ( work->game_phase != GAME_PHASE_END && work->game_phase != GAME_PHASE_START ){
		int				i, max_rays, condition, n_rays ;

		max_rays = 0 ;
		n_rays = 0 ;
		for ( i = 0 ; i < 3 ; i++ ){
			//r_ctrl = GetRayControl( work, i );
			r_ctrl = &work->ray_control[ i ];
			if ( r_ctrl->list_num != -1 ){
				condition = PDRAY_GetGameCondition( work->ray_lists[ r_ctrl->list_num ].work );
				if ( condition == GAME_CONDITION_BREAK ){
					r_ctrl->list_num = -1 ;
					r_ctrl = GetRayControl( work, i );
					if ( r_ctrl->list_num != -1 ){
						r_ctrl->pos_angle = 2048 ;
						r_ctrl->pos_length = CIRCLE_RANGE ;
						SetRayNewPosition( work, i );
						max_rays++ ;
					}
					work->use_ray_count++ ;
					//使用しなくなったので//ExecProc( work->end_proc, work->use_ray_count );
					if ( work->game_phase == GAME_PHASE_ATTACK4 ){
						work->on_select_in_attack = 1 ;
					}
				} else if ( ( condition != GAME_CONDITION_WAIT ) && ( condition != GAME_CONDITION_ESCAPE ) ){
					n_rays++ ;
					max_rays++ ;
				} else {
					max_rays++ ;
				}
			}
		}
		if ( max_rays == 0 ){
			/* ＲＡＹ全滅処理 */
			printf("game end!!!!\n");
			//CHANGE_PHASE( GAME_PHASE_END );
			//使用しなくなったので//ExecProc( work->end_proc, -1 );
		}
		work->current_n_rays = n_rays ;
		work->current_max_rays = max_rays ;
	}

	if ( ( work->n_rays > 0 ) && ( work->total_life <= 0 ) ){
		/* ＲＡＹ全滅処理 */
		//CHANGE_PHASE( GAME_PHASE_END );
		if ( work->end_proc != 0 ){
			printf("game end!!!!\n");
			ExecProc( work->end_proc, -1 );
		}
		work->end_proc = 0 ;
	}
	//printf("total life %d(%d)\n", work->total_life, work->mark_num );

	/* ゲーム進行管理 */
	switch ( work->game_phase ){
	  case GAME_PHASE_START:
		ActPhase_Start( work );
		break ;
	  case GAME_PHASE_STANDBY:
		ActPhase_Standby( work );
		break ;
	  case GAME_PHASE_ATTACK1:
		//ActPhase_Attack1( work );
		break ;
	  case GAME_PHASE_ATTACK2:
		//ActPhase_Attack1( work );
		break ;
	  case GAME_PHASE_ATTACK3:
		ActPhase_Attack3( work );
		break ;
	  case GAME_PHASE_ATTACK4:
		ActPhase_Attack4( work );
		break ;
	  case GAME_PHASE_END:
		DEBUG_Locate( 256, 224, 2 );
		DEBUG_Printf("GAME CLEAR !!");
		break ;
	}

	/* ＲＡＹに渡すパラメータをセットする */
	for ( i = 0 ; i < 3 ; i++ ){
		//r_ctrl = GetRayControl( work, i );
		r_ctrl = &work->ray_control[ i ];
		if ( r_ctrl->list_num != -1 ){
			RAY_INFO	*info ;
			info = r_ctrl->info ;
			info->color_id = i ;
			if ( info->condition != GAME_CONDITION_WAIT ){
				info->command = work->ray_control[ i ].command ;
			}
			info->command |= work->ray_control[ i ].command & RAY_COMMAND_ATTACK2 ;
			if ( r_ctrl->status & RAY_STATUS_ACTIVE ) info->command |= RAY_COMMAND_ACTIVE ;
			if ( r_ctrl->status & RAY_STATUS_FIRST ) info->command |= RAY_COMMAND_QUICKSTART ;
			CalcPosition( &info->standby_pos, r_ctrl->pos_length, r_ctrl->pos_angle );
#if 0
			{/* 現在の座標と移動目的座標が離れている場合には位置補正を要求する */
				FVECTOR		tmp_vec ;
				float		len ;
				GTE_SubVector( &tmp_vec, &info->standby_pos, &info->pos );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len > 5000.0f ){
					info->command |= RAY_COMMAND_ADJUST ;
				}
			}
#endif
		}
	}

	/* ＡＩ管理切り替え */
	work->game_ai_change_time += TIME_BASE ;
	if ( work->game_ai_change_time >= AI_CHANGE_INTERVAL ){
		work->game_ai_change_time -= AI_CHANGE_INTERVAL ;
		work->game_ai_target_num++ ;
		if ( work->game_ai_target_num >= 3 ) work->game_ai_target_num = 0 ;
	}

	{/* ＲＡＹロックオン処理 */
		short	dir ;
		/* ロック中のＲＡＹの方を向くようにカメラ角度変更 */
		if ( work->lock_on_ray_num != -1 ){
			//r_ctrl = GetRayControl( work, work->lock_on_ray_num );
			r_ctrl = &work->ray_control[ work->lock_on_ray_num ];
			if ( r_ctrl->list_num != -1 && work->enable_lockon ){
				int		tmp ;
				FVECTOR	tmp_vec, r_vec ;
				float	len ;

				GTE_InterVector( &work->cam_player_tpos, &work->cam_player_tpos, &GM_PlayerPosition, 0.125f );
				GTE_SubVector( &tmp_vec, &work->cam_player_tpos, &work->cam_player_pos );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len > 750.0f ){
					GTE_ScaleVector( &r_vec, &tmp_vec, 750.0f / len );
					GTE_SubVector( &tmp_vec, &tmp_vec, &r_vec );
					GTE_AddVector( &work->cam_player_pos, &work->cam_player_pos, &tmp_vec );
				}

				GTE_InterVector( &work->cam_pdray_tpos, &work->cam_pdray_tpos, &r_ctrl->info->pos, 0.125f );
				GTE_SubVector( &tmp_vec, &work->cam_pdray_tpos, &work->cam_pdray_pos );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len > 2000.0f ){
					GTE_ScaleVector( &r_vec, &tmp_vec, 2000.0f / len );
					GTE_SubVector( &tmp_vec, &tmp_vec, &r_vec );
					GTE_AddVector( &work->cam_pdray_pos, &work->cam_pdray_pos, &tmp_vec );
				}

#if 0
				GTE_SubVector( &tmp_vec, &work->cam_pdray_pos, &work->cam_player_pos );
				tmp_vec.vy = 0 ;
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len < 15000.0f ){
					work->camera_mode = 1 ;
				} else {
					work->camera_mode = 0 ;
				}
#endif

				/* 画面内捕らえチェック用に補間を行わない座標で角度差を計算する */
				dir = GV_VecDir2FromTo( &GM_PlayerPosition, &r_ctrl->info->pos );
				work->cam_diff_angle = GTE_REGULAR_ANGLE( dir - work->camera_rotate.vy ) ;

				dir = GV_VecDir2FromTo( &work->cam_player_pos, &work->cam_pdray_pos );
				tmp = GTE_REGULAR_ANGLE( dir - work->camera_rotate.vy ) ;
				//printf("%d =  %d - %d\n", tmp, dir, work->camera_rotate.vy );
				if ( tmp > MAX_CAMERA_TURN ) tmp = MAX_CAMERA_TURN ;
				if ( tmp < -MAX_CAMERA_TURN ) tmp = -MAX_CAMERA_TURN ;
				work->camera_rotate.vy += tmp ;

			}
		} else {
			int		tmp ;

			/* 画面内捕らえチェック用に補間を行わない座標で角度差を計算する */
			//dir = GV_VecDir2FromTo( &work->cam_player_pos, &work->cam_pdray_pos );
			dir = 2048 ;
			tmp = GTE_REGULAR_ANGLE( dir - work->camera_rotate.vy ) ;
			//printf("%d =  %d - %d\n", tmp, dir, work->camera_rotate.vy );
			if ( tmp > MAX_CAMERA_TURN ) tmp = MAX_CAMERA_TURN ;
			if ( tmp < -MAX_CAMERA_TURN ) tmp = -MAX_CAMERA_TURN ;
			work->camera_rotate.vy += tmp ;
		}

		switch ( work->camera_mode ){
		  case 0:
			/* 水平見上げＲＡＹ追従カメラ */
			//work->camera_rotate.vx = 4050 ;
			work->camera_rotate.vx = 14 ;
			GM_SetCameraTrack( work->camera, 10000 );
			GM_SetCameraRotate( work->camera, &work->camera_rotate );
			break ;
		  case 1:
			/* 上空見下ろしＲＡＹ追従カメラ */
			work->camera_rotate.vx = 400 ;
			GM_SetCameraTrack( work->camera, 20000 );
			GM_SetCameraRotate( work->camera, &work->camera_rotate );
			break ;
		  case 2:
			/* プレイヤー見下ろし通常客観カメラ */
			GM_SetCameraTrack( work->camera, 30000 );
			{
				SVECTOR		rot ;
				rot.vx = 650 ;
				rot.vy = 2048 ;
				rot.vz = 0 ;
				GM_SetCameraRotate( work->camera, &rot );
			}
			break ;
		  case 3:
			/* プレイヤー見下ろし通常客観カメラ */
			GM_SetCameraTrack( work->camera, 30000 );
			{
				SVECTOR		rot ;
				rot.vx = 110 ;
				rot.vy = 2048 ;
				rot.vz = 0 ;
				GM_SetCameraRotate( work->camera, &rot );
			}
			break ;
		  case -1:
			work->camera_mode = 3 ;
			break ;
		  case 4:
			work->camera_mode = 0 ;
			break ;
		}
	}

	/* ＲＡＹ共通ステータス関連処理 */
	RAYSERVER_CommonStatus = work->next_common_status ;
	work->next_common_status = 0 ;

	/* ミサイル効果音処理 */
	if ( work->missile_near_len != 0.0f ){
		work->missile_count += 1500 ;
		if ( work->missile_near_len < work->missile_count ){
			work->missile_count = 0 ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_E_AMALERT1 ) ;
		}
	} else {
		work->missile_count = 0 ;
	}
	work->missile_near_len = 0 ;

#if 0
#ifdef DEBUG_MODE
	/* デバッグ用のパッド入力 */
	work->status = GetUsbPad();
	work->press = work->status & ~work->old_status ;
	work->release = ~work->status & work->old_status ;
	work->toggle = work->toggle ^ work->press ;
	work->old_status = work->status ;
#endif
#endif

}
/* ---------------------------------------------------------------- */
	/*
		終了
	*/
static void Die( Work *work )
{
	int		i ;
	for ( i = 0 ; i < 6 ; i++ ){
		if ( work->number_parts_r[i] != NULL ){
			DG_DequeueObjs( work->number_parts_r[i] );
			DG_FreeObjs( work->number_parts_r[i] );
		}
		if ( work->number_parts_l[i] != NULL ){
			DG_DequeueObjs( work->number_parts_l[i] );
			DG_FreeObjs( work->number_parts_l[i] );
		}
//#ifdef PSX2
		if ( work->number_replace[ i ] != NULL ){
			DG_ResetMoveReplaceTexture( work->number_replace[ i ] );
			DG_FreeMoveReplacePacket( work->number_replace[ i ] );
		}
//#endif
	}
	work_ptr = NULL ;
}
/* ---------------------------------------------------------------- */
	/*
		初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	/* ＲＡＹ戦専用ゲームレベル設定 */
	switch ( GM_GameLevel ){
	  case GM_LEVEL_VERYEASY:
		RAYSERVER_GameLevel = 1 ;
		RAYSERVER_GameLevelOffset = -2 ;
		RAYSERVER_GameDownLevel = 2 ;
		RAYSERVER_GameUpLevel = 0 ;
		break ;
	  case GM_LEVEL_EASY:
		RAYSERVER_GameLevel = 2 ;
		RAYSERVER_GameLevelOffset = -1 ;
		RAYSERVER_GameDownLevel = 1 ;
		RAYSERVER_GameUpLevel = 0 ;
		break ;
	  case GM_LEVEL_NORMAL:
		RAYSERVER_GameLevel = 3 ;
		RAYSERVER_GameLevelOffset = 0 ;
		RAYSERVER_GameDownLevel = 0 ;
		RAYSERVER_GameUpLevel = 0 ;
		break ;
	  case GM_LEVEL_HARD:
		RAYSERVER_GameLevel = 4 ;
		RAYSERVER_GameLevelOffset = 1 ;
		RAYSERVER_GameDownLevel = 0 ;
		RAYSERVER_GameUpLevel = 1 ;
		break ;
	  case GM_LEVEL_EXTREME:
		RAYSERVER_GameLevel = 5 ;
		RAYSERVER_GameLevelOffset = 2 ;
		RAYSERVER_GameDownLevel = 0 ;
		RAYSERVER_GameUpLevel = 2 ;
		break ;
	  default:
		if ( GM_GameLevel < GM_LEVEL_VERYEASY ){
			RAYSERVER_GameLevel = 0 ;
			RAYSERVER_GameLevelOffset = -3 ;
			RAYSERVER_GameDownLevel = 3 ;
			RAYSERVER_GameUpLevel = 0 ;
		}
		if ( GM_GameLevel > GM_LEVEL_EXTREME ){
			/* EXTREME以上はEXTREMEと同じに設定する */
			RAYSERVER_GameLevel = 5 ;
			RAYSERVER_GameLevelOffset = 2 ;
			RAYSERVER_GameDownLevel = 0 ;
			RAYSERVER_GameUpLevel = 2 ;
		}
		break ;
	};

	if ( GCL_GetOption( 'p' ) != NULL ){
		work->end_proc = GCL_GetNextInt();
	}

	if ( GCL_GetOption( 'n' ) != NULL ){
		work->mark_num = GCL_GetNextInt();
		work->total_life = work->mark_num * MAX_LIFE ;
	} else {
		static short max_mark_list[7] = { 3,  3,3,5,10,20,  20};
		work->mark_num = max_mark_list[ RAYSERVER_GameLevel ] ;
		work->total_life = work->mark_num * MAX_LIFE ;
	}

	work->ray_control[ 0 ].list_num = -1 ;
	work->ray_control[ 1 ].list_num = -1 ;
	work->ray_control[ 2 ].list_num = -1 ;

	CHANGE_PHASE( GAME_PHASE_START );


	/* カメラ設定 */
	work->lock_on_ray_num = -1 ;
	work->camera_rotate.vx = 4050 ;
	work->camera_rotate.vy = 2048 ;
	work->camera_rotate.vz = 0 ;
	work->camera = NewProgramCamera( name, 0, GM_CAMERA_PROG4, 128 );
	GM_SetCameraType( work->camera, GM_CAM_TYPE_TARGET_AND_ROTATE,
					 CAM_FLAG_TRACE | CAM_FLAG_PAD_ADJUST | CAM_FLAG_BOUND | CAM_FLAG_LIMIT );
	GM_SetCameraAngle( work->camera, 2.0F ) ;
	GM_SetCameraTrack( work->camera, 10000 );
	GM_SetCameraRotate( work->camera, &work->camera_rotate );
	GM_SetCameraInterpMode( work->camera, GM_CAM_INTERP_EXP4, GM_CAM_INTERP_EXP4, 0, 0);
	//GM_SetCameraInterpMode( work->camera, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK, 0, 0);
	work->camera->on = 1 ;
	{
		float	bound[6] = {-100000,4250,-100000,100000,100000,100000};
		GM_SetCameraBound( work->camera, bound );
		GM_SetCameraLimit( work->camera, bound );
	}
	GM_ChangeCamera( 0 );


	{/* ＲＡＹ個別ナンバリング関係 */
		static int	model_list_r[6] = {NUMBER_MODEL_R0,NUMBER_MODEL_R1,NUMBER_MODEL_R2,
										 NUMBER_MODEL_R3,NUMBER_MODEL_R4,NUMBER_MODEL_R5};
		static int	model_list_l[6] = {NUMBER_MODEL_L0,NUMBER_MODEL_L1,NUMBER_MODEL_L2,
										 NUMBER_MODEL_L3,NUMBER_MODEL_L4,NUMBER_MODEL_L5};
		DG_DEF		*def ;
		int			i ;
		for ( i = 0 ; i < 6 ; i++ ){
			work->number_free_buffer[i] = i ;
			def = GV_GetCache( GV_CacheID( model_list_r[i], 'k' ) );
			if ( def != NULL ){
				work->number_parts_r[i] = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION, 0 );
				DG_QueueObjs( work->number_parts_r[ i ] );
				work->number_parts_r[ i ]->flag |= DG_FLAG_INVISIBLE ;
			}
#ifdef DEBUG_MODE
			if ( def == NULL ){
				printf("%s:not found model !!(%d)\n", __FILE__, model_list_r[i] );
			}
#endif
			def = GV_GetCache( GV_CacheID( model_list_l[i], 'k' ) );
			if ( def != NULL ){
				work->number_parts_l[i] = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION, 0 );
				DG_QueueObjs( work->number_parts_l[ i ] );
				work->number_parts_l[ i ]->flag |= DG_FLAG_INVISIBLE ;
			}
#ifdef DEBUG_MODE
			if ( def == NULL ){
				printf("%s:not found model !!(%d)\n", __FILE__, model_list_l[i] );
			}
#endif
		}
	}


	/* ゲーム進行開始 */
	ChangeGamePhase( work );

#ifdef DEBUG_MODE
	NewRServerDebugActor();
#endif

	{/* ステージ環境の設定 */
		extern void *NewPDRayStageEnvironment( void );
		NewPDRayStageEnvironment();
	}

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewPDRayServerSet( int name, int where )
{
	Work *work ;

	if ( work_ptr != NULL ) return ( NULL );

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );
	work_ptr = work ;
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




/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* ＲＡＹサーバーにシグナルを設定 */
int RAYSERVER_SetGameSignal( int signal, int param )
{
	Work	*work ;
	int		res = 0 ;
	if ( work_ptr == NULL ) return (-1);

	work = work_ptr ;
	switch ( signal ){
	  case GAME_SIGNAL_START_IN_ATTACK:
		{/* ロックオンシグナルと同時に死に場所決定 */
			RAY_CONTROL		*r_ctrl ;
			//r_ctrl = GetRayControl( work, work->attacker );
			r_ctrl = &work->ray_control[ work->attacker ];
			r_ctrl->pos_length = 50000 ;
			r_ctrl->pos_angle = GTE_REGULAR_ANGLE( RND(4096) ) ;
		}
		break ;
	  case GAME_SIGNAL_END_IN_ATTACK:/* ステージ内攻撃終了通知 */
		//work->on_select_in_attack = 1 ;
		{/* 新しいＲＡＹを選出する */
			//RAY_CONTROL		*r_ctrl ;
			//r_ctrl = GetRayControl( work, work->attacker );
#if 0
			r_ctrl = &work->ray_control[ work->attacker ];			
			if ( r_ctrl->list_num != -1 ){
				//r_ctrl->list_num = -1 ;
				//r_ctrl = GetRayControl( work, work->attacker );
				r_ctrl = &work->ray_control[ work->attacker ];
				if ( r_ctrl->list_num != -1 ){
					r_ctrl->pos_angle = 2048 ;
					r_ctrl->pos_length = CIRCLE_RANGE ;
				}
			}
#endif
		}
		break ;
	  case GAME_SIGNAL_LOCKON_ENABLE:
		work->enable_lockon = 1 ;
		break ;
	  case GAME_SIGNAL_LOCKON_DISABLE:
		work->enable_lockon = 0 ;
		break ;
	  case GAME_SIGNAL_DEMAND_NEXT_PHASE:
		work->demand_next_phase_flag = 1 ;
		break ;
	  case GAME_SIGNAL_DEMAND_NEXT_STEP:
		work->demand_next_step_flag = 1 ;
		break ;
	  case GAME_SIGNAL_GET_CAM_DIFF_ANGLE:
		res = work->cam_diff_angle ;
		break ;
	  case GAME_SIGNAL_CHANGE_CAMERA:
		work->camera_mode = param ;
		break ;
	  case GAME_SIGNAL_NOTICE:
		work->next_common_status |= param ;
		break ;
	  case GAME_SIGNAL_DAMAGE:
		work->total_life -= param ;
		break ;
	}
	return ( res );
}

/* ＲＡＹをサーバーに登録する */
int RAYSERVER_AddRayControl( void *ray_work, RAY_INFO *info, CONTROL *ctrl )
{
	Work	*work ;
	RAY_LIST	*r_list ;
	if ( work_ptr == NULL ) return (-1);

	work = work_ptr ;
	r_list = &work->ray_lists[ work->n_rays ] ;
	r_list->no = work->n_rays++ ;
	r_list->work = ray_work ;
	r_list->info = info ;
	r_list->ctrl = ctrl ;

	return ( r_list->no );
}
/* プレイヤーのスティンガー速度の取得 */
void RAYSERVER_GetPlayerSpeed( FVECTOR *vec )
{
	Work	*work ;

	if ( work_ptr == NULL ) return ;
	work = work_ptr ;

	*vec = work->player_speed ;
}
/* プレイヤーのスティンガー速度の取得 */
void RAYSERVER_GetStingerVelocity( FVECTOR *vec )
{
	Work	*work ;

	if ( work_ptr == NULL ) return ;
	work = work_ptr ;

	*vec = work->stinger_speed ;
}

static int	number_tex_list[ 25 ] = {
	NUMBER_TEXTURE_00,NUMBER_TEXTURE_01,NUMBER_TEXTURE_02,NUMBER_TEXTURE_03,NUMBER_TEXTURE_04,
	NUMBER_TEXTURE_05,NUMBER_TEXTURE_06,NUMBER_TEXTURE_07,NUMBER_TEXTURE_08,NUMBER_TEXTURE_09,
	NUMBER_TEXTURE_10,NUMBER_TEXTURE_11,NUMBER_TEXTURE_12,NUMBER_TEXTURE_13,NUMBER_TEXTURE_14,
	NUMBER_TEXTURE_15,NUMBER_TEXTURE_16,NUMBER_TEXTURE_17,NUMBER_TEXTURE_18,NUMBER_TEXTURE_19,
	NUMBER_TEXTURE_20,NUMBER_TEXTURE_21,NUMBER_TEXTURE_22,NUMBER_TEXTURE_23,NUMBER_TEXTURE_24
};
/* ナンバーモデルの取得 */
int RAYSERVER_GetNumberModel( DG_OBJS *body )
{
	Work	*work ;
	int			i, handle ;

	if ( work_ptr == NULL ) return ( -1 ) ;
	work = work_ptr ;

	/* 空モデルの検索 */
	handle = work->number_free_buffer[ 0 ] ;
	if ( handle == -1 ) return ( -1 );
	for ( i = 0 ; i < 5 ; i++ ){
		work->number_free_buffer[ i ] = work->number_free_buffer[ i + 1 ] ;
	}
	work->number_free_buffer[ 5 ] = -1 ;

	/* モデルの取り付け */
	if ( work->number_parts_r[ handle ] != NULL ){
		work->number_parts_r[ handle ]->flag &= ~DG_FLAG_INVISIBLE ;
		work->number_parts_r[ handle ]->root = &body->objs[ 22 ].world ;
		DG_SetLightMatrix( work->number_parts_r[ handle ], body->light );
	}
	if ( work->number_parts_l[ handle ] != NULL ){
		work->number_parts_l[ handle ]->flag &= ~DG_FLAG_INVISIBLE ;
		work->number_parts_l[ handle ]->root = &body->objs[ 28 ].world ;
		DG_SetLightMatrix( work->number_parts_l[ handle ], body->light );
	}
//#ifdef PSX2
	/* テクスチャ入れ替え */
	if ( work->number_count >= 6 && work->number_count < 25 ){
		if ( work->number_replace[ handle ] != NULL ){
			DG_ResetMoveReplaceTexture( work->number_replace[ handle ] );
			DG_FreeMoveReplacePacket( work->number_replace[ handle ] );
			work->number_replace[ handle ] = NULL ;
		}
		work->number_replace[ handle ] =
		  DG_MakeMoveReplacePacket( NUMBER_TRI, number_tex_list[ handle ], number_tex_list[ work->number_count ] );
		if ( work->number_replace[ handle ] != NULL ){
			DG_SetMoveReplaceTexture( work->number_replace[ handle ] );
		}
	}
//#endif
	work->number_count++ ;
	return ( handle );
}

/* ナンバーモデルの開放 */
void RAYSERVER_FreeNumberModel( int handle )
{
	Work	*work ;
	int			i ;

	if ( work_ptr == NULL ) return ;
	work = work_ptr ;

	if ( handle == -1 ) return ;

	for ( i = 0 ; i < 6 ; i++ ){
		if ( work->number_free_buffer[ i ] == -1 ){
			work->number_free_buffer[ i ] = handle ;
			return ;
		}
	}
#ifdef DEBUG_MODE
	printf("%s:free number error!!\n", __FILE__);
#endif
}


/* ミサイル警告音用にミサイルの現在位置を登録 */
void RAYSERVER_SetMissile( FVECTOR *pos )
{
	FVECTOR		tmp_vec ;
	Work	*work ;
	float	len ;

	work = work_ptr ;
	if ( work_ptr == NULL ) return ;

	GTE_SubVector( &tmp_vec, &GM_PlayerPosition, pos );
	len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
	if ( ( work->missile_near_len == 0.0f ) || ( work->missile_near_len > len ) ){
		work->missile_near_len = len ;
	}
}
