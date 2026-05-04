/*
   pl_work.h
   プレイヤーワークヘッダ

   2000/03/21 M.Sonoyama
   $Id: pl_work.h,v 1.1.1.3 2002/11/19 11:50:59 Yoshizawa1 Exp $
*/

#ifndef __pl_work_h__
#define	__pl_work_h__

#include	"libutl.h"
#include	"camera.h"

#if defined(BP_VITA)
#include <systemgesture.h>
#endif

#define	COMPILE		(1)

/*-----------------------------------------------------------------

  各モード定義

-----------------------------------------------------------------*/

/* 強制系構造体 */
typedef	ALIGN16_DECL(struct)	{
    int		motion ;
	int		loop ;
    int		s_turn ;
    int		e_turn ;
    int		e_stance ;
    int		flag ;
    int		marfile ;
	int		end_proc ;
	int		end_proc_motion ;
	int		proc ;
	int		proc_flag ;
	int		proc_args[ 4 ] ;
} FORCE  ;

/* 強制モーション */
enum {
    FA_NO_CHECK_SEG	= 0x00000001,	/* 壁見ない */
    FA_NO_CHECK_FLR	= 0x00000002,	/* 床見ない == 重力処理しない */
    FA_NO_CHECK_TRP	= 0x00000004,	/* トラップ見ない */
    FA_NO_RECHECK_TRP	= 0x00000008,	/* トラップチェック再開しない */
    FA_USE_DEFAULT	= 0x00000100,	/* デフォルトのmarから指定 */
    FA_NO_WEAPON	= 0x00000200,	/* 装備武器を非表示 */
	FA_NO_INTERP 	= 0x00000400,	/* 補完なし */
	FA_SET_START_Y	= 	0x00000800,	/* 開始時Ｙ座標指定あり */
	FA_USE_NOW_HEIGHT =	0x00001000,	/* 無理やり今の高さからはじめる */
	FA_ELUDE_FALL_SPECIAL = 0x00002000,	/* エルード落下中特殊 */
	FA_BOX_EXIST =		0x00008000,	/* ダンボール外さない */
	FA_DIRECT_MOVE =    0x00010000,	/* 強制移動用、ゾーンを使用せず一直線に移動する。*/	
	FA_STAGESTART =		0x00020000,	/* ステージ開始時の強制モーション／移動であることを明示 */
    FA_GRAVITY_ON  =		0x00040000,	/* 重力をオンにする */
	FA_CANCEL =			0x80000000,	/* キャンセル命令 */
} ;

enum {
	FA_PROC_FLAG_PROC  = 0x0000,	/* プロック実行（デフォルト） */
	FA_PROC_FLAG_EVERY = 0x0001,	/* 毎フレーム実行 */
	FA_PROC_FLAG_END   = 0x0002,	/* 終了時も実行 */
	FA_PROC_FLAG_BLOCK = 0x0010,	/* ブロック実行 */
} ;

/* 強制モーションプロックモード */
enum {
	FA_PROCMODE_MOTION 		= 0,		//強制モーション
	FA_PROCMODE_RUN    		= 1,		//強制移動
	FA_PROCMODE_STANCE 		= 2,		//強制姿勢変更
	FA_PROCMODE_LADDER_IN 	= 3,		//はしごつかまり
	FA_PROCMODE_LADDER_OUT 	= 4,		//はしご離し
	FA_PROCMODE_ELUDE_IN 	= 5,		//エルード開始
	FA_PROCMODE_ELUDE_OUT 	= 6,		//エルード終了
	FA_PROCMODE_ELUDE_FALL 	= 7			//エルード落下開始
} ;

enum {
    FA_END_STAND =	 0,
    FA_END_SQUAT,
    FA_END_GROUND,
    FA_END_INTRUDE,
    FA_END_BEYOND,
	FA_END_DOWN_FACE,	/* うつぶせダウン */
	FA_END_DOWN_BACK,	/* あおむけダウン */
} ;

/*-------------------------------------------------------------*/

