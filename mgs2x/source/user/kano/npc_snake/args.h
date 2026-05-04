/*
	args.h

	2001/02/13 K.Kano
	$Id: args.h,v 1.1.1.3 2002/11/19 11:43:20 Yoshizawa1 Exp $
 */


#ifndef _args_h_
#define _args_h_


enum {
	SMP_MOT_SQUAT = NPC_BASE_MOT_END ,
	SMP_MOT_IYA_IDLE,
	SMP_MOT_DOWN,
	SMP_MOT_SUMMER,
	SMP_MOT_SLIDING,
	SMP_MOT_ELBOW,
	SMP_MOT_SHOUTEI,
	SMP_MOT_SPIRAL,
	SMP_MOT_SPINKICK_HIGH,
	SMP_MOT_SPINKICK_LOW,

	SMP_MOT_DAM_PUNCH_L,
	SMP_MOT_DAM_PUNCH_R,
	SMP_MOT_DAM_KICK,
	SMP_MOT_DOWN_F,
	SMP_MOT_WAKEUP_F,
} ;

enum {
	SMP_TARGET_CHILD_HEAD=0,
	SMP_TARGET_CHILD_HART,

	SMP_TARGET_CHILD_ARMR1,
	SMP_TARGET_CHILD_ARMR2,
	SMP_TARGET_CHILD_ARML1,
	SMP_TARGET_CHILD_ARML2,

	SMP_TARGET_CHILD_LEGR1,
	SMP_TARGET_CHILD_LEGR2,
	SMP_TARGET_CHILD_LEGL1,
	SMP_TARGET_CHILD_LEGL2,

	SMP_TARGET_CHILD_BPDY,

	CHILD_TARGET_NUM,
};


/* モーションに対してのフラグ */
enum {
	/* モーションをキャンセルできる */
	SNAKE_STATUS_CANCELABLE=0x0001,

	/* アクションの終了。
	   モーションの終了ではなく、ある動作単位の終了をあらわす。*/
	SNAKE_STATUS_ACTION_FINISHED=0x0002,

	/* ショットアクションの終了。
	   移動との組み合わせのケースがあるので、ショットのみ別にした。 */
	SNAKE_STATUS_SHOOT_ACTION_FINISHED=0x0004,

	/* 攻撃開始のために敵にロックオンする */
	SNAKE_STATUS_LOCKON_ENEMY=0x0008,


	/* ライデンに対して先行しすぎた場合に立つフラグ */
	SNAKE_STATUS_TOO_OVER_RAIDEN=0x0010,
	SNAKE_STATUS_OVER_RAIDEN=0x0020,
	SNAKE_STATUS_OVER_RAIDEN0=0x0040,

	/* ライデンが先行して進んでいった場合 */
	SNAKE_STATUS_BEHIND_RAIDEN_L0=0x0100,
	SNAKE_STATUS_BEHIND_RAIDEN_L1=0x0200,
	SNAKE_STATUS_BEHIND_RAIDEN_L2=0x0400,
	SNAKE_STATUS_BEHIND_RAIDEN_L3=0x0800,

	/* スネークが今持っている武器 */
	SNAKE_STATUS_HAS_USP=0x1000,
	SNAKE_STATUS_HAS_FMS=0x2000,

	/* スネークは座っている */
	SNAKE_STATUS_SITTING_MOTION=0x4000,

	/* スネークは歩いている */
	SNAKE_STATUS_WALKING_MOTION=0x8000,

	/* スネークは移動している */
	SNAKE_STATUS_MOVING_MOTION=0x00010000,

	/* スネークはUSPを使うべき */
	SNAKE_STATUS_USE_USP=0x00020000,

	/* スネークはファマス(もしくはM4)を使うべき */
	SNAKE_STATUS_USE_FMS=0x00040000,

	/* スネークはかなりライデンを待っている */
	SNAKE_STATUS_WAIT_RAIDEN_OVERTIME=0x00080000,

	/* スネークの射撃の射線を合わせるため補正を行う。*/
	SNAKE_STATUS_CALCADJUST=0x00100000,
	SNAKE_STATUS_CALCADJUST0=0x00200000,

	/* 弾を撃つタイミングを指示 */
	SNAKE_STATUS_MAKEBULLET=0x00400000,

	/* 狙撃の依頼がきた(w32a) */
	SNAKE_STATUS_ORDER_TO_SNIPE=0x00800000,

	/* スネークはPSG-1を使っている */
	SNAKE_STATUS_HAS_PSG=0x01000000,

