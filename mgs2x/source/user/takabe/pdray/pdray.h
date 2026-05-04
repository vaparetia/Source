/*
	pdray.h
	量産型ＲＡＹ用ヘッダー

	2001/03/27 K.Takabe
	$Id: pdray.h,v 1.1.1.3 2002/11/19 11:51:26 Yoshizawa1 Exp $
*/

#include "r_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------- */
#ifdef DEBUG_MODE
//#define MOTION_BLUR			/* お遊び用モードＯＮ */
#define BLUR_INTERVAL	(4)
#define BLUR_NUM		(4)
#endif
/* ---------------------------------------------------------------- */
#define CTRL_FLAG	(CTRL_SKIP_CHANGE_MAP|CTRL_SKIP_GET_ADDRESS|CTRL_SKIP_TRAP)
#define BODY_FLAG	(DG_FLAG_FINISHEDCALC|DG_FLAG_IRREACTION)
//#define BODY_FLAG	(DG_FLAG_QUATROT)
#define STANDBY_MODEL_NAME	(15727783)		/* "pdray_standby_lod" */
#define BROKEN_MODEL_NAME	(5461654)		/* "pdray_out_p_lod" */
#define MODEL_FLAG	(DG_FLAG_ONEPIECE)

extern float BP_AdjustTick3(float);
#define V_SCALE				(BP_AdjustTick3(1.0f))		/* 速度の補正などに使用する */

#define PDRAY_HAZARD_JOINTS	(17)	

#define PDRAY_MAX_JOINTS	(34)
#define PDRAY_JOINT_WAIST		( 0 )
#define PDRAY_JOINT_BUST		( 2 )
#define PDRAY_JOINT_RIGHT_HAND	( 7 )
#define PDRAY_JOINT_LEFT_HAND	( 12 )
#define PDRAY_JOINT_HEAD		( 14 )
#define PDRAY_JOINT_NECK		( 13 )
#define PDRAY_JOINT_RIGHT_THIGH	( 22 )
#define PDRAY_JOINT_RIGHT_LEG1	( 23 )
#define PDRAY_JOINT_RIGHT_LEG2	( 24 )
#define PDRAY_JOINT_RIGHT_ANKLE	( 25 )
#define PDRAY_JOINT_RIGHT_TOE	( 26 )
#define PDRAY_JOINT_LEFT_THIGH	( 28 )
#define PDRAY_JOINT_LEFT_LEG1	( 29 )
#define PDRAY_JOINT_LEFT_LEG2	( 30 )
#define PDRAY_JOINT_LEFT_ANKLE	( 31 )
#define PDRAY_JOINT_LEFT_TOE	( 32 )
#define MOTION_INTERP_TIME	( 4 * 5 )

/* モーション再生用上半身マスク */
#define PDRAY_JOINT_MASK_UPPER	(0x1fffe)	/* 1_1111_1111_1111_1111_1110 */

#define DEG2RAD( _d ) ((_d)*3.14159265f/180.0f)

/* ＰＳ式角度を-2048~2047の範囲になるように正規化する */
#define REGULAR_ANGLE( _a )	( (signed short)( ((unsigned short)(_a)) << 4 ) >> 4 )

//#define RND(_n)		( ( ( BP_PS2_rand() >> 16 ) * ( _n ) ) >> 15 )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
#define ABS(_a)		( ( (_a) < 0 ) ? -(_a) : (_a) )

typedef ALIGN16_DECL(union) {
	float	m[4][4] ;
	FVECTOR	v[4] ;
} FMATRIX2 ;
#define _FMATRIX	FMATRIX2

/* ---------------------------------------------------------------- */
	/*
		構造体前宣言
	*/
struct _work ;
struct _proc_work;
/* ---------------------------------------------------------------- */
	/*
		プロシージャー管理システム
	*/
typedef int (*PROC_CALLBACK)( struct _work *, struct _proc_work * );
typedef struct _proc_work {
	int				mode ;			/* 思考モード */
	int				next_mode ;		/* 次の思考モード */
	int				old_mode ;		/* 前の思考モード */
	int				flag ;			/* 思考用各種フラグ */
	int				phase ;			/* 思考フェーズ */
	int				count ;			/* 思考用カウンタ */
	int				type ;			/* タイプ（類似ルーチン判別用） */
	char			*mark ;
	PROC_CALLBACK	act_func ;			/* メイン処理 */
	PROC_CALLBACK	die_func ;			/* -1以外を返せば終了処理完了 */
	PROC_CALLBACK	*new_func_list ;	/* 初期化関数リスト */
} PROC_WORK ;
enum {
	PROC_FLAG_PROTECT			= 0x00000001,	/* 思考切り替えの保護 */
};
#define PROC_SET_CALLBACK( _proc, _act, _die ) {(_proc)->act_func = (_act) ; (_proc)->die_func = (_die) ;}