/* ビヨンドモード */
typedef	ALIGN16_DECL(struct)	{
    u_char	still ;		/* 静止モーション */
    u_char	move1 ;		/* 移動モーション１ */
    u_char	move2 ;		/* 移動モーション２ */
    u_char	back ;		/* 通常に戻るモーション */

    u_char	back2 ;		/* 通常に戻るモーション壁あり */
    u_char	tired ;		/* 疲れた時の静止モーション */
    u_char	fall_start ;	/* 落ちはじめ */
    u_char	fall_loop ;	/* 落ち中 */

    u_char	fall_damage ;	/* 落下ダメージ */
    u_char	fall_out ;	/* 落下ゲームオーバー */
    u_char	fall_safe ;	/* 落下無事 */
    u_char	damage ;	/* 通常ダメージ */
    
	u_char	up_start ;	/* 懸垂開始 */
	u_char	up_idle ;	/* 懸垂静止 */
	u_char	up_end ;	/* 懸垂終了 */
	u_char	padding ;

    u_char	object ;	/* 床チェックする関節番号 */
    u_char	wall_obj ;	/* 戻り時に壁チェックする関節番号 */
    short	flag ;		/* フラグ */
    short	s_turn ;	/* 開始向き */
    short	e_turn ;	/* 終了向き */

	int		x_min, x_max ;	/* 移動範囲 */
	int		z_min, z_max ;	

    int		trap_id ;	/* トラップ名 */
} BEYOND  ;

#define	NOBEYOND_TRAP	(9557678)	/* nobeyond */
#define	ELUDE_HAND_POWER_MAX_MAX	(FRAME_PER_SEC * 90)
#define	ELUDE_FALL_DAMAGE	(10)

#define	ELUDE_HAND_POWER_LEVELUP1	(100)
#define	ELUDE_HAND_POWER_LEVELUP2	(200)

extern int BP_AdjustTick2(int);
#define	ELUDE_FALL_SPEED_FIRST		(BP_AdjustTick2(64))
#define	ELUDE_FALL_SPEED			(BP_AdjustTick2(8))

extern int gArm_PeekLAnalogMinValue;
extern int gArm_PeekLAnalogMaxValue;

extern int gArm_PeekRAnalogMinValue;
extern int gArm_PeekRAnalogMaxValue;

enum {
    BY_CHECK_FLOOR = 		0x0000,	/* 床からエッジ検出 */
    BY_CHECK_SEGMENT = 		0x0001,	/* 壁からエッジ検出 */
    BY_NOFOOTSHADOW = 		0x0002,	/* 足影なし */
	BY_FALL_DEAD =			0x0004,	/* 落ちたら死ぬぜ */
	BY_CANNOT_FALL =		0x0008,	/* パッド操作での落下禁止 */
	BY_NO_STAMP =			0x0010,	/* ふみつけなし */
	BY_FALL_IMMEDIATELY = 	0x0020,	/* すぐ降りエルード */
	BY_NO_MOVE_LR =			0x0040,	/* 左右移動なし */
	BY_DEC_GRIP_FAST =		0x0080,	/* グリップ減少速度大 */
	BY_X_RANGE_EXIST = 		0x0100,	/* Ｘ範囲指定あり */
	BY_Z_RANGE_EXIST = 		0x0200,	/* Ｚ範囲指定あり */
    BY_ROT_ADJUST = 		0x1000,	/* 終了時回転補正 */
	BY_RESET_HEIGHT =		0x2000,	/* 戻り時高さリセット */
	BY_NO_RETURNSTAMP =		0x4000,	/* 戻り時踏み付けなし */
} ;

#define	BY_RANGE_EXIST	(BY_X_RANGE_EXIST|BY_Z_RANGE_EXIST)

/* モーションセット */

typedef	struct {
    short	*change ;
    short	*attack ;
    short	*shared ;
	short	reserved ;
} PL_MOTION_SET ;

/*-----------------------------------------------------------------

  プレイヤーワーク

-----------------------------------------------------------------*/