	/* スネークはサイファーを狙っている */
	SNAKE_STATUS_CYPHER_IN_SIGHT=0x02000000,

	/* スネークにレーザーサイトが当たってる(w14a) */
	SNAKE_STATUS_LASERSIGHT=0x04000000,

	/* ダメージを受けた */
	SNAKE_STATUS_DAMAGED_FOR_BEHIND=0x08000000,

	/* 顔と体の向きの修正 */
	SNAKE_STATUS_CALCADJUST_W43A=0x10000000,
	SNAKE_STATUS_CALCADJUST_BOOK=0x20000000,

	/* スネークはシステムによってダメージ処理中 */
	SNAKE_STATUS_SYS_DAMAGED=0x40000000,

	/* スネークはライフがなくなった */
	SNAKE_STATUS_GAMEOVER=0x80000000,
};

enum {
	/* スネークがアイテムを既に投げたかどうかのフラグ */
	SNAKE_STATUS2_THROWED_RATION=0x00000001,
	SNAKE_STATUS2_THROWED_SOCOMS_BULLET=0x00000002,
	SNAKE_STATUS2_THROWED_M4S_BULLET=0x00000004,
	SNAKE_STATUS2_THROWED_AKS_BULLET=0x00000008,

	SNAKE_STATUS2_THROWED_RGB6_BULLET=0x00000010,

	// SNAKE_STATUS2_THROWED_TABACO=0x00000010,
	// SNAKE_STATUS2_THROWED_DOGTAG=0x00000020,

	/* スネークがクエスチョンマークを出す(w14a) */
	SNAKE_STATUS2_START_QUESTION_MARK=0x00000040,

	/* スネークが!マークを出す(w14a) */
	SNAKE_STATUS2_START_QUATATION_MARK=0x00000080,

	/* スネークはすでにクエスチョンマークを出した(w14a) */
	SNAKE_STATUS2_ALREADY_DISP_QUESTION_MARK=0x00000100,

	/* 羽交い締めされるモード */
	SNAKE_STATUS2_BE_CAPTURED_MODE=0x00000200,
	SNAKE_STATUS2_DISABLE_CAPTURED=0x00000400,

	/* オートで白い息を出す */
	SNAKE_STATUS2_AUTO_BREATH=0x00000800,

	/* いびきをかくモード */
	SNAKE_STATUS2_ZZZ_SOUND=0x00001000,

	/* 音声のコール */
	SNAKE_STATUS2_VOICE=0x00002000,

	/* 目を開いているかどうかのフラグ */
	SNAKE_STATUS2_SIGHT_CLOSE=0x00004000,

	/* ライデンに攻撃された */
	SNAKE_STATUS2_DAMAGED_BY_RAIDEN=0x00008000,

	/* スネークは目を閉じている */
	SNAKE_STATUS2_EVM_CLOSE_EYE=0x00010000,

	/* スネークはまばたきをしている */
	SNAKE_STATUS2_EVM_FLUSH_EYE=0x00020000,

	/* 'HIDE'禁止状態 */
	SNAKE_STATUS2_PROHIBIT_TO_HIDE=0x00040000,

	/* 声を出したかどうかのフラグ */
	SNAKE_STATUS2_VOICE_INDEPEND=0x00080000,
	SNAKE_STATUS2_VOICE_ENEMY_ARROUND=0x00100000,
	SNAKE_STATUS2_VOICE_ENEMY_ARROUND2=0x00200000,
	SNAKE_STATUS2_VOICE_SEARCHENEMY=0x00400000,

	/* w44aで最終まできた */
	SNAKE_STATUS2_W44A_FINAL_FIGHT=0x00800000,

	/* ダメージ処理のコールバックが既に呼ばれた場合の処理 */
	SNAKE_STATUS2_CALLED_DAMAGE=0x01000000,

	/* スネークとライデンが衝突するコースにいる */
	SNAKE_STATUS2_CRASH_ZONE=0x02000000,

	/* ローリング発動 */
	SNAKE_STATUS2_START_ROLLING=0x04000000,

	/* スネークは気絶中 */
	SNAKE_STATUS2_FAINT=0x08000000,

	/* ゲームオーバー音声を呼び終えた */
	SNAKE_STATUS2_CALLED_GAMEOVER_VOICE=0x10000000,

	SNAKE_STATUS2_CALLED_CAPTURE_VIB=0x20000000,

	/* ライデンに捕まった */
	SNAKE_STATUS2_CAPTURED_BY_RAIDEN=0x40000000,
};