/* ---------------------------------------------------------------- */
typedef struct _work {
	GV_ACT_EX		actor ;
	int				name ;
	int				map ;
	int				hzx_id ;
	int				no ;						/* キャラクタ通し番号 */
	char			disp_name[8] ;				/* 表示用キャラクタ名(RAY01~RAY??) */
	int				flags ;						/* 各種ステータスフラグ */
	int				game_condition ;			/* ゲーム状態ステータス（サーバー監視用） */

	CONTROL			control ;
	OBJECT			object ;
	FMATRIX			light[2] ;
	FVECTOR			rots[ PDRAY_MAX_JOINTS ];
	FVECTOR			abs_rots[ PDRAY_MAX_JOINTS ];
	DG_OBJS			*standby_lod_objs ;				/* 待機ワンスキンモデル */
	DG_OBJS			*broken_lod_objs ;				/* 破壊ワンスキンモデル */

	FMATRIX			world ;			/* 設置位置（Ｙ＝０でのマトリクス） */

	int				adjust_target_rot_y ;		/* Ｙ回転補正目標値 */
	int				adjust_rot_y ;				/* Ｙ回転補正量 */

	/* ライフゲージ表示 */
	GM_GageSet		life_gauge ;

	/* ゲーム管理ワーク */
	int				life ;					/* ライフ */
	int				life_max ;				/* ライフ最大値 */
	int				invinsible_time ;		/* 無敵時間 */
	int				chaff_freeze_count ;	/* チャフ固定化カウンタ */

	/* 各種状態記録用 */
	int				active_flag ;				/* 動作状態フラグ（０の時には必要最小限の処理しかしない） */
	int				damaged_flag ;				/* ダメージフラグ */
	int				on_stage_flag ;				/* ステージ上乗り込みフラグ */
	int				enter_attack_area_flag ;	/* プレイヤーが攻撃可能領域に入ったことを示すフラグ */
	int				chaff_disable ;				/* チャフ効果無効フラグ */
	int				chaff_flag ;				/* チャフダメージ状態フラグ */
	int				open_mouth_flag ;			/* 口オープンフラグ */
	int				semitrans_count ;			/* 半透明化カウンタ */
	int				disable_attack1_count ;		/* 水圧カッター攻撃中止カウンタ */
	int				disable_attack2_count ;		/* バルカン攻撃中止カウンタ */
	int				disable_attack3_count ;		/* ミサイル攻撃中止カウンタ */
	int				disable_attack4_count ;		/* 踏みつけ攻撃中止カウンタ */
	int				disable_head_ik_flag ;		/* 首ＩＫ禁止フラグ */
	int				n_attack_count ;			/* ＲＡＹの連続攻撃回数カウント */
	int				idle_time ;					/* 何もしない時間計測用 */
	int				player_tumble_wait_time ;	/* プレイヤーを転ばせてからの攻撃猶予時間 */
	int				mini_invinsible_time ;		/* 単一攻撃による連続ダメージ禁止用無敵時間 */
	int				move1_stop_flag ;			/* MOVE1の強制停止フラグ */

	/* 思考関連 */
	int				think_player_length ;		/* プレイヤーまでの距離 */
	int				think_player_angle ;		/* プレイヤー位置への相対方向 */
	FVECTOR			stage_stand_pos ;		/* ステージ上での立ち位置 */
	int				think_area_check[8];	/* エリアチェック計算結果 */
	/* 第１思考モード（最上位） */
	PROC_WORK		think1_work ;			/* 思考ワーク */
	/* 第２思考モード */
	PROC_WORK		think2_work ;			/* 思考ワーク */
	/* 第３思考モード（必要に応じて） */
	PROC_WORK		think3_work ;			/* 思考ワーク */

	/* 動作制御 */
	int				demand_action_bit ;		/* 要求されているアクションビット */
	int				select_action_bit ;		/* 現在選択中のアクションビット */
	int				true_action_bit ;		/* 実際に実行されているアクションビット */

	int				true_action ;			/* 実際に実行されているアクション */
	PROC_WORK		action_work ;

	/* モーション発動関連 */
	int				current_motion_num[4] ;		/* 現在再生中のモーション番号 */
	int				current_motion_flag[4] ;	/* 現在再生中のモーションフラグ */
	float			motion_speed ;				/* モーションの再生スピード設定（モーション再生ごとに1.0にリセット） */

	/* 混乱モーション関連（０番は腰移動量、それ以外は関節回転量） */
	FVECTOR			confusion_offset_trg[4] ;	/* 混乱モーション時オフセット目標値 */
	FVECTOR			confusion_offset[4] ;		/* 混乱モーション時オフセット */
	int				confusion_offset_count[4];	/* 混乱モーション時カウント */

	/* 移動関連ワーク */
	PROC_WORK		move_work ;
	FVECTOR			move_target ;			/* 移動目標座標 */
	FVECTOR			dir_target ;			/* 方向転換目標座標 */
	FVECTOR			move_cursor_mov ;		/* 移動用カーソル座標 */
	SVECTOR			move_cursor_rot ;		/* 移動用カーソル方向 */
	int				r_leg_angle_to ;		/* ＩＫ目標位置の足首角度（右） */
	int				r_leg_angle_from ;		/* ＩＫ目標位置の足首角度（右） */
	int				r_leg_angle_diff ;		/* ＩＫ目標位置の足首角度（右） */
	int				l_leg_angle_to ;		/* ＩＫ目標位置の足首角度（左） */
	int				l_leg_angle_from ;		/* ＩＫ目標位置の足首角度（左） */
	int				l_leg_angle_diff ;		/* ＩＫ目標位置の足首角度（左） */
	int				last_move_leg ;			/* 最後に動かした足（０：右、１：左） */
	int				move_cancel_flag ;		/* 移動処理キャンセルフラグ */
	int				move_type ;				/* 移動方法（0:直線移動 1:極座標移動） */
	FVECTOR			move_last_check_mov_r ;	/* 最後にチェックしたカーソル座標 */
	FVECTOR			move_last_check_mov_l ;	/* 最後にチェックしたカーソル座標 */
	SVECTOR			move_last_check_rot_r ;	/* 最後にチェックしたカーゾル方向 */
	SVECTOR			move_last_check_rot_l ;	/* 最後にチェックしたカーゾル方向 */
	/* 歩きパターン差吸収用パラメータ保持ワーク */
	int				move_speed ;
	int				turn_speed ;
	int				motion_idle2walk ;
	int				motion_walk ;
	int				motion_walk2idle ;
	int				walk_phase2_time ;
	/* 移動：ジャンプ時固有ワーク */
	FVECTOR			jump_start_pos ;		/* ジャンプ開始座標 */
	FVECTOR			jump_end_pos ;			/* ジャンプ終了座標 */
	FVECTOR			jump_offset ;			/* ジャンプ移動オフセット */

	/* ＩＫ関連 */
	int				right_leg_ik_mode ;		/* ＩＫ制御フラグ */
	int				left_leg_ik_mode ;		/* ＩＫ制御フラグ */
	int				right_arm_ik_mode ;		/* ＩＫ制御フラグ */
	int				left_arm_ik_mode ;		/* ＩＫ制御フラグ */
	MT_IK_CONTROL	*ik_right_leg ;			/* ＩＫワーク */
	MT_IK_CONTROL	*ik_left_leg ;			/* ＩＫワーク */
	MT_IK_CONTROL	*ik_right_arm ;			/* ＩＫワーク */
	MT_IK_CONTROL	*ik_left_arm ;			/* ＩＫワーク */
	FVECTOR			right_leg_target_pos ;	/* ＩＫ関節制御座標 */
	FVECTOR			left_leg_target_pos ;	/* ＩＫ関節制御座標 */
	FVECTOR			right_arm_target_pos ;	/* ＩＫ関節制御座標 */
	FVECTOR			left_arm_target_pos ;	/* ＩＫ関節制御座標 */
	FVECTOR			next_right_leg_target_pos ;	/* 次の右足移動先座標 */
	FVECTOR			next_left_leg_target_pos ;	/* 次の左足移動先座標 */
	float			right_leg_fall_speed ;	/* 右足の足の落下スピード */
 	float			left_leg_fall_speed ;	/* 左足の足の落下スピード */
	/* ＩＫ制御点アニメーション関連 */
	FVECTOR			ik_target ;				/* ＩＫ制御点操作座標（以下の４つからエルミート補間で生成） */
	FVECTOR			ik_target_from ;		/* ＩＫ制御点補間元 */
	FVECTOR			ik_target_to ;			/* ＩＫ制御点補間先 */
	FVECTOR			ik_target_from_speed ;	/* ＩＫ制御点補間元スピード */
	FVECTOR			ik_target_to_speed ;	/* ＩＫ制御点補間先スピード */

	/* 首のアニメーション関連 */
	FVECTOR			head_look_pos ;			/* 頭の向いている地点 */
	FVECTOR			head_look_old_pos ;		/* 以前頭の向いていた方向 */
	FVECTOR			head_look_start_pos ;	/* 首ＩＫアニメーション用ワーク */
	FVECTOR			head_look_end_pos ;		/* 首ＩＫアニメーション用ワーク */
	int				head_look_flag ;		/*  */
	MT_IK_CONTROL	*ik_head ;				/*  */

	/* 攻撃関連 */
	FVECTOR			attack_pos ;			/* 現在攻撃中座標（以下の４つからエルミート補間で生成） */
	FVECTOR			attack_start_pos ;		/* 攻撃開始座標 */
	FVECTOR			attack_end_pos ;		/* 攻撃終了座標 */
	FVECTOR			attack_start_speed ;	/* 攻撃開始スピード */
	FVECTOR			attack_end_speed ;		/* 攻撃終了スピード */
	FVECTOR			attack_player_old_pos ;	/* 攻撃処理に使用するプレイヤーの旧座標（動き予測用） */
	int				attack_type ;			/* 攻撃タイプ */
	int				attack_count ;			/* 攻撃用カウンタ */
	int				attack_end_count ;		/* 攻撃用終了カウンタ（タイミング決定用） */
	int				attack_length ;			/* 攻撃用距離保持 */
	int				attack_angle ;			/* 攻撃用方向保持 */
	int				attack_phase ;			/* 攻撃用フェーズ */
	void			*attack_effect_work ;	/* 攻撃用エフェクトワーク保持用 */
	int				attack_type_cycle ;		/* 攻撃方法サイクル記憶用 */
	int				n_water_tank ;			/* 水圧カッター用水タンク残り量 */

	/* ターゲット関連 */
	TARGET			lock_on_target ;			/* ロックオン用ターゲット */
	TARGET			lock_on_target_r_leg ;			/* ロックオン用ターゲット */
	TARGET			lock_on_target_l_leg ;			/* ロックオン用ターゲット */
	TARGET			collision_dummy_target ;	/* スティンガー破壊用ダミー防御ターゲット */
	POWER_TARGET	power_target ;				/* パワーターゲット */
	FVECTOR			target_force ;				/* ターゲット力積ベクトル */
	FVECTOR			damage_pos ;				/* ダメージ座標 */
	TARGET			earthquake_target ;			/* 地震吹っ飛び用ターゲット */
	POWER_TARGET	earthquake_power_target ;	/* 地震吹っ飛び用パワーターゲット */
	FVECTOR			earthquake_pos ;			/* 地震座標 */
	TARGET			right_leg_attack ;			/* 右足攻撃判定 */
	TARGET			left_leg_attack ;			/* 左足攻撃判定 */
	POWER_TARGET	right_leg_power ;			/* 右足パワーターゲット */
	POWER_TARGET	left_leg_power ;			/* 左足パワーターゲット */
	int				target_damage_flag ;		/* 食らい判定フラグ */
	float			collision_check_range ;		/* あたり判定判定開始距離（の二乗） */
	int				force_stg_break_flag ;		/* 強制的にスティンガーを破壊するフラグ */
	int				earthquake_flag ;			/* 地震発生フラグ */
	int				earthquake_effect_flag ;	/* 地震発生エフェクトフラグ */

	/* 動的フロア（跳弾当たり用） */
	HZX_D_FLOOR		*d_floors[PDRAY_HAZARD_JOINTS][6] ;	/* ダイナミックフロア（レベルチェック禁止！） */

	/* 固定パラメータ等 */
	FVECTOR			ik_base_right_leg_offset ;	/* 基本足位置へのオフセット（右足） */
	FVECTOR			ik_base_left_leg_offset ;	/* 基本足位置へのオフセット（左足） */

	/* ＳＥ関連 */
	int				chaff_se_timing ;			/* チャフ軋み時ＳＥ呼び出しタイミング */
	int				chaff_se_code ;				/* チャフ軋み時ＳＥ選択コード */

	/* エフェクト関連 */
	int				initialized_effect_flag ;	/* エフェクト初期化済みフラグ */
	/* 足水しぶきエフェクト用 */
	FVECTOR			old_leg_position[2] ;		/* 水しぶき用（0:右足,1:左足） */
	int				in_water_leg_flag[2] ;		/* 水しぶき用足着水フラグ（0:右足,1:左足） */
	/* 目エフェクト用 */
	void			*eye_look_effect_ptr ;		/* 目エフェクトアクターへのポインタ */
	FMATRIX			eye_look_pos ;				/* 目エフェクト注目座標 */
	int				eye_color ;					/* 目エフェクトカラー */
	int				eye_mode ;					/* 目エフェクトモード */
	/* 滲み血エフェクト用 */
	void			*oozeblood_effect_ptr ;		/* 滲み血エフェクトアクターへのポインタ */
	int				oozeblood_hit_joint ;		/* 滲み血用攻撃当たり関節番号記憶用 */
	int				oozeblood_count ;
	/* 出血エフェクト用 */
	int				blood_flag ;				/* 出血フラグ */
	int				blood_wait_time ;			/* 出血禁止時間 */
	int				blood_joint ;				/* 出血関節番号 */
	FMATRIX			blood_offset ;				/* 出血エフェクト用オフセット */
	/* 破壊煙用 */
	int				n_break_smokes ;			/* 現在出しているエフェクト数 */
	int				break_smoke_pos_flag ;		/* エフェクト生成個所フラグ */
	/* ブラー煙用 */
	void			*smoke_blur_work_ptr ;		/* アクターへのポインタ */

	/* サーバーとの通信用 */
	RAY_INFO		info ;
	int				number_model_handle ;		/* ナンバリングデータ取得ハンドル */

	/* シナリオ関連 */
	int				proc_id ;

	/* debug */
	FVECTOR			debug_target_pos ;
	char			*action_mark ;			/* アクション状態表示用 */
	char			*think_mark ;			/* 思考状態表示用 */
	char			*think1_mark ;			/* 思考状態表示用 */
	char			*think2_mark ;			/* 思考状態表示用 */
	char			*think3_mark ;			/* 思考状態表示用 */

	/* お遊び用 */
#ifdef MOTION_BLUR
	DG_OBJS			*blur_objs[BLUR_NUM] ;
	FMATRIX			blur_pos[BLUR_NUM*BLUR_INTERVAL];
	FVECTOR			blur_rots[BLUR_NUM*BLUR_INTERVAL][ PDRAY_MAX_JOINTS ] ;
#endif


} Work ;

