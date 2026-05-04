/*
   pl_define.h
   プレイヤー共通定義ヘッダ

   2001/01/26	M.Sonoyama
   $Id: pl_define.h,v 1.1.1.3 2002/11/19 11:50:58 Yoshizawa1 Exp $
*/

#ifndef __pl_define_h__
#define	__pl_define_h__

extern int BP_FRAMES_PER_SEC();
#define	FRAME_PER_SEC	(BP_FRAMES_PER_SEC())

#define	ONE_CLOCK	(300 / FRAME_PER_SEC)
#define	NTSC_TIME_BASE	(5)

#ifdef KP_XBOX
#define IMM(_num)  _num##i64
#else
#define IMM(_num)  _num##L
#endif

/* プレイヤーフラグ */
/* フラグは毎フレームリセットされる */
#define    FLAG_NORMAL			IMM(0x0000)
#define    FLAG_CAMERA_ON_WAIST 	IMM(0x0001)		/* 腰にカメラ */
#define    FLAG_DONOT_CHECK_WATCH  	IMM(0x0002)		/* 主観チェックしない */
#define    FLAG_PEEPING 	 	IMM(0x0004)		/* 覗き込み中 */
#define    FLAG_SUBJECT_HORIZON_LIMIT IMM(0x0008)		/* 主観左右回転制限付き */

#define    FLAG_NO_STEP  		IMM(0x0010)		/* 移動させない */
#define    FLAG_NO_IK  		IMM(0x0020)		/* ＩＫ計算しない */
#define    FLAG_FORCE  		IMM(0x0040)		/* 強制状態 */
#define    FLAG_FORCE_END  		IMM(0x0080)		/* 強制状態一応終了 */

#define    FLAG_CANNOT_CHANGE_WEAPON  IMM(0x0100)		/* 武器変更不可 */
#define    FLAG_CANNOT_CHANGE_ITEM    IMM(0x0200)		/* アイテム変更不可 */
#define    FLAG_DONOT_EXEC_MESSAGE    IMM(0x0400)		/* メッセージ実行不可 */
#define    FLAG_DONOT_RECEIVE_MESSAGE  IMM(0x0800)	/* メッセージ受信不可 */

#define    FLAG_BEHIND_PEEP  		IMM(0x1000)		/* ビハインド覗き中 */
#define    FLAG_CANNOT_PEEP 		IMM(0x2000)		/* 主観中覗き込み禁止 */
#define    FLAG_NO_HEADTOENEMY  	IMM(0x4000)		/* 敵の方を見る禁止 */
#define    FLAG_CANNOT_PEEP_LR 	IMM(0x8000)		/* 主観中覗き込みＬＲ禁止 */

#define    FLAG_CANNOT_USE_RATION 	IMM(0x00010000)	/* レーション使わない */
#define    FLAG_BEHIND_ATTACK		IMM(0x00020000)	/* ビハインドアタック */
#define    FLAG_NO_MOTION_STEP_Y 	IMM(0x00040000)	/* モーション上下移動量無視 */
#define    FLAG_CTRL_HEIGHT_NOT_UPDATE  IMM(0x00080000)	/* CONTROL->heightを更新しない */

#define    FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT  IMM(0x00100000) /* CONTROL->heightはオブジェの最高－最低 */
#define    FLAG_NO_GRAVITY 		IMM(0x00200000)	/* 重力なし */
#define	FLAG_FINDPOS_IS_WAIST  IMM(0x00400000)	/* 敵の見つけポイントは腰 */
#define	FLAG_CANNOT_ATTACK 	IMM(0x00800000)	/* 攻撃できない */

#define	FLAG_NO_TOUCH_DAMAGE 	IMM(0x01000000)	/* 交差ダメージなし */
#define	FLAG_NO_MOTION_STEP_XZ  IMM(0x02000000)	/* モーション水平移動量無視 */
#define	FLAG_NO_CAUTION  	IMM(0x04000000)		/* 張り付かない */
#define	FLAG_SUBJECT_ARM_ADJUST  IMM(0x08000000)	/* 主観時に客観腕をＸ角度調節する */

#define	FLAG_RECOVER_ENABLE 	  IMM(0x20000000)	/* 回復可 */
#define	FLAG_PEEPING_UP 		  IMM(0x40000000)	/* 上覗き込み */
#define	FLAG_CANNOT_SUBJECT_LR   IMM(0x80000000)	/* 主観中ＬＲ回転禁止 */