enum {
	/* ライデンに近付き過ぎたので強制移動中 */
	NPC_ACT_SNAKE_FORCE_MOVE=0x00010000,

	/* スネークはパンチで攻撃中 */
	NPC_ACT_SNAKE_PUNCH_KICK=0x00020000,
};

enum {
	NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER=0x0001,
	NPCSNAKE_RETFLAG_CAPTURED_FROM_PLAYER=0x0002,
};

enum {
	/* ライデンは移動中 */
	NPCSNAKE_RAIDEN_MOVING=0x0001,

	/* 現在、ライデンのライフが少ない */
	NPCSNAKE_RAIDEN_LIFE_DANGER=0x0002,

	/* ライデンの前方に敵 */
	NPCSNAKE_RAIDEN_ENEMY_AHEAD=0x0004,

	/* ライデンの後方に敵 */
	NPCSNAKE_RAIDEN_ENEMY_BEHIND=0x0008,

	/* ライデンは二人以上の敵を相手している */
	NPCSNAKE_RAIDEN_2ENEMYS=0x0010,

	/* ライデンは多数の敵を相手している */
	NPCSNAKE_RAIDEN_MANY_ENEMYS=0x0020,

	/* ライデンからスネークが見える */
	NPCSNAKE_RAIDEN_LOOKS_SNAKE=0x0040,

	/* ライデンがダメージ状態になった */
	NPCSNAKE_RAIDEN_DAMAGED=0x0080,

	/* 2001/9/17 K.Kano 追加 */
	/* 敵がいる */
	NPCSNAKE_RAIDEN_ENEMY_EXIST=0x0100,
};

/* 現在のスネークの思考モード */
enum {
	/* スネークは現在自己判断モード */
	NPCSNAKE_MODE_INDEPEND=0,

	/* スネークは周囲索敵モード */
	NPCSNAKE_MODE_SEARCHENEMY,

	/* スネークは現在ライデン追跡モード */
	NPCSNAKE_MODE_FOLLOW_RAIDEN,

	/* スネークは現在ライデン後方援護モード */
	NPCSNAKE_MODE_BEHIND_RAIDEN,

	/* スネークはライデンお迎えモード
	   (先行し過ぎてしばらく立つと、ライデンを呼びに来る) */
	NPCSNAKE_MODE_PICKUP_RAIDEN,


	/* w45a用 */
	/* ライデンを守る */
	NPCSNAKE_MODE_GURD_RAIDEN,

	/* 敵を積極的に攻撃 */
	NPCSNAKE_MODE_ATTACK_ENEMY,


	/* ステージ間を移動中 */
	NPCSNAKE_MODE_BETWEEN_STAGES,


	/* w14a用、座り込んでいるプリスキン */
	NPCSNAKE_MODE_W14A,

	/* w17a用、ダンボールプリスキン */
	NPCSNAKE_MODE_W17A,

	/* w32a用、狙撃スネーク */
	NPCSNAKE_MODE_W32A,

	/* w32a用？、何もしない表示もされないスネーク */
	NPCSNAKE_MODE_DUMMY,

	/* w43a用、ただ立っているだけのスネーク */
	NPCSNAKE_MODE_W43A,



	/* 2001/9/18  K.Kano
	   w44aでの最後の山場でのスネークの動きを改善
	   新モードを追加 */

	/* w44aFinal用 */
	/* ライデンを守る */
	NPCSNAKE_MODE_GURD_RAIDEN_W44A,

	/* 敵を積極的に攻撃 */
	NPCSNAKE_MODE_ATTACK_ENEMY_W44A,
};


/* メインの状態 */
enum {
	/* 移動の状態 */
	NPCSNAKE_ACT_MOVEPOS=0,
	NPCSNAKE_ACT_MOVEZONE,
	NPCSNAKE_ACT_MOVEZONE2,
	NPCSNAKE_ACT_FLWRAIDEN,
	NPCSNAKE_ACT_STAY,

	/* 0x05 */
	NPCSNAKE_ACT_MOVEAROUNDRAIDEN,
	NPCSNAKE_ACT_FLWRAIDEN_ONLY,

	/* 0x07 */
	NPCSNAKE_ACT_POINT_TARGET,
	NPCSNAKE_ACT_WAIT_POINTING,
	NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT,
	NPCSNAKE_ACT_SHOOT_RAIDEN,
	NPCSNAKE_ACT_SHOOT_RAIDEN_INTERVAL,

	/* 0x0c */
	NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT,
	NPCSNAKE_ACT_ANGRY,
	NPCSNAKE_ACT_PUNCH_RAIDEN,