#ifdef DEBUG_MODE
#define SET_MARK(_w,_s)	{(_w)->mark = (_s) ;}
#define ACTION_MARK(_s)	{work->action_mark = (_s);}
#define THINK_MARK(_s)	{work->think_mark = (_s);}
#define THINK1_MARK(_s)	{work->think1_mark = (_s);}
#define THINK2_MARK(_s)	{work->think2_mark = (_s);}
#define THINK3_MARK(_s)	{work->think3_mark = (_s);}
#else
#define SET_MARK(_w,_s)	{}
#define ACTION_MARK(_s)	{}
#define THINK_MARK(_s)	{}
#define THINK1_MARK(_s)	{}
#define THINK2_MARK(_s)	{}
#define THINK3_MARK(_s)	{}
#endif

/* ---------------------------------------------------------------- */
/* 状態ステータスフラグ */
enum {
	FLAG_NONE			= 0x0000,	/**/
	FLAG_JUMP			= 0x0001,	/* ジャンプ中（重力関係の計算必要なし） */
	FLAG_CONFUSION		= 0x0002,	/* 混乱（現在未使用） */
	FLAG_DAMAGE			= 0x0004,	/* ダメージ中（現在未使用） */
	FLAG_ON_STAGE		= 0x0008,	/* ステージ乗っかりフラグ（設置処理専用） */
};