//BP_INPUT - extra controller input processing for X360 etc

// Weapon states
enum WeaponState
{
   WS_Holster,       // Force holster of weapon
   WS_HolsterQuick,  // Force quick holster of weapon
   WS_Draw,          // Force weapon to be drawn
};

// Button states
enum ButtonState
{
   BS_Default,       // Default button behavior
   BS_SoftPress,     // Simulate soft press
   BS_SoftHold,      // Simulate soft hold
   BS_SoftRelease,   // Simulate soft release
   BS_PressureHold,  // Simulate analog pressure hold
   BS_Clear          // Clear button
};

// Player pad structure that can be modified for extra controller input logic processing
typedef	struct _BP_PlayerPad 
{
   int         enable;                 // Enable BP player pad
   GV_PAD      pad;                    // Processed pad input
   int         weaponState;            // Weapon state
   int         buttonState;            // Button state
   float       leftStickNonZeroTime;   // Time since left stick was non-zero force
   int         forcePunch;             // Forces punch
} BP_PlayerPad;

// Modified player pad data
extern   BP_PlayerPad   PlayerPad;  

// Extra controller tweakables
extern   float          PL_PAD_ZOOM_LOCKER_FAST_TH;
extern   float          PL_PAD_ZOOM_LOCKER_MEDIUM_TH;

extern   float          PL_ZOOM_LOCKER_FAST_SPEED;
extern   float          PL_ZOOM_LOCKER_MEDIUM_SPEED;
extern   float          PL_ZOOM_LOCKER_SLOW_SPEED;

extern   float          PL_PAD_SPRAY_MIN_TH;
extern   float          PL_PAD_SPRAY_MAX_TH;

extern   float          PL_PAD_QUICK_HOLSTER_STICK_Y_TH;
extern   float          PL_PAD_QUICK_HOLSTER_STICK_X_TH;
extern   float          PL_PAD_QUICK_HOLSTER_TIME_TH;

extern   float          PL_PAD_QUICK_HOLSTER_STICK_MAG_TH;

extern   float          PL_SCOPE_ZOOM_IN_SPEED;
extern   float          PL_SCOPE_ZOOM_OUT_SPEED;

extern   int            PL_SQUAT_TO_GROUND_FRAMES;



// Extra input defines
#define  PL_PAD_DRAW_HOLSTER_WEAPON    PAD_AL   // Left stick

//BP_INPUT - extra controller input processing for X360 etc


/* ローカルメッセージバッファ数 */
#define	MAX_LOCAL_MESSAGES	(12)

/* ワーク外変数 */
typedef	ALIGN16_DECL(struct)	{
	FVECTOR			resultPoint ;				/* オンライン検出ポイント */
	HZX_HZD			resultHzx ;					/* オンライン検出ハザード */

	HZX_HZD			*resultHzxPtr ;				/* 上へのポインタ */
	int				resultAtr ;					/* オンライン検出ハザード属性 */
	HZX_BEHIND		*behindPtr ;				/* 現在のビハインドトラップ */
	HZX_SEG			*wall ;						/* 検出壁 */

	int				wallTo ;					/* 上への方向 */
	FVECTOR			*wallVec ;					/* 上へのベクトル */
	int				wallAtr ;					/* 上の属性 */
	int				liable ;					/* はりつき属性 */

	int				padTo ;						/* パッド方向 */
	int				padForce ;					/* 方向キー強さ */
	PL_MOTION_SET	ms ;						/* モーション構造体 */

	FORCE			force ;						/* 強制管理構造体 */
	BEYOND			beyond ;					/* エルード構造体 */

	FVECTOR			blood_force ;				/* 血の方向 */
	FVECTOR			fv ;						/* 汎用 */

	int				eludeHandPowerMax ;			/* エルード握力最大 */

	int				idata ;						/* 汎用 */	
	int				idata2 ;					/* 汎用 */
	int				motion2cutframe ;			/* オーバーライドモーションをカットするフレーム */
	int				subjectStanceCtrl ;			/* 主観武器姿勢制御用 */
	
	float			result_len ;				/* オンライン距離 */
	int				hit_se ;					/* 敵の攻撃がヒットしたときのＳＥ */
} WorkL  ;