	/* 0x0f */
	NPCSNAKE_ACT_ANGRY2,
	NPCSNAKE_ACT_PUNCH_RAIDEN2,

	/* 0x11 */
	NPCSNAKE_ACT_W14A_0,
	NPCSNAKE_ACT_W14A_1,
	NPCSNAKE_ACT_W14A_2,
	NPCSNAKE_ACT_W14A_2_1,
	NPCSNAKE_ACT_W14A_2_INTERVAL,
	NPCSNAKE_ACT_W14A_3,
	NPCSNAKE_ACT_W14A_4, /* 星三つ */
	NPCSNAKE_ACT_W14A_5, /* 星二つ */
	NPCSNAKE_ACT_W14A_6, /* 星一つ */

	/* 0x1a */
	NPCSNAKE_ACT_W17A_0,
	NPCSNAKE_ACT_W17A_1,
	NPCSNAKE_ACT_W17A_2,
	NPCSNAKE_ACT_W17A_3,
	NPCSNAKE_ACT_W17A_4,
	NPCSNAKE_ACT_W17A_5,

	/* 0x20 */
	NPCSNAKE_ACT_W32A_WAIT,
	NPCSNAKE_ACT_W32A_SHOOT,
	NPCSNAKE_ACT_W32A_AFTER_SHOOT,
	NPCSNAKE_ACT_W32A_PRE_COUNTER,
	NPCSNAKE_ACT_W32A_PRE_COUNTER2,
	NPCSNAKE_ACT_W32A_COUNTER,
	NPCSNAKE_ACT_W32A_SLEEP,
	NPCSNAKE_ACT_W32A_AFTER_SLEEP,

	/* 0x28 */
	NPCSNAKE_ACT_DUMMY,

	/* 0x29 */
	NPCSNAKE_ACT_W43A,
	NPCSNAKE_ACT_W43A_MOVE_POINT,

	NPCSNAKE_ACT_AVOID_RAIDEN,
	NPCSNAKE_ACT_ROLLING,

	/* 2001/9/19  K.Kano
	   w44aでの最後の山場でのスネークの動きを改善
	   新モードを追加 */
	NPCSNAKE_ACT_MOVEAROUNDRAIDEN2,

	NPCSNAKE_ACT_MASK=0x00ff,

	/* 射撃の状態 */
	NPCSNAKE_SHOOT_NONE=0x0000,
	NPCSNAKE_SHOOT=0x0100,
	NPCSNAKE_PUNCH=0x0200,
	NPCSNAKE_SHOOT_MASK=0x0f00,

	/* 隠れの状態 */
	NPCSNAKE_HIDE_NO_HIDING=0x0000,
	NPCSNAKE_HIDE_MOVE_TO_HIDE=0x1000,
	NPCSNAKE_HIDE_SITDOWN_TO_HIDE=0x2000,
	NPCSNAKE_HIDE_HIDING=0x3000,
	NPCSNAKE_HIDE_ATTACK=0x4000,
	NPCSNAKE_HIDE_PEEPING=0x5000,
	NPCSNAKE_HIDE_BACK_TO_HIDE=0x6000,
	NPCSNAKE_HIDE_MASK=0xf000,
};


/* モーションの番号 */
enum {
	PAD_NONE=0,		 /* STAND OR RUN */


	/* 歩き、しゃがみ */
	PAD_STAND=none_stand2,
	PAD_WALK=none_walk,
	PAD_RUN=none_run2,
	PAD_SITTING=none_squat,

	/* ビハインドモードでの動作 */
	PAD_BEHIND=none_caution,
	PAD_BEHIND_AND_SITTING=none_squat_caution,

	PAD_START_LOOK_L_BEHIND=none_behind_lb_start,
	PAD_LOOK_L_BEHIND=none_behind_lb,
	PAD_END_LOOK_L_BEHIND=none_behind_lb_end,
	PAD_START_LOOK_R_BEHIND=none_behind_rb_start,
	PAD_LOOK_R_BEHIND=none_behind_rb,
	PAD_END_LOOK_R_BEHIND=none_behind_rb_end,

	PAD_START_LOOK_L_BEHIND_AND_SITTING=non_behind_lb_start_sq,
	PAD_LOOK_L_BEHIND_AND_SITTING=non_behind_lb_sq,
	PAD_END_LOOK_L_BEHIND_AND_SITTING=non_behind_lb_end_sq,
	PAD_START_LOOK_R_BEHIND_AND_SITTING=non_behind_rb_start_sq,
	PAD_LOOK_R_BEHIND_AND_SITTING=non_behind_rb_sq,
	PAD_END_LOOK_R_BEHIND_AND_SITTING=non_behind_rb_end_sq,