/* 動作要求ビット */
/* （最初はサーバーから指示するのに使用したが、現在のバージョンではＲＡＹ内部からしか使用していない） */
enum {
	RAY_STATUS_NONE			= 0x00000000,		/* 何もなし */
	RAY_STATUS_ACTIVE		= 0x00000001,		/* 起動済み */
	RAY_STATUS_BROKEN		= 0x00000002,		/* 破壊済み */
	RAY_STATUS_IDLE			= 0x00000004,		/* 待機中 */
	RAY_STATUS_MOVE			= 0x000000f0,		/* 移動動作中（１・２・３・４） */
	RAY_STATUS_MOVE1		= 0x00000010,		/* 指定地点への移動中 */
	RAY_STATUS_MOVE2		= 0x00000020,		/* 自動モード用プレイヤー追っかけ移動 */
	RAY_STATUS_MOVE3		= 0x00000040,		/*  */
	RAY_STATUS_MOVE4		= 0x00000080,		/*  */
	RAY_STATUS_ATTACK		= 0x00000f00,		/* 攻撃動作中（１・２・３・４） */
	RAY_STATUS_ATTACK1		= 0x00000100,		/* 攻撃１動作中（水圧カッター） */
	RAY_STATUS_ATTACK2		= 0x00000200,		/* 攻撃２動作中（バルカン） */
	RAY_STATUS_ATTACK3		= 0x00000400,		/* 攻撃３動作中（ミサイル） */
	RAY_STATUS_ATTACK4		= 0x00000800,		/* 攻撃４動作中（キック） */
	RAY_STATUS_ROAR			= 0x00001000,		/* 叫び動作中 */
	RAY_STATUS_GUARD		= 0x00002000,		/* 警戒動作中 */
	RAY_STATUS_DAMAGE		= 0x00004000,		/* ダメージ動作中 */
	RAY_STATUS_CONFUSION	= 0x00008000,		/* 混乱動作中 */
	RAY_STATUS_JUMP1		= 0x00010000,		/* ステージにジャンプ移動 */
	RAY_STATUS_JUMP2		= 0x00020000,		/* ステージ外にジャンプ移動 */
	RAY_STATUS_JUMP3		= 0x00040000,		/* ジャンプ移動 */
	RAY_STATUS_BREAK		= 0x00100000,		/* 破壊崩れこみ */
	RAY_STATUS_STARTRUN		= 0x00200000,		/* 開始走り */
	RAY_STATUS_DAMAGE2		= 0x00400000,		/* 右足ダメージ動作中 */
	RAY_STATUS_DAMAGE3		= 0x00800000,		/* 左足ダメージ動作中 */
	RAY_STATUS_SATTACK		= 0x0f000000,		/* サポート攻撃動作中（１・２・３・４） */
	RAY_STATUS_SATTACK1		= 0x01000000,		/* サポート攻撃１ */
	RAY_STATUS_SATTACK2		= 0x02000000,		/* サポート攻撃２ */
	RAY_STATUS_SATTACK3		= 0x04000000,		/* サポート攻撃３ */
	RAY_STATUS_SATTACK4		= 0x08000000,		/* サポート攻撃４ */
	RAY_STATUS_STEP			= 0x10000000,		/* 足踏み */
	RAY_STATUS_DRINK		= 0x20000000,		/* 水のみ */
	RAY_STATUS_TELEPORT		= 0x40000000,		/* テレポート */
};