/* ワーク */
typedef	struct _PlayerWork {
	/* 移動、表示ワーク */
    GV_ACT_EX			actor ;				/* リンク */
    CONTROL				control ;			/* 移動制御 */
    OBJECT				body ;				/* モデル */
    FMATRIX				lights[ 2 ] ;		/* ライト */
    RADAR_CTRL			radar ;				/* レーダー */
    GM_GageSet			life ;				/* ライフゲージ */
    GM_GageSet			hand_power ;		/* ハンドパワーゲージ */
	WorkL				work_l ;			/* 別ワークだったけど一緒にした */
    FVECTOR				root_shift ;		/* ルート位置計算用 */
    FVECTOR				root_diff ;			/* 実位置とルート位置の差 */
	FVECTOR				peep_diff ;			/* 覗き込み時のモデルシフト量 */
#if 0
	MT3_MOVE_SEGMENT	override_move ;		/* オーバーライドモーションの移動量計算用 */
	MT3_MOVE_SEGMENT2	override_move2 ;	/* オーバーライドモーションの移動量計算用 */
	MT3_ROOT_SEGMENT	override_root ;		/* オーバーライドモーションの移動量計算用 */
#endif
    FVECTOR				pre_mov ;			/* 前フレームの位置 */
	FVECTOR				pre_body_mov ;		/* 前フレームの位置（実際の表示位置）*/
    SVECTOR				pre_turn ;			/* 前フレームの角度目標 */
    int					motion1 ;			/* モーション番号１レイヤー */	
    int					motion2 ;			/* モーション番号２レイヤー */	

    SVECTOR				turn_adjusts[ 24 ] ;	/* 角度アジャストワーク（目標） */
    SVECTOR				rot_adjusts[ 24 ] ;		/* 角度アジャストワーク（現在値）*/

	/* 当たり判定ワーク */
    TARGET				def ;				/* 防御ターゲット */
    POWER_TARGET		power ;				/* 防御用パワーターゲット */
    TARGET				offense ;			/* 攻撃用ターゲット */
    POWER_TARGET		attack ;			/* 打撃攻撃用 */
    CAPTURE_TARGET		capture ;			/* つかみ攻撃用 */
	TARGET				touch ;				/* 接触用ターゲット */

	/* めり込み回避ワーク */
	FVECTOR				front_adj ;
	FVECTOR				back_adj ;
	FVECTOR				right_adj ;
	FVECTOR				left_adj ;
	FVECTOR				front_chk_pos ;
	FVECTOR				back_chk_pos ;
	float				front_len ;
	float				back_len ;
	float				right_len ;
	float				left_len ;

	/* 武器アイテムワーク */
    u_int				weapon ;			/* 武器番号 */
    u_int				trigger ;			/* 武器トリガ */
    void				*wp_act ;			/* 武器関数ワーク */
    void				*wp_set ;			/* 武器定義へのポインタ */
    OBJECT				*weapon_body ;		/* 武器に渡すオブジェクト */
    int					weapon_unit ;		/* 武器に渡すユニット番号 */

    u_int				item ;				/* アイテム番号 */
    int					it_trg ;			/* アイテムトリガー */
    void				*it_act ;			/* アイテム関数ワーク */
    void				*it_set ;			/* アイテム定義へのポインタ */
    OBJECT				*item_body ;		/* 装備品に渡すオブジェクト */
    int					item_unit ;			/* 装備品に渡すユニット番号 */

	PL_StickWPWork		stick_wp_work ;		/* 設置武器用ワーク */

	/* 行動ワーク */
    u_int				act_name ;			/* アクション名前 */
    void				( *action )( struct _PlayerWork *, int ) ; /* 行動関数 */
    u_int				time ;				/* 行動関数継続時間 */
    int					ftime ;				/* モード経過時間１ */
    int					ftime_count ;

	u_int				act_name2 ;			/* アクション名前 */
    void				( *action2 )( struct _PlayerWork *, int ) ; /* 行動関数 */
    u_int				time2 ;				/* 行動関数継続時間 */
    int					ftime2 ;			/* モード経過時間２ */
    int					ftime2_count ;

    long64				flag ;				/* プレイヤーフラグ */

	/* 汎用変数ワーク */
    int					data ;		/* ローカル変数 */
    int					data2 ;		/* ローカル変数 */
    int					data3 ;		/* action2用ローカル変数 */
    int					data4 ;		/* action2用ローカル変数 */

	int					idata ;		/* 汎用変数 */
	int					idata2 ;
	float				fdata ;
	float				fdata2 ;

	SVECTOR				sv ;
	SVECTOR				sv2 ;
	FVECTOR				fv ;
	FVECTOR				fv2 ;

	/* 主観腕制御ワーク */
    u_char				arm_motion ;	/* 腕モーション番号 */
	u_char				arm_interp ;	/* モーション補完時間 */
	short				arm_start_time ;/* 腕モーション開始フレーム */
    int					arm_trigger ;	/* 腕トリガー */
    OBJECT				*arm ;	       	/* 腕オブジェクト */
	/* パッド */
    GV_PAD				*pad ;		/* パッド */

	/* カメラ */
    FVECTOR				camera ;			/* カメラ設定 */
    SVECTOR				camdir ; 			/* カメラ方向 */
    GM_CameraSet		*subject_camera ; 	/* 主観カメラ */
    GM_CameraSet		*behind_camera ;  	/* ビハインドカメラ */
    GM_CameraSet		*weapon_camera ;  	/* 主観攻撃カメラ */
	/* 当たり調整ワーク */
    float				hzx_height ;	/* 当たりチェック高さ */
    float				hzx_base_adjust ;	/* 床当たりベース調整値 */
    float				hzx_height_adjust ;	/* 床当たり高さ調整値 */

	/* ビハインド制御ワーク */
    FVECTOR				peep ;						/* 覗き込みポイント */
    FVECTOR				peep_base ;					/* 覗き込み開始ポイント */
    FVECTOR				behind_atk_mov ;			/* ビハインド飛び出し攻撃位置 */
    FVECTOR				behindcam_add ;				/* ビハインド覗きカメラ位置シフト */
    FVECTOR				behindtrg_add ;				/* ビハインド覗きカメラ注視点シフト */
    int					caution_dir ;				/* 張り付き開始パッド方向 */
    int					caution_time ;				/* 張り付きカウンタ */
	int					behind_peep_lastpressL ;	/* 覗きボタンＬが押された時間 */
	int					behind_peep_lastpressR ;	/* 覗きボタンＲが押された時間 */
   int               allow_analog_peep;
   int               enable_analog_peep_r;
   int               enable_analog_peep_l;
    u_short				behind_time ;				/* ビハインドカウンタ */
    u_short				behind_camera_enable ; 		/* トラップビハインド用 */
	/* 壁床音番号 */
	int					seNoSeg ;
	int					seNoFlr ;
	/* メッセージバッファ */
    int					n_msg ;		   /* ローカルメッセージ数 */
    GV_MSG				message[ MAX_LOCAL_MESSAGES ] ; /* メッセージバッファ */
	int					msgbuf[ MAX_LOCAL_MESSAGES ][ 16 ] ; /* メッセージ内容バッファ */

	/* その他 */
    FVECTOR				force ;				/* 吹っ飛び力 */

    short				stance ;			/* プレイヤー姿勢 */
    signed char			vwait ;				/* 可視制御カウンタ */
    signed char			vwait2 ;			/* 武器カメラ制御カウンタ */
    short				r_sphere ;			/* 壁検出半径 */
    short				g_rot ;				/* 床傾き角度 */
    int					floor_atr ;			/* 床アトリビュート */
    short				invincible_time ; 	/* 無敵時間 */
    short				touch_invincible_time ; /* 交差無敵時間 */

    int					shadow ;     		/* 影フラグ */
    int					down_dir ;			/* ダウン向き */
    SVECTOR				adj_rot ;			/* ホーミング用 */

    HOMING_TRG			*homing ;			/* ホーミング用 */
    int					homing_near ;		/* 次ホーミング方向 */
    int					vanime_flag ;		/* 頂点アニメフラグ */
    HUMANMA_WORK		*left_leg ;			/* 左足頂点アニメワーク */

    u_short				capture_count ;		/* 首絞め回数 */
    u_short				recover_count ; 	/* 出血停止用カウンタ */

    u_short				decrease_count ; 	/* 出血ライフ減少用カウンタ */
    u_short				blood_drop_count ; 	/* ぽたぽた血カウンタ（０のとき出す） */
    u_short				blood_drop_time ;  	/* デバグ用ぽたぽた血継続時間 */
    short				se_tableID ;	   	/* ＳＥ切り替えテーブルＩＤ */
    int					name ;		   		/* 名前 */

    int					org_motion ;		/* モーションファイル */
    int					gagetime ;			/* ゲージ表示用 */
    void				*attach_work;	    /* 装備品ワーク */
    int					chanl ;				/* 自分の表示画面 */

    int					post ;				/* 内部通信用 */
    int					send_to ;    		/* メッセージ送信相手 */
    short				subject_turn_accel[ 2 ] ; /* 主観回転加速 */
    int					counter ;		  	/* 汎用カウンターワーク */

    SVECTOR				adj_rot2 ;
	int					last_damaged_time ;
	void				*foot_work ;		/* 足跡ワーク */

	/* ダメージ時コールバック */
    void				( *dmg_callback )( struct _PlayerWork *, TARGET *, TARGET * ) ;	
	int					current_mar ;		/* 現在のモーションファイル */

	VAR_CONTROL			vctrl ;				/* 振動制御 */
	
	int					weapon_quick ;
    int					counter2 ;		  	/* 汎用カウンターワーク２ */

	long64				flag2 ;				/* フラグ２ */
	int					actblur_flag ;		/* モーションブラーフラグ */
	DG_OBJS				*shadow_obj ;		/* 影用モデル */
	int					last_damagecallback_time ;	/* 最後にダメージコールバックが呼ばれた時間 */

	void				*scrdrop ;			/* カメラ前水滴 */
	int                 arm_count ;         /* M4スネークの腕を壁にめり込ませないためのもの normal.c PL_AvoidWeaponIntoWall */

   //BP_INPUT - adding MGS3 hold "squat" button to crawl logic
   int               squat_held_count;
   int               squat_held_triggered;
   //BP_INPUT - adding MGS3 hold "squat" button to crawl logic

#ifdef DEBUG_MODE
    int					trap_view ;
    int					behind_view ;
#endif
	int knockHappened;
   int inCautionMode;

#if defined(BP_VITA)
   int gesturePadFilterType;
#endif
} PlayerWork ;