	/* 歩き、しゃがみ */
	PAD_STAND_WUSP=m92_stand,
	PAD_WALK_WUSP=m92_walk,
	PAD_RUN_WUSP=m92_run,
	PAD_SITTING_WUSP=m92_squat,

	/* ビハインドモードでの動作 */
	PAD_BEHIND_WUSP=m92_caution,
	PAD_BEHIND_AND_SITTING_WUSP=m92_squat_caution,

	PAD_START_LOOK_L_BEHIND_WUSP=m92_behind_lb_start,
	PAD_LOOK_L_BEHIND_WUSP=m92_behind_lb,
	PAD_END_LOOK_L_BEHIND_WUSP=m92_behind_lb_end,
	PAD_START_LOOK_R_BEHIND_WUSP=m92_behind_rb_start,
	PAD_LOOK_R_BEHIND_WUSP=m92_behind_rb,
	PAD_END_LOOK_R_BEHIND_WUSP=m92_behind_rb_end,

	PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP=m92_behind_lb_start_sq,
	PAD_LOOK_L_BEHIND_AND_SITTING_WUSP=m92_behind_lb_sq,
	PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP=m92_behind_lb_end_sq,
	PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP=m92_behind_rb_start_sq,
	PAD_LOOK_R_BEHIND_AND_SITTING_WUSP=m92_behind_rb_sq,
	PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP=m92_behind_rb_end_sq,

	PAD_START_SHOOT_USP_L_FROM_BEHIND=m92_behind_at_l_s,
	PAD_END_SHOOT_USP_L_FROM_BEHIND=m92_behind_at_l_e,
	PAD_START_SHOOT_USP_R_FROM_BEHIND=m92_behind_at_r_s,
	PAD_END_SHOOT_USP_R_FROM_BEHIND=m92_behind_at_r_e,

	PAD_START_SHOOT_USP_L_FROM_BEHIND_AND_SITTING=m92_behind_at_l_s_sq,
	PAD_END_SHOOT_USP_L_FROM_BEHIND_AND_SITTING=m92_behind_at_l_e_sq,
	PAD_START_SHOOT_USP_R_FROM_BEHIND_AND_SITTING=m92_behind_at_r_s_sq,
	PAD_END_SHOOT_USP_R_FROM_BEHIND_AND_SITTING=m92_behind_at_r_e_sq,


	/* 歩き、しゃがみ */
	PAD_STAND_WFMS=fms_stand,
	PAD_WALK_WFMS=fms_walk,
	PAD_RUN_WFMS=fms_run,
	PAD_SITTING_WFMS=fms_squat,

	/* ビハインドモードでの動作 */
	PAD_BEHIND_WFMS=fms_caution,
	PAD_BEHIND_AND_SITTING_WFMS=fms_squat_caution,

	PAD_START_LOOK_L_BEHIND_WFMS=fms_behind_lb_start,
	PAD_LOOK_L_BEHIND_WFMS=fms_behind_lb,
	PAD_END_LOOK_L_BEHIND_WFMS=fms_behind_lb_end,
	PAD_START_LOOK_R_BEHIND_WFMS=fms_behind_rb_start,
	PAD_LOOK_R_BEHIND_WFMS=fms_behind_rb,
	PAD_END_LOOK_R_BEHIND_WFMS=fms_behind_rb_end,

	PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS=fms_behind_lb_start_sq,
	PAD_LOOK_L_BEHIND_AND_SITTING_WFMS=fms_behind_lb_sq,
	PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS=fms_behind_lb_end_sq,
	PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS=fms_behind_rb_start_sq,
	PAD_LOOK_R_BEHIND_AND_SITTING_WFMS=fms_behind_rb_sq,
	PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS=fms_behind_rb_end_sq,

	PAD_START_SHOOT_FMS_L_FROM_BEHIND=fms_behind_at_l_s,
	PAD_END_SHOOT_FMS_L_FROM_BEHIND=fms_behind_at_l_e,
	PAD_START_SHOOT_FMS_R_FROM_BEHIND=fms_behind_at_r_s,
	PAD_END_SHOOT_FMS_R_FROM_BEHIND=fms_behind_at_r_e,