/* アクションステータス */
enum {
	ACTION_SLEEP,			/* 静止 */
	ACTION_IDLE,			/* 静止 */
	ACTION_MOVE1,			/* 移動１（指定位置への移動） */
	ACTION_MOVE2,			/* 移動２（プレイヤー追従移動） */
	ACTION_MOVE3,			/* 移動３（） */
	ACTION_MOVE4,			/* 移動４（ダメージ後退） */
	ACTION_ATTACK1,			/* 攻撃１（口から水圧カッター） */
	ACTION_ATTACK2,			/* 攻撃２（腕からバルカン） */
	ACTION_ATTACK3,			/* 攻撃３（ミサイル攻撃） */
	ACTION_ATTACK4,			/* 攻撃４（踏み潰し攻撃） */
	ACTION_ROAR,			/* 叫ぶ */
	ACTION_DAMAGE,			/* ダメージ */
	ACTION_JUMP1,			/* ジャンプ */
	ACTION_JUMP2,			/* ジャンプ */
	ACTION_CHARGE1,			/* 溜め */
	ACTION_GUARD,			/* 警戒（防御体制） */
	ACTION_CONFUSION,		/* 混乱 */
	ACTION_SATTACK1,		/* 場外攻撃１ */
	ACTION_SATTACK2,		/* 場外攻撃２ */
	ACTION_SATTACK3,		/* 場外攻撃３ */
	ACTION_SATTACK4,		/* 場外攻撃４ */
	ACTION_BREAK,			/* 崩れこみ */
	ACTION_RUN,				/* 走り */
	ACTION_JUMP3,			/* ジャンプ */
	ACTION_DAMAGE2,			/* 右足ダメージ */
	ACTION_DAMAGE3,			/* 左足ダメージ */
	ACTION_STEP,			/* 足踏み（体勢取り戻し用） */
	ACTION_DRINK,			/* 水のみ */
	ACTION_TELEPORT,		/* 瞬間移動 */
};
/* 移動アクションステータス */
enum {
	MOVE_IDLE,				/* 静止 */
	MOVE_WALK_READY,		/* 移動待機 */
	MOVE_WALK,				/* 歩き */
	MOVE_STEP,				/* 任意方向ステップ */
	MOVE_WALK2,				/* 歩き */
	MOVE_STEP2,				/* 任意方向ステップ */
	MOVE_FREE,				/* 自由移動 */
	MOVE_FREE2,				/* 自由移動 */

	MOVE_WALK_FORWARD,		/* 歩き前進 */
	MOVE_WALK_BACK,			/* 歩き後退 */
	MOVE_RUN_FORWARD,		/* 走り前進 */
	MOVE_RUN_BACK,			/* 走り後退 */
	MOVE_SIDE_RIGHT,		/* サイドステップ右 */
	MOVE_SIDE_LEFT,			/* サイドステップ左 */
	MOVE_VOID,				/* 移動の無効（アクションモーションに依存する場合） */
};

/* モーション再生制御フラグ */
enum {
	MOTION_FLAG_BODY_REVERS		= 0x0001,	/* 上半身左右反転 */
	MOTION_FLAG_LEG_REVERS		= 0x0002,	/* 下半身左右反転 */
	/* 補助フラグ */
	MOTION_FLAG_SUBMASK			= 0xf000,	/* 制御用サブフラグ用マスク */
	MOTION_FLAG_NOINTERP		= 0x2000,	/* 補間なし再生 */
	MOTION_FLAG_AGAIN			= 0x4000,	/* 同一モーションであってもモーション発行処理を行う */
	MOTION_FLAG_FORCE			= 0x8000,	/* 強制モーション切り替え */
};

/* ＩＫ制御モード */
enum {
	IK_MODE_FREE				= 0,		/* 自由（モーション通りに駆動＝固定用に座標記録） */
	IK_MODE_FIX					= 1,		/* 固定（以前に記録した座標にＩＫ制御。但し高さはモーションそのまま） */
	IK_MODE_CONTROL				= 2,		/* 操作（ＩＫ制御点をプログラム側で操作。但し高さはモーションそのまま） */
	IK_MODE_FIXFALL				= 3,		/* 固定落下（高さは自由落下で接地させる） */
};

/* 第１思考モード */
enum {
	THINK1_NULL,			/* 動作定義なし */
	THINK1_NONE,			/**/
	THINK1_STANDBY,			/* ステージ外待機状態 */
	THINK1_INATTACK,		/* ステージ内攻撃時 */
	THINK1_OUTATTACK,		/* ステージ外攻撃時 */
	THINK1_SUPPORT,			/* 場外補助攻撃時 */
	THINK1_BREAK,			/* 破壊後離脱処理 */
};
/* 第２思考モード */
enum {
	THINK2_NULL,		/* 動作定義なし */
	/* THINK1_INATTACK用関連 */
	THINK2_JUMPIN,		/* ステージ内乱入処理 */
	THINK2_JUMPOUT,		/* ステージ外退場処理 */
	THINK2_SEEK,		/* プレイヤー検索処理 */
	THINK2_PURSUE,		/* プレイヤー追従処理 */
	/* 未使用 */
	THINK2_ATTACK,		/* プレイヤー攻撃 */
	THINK2_DAMAGE,		/* プレイヤー攻撃 */
	/* THINK1_SUPPORT用関連 */
	THINK2_SA_STANDBY,	/* サポートアタック待機 */
	THINK2_SA_EXEC,		/* サポートアタック実行 */
	/* THINK1_OUTATTACK用関連 */
	THINK2_OA_STANDBY,	/* ステージ攻撃待機処理 */
	THINK2_OA_EXEC,		/* ステージ外攻撃処理 */
	THINK2_OA_MOVE,		/* ステージ外旋回移動処理 */
};
/* 第３思考モード */
enum {
	THINK3_NULL,		/* 動作定義なし */
};

/* 思考用エリアチェック種類 */
enum {
	AREA_KICK = 0,		/* キック攻撃エリア */
	//AREA_KICK_OVER,		/* キック攻撃キャンセル実行エリア */
	AREA_CUTTER,		/* 水圧カッター攻撃エリア */
	//AREA_CUTTER_OVER,	/* 水圧カッター攻撃キャンセル実行エリア */
	AREA_VALCAN,		/* バルカン攻撃エリア */
	//AREA_VALCAN_OVER,	/* バルカン攻撃キャンセル実行エリア */
	AREA_VALCAN_OVER_RIGHT,	/* バルカン攻撃キャンセル実行エリア */
	AREA_VALCAN_OVER_LEFT,	/* バルカン攻撃キャンセル実行エリア */
	AREA_MISSILE,		/* ミサイル攻撃エリア */
	//AREA_MISSILE_OVER,	/* ミサイル攻撃キャンセル実行エリア */
	AREA_MAX			/* 最大チェックエリア数 */
};

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		動作関連定数定義
	*/