#define	FLAG_CANNOT_SUBJECT_U   	IMM(0x0000000100000000)/* 主観中Ｕ回転禁止 */
#define	FLAG_CANNOT_SUBJECT_D   	IMM(0x0000000200000000)/* 主観中Ｄ回転禁止 */
#define	FLAG_BEHIND_PEEP_R_MAX 	IMM(0x0000000400000000)	/* 右後ろ覗き込み最大 */
#define	FLAG_BEHIND_PEEP_L_MAX 	IMM(0x0000000800000000)	/* 左後ろ覗き込み最大 */
	
#define	FLAG_KNOCK_WALL 			IMM(0x0000001000000000)	/* 壁こんこん今 */
#define	FLAG_HZX_HEIGHT_QUICK 		IMM(0x0000002000000000)	/* 当たりチェック高さを瞬時に変える */
#define	FLAG_BLOOD_SPLASH 			IMM(0x0000004000000000)	/* 飛び散り血発生 */	
#define	FLAG_SPECIAL_MODE 			IMM(0x0000008000000000)	/* 特別なモード状態 */

#define    FLAG_ALL					IMM(0x000000ffffffffff)
    /* これより下はリセットされない */
#define	    FLAG_NO_WAIST_INTERP 		IMM(0x0000010000000000)	/* 次のモーション、腰補完なし */
#define	FLAG_RESET_MOTION2_AT_END  IMM(0x0000020000000000)	/* 再生中のモーション２が終了したら
														   モーション2を不使用にする */
#define	FLAG_MSG_SHADOW_OFF			IMM(0x0000040000000000)	/* メッセージで影ＯＦＦ */
#define	FLAG_RELOADING 			IMM(0x0000080000000000)	/* リロード中 */

#define	FLAG_NEED_NEWPRESS_PEEP 	IMM(0x0000100000000000)	/* 覗き込みボタン押し直し必要 */
#define	FLAG_HOMING				  	IMM(0x0000200000000000)	/* ホーミング中 */
#define	FLAG_WEAPON_CAMERA_ON 		IMM(0x0000400000000000)	/* 武器カメラＯＮ */
#define	FLAG_MOTION_VIB_OFF 		IMM(0x0000800000000000)	/* モーション振動止める */
#define	FLAG_JOINT_INVISIBLE 		IMM(0x0001000000000000)	/* 関節も非表示にしている */
#define	FLAG_NO_WAIST_INTERP_READY IMM(0x0002000000000000)	/* 次のSetActionのときNO_WAIST_INTERP */
#define	FLAG_RECHECK_HAZARD_READY  IMM(0x0004000000000000)	/* 次のActControl後ハザードチェック復活 */
#define	FLAG_SUBJECT_NEWPRESS 		IMM(0x0008000000000000)	/* 主観ボタンプレス */
#define	FLAG_SET_FORCE_NOW 		IMM(0x0010000000000000)	/* PLAYER_FORCEを受信 */
#define	FLAG_MOTION_STEP_OVERRIDE 	IMM(0x0020000000000000)	/* オーバーライドモーションのステップ値を使う */
#define	FLAG_ACTION_MUST_CHANGE 	IMM(0x0040000000000000)	/* モーション番号が同じでもを必ずセットしなおし */
#define	FLAG_RETURNMODE_SET 		IMM(0x0080000000000000)	/* ReturnModeあり */

#define	FLAG_NOMUTEKI_ATTACKED 	IMM(0x0100000000000000)	/* 無敵時間なし攻撃を食らった */
#define	FLAG_DEMO_INVISIBLE_ORDER 	IMM(0x0200000000000000)	/* デモなどで不可視にしたい */
#define	FLAG_DEMO_INVISIBLE 		IMM(0x0400000000000000)	/* デモなどで不可視になっている */
#define	FLAG_SUBJECTCAMERA_ADJUST 	IMM(0x0800000000000000)	/* 主観カメラ位置補正中 */


#define	FLAG_NO_MOTION_STEP	(FLAG_NO_MOTION_STEP_XZ|FLAG_NO_MOTION_STEP_Y)
#define	FLAG_CANNOT_CHANGE	(FLAG_CANNOT_CHANGE_WEAPON|FLAG_CANNOT_CHANGE_ITEM)
#define	FLAG_RETURN_MOTION_ARC	FLAG_FORCE_END
#define	FLAG_CANNOT_SUBJECT_UD	(FLAG_CANNOT_SUBJECT_U|FLAG_CANNOT_SUBJECT_D)

#define	FLAG_RESET_ALL			(FLAG_ALL|FLAG_MOTION_STEP_OVERRIDE)

/* フラグ２ */
/* リセットは個々に行いましょう */