	PAD_START_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING=fms_behind_at_l_s_sq,
	PAD_END_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING=fms_behind_at_l_e_sq,
	PAD_START_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING=fms_behind_at_r_s_sq,
	PAD_END_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING=fms_behind_at_r_e_sq,


	/* パンチキック */
	PAD_PUNCH=none_combo,
	PAD_PUNCH_WFMS=m4a_combo,


	/* 射撃 */
	PAD_START_SHOOT_USP=m92_fire_ready,
	PAD_SHOOT_USP=m92_fire,
	PAD_START_SHOOT_FMS=fms_fire_ready,
	PAD_SHOOT_FMS=fms_fire,


	/* ダメージモーション */
	PAD_DAMAGE_F=none_damage_f,					/* その場前のめり */
	PAD_DAMAGE_B=none_damage_b,					/* その場のけぞり */
	PAD_DAMAGE_GAMEOVER=none_damage_out,		/* GAME OVERモーション */
	PAD_DAMAGE_KNOCKDOWN=none_damage_near,		/* ふっ飛び→ダウンモーション */
	PAD_DAMAGE_SLEEP_F=none_down_f,				/* ダウン継続モーション */
	PAD_DAMAGE_SLEEP_B=none_down,				/* ダウン継続モーション */
	PAD_DAMAGE_WAKEUP_F=none_down_stand_f,		/* 通常仰向けからの起き上がり */
	PAD_DAMAGE_WAKEUP_B=none_down_stand_b,		/* 通常うつぶせからの起き上がり */
	PAD_SLEEP_B=sleep_down_b,					/* 麻酔うつぶせ倒れ */
	PAD_SLEEP_WAKEUP_F=none_down_stand_f,		/* 麻酔仰向けから起きる */
	PAD_SLEEP_WAKEUP_B=wakeup_from_sleep_b,		/* 麻酔うつぶせから起きる */


	/* ダンボールプリスキン */
	PAD_BOX_IDLE=box_idle,
	PAD_BOX_WALK=box_walk,
	PAD_BOX_RUN=box_run,
	PAD_BOX_DASH=box_dash,


	/* 狙撃スネーク */
	PAD_CROUCHON=none_squat_crouch,
	PAD_CROUCHING=none_crouch,
	PAD_CROUCHING_WPSG=psg_fire_ready,
	PAD_DAMAGE_CROUCHING=none_damage_crouch,

	/* w14a居眠りスネーク */
	PAD_SITTING_AND_SLEEP_WM4A1=w14a_sit_and_sleep,
	PAD_SITTING_AND_START_SHOOT_WM4A1=w14a_sit_and_fire_ready_m4a,
	PAD_SITTING_AND_SHOOT_WM4A1=w14a_sit_and_fire_m4a,
	PAD_SITTING_AND_LOOK_RIGHT_WM4A1=w14a_sitting_and_look_right,
	PAD_SITTING_AND_LOOK_LEFT_WM4A1=w14a_sitting_and_look_left,
	PAD_SITTING_AND_DAMAGED_WM4A1=w14a_sitting_and_damage,

	/* ローリング開始、終了 */
	PAD_TUMBLE_START=tumble_start,
	PAD_TUMBLE_END=tumble_end,

	/* モーションが無いもの
	   モーション補間のみであったり、モーションオーバーライドで
	   あったりして、特有のモーションが無い */
	PAD_SITDOWN=0x0400,
	PAD_STANDUP,
	PAD_SITDOWN_WUSP,
	PAD_STANDUP_WUSP,
	PAD_SITDOWN_WFMS,
	PAD_STANDUP_WFMS,

	PAD_START_BEHIND,
	PAD_END_BEHIND,
	PAD_START_BEHIND_WUSP,
	PAD_END_BEHIND_WUSP,
	PAD_START_BEHIND_WFMS,
	PAD_END_BEHIND_WFMS,

	PAD_START_BEHIND_AND_SITTING,
	PAD_END_BEHIND_AND_SITTING,
	PAD_START_BEHIND_AND_SITTING_WUSP,
	PAD_END_BEHIND_AND_SITTING_WUSP,
	PAD_START_BEHIND_AND_SITTING_WFMS,
	PAD_END_BEHIND_AND_SITTING_WFMS,

	/* 座りながら射撃 */
	PAD_SITTING_AND_START_SHOOT_USP,
	PAD_SITTING_AND_SHOOT_USP,
	PAD_SITTING_AND_START_SHOOT_FMS,
	PAD_SITTING_AND_SHOOT_FMS,