#define WALK_TIME_MOVERIGHT		( 300 )		/* 右足駆動時間 */
#define WALK_TIME_MOVELEFT		( 300 )		/* 左足駆動時間 */
#define WALK_TIME_BALANCE		( 150 )		/* 重心移動時間 */
#define WALK_WIDTH				( 6000 )	/* 歩幅 */
#define WALK_HEIGHT				( 10000 )	/* 歩行時の腰高さ */
#define RUN_WIDTH				( 33500 )
#define RUN_STOP_RANGE			( 2500 )	/* 停止に必要な距離 */
//#define WALK_PHASE0_TIME		( 0 )		/* フェーズ切り替えモーションタイミング０ */
//#define WALK_PHASE1_TIME		( 145 )		/* フェーズ切り替えモーションタイミング１ */
//#define WALK_PHASE2_TIME		( 275 )		/* フェーズ切り替えモーションタイミング２ */
//#define WALK_PHASE3_TIME		( 495 )		/* フェーズ切り替えモーションタイミング３ */
#if 0
#define WALK_PHASE0_TIME		( 0 )		/* フェーズ切り替えモーションタイミング０ */
#define WALK_PHASE1_TIME		( 45 )		/* フェーズ切り替えモーションタイミング１ */
#define WALK_PHASE2_TIME		( 125 )		/* フェーズ切り替えモーションタイミング２ */
#define WALK_PHASE3_TIME		( 195 )		/* フェーズ切り替えモーションタイミング３ */
#else
#define WALK_PHASE0_TIME		( 0 )		/* フェーズ切り替えモーションタイミング０ */
#define WALK_PHASE1_TIME		( 6 )		/* フェーズ切り替えモーションタイミング１ */
#define WALK_PHASE2_TIME		( 189 )		/* フェーズ切り替えモーションタイミング２ */
#define WALK_PHASE3_TIME		( 195 )		/* フェーズ切り替えモーションタイミング３ */
#endif
#define MOVE_SPEED				( 150 )		/* 移動スピード（最終的には可変にするかも） */
#define TURN_SPEED				( 20 )		/* ターンスピード */
#define MOVE_SPEED2				( 140 )		/* 移動スピード（最終的には可変にするかも） */
#define TURN_SPEED2				( 18 )		/* ターンスピード */

#define STAGE_IN_RANGE			(13000)		/* ステージ内出入り基準距離 */
#define STAGE_OUT_RANGE			(13000)		/* ステージ内出入り基準距離 */
#define JUMP_OUT_RANGE			(50000)		/* ジャンプアウト時ステージ外着地距離 */

//#define TURN_MAX_ANGLE			( 800 )		/* 最大ターン角度 */
#define TURN_MAX_ANGLE			( 1024 )		/* 最大ターン角度 */
#define MAX_ADJUST_ANGLE		( 600 )			/* 腰の最大補正量 */

#define WALK2_WIDTH				( 4000 )	/* 歩幅 */
#define WALK2_PHASE0_TIME		( 0 )		/* フェーズ切り替えモーションタイミング０ */
#define WALK2_PHASE1_TIME		( 6 )		/* フェーズ切り替えモーションタイミング１ */
#define WALK2_PHASE2_TIME		( 288 )		/* フェーズ切り替えモーションタイミング２ */
#define WALK2_PHASE3_TIME		( 295 )		/* フェーズ切り替えモーションタイミング３ */

#define ATTACK1_NEAR_LENGTH		( 8000 )	/* 攻撃開始距離 */
#define ATTACK1_FAR_LENGTH		( 22000 )	/* 攻撃終了距離 */
#define ATTACK2_WIDTH			( 6000 )	/* バルカン掃射幅／２ */
#define ATTACK4_RANGE			( 10000 )	/* キック許可距離（元の位置から踏みつけ位置までの距離） */

/* 水圧カッター攻撃パラメータ */
#define ATTACK1_PHASE0_TIME		( 0 )		/*  */
#define ATTACK1_PHASE1_TIME		( 200-RAYSERVER_GameUpLevel*60 )		/* 攻撃開始 */
//#define ATTACK1_PHASE2_TIME		( 650 )		/* 攻撃終了 */
//#define ATTACK1_PHASE3_TIME		( 850 )		/* モーション中止時間 */
#define ATTACK1_PHASE2_TIME		( 950-RAYSERVER_GameUpLevel*180 )		/* 攻撃終了 */
#define ATTACK1_PHASE3_TIME		( 1250-RAYSERVER_GameUpLevel*300 )		/* モーション中止時間 */
/* 腕バルカン攻撃パラメータ */
#define ATTACK2_PHASE0_TIME		( 0 )		/*  */
#define ATTACK2_PHASE1_TIME		( 100 )		/* （未使用） */
#define ATTACK2_PHASE2_TIME		( 600-RAYSERVER_GameUpLevel*75 )		/* 攻撃時間 */
#define ATTACK2_PHASE3_TIME		( 1300 )	/* （未使用） */
/* ミサイル攻撃パラメータ */
#define ATTACK3_PHASE0_TIME		( 0 )		/*  */
#define ATTACK3_PHASE1_TIME		( 100 )		/*  */
#define ATTACK3_PHASE2_TIME		( 800 )		/*  */
#define ATTACK3_PHASE3_TIME		( 1700 )		/*  */
/* 踏みつけ攻撃パラメータ */
#define ATTACK4_PHASE0_TIME		( 0 )		/*  */
#define ATTACK4_PHASE1_TIME		( 415 )		/*  */
#define ATTACK4_PHASE2_TIME		( 495 )		/* 未使用 */
#define ATTACK4_PHASE3_TIME		( 495+320 )		/* 未使用 */
/* ジャンプパラメータ */
#define JUMP_PHASE0_TIME		(120)			/* ジャンプ開始フレーム（1/300秒単位） */
#define JUMP_PHASE1_TIME		(355)			/* ジャンプ終了フレーム（1/300秒単位） */
/* その他時間パラメータ */
#define DRINK_TIME				(900)		/* 水のみ時間 */
#define ROAR_BREATH_TIME0		(240)		/* 息エフェクト発生開始時間 */
#define ROAR_BREATH_TIME1		(705)		/* 息エフェクト発生終了時間 */
#define IDLE_MAX_TIME			(300*10)			/* アイドル許容最大時間 */