#define	FLAG2_NONE									IMM(0x0000000000000000)
#define	FLAG2_SUBJECTCAMERA_ADJUST_NEAR_SE			IMM(0x0000000000000001)	/* 主観アジャストＳＥ */
#define	FLAG2_NO_TIMERESET_WHEN_CHANGE				IMM(0x0000000000000002)	/* 装備変更でもtimeが０にしない */
#define	FLAG2_CUTIN_DEMO_VISIBLE 					IMM(0x0000000000000004)	/* カットインデモのときに強制表示 */
#define	FLAG2_CUTIN_DEMO_INVISIBLE_ARM 			IMM(0x0000000000000008)	/* カットデモのときに腕強制非表示 */

#define	FLAG2_CANNOT_CHANGE_WEAPON2 				IMM(0x0000000000000010)	/* ACTION2用武器変更不可 */
#define	FLAG2_CANNOT_CHANGE_ITEM2 					IMM(0x0000000000000020)	/* ACTION2用アイテム変更不可 */
#define	FLAG2_CUTIN_DEMO_VISIBLE_ALLOBJS			IMM(0x0000000000000040)	/* カットインデモのときに強制表示全 */
#define	FLAG2_INTRUDE_EFFECT_ON 					IMM(0x0000000000000080)	/* イントルードエフェクトを出す */

#define	FLAG2_INTRUDE_EFFECT_OFF 					IMM(0x0000000000000100)	/* イントルードエフェクトを出さない */
#define	FLAG2_FORCE_VISIBLE 						IMM(0x0000000000000200)	/* 強制可視フラグ */
#define	FLAG2_TRPCHECK_NOROOT 						IMM(0x0000000000000400)	/* トラップチェックルートでない */
#define	FLAG2_STAGESTART 							IMM(0x0000000000000800)	/* スタート時 */

#define	FLAG2_SUBJECT_MOVE 						IMM(0x0000000000001000)	/* 主観でも移動あり */
#define	FLAG2_SUBJECT_START 						IMM(0x0000000000002000)	/* 主観でスタート */
#define	FLAG2_ADJUST_SLOW 							IMM(0x0000000000004000)	/* ＩＫ速度遅い */
#define	FLAG2_OBJ_SHIFT 							IMM(0x0000000000008000)	/* OBJECT SHIFTあり */



/*----------------------------------------------------------------*/

/* 内部通信用フラグ */
enum {
    MAIL_NOTHING =	 		0x00000000,
    MAIL_ATTACK_CANCEL = 	0x00000001,
	MAIL_PUSH_WIN =			0x00000010,
	MAIL_PUSH_LOSE =		0x00000020,
	MAIL_PUSH_LEFT =		0x00000040,
	MAIL_PUSH_RIGHT =		0x00000080,
	MAIL_PUSH_FRONT =		0x00000100,
	MAIL_PUSH_BACK = 		0x00000200,
	MAIL_EMPTY_SHOT =		0x00000400,
	MAIL_ELUDE_STAMP =		0x00001000,
	MAIL_SPS_DAMAGE_F =		0x00002000,
	MAIL_SPS_DAMAGE_B =		0x00004000,
	MAIL_TUMBLE_F =			0x00010000,
	MAIL_TUMBLE_B =			0x00020000,
	MAIL_HANG_THROUGH =		0x00040000,
} ;

#define	MAIL_PUSH_ALL	(MAIL_PUSH_WIN|MAIL_PUSH_LOSE|MAIL_PUSH_LEFT| \
						 MAIL_PUSH_RIGHT|MAIL_PUSH_FRONT|MAIL_PUSH_BACK)
/* 受けとった側は必ずリセットすること */

/*----------------------------------------------------------------*/


/* パッド定義 */
#ifndef PSX2

#else
#define		PATTERN_A
#endif

/*----------------------------------------------------------------*/

/* 開始時に使うフラグセット */
enum {
    START_FLAG_BODYSPLASH = 0x00000001,		/* 体水飛沫あり */
    START_FLAG_NOFOOTSHADOW = 0x00000002,	/* 足影なし */
	START_FLAG_IS_SNAKE =	  0x00000010,	/* プレイヤーはスネーク  */
	START_FLAG_IS_RAIDEN =	  0x00000020,	/* プレイヤーはライデン  */
        START_FLAG_IS_NINJA  = 	  0x00000040,	/* プレイヤーは忍者(附属)*/
        START_FLAG_IS_PLISKIN  =  0x00000080,	/* プレイヤーはプリスキン(附属)*/
} ;

/* 姿勢 */
enum {
    STAND = 0,
    SQUAT,
    GROUND,
	INTRUDE,
} ;

/* 仰向け・うつぶせ */
enum {
    DOWN_FACE = 0,	/* うつぶせ */
    DOWN_BACK = 1,	/* 仰向け */
} ;

enum {
    PL_MSG_MOTION = 0,	/* 強制モーション */
    PL_MSG_POSITION,	/* 強制座標セット */
    PL_MSG_POSITION2,	/* 強制座標セット（軸毎に設定可） */
    PL_MSG_MENU,		/* メニュー表示設定 */
    PL_MSG_BEYOND_TRAP,	/* エルードトラップ設定 */
    PL_MSG_LOCKER,		/* ロッカー */
    PL_MSG_WT_DOOR,		/* 水密ドア */
    PL_MSG_LADDER,		/* はしご */
    PL_MSG_SHADOW,		/* 足影ＯＮ/ＯＦＦ */
    PL_MSG_HEADMARK,	/* 赤びっくり */
	/* 10 */
	PL_MSG_STANCE,		/* 強制姿勢 */
	PL_MSG_WATER,		/* 水中モード */
	PL_MSG_RUN,			/* 強制移動 */
	PL_MSG_BELTCONV,	/* ベルトコンベア */
	PL_MSG_ROTATE,		/* Ｙ回転セット */
	PL_MSG_ELEVATOR,	/* エレベータパネル操作 */
	PL_MSG_FALL,		/* 落下 */
	PL_MSG_LIFTUP,		/* 蛇手に捕まった */
	PL_MSG_KAGESHIBARI,	/* 影縛り */
	PL_MSG_SUPERBLOW,	/* 強制吹っ飛ばし */
} ;

#define	PL_MSGSW_ON		(3662)	
#define	PL_MSGSW_OFF	(117030)

/* その他全プレイヤー共通定義 */

/* 壁当たりチェック半径 */
#define	CHECK_SPHERE		(500)
#define	NORMAL_SPHERE		(474)
#define	CAUTION_SPHERE		(249)

/* ニアチェック高さ */
#define	NEAR_HEIGHT_STAND	(748.0F)
#define	NEAR_HEIGHT_SQUAT	(248.0F)
#define	NEAR_HEIGHT_GROUND	(248.0F)

/* 歩き-走り-ダッシュ */
#define	PAD_WALK_TH		(150)
#define	PAD_RUN_TH		(252)

/* はりつきパッド強さ */
#define	PAD_CAUTION_TH	(150)

/* はりつき方向 */
enum {
    NEUTRAL = -1,
    FRONT,
    BACK,
    RIGHT,
    LEFT,
} ;

/* モード名の定義 */
#define	STAND_STILL				(4380228)
#define	STAND_RUN				(1445079)
#define	SQUAT_STILL				(4223780)
#define	GROUND_STILL			(1996644)
#define	STAND_CAUTION_STILL		(10866777)
#define	SQUAT_CAUTION_STILL		(9228358)
#define	STAND_CAUTION_LEFT		(6910503)
#define	STAND_CAUTION_RIGHT		(9455585)
#define	SQUAT_CAUTION_LEFT		(13675046)
#define	SQUAT_CAUTION_RIGHT		(7817166)
#define	INTRUDE_STILL			(6396418)

#define	GROUND_MOVE_FRONT		(1477850)
#define	GROUND_MOVE_BACK		(3043040)

#define	ELUDE_STILL_MODE		(8090564)	/* EludeStill */

#define	ELUDE_TOUCHDOWN			(2907512)
#define	ELUDE_RETURN_MODE		(8658362)
#define	ELUDE_FALL				(182117)
#define	BLADE_SLASH				(1488032)

#define	COMBO					(6897843)	

#define	PULL_BODY				(15034135)
#define	ENEMY_HANG				(16653384)

#define	WeaponSet		PL_WeaponSet
#define	ItemSet			PL_ItemSet

/* ワークのデファイン */
#define	workL		(&GM_PlayerWork->work_l)
#define	ForceWork	workL->force
#define	Beyond		workL->beyond
#define	ResultPoint	workL->resultPoint
#define	ResultHzx	workL->resultHzx
#define	ResultHzxPtr	workL->resultHzxPtr
#define	ResultAtr	workL->resultAtr
#define	BehindPtr	workL->behindPtr 
#define	Wall		workL->wall
#define	WallTo		workL->wallTo
#define	WallVec		workL->wallVec
#define	WallAtr		workL->wallAtr
#define	Liable		workL->liable
#define	PadTo		workL->padTo
#define	PadForce	workL->padForce
#define	ELUDE_HAND_POWER_MAX	workL->eludeHandPowerMax
#define	MS			workL->ms

#define	FrontAdj	work->front_adj 
#define	BackAdj		work->back_adj 
#define	RightAdj	work->right_adj 
#define	LeftAdj		work->left_adj 
#define	FrontLen	work->front_len 
#define	BackLen		work->back_len 
#define	RightLen	work->right_len
#define	LeftLen		work->left_len 

#endif