	/* 武器を持ち上げたまま固定 */
	PAD_BRINGING_USP,
	PAD_BRINGING_FMS,
	PAD_BRINGING_USP_AND_SITTING,
	PAD_BRINGING_FMS_AND_SITTING,

	/* ダンボールをかぶり歩くモーションでストップする */
	PAD_BOX_STOP,

	/* 狙撃スネークの狙撃モーション */
	PAD_SHOOT_PSG,

	PAD_LOCK,

	PAD_PUNCH_WUSP,

	PAD_SITTING_AND_BRINGING_WM4A1,
	PAD_SITTING_AND_DAMAGED,
	PAD_SITTING_AND_M9SLEEP,

	PAD_DAMAGE_PSG1T_CROUNING,

	PAD_HANG_GAMEOVER,

	PAD_TUMBLE_START_WUSP,
	PAD_TUMBLE_START_WFMS,
	PAD_TUMBLE_END_WUSP,
	PAD_TUMBLE_END_WFMS,

	PAD_DAMAGE_F_SHORT,						/* その場前のめり(短いバージョン) */
	PAD_DAMAGE_B_SHORT,						/* その場のけぞり(短いバージョン) */
};


enum {
	/* 隠れるポジション */
	POS_FLAG_HIDEPOS=0x0001,

	/* 座ると隠れる */
	POS_FLAG_SITTING_HIDEPOS=0x0002,

	/* 二階には撃てない */
	POS_FLAG_DISABLE_SHOOT_UPSTAIR=0x0004,

	/* 右にのぞき込む */
	POS_FLAG_PEEP_RIGHT=0x0010,

	/* 左にのぞき込む */
	POS_FLAG_PEEP_LEFT=0x0020,

	/* 羽交い締めされる場所 */
	POS_BE_CAPTURED=0x0100,

	/* 羽交い締めの場所の番号 */
	POS_CAPTURE_NUMBER_MASK=0x00ff0000,
};


enum {
	NPCSNAKE_STAGE_W44A=0,
	NPCSNAKE_STAGE_W45A,
	NPCSNAKE_STAGE_BETWEEN_STAGES,
	NPCSNAKE_STAGE_W14A,
	NPCSNAKE_STAGE_W17A,
	NPCSNAKE_STAGE_W32A,
	NPCSNAKE_STAGE_DUMMY,
	NPCSNAKE_STAGE_W43A,
};


enum {
	NPCSNAKE_TARGET_SITTING=NPC_TARGET_SIZE_DOWN+1,
	NPCSNAKE_TARGET_SITTING_W14A,
	NPCSNAKE_TARGET_BEHIND,
};


enum {
	NPCSNAKE_SCNSTATUS_NORMAL=0,

	// 逃走中
	NPCSNAKE_SCNSTATUS_RUNAWAY,

	// 寝ています
	NPCSNAKE_SCNSTATUS_SLEEP,

	// ダメージ中
	NPCSNAKE_SCNSTATUS_DAMAGE,

	// 気絶中
	NPCSNAKE_SCNSTATUS_FAINT,

	// ライデンに反撃
	NPCSNAKE_SCNSTATUS_COUNTERATTACK,

	// 銃を構えている
	NPCSNAKE_SCNSTATUS_POINT_TARGET,

	// 銃を撃っている
	NPCSNAKE_SCNSTATUS_SHOOT,

	// ライデンの攻撃によるダメージ中
	NPCSNAKE_SCNSTATUS_DAMAGE_BY_RAIDEN,
};


#define SET_SCN_STATUS(x)			(npcsnake_work->scenario_snake_status=(x))
#define SET_SCN_NORMAL()			SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_NORMAL)
#define SET_SCN_RUNAWAY()			SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_RUNAWAY)
#define SET_SCN_SLEEP()				SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_SLEEP)
#define SET_SCN_DAMAGE()			SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_DAMAGE)
#define SET_SCN_FAINT()				SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_FAINT)
#define SET_SCN_COUNTERATTACK()		SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_COUNTERATTACK)
#define SET_SCN_POINT_TARGET()		SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_POINT_TARGET)
#define SET_SCN_SHOOT()				SET_SCN_STATUS(NPCSNAKE_SCNSTATUS_SHOOT)



#define NPC_SNAKE_LIFE			400
#define NPC_SNAKE_FAINT			10
#define NPC_SNAKE_DOWN_DAMAGE	40

/* 気絶時間 */
#if 0
#define NPC_SNAKE_FAINT_COUNT	DIRECT_TICK((int)(60.0f*4.0f))		/* 4秒で起きる */
#elif 0
#define NPC_SNAKE_FAINT_COUNT	DIRECT_TICK((int)(60.0f*10.0f))		/* 10秒で起きる */
#else
#define NPC_SNAKE_FAINT_COUNT	DIRECT_TICK((int)(60.0f*30.0f))		/* 30秒で起きる */
#endif

/* 麻酔睡眠時間 */
#define NPC_SNAKE_SLEEP_COUNT	DIRECT_TICK((int)(60.0f*90.0f))		/* 90秒で起きる */

#define NPC_SNAKE_FAINT_DIS		CVM2N(8.0f)


#define NPC_SNAKE_SUPPORT_LEVEL_MAX		8

#if 0
#define DIR_ENABLE_SHOOT	((int)(10.0f/360.0f*4096.0f))
#else
#define DIR_ENABLE_SHOOT	((int)(20.0f/360.0f*4096.0f))
#endif

/* ダンボールスネークの場合の、ヘッドマークの腰からの高さ */
#define HEADMARK_HEIGHT_OFFSET	CVM2N(0.7f)


/* w17aでのスネークの動きの制御カウンター */
#define W17A_RUN_COUNT		DIRECT_TICK(90)
#define W17A_IDLE_COUNT		DIRECT_TICK(150)


/* 狙撃スネークの狙撃間隔のデフォルト値 */
#define W32A_DEFAULT_SNIPE_TIME				DIRECT_TICK(80)
#define W32A_DEFAULT_AFTER_SNIPE_TIME		DIRECT_TICK(130)


/* ダメージを受けて寝ている期間 */
// #define KNOCKDOWN_COUNT				DIRECT_TICK((int)(60.0f*3.0f))	/* 3秒 */

/* ダメージを受けない期間 */
#define NODAMAGE_VLONG_COUNT		DIRECT_TICK((int)(60.0f*5.0f))	/* 5秒 */
#define NODAMAGE_LONG_COUNT			DIRECT_TICK((int)(60.0f*3.0f))	/* 3秒 */
#define NODAMAGE_COUNT				DIRECT_TICK((int)(60.0f*1.0f))	/* 1秒 */
#define NODAMAGE_SHORT_COUNT		DIRECT_TICK((int)(60.0f*0.5f))	/* 0.5秒 */
#define NODAMAGE_VSHORT_COUNT		DIRECT_TICK((int)(60.0f*0.1f))	/* 0.1秒 */

/* ダメージを受けない攻撃を加えられた場合のぴくぴくと震える回数 */
#define SHORT_REACTION_COUNT		6		/* 現在のところ最大回数 */


/* スプレーを連続でかけられた時の対応
   この時間以内にかけられた場合、すぐさま反撃する */
#define SPRAY_TIME				DIRECT_TICK(5.0f*60.0f)		/* 5秒 */

/* ライデンに押された場合の対応
   この時間以内に押された場合、ローリングで回避する */
#define ROLLING_TIME			DIRECT_TICK(0.8f*60.0f)		/* 0.8秒 */


/* アイテムを投げる間隔の調節 */
#define THROW_ITEM_COUNT		DIRECT_TICK(1.5f*60.0f)		/* 1.5秒 */


/* 狙撃スネークがカウンターショットを撃ち出すまでの時間 */
#define INTERVAL_BEFORE_COUNTER		DIRECT_TICK((int)(60.0f*2.0f/3.0f))	/* 2/3秒 */


#define NPCSNAKE_USP_EFFECT_NAME	0x003687d3			/* NPCSNAKE_USP */
#define NPCSNAKE_FMS_EFFECT_NAME	0x00364b16			/* NPCSNAKE_FMS */
#define NPCSNAKE_PSG_EFFECT_NAME	0x003673ca			/* NPCSNAKE_PSG */


/* マガジン内の弾の最大数 */
#define MAX_BULLETS_IN_SOCOM	12
#define MAX_BULLETS_IN_USP		15
#define MAX_BULLETS_IN_AKS		30
#define MAX_BULLETS_IN_M4		30
#define MAX_BULLETS_IN_RGB6		6


/* ドックタグのモデル */
#define DOCTAG_MDLNAME			0x0093aff6		/* dogtag_sna */


/* 顔アニメのアクター名 */
#define NPCSNAKE_FACEANIM_ACTOR		0x001d2679			/* face_anim_actor */

/* まばたきの間隔 */
#define NPCSNAKE_EYE_FLUSH_INTERVAL		DIRECT_TICK((int)(5.0f*60.0f))

#define INFINITY_MOVE_SIZE			10


#endif