#define PLAYER_TUMBLE_WAIT_TIME	(700)			/* プレイヤーを転ばせた時の攻撃猶予時間 */

/* 同一タイプ攻撃禁止期間パラメータ */
#define DISABLE_ATTACK1_COUNT	(1500-RAYSERVER_GameUpLevel*200)
#define DISABLE_ATTACK2_COUNT	(1500-RAYSERVER_GameUpLevel*200)
#define DISABLE_ATTACK3_COUNT	(1500-RAYSERVER_GameUpLevel*200)
#define DISABLE_ATTACK4_COUNT	(1500-RAYSERVER_GameUpLevel*200)
/* 出血エフェクト関係パラメータ */
#define BLOOD_WAIT_TIME			(300)			/* 次の出血までのウェイト時間 */
#define BLOOD_TIME				(450)			/* 血の噴出す時間 */

#define INATTACK_TIME			( 30 * 300 )	/* ステージ内攻撃フェーズ */

#define ATTACK_ROAR_COUNT		(4)			/* 叫びを発動させるのに必要な攻撃回数 */


#define INVINSIBLE_TIME			(595+195*2+220*2)		/* 無敵時間 */
#define MINI_INVINSIBLE_TIME	(5*6)					/* 不具合回避用ミニ無敵時間 */

/* 各種サイズ座標関連パラメータ */
#define TARGET_HEAD_SIZE_X		(2500/2)
#define TARGET_HEAD_SIZE_Y		(2500/2)
#define TARGET_HEAD_SIZE_Z		(2500/2)
#define TARGET_SIZE_X		(2200)
#define TARGET_SIZE_Y		(2200)
#define TARGET_SIZE_Z		(2200)
#define EARTHQUAKE_RANGE1	(6000)		/* 踏み潰し時地震影響半径 */
#define EARTHQUAKE_RANGE2	(12000)		/* ジャンプ時地震影響半径 */
#define KICKDAMAGE_RANGE	(3000)		/* キックダメージ時の吹っ飛び作用半径 */
#define KICKDAMAGE_SPEED	(400*V_SCALE)		/* キックダメージ時の吹っ飛び最大スピード */
#define KICKDAMAGE_THRESHOLD	(100*V_SCALE)	/* キックターゲットのダメージ・押し出し閾値スピード */
#define LEG_PUSH_HEIGHT		(1500)		/* 足の押し出し開始高さ差分値 */
#define LEG_PUSH_RANGE		(2000)		/* 足の押し出し半径 */

/* ダメージ関連パラメータ */
#define MAX_LIFE			(1024)		/* ＲＡＹの最大ライフ */
//#define MAX_LIFE			(180)		/* ＲＡＹの最大ライフ */
//#define DAMAGE_STINGER		(160)		/* スティンガーにより受けるダメージ量（未使用） */
//#define DAMAGE_RGB6			(100)		/* スティンガーにより受けるダメージ量（未使用） */
#define DAMAGE_STINGER_A	(100)		/* スティンガーにより受けるダメージ量（口閉じ時） */
#define DAMAGE_STINGER_B	(320)		/* スティンガーにより受けるダメージ量（口開け時） */
#define DAMAGE_RGB6_A		( 70)		/* ＲＧＢ６により受けるダメージ量（口閉じ時） */
#define DAMAGE_RGB6_B		(200)		/* ＲＧＢ６により受けるダメージ量（口開け時） */
//#define DAMAGE_STINGER		(380)		/* スティンガーにより受けるダメージ量 */
#define KICK_DAMAGE			(48)		/* キックダメージ量 */
#define VALCAN_DAMAGE		(16)		/* バルカンダメージ量 */
#define CUTTER_DAMAGE		(24)		/* 水圧カッターダメージ量 */

/* 歩き待機中の足の腰からのオフセット位置 */
#define RIGHT_LEG_OFFSET_X	(-4050.792969f)
#define RIGHT_LEG_OFFSET_Y	(-10249.565430f)
#define RIGHT_LEG_OFFSET_Z	(2655.476562f)
#define LEFT_LEG_OFFSET_X	(4050.793457f)
#define LEFT_LEG_OFFSET_Y	(-10249.565430f)
#define LEFT_LEG_OFFSET_Z	(2655.476562f)
/* 足首のフロアからの高さ */
#define ANKLE_HEIGHT		(1950.0f)
#define ANKLE_ACCELERATE	(10.0f)			/* 落下加速度 */


/* プレイヤーフロア高さ */
#define PLAYER_LEVEL		(4000.0f)

#define WATER_LEVEL			(1500.0f)		/* 場外での仮想水面高さ */


/* ---------------------------------------------------------------- */
/* 足のＩＫモードを変更する */
#define SET_LEG_IK_MODE( _r, _l )	{ work->right_leg_ik_mode = _r; work->left_leg_ik_mode = _l ; }

#ifdef __GNUC__
/* 攻撃タイミングが指定時間を経過したかどうかをチェックする（１/３００秒単位） */
#define PDRAY_ATTACK_TIMING( _n ) \
({\
	int _f = 0 ; \
	if ( work->attack_count >= (_n) && work->attack_end_count < (_n) ){\
		work->attack_end_count = work->attack_count ;\
		_f = 1 ;\
	}\
	(_f);\
})

/* 現在のモーションの経過割合を０～１で返す（２ループ目移行は常に１） */
#define MOTION_PLAY_RATE( _body, _l )	({\
	float	_t ;\
	_t = (_body)->m_ctrl->mt3_ctrl[ _l ].play_time / (_body)->m_ctrl->mt3_ctrl[ _l ].motion_total_time ;\
	if ( (_body)->m_ctrl->mt3_ctrl[ _l ].loop != 0 ) _t = 1.0f ;\
	(_t);\
})
#else
/* 攻撃タイミングが指定時間を経過したかどうかをチェックする（１/３００秒単位） */
#define PDRAY_ATTACK_TIMING( _n ) _PDRAY_ATTACK_TIMING( work, _n )
static inline int _PDRAY_ATTACK_TIMING( Work *work, int _n )
{
	int _f = 0 ;
	if ( work->attack_count >= (_n) && work->attack_end_count < (_n) ){
		work->attack_end_count = work->attack_count ;
		_f = 1 ;
	}
	return (_f);
}