typedef void	( *PL_ACTION )( PlayerWork *, int ) ;
typedef void	( *PL_OPERATE )( PlayerWork * ) ;
typedef	void	( *PL_DAMAGEFUNC )( PlayerWork *, long64, int ) ;
typedef	int		( *PL_TARGETCALLBACKFUNC )( PlayerWork *, TARGET *, TARGET * ) ;

/*-----------------------------------------------------------------

  モーション

-----------------------------------------------------------------*/

/*--------武器毎に変更--------*/

enum {
    /* 立ち */
    Mstand = 0,
    Mstand_alert,
	Mstand_wall,
    Mwalk,	
    Mrun,
    Mdash,
	Mrun_wall,
    Mstair_stand,
    Mstair_walk,
    Mstair_run,
	Mwalk_on,
    /* しゃがみ */
    Msquat,
    /* 匍匐 */
    Mcrouch,
    Mcrouch_f,
    Mcrouch_b,
    Mcrouch_f_fast,
    /* 張り付き */
    Mcaution,
    Mcaution_squat,
    Mcaution_r,
    Mcaution_l,
    Mcaution_r_sq,
    Mcaution_l_sq,
    /* ビハインド */
    Mbehind_r,
    Mbehind_rb_start,
    Mbehind_rb,
    Mbehind_rb_end,
    Mbehind_l,
    Mbehind_lb_start,
    Mbehind_lb,
    Mbehind_lb_end,
    /* ビハインド攻撃 */
    Mbehind_at_l_s,
    Mbehind_at_l_e,
    Mbehind_at_r_s,
    Mbehind_at_r_e,
    /* ビハインドしゃがみ */
    Mbehind_r_sq,
    Mbehind_l_sq,
    Mbehind_rb_start_sq,
    Mbehind_rb_sq,
    Mbehind_rb_end_sq,
    Mbehind_lb_start_sq,
    Mbehind_lb_sq,
    Mbehind_lb_end_sq,
    /* ビハインドしゃがみ攻撃 */
    Mbehind_at_r_s_sq,
    Mbehind_at_r_e_sq,
    Mbehind_at_l_s_sq,
    Mbehind_at_l_e_sq,
    /* 壁たたき */
    Mknock_r1,
    Mknock_l1,
    Mknock_r2,
    Mknock_l2,
	Mknock_sq1,
	Mknock_sq2,
	/* 移行 */
    Msquat_crouch,
    Mcrouch_stand,
	Msquat_crouch_int,
	Mcrouch_stand_int,
	Mcr2sq_f,
	Mcr2sq_b,
} ;

/*--------武器攻撃用--------*/

enum {
    Mready = 0,
    Mfire,
    Mreload,
    Mfire_end,
    Mready_crouch,
    Mfire_crouch,
    Mreload_crouch,
    Mfire_end_crouch
} ;

/* グレネード特殊 */
enum {
	Mgm_start = 0,
	Mgm_throw_min,
	Mgm_throw_max,
	Mgm_5pose,
	Mgm_start_g,
	Mgm_min_g,
	Mgm_max_g,
	Mgm_5pose_g,
	Mgm_start_bhl,
	Mgm_min_bhl,
	Mgm_max_bhl,
	Mgm_5pose_bhl,
	Mgm_start_bhr,
	Mgm_min_bhr,
	Mgm_max_bhr,
	Mgm_5pose_bhr,
} ;

/*--------共通--------*/

enum {
    /* 移行 */
    Mrun_squat = 0,
    Mrun_squat_end_sq,
    Mrun_squat_end_cr,
    Mrun_squat_end_st,
    Mrun_squat_end_do,
	Mrun_squat_end_face,
    /* ダメージ */
    Mdamage_f,
    Mdamage_b,
	Mdamage_f_sps,
	Mdamage_b_sps,
	Mdam_trip_f,
	Mdam_trip_b,
    Mdamage_out,
    Mdamage_near,
	Mblow_soft,
    Mdown,
    Mrise,
    Mdown_b,
    Mrise_b,
    Mdamage_crouch,
    Mdamage_down,
	Mdamage_down_f,
    Mdamage_down_out,
    Mdamage_down_out_f,
	Mdamage_caution,
	Mdamage_caution_sq,
	Mout_caution,
	Mout_caution_sq,
	Mout_intrude,
	/* 押し押され */
	Mpush_walk_l_l,
	Mpush_walk_l_r,
	Mpushed_lf,
	Mpushed_lb,
    /* 首絞め */
    Mhang,
    Mtie,
    Mkill,
    Mhang_walk,
	Mhang_escape,
	Mhang_release,
	Mhang_miss,
    /* 投げ */
    Mthrow_r,
    Mthrow_l,
    /* コンボ */
    Mcombo,
    /* 死体運び */
    Mcarry_b_start_f,
    Mcarry_b_start_b,
    Mcarry_b,
    Mcarry_b_walk,
    Mcarry_b_end,
    Mcarry_l_start,
    Mcarry_l,
    Mcarry_l_walk,
    Mcarry_l_end,
    Mcarry_miss,
    /* ダンボール */
    Mbox_idle,
    Mbox_dash,
    Mbox_run,
    Mbox_walk,
	Mbox_stop,
	Mbox_walk_on,
	/* くしゃみ */
	Msnz_idle,
	Msnz_squat,
	Msnz_crouch,
	Msnz_caution
} ;

/*-----------------------------------------------------------------

  武器アイテム・定義構造体

-----------------------------------------------------------------*/

typedef	void	*( WEAPON )( CONTROL *, OBJECT **, int *, u_int *, int ) ;

typedef	struct {
    WEAPON	*func ;
    PL_ACTION	action ;
    int		type ;
    PL_ACTION	shoot ;
	PL_ACTION	callback ;
} PL_WeaponSet ;

typedef	void	*( ITEM )( CONTROL *, OBJECT **, int *, int * ) ;

typedef	struct {
    ITEM	*func ;
    PL_ACTION	action ;
    int		type ;
    PL_ACTION	shoot ;
    int		unit ;
    PL_ACTION	callback ;
} PL_ItemSet ;

/*-----------------------------------------------------------------

  プラグイン登録用

-----------------------------------------------------------------*/

typedef	struct _PL_PluginSet {
    int			plugin_no ;	/* プラグイン番号 */
    int			( *init )( PlayerWork *, GV_MSG *, int ) ; /* 設定関数 */    
    void		( *action )( PlayerWork *, int ) ;	/* 行動関数 */
    struct _PL_PluginSet	*next ;			/* リンクリスト */
} PL_PluginSet ;

typedef int	( *PL_PLUGIN_INIT )( PlayerWork *, GV_MSG *, int ) ;

/*-----------------------------------------------------------------

  ポーリング関数登録用

-----------------------------------------------------------------*/

typedef	struct _PL_PollingSet {
	int							( *func )( PlayerWork * ) ;
    struct _PL_PollingSet		*next ;			/* リンクリスト */
} PL_PollingSet ;

typedef	int		( *PL_POLLING_FUNC )( PlayerWork * ) ;

/*-----------------------------------------------------------------

  特殊ゲームオーバーアクションリンク

-----------------------------------------------------------------*/

typedef	int		( *PL_DEADMODE_COND_FUNC )( PlayerWork * ) ;

typedef	struct	_PL_DeadMode {
	struct _PL_DeadMode			*next ;
	PL_ACTION					action ;
	PL_DEADMODE_COND_FUNC		cond ;
} PL_DeadMode ;

/*-----------------------------------------------------------------

  インライン

-----------------------------------------------------------------*/

/* 風邪引きカウンタの更新 */
static	inline	void	PL_UpdateColdCount( void )
{
	if ( !GM_CheckPlayerStatus( PLAYER_COLD ) && 
		 GM_CheckPlayerStatusEX( PLAYER_IN_THE_WATER, PLAYER2_NUDE | PLAYER2_POWDER ) ) {
		GM_PlayerColdCount ++ ;
	}
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_POWDER ) ;
}

/*-----------------------------------------------------------------

  外部変数・関数宣言 

-----------------------------------------------------------------*/

#include	"pl_work.x"

/*----------------------------------------------------------------*/

extern	void	*NewPadVibration( char *, int ) ;
extern	void	*PL_SubjectCameraManager( GM_CameraSet *subject, GM_CameraSet *weapon, int mode ) ;
extern	void	PL_SubjectCameraOn( void ) ;
extern	void	PL_SubjectCameraOff( void ) ;

//BP_INPUT - Adding MGS3 hold "X" for squat to ground functionality
void PL_UpdatePressSquat( PlayerWork *work );
int PL_IsPressSquat( PlayerWork *work );

#if defined(BP_VITA)
extern int PL_PadGetGestureRelease( PlayerWork *pl_work );
#endif

#ifdef DEBUG_MODE
/* でバグ用 */
extern	void PosBox(FVECTOR	*pos,float wide,SVECTOR	*rgb) ;
#endif

#endif /* __pl_work_h__ */