/* 現在のモーションの経過割合を０～１で返す（２ループ目移行は常に１） */
static inline float MOTION_PLAY_RATE( OBJECT *_body, int _l )
{
	float	_t ;
	_t = (_body)->m_ctrl->mt3_ctrl[ _l ].play_time / (_body)->m_ctrl->mt3_ctrl[ _l ].motion_total_time ;
	if ( (_body)->m_ctrl->mt3_ctrl[ _l ].loop != 0 ) _t = 1.0f ;
	return (_t);
}
#endif

/* ---------------------------------------------------------------- */
/* モーション番号定義 */
enum {
	MOTION_IDLE,			/* 通常静止 */
	MOTION_WALK_READY,		/* 移動用静止 */
	MOTION_IDLE2WALK,		/* 通常静止から歩きスタンバイ */
	MOTION_WALK2IDLE,		/* 歩きスタンバイから静止へ */
	MOTION_WALK,			/* 歩き（右足のみ。左足は反転して再生） */
	MOTION_ROAR,			/* 叫び */
	MOTION_CONFUSION,		/* チャフによる混乱 */
	MOTION_GUARD,			/* 警戒 */
	MOTION_SLAP,			/* スティンガーミサイルはたき */
	MOTION_ATTACK1_START,	/* 攻撃１開始 */
	MOTION_ATTACK1_LOOP,	/* 攻撃１攻撃中ループ */
	MOTION_ATTACK1_END,		/* 攻撃１終了 */
	MOTION_ATTACK2_START,	/* 攻撃１開始 */
	MOTION_ATTACK2_LOOP,	/* 攻撃１攻撃中ループ */
	MOTION_ATTACK2_END,		/* 攻撃１終了 */
	MOTION_ATTACK3_START,	/* 攻撃１開始 */
	MOTION_ATTACK3_LOOP,	/* 攻撃１攻撃中ループ */
	MOTION_ATTACK3_END,		/* 攻撃１終了 */
	MOTION_JUMP,			/* ジャンプ */
	MOTION_KICK,			/* キック */
	MOTION_DAMAGE,			/* ダメージ */
	MOTION_BREAK_WALK,		/* ふらつき歩き */
	MOTION_BREAK,			/* 崩れこみ */
	MOTION_BREAK_PAUSE,		/* 崩れこみ静止 */
	MOTION_RUN,				/* 走り */
	MOTION_RUN_STOP,		/* 走り終了 */
	MOTION_RUN_START,		/* 走り開始 */
	MOTION_LEG_DAMAGE,		/* 右足攻撃受け */
	MOTION_STANDBY,			/* 待機状態 */
	MOTION_STANDBY2IDLE,	/* 待機状態からアイドリングへ */
	MOTION_DRINK_START,		/* 水のみ開始 */
	MOTION_DRINK_LOOP,		/* 水のみループ */
	MOTION_DRINK_END,		/* 水のみ終了 */
	MOTION_END
};
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/* プロトタイプ宣言 */
/* pdray.c */
extern void PDRAY_SetDemandActionBit( Work *work, int action_bit );
extern void PDRAY_ResetDemandActionBit( Work *work, int action_bit );
extern int PDRAY_GetGameCondition( Work *work );
extern void PDRAY_DisplayStatus( Work *work );


/* pdr_move.c */
extern void PDRAY_RecordIKPosition( Work *work );
extern void PDRAY_ResetMoveAction( Work *work );
extern int PDRAY_SetNextMoveAction( Work *work, int next_move_action );
extern void PDRAY_UpdateMoveActionStatus( Work *work );

/* pdr_thk.c */
extern void PDRAY_InitThink( Work *work );
extern void PDRAY_ThinkAct( Work *work );
extern void PDRAY_InitProcWork( Work *work, PROC_WORK *think_work, PROC_CALLBACK *proc_init_list );
extern void PDRAY_SetNextProc( PROC_WORK *proc_work, int next_mode, int flag );
extern int PDRAY_ActProc( Work *work, PROC_WORK *proc_work );
extern void PDRAY_DumpProc( PROC_WORK *proc_work, char *header );

/* pdr_act.c */
extern int PDRAY_SelectAction( Work *work );
extern void PDRAY_ResetAction( Work *work );
extern int PDRAY_SetNextAction( Work *work, int next_action );

/* pdr_coll.c */
extern void PDRAY_InitCollision( Work *work );
extern void PDRAY_EndCollision( Work *work );
extern void PDRAY_ActCollision( Work *work );
extern int PDRAY_CheckNearStinger( Work *work );
extern int PDRAY_GetFloor( Work *work, FVECTOR *pos );
extern void PDRAY_CollisionEdit( Work *work );

/* pdr_move.c */
extern void PDRAY_ForceRecordLegPosition( Work *work );
extern void PDRAY_RecordIKPosition( Work *work );
extern int PDRAY_SetMotion( Work *work, int motion_num, int flag );
extern int PDRAY_SetMotionUpper( Work *work, int motion_num, int flag );
extern int PDRAY_SetMotionCheck( Work *work, int motion_num, int flag );
extern void PDRAY_ResetMoveAction( Work *work );
extern int PDRAY_SetNextMoveAction( Work *work, int next_move_action );

/* pdr_thk1.c */
extern void PDRAY_InitThink1Work( Work *work );

/* pdr_thk2.c */
extern void PDRAY_GetPositionInfo2D( FVECTOR *_diff, int *_len, int *_angle, FVECTOR *from, FVECTOR *to, int rot_base );
extern int PDRAY_SetNextThink2( Work *work, int next_think2_mode );
extern void PDRAY_InitThink2( Work *work );

/* pdr_thk3.c */
extern int PDRAY_SetNextThink3( Work *work, int next_think3_mode );
extern void PDRAY_InitThink3( Work *work );

/* pdr_efct.c */
extern void PDRAY_InitEffect( Work *work );
extern void PDRAY_EndEffect( Work *work );
extern void PDRAY_ActEffect( Work *work );
extern void PDRAY_SetBreakSmoke( Work *work, int level );

/* pdr_ik.c */
extern void PDRAY_ActIk( Work *work );
extern void PDRAY_EndIk( Work *work );
extern void PDRAY_InitIk( Work *work );


#ifdef KP_XBOX
#define GM_SeSetMode( _a, _b, _c ) GM_SeSetModeAddr( _a, _b, _c, GM_INVALID_ADDR )
#endif


/* ---------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif



