/*
	enemy.h
	敵兵ルーチンヘッダ

	1999/07/07 Y.Korekado
	$Id: enemy.h,v 1.1.1.3 2002/11/19 11:44:09 Yoshizawa1 Exp $
	
*/
#ifndef __ENEMY___
#define	__ENEMY___ 1

/*----- インクルード --------------------------------------------------*/
#include	"korekado/action/action.h"
#include	"korekado/conv/define.h"
#include	"speak.h"
#include	"korekado/conv/cnctobj.h"
#include	"korekado/conv/korekado.x"
#include	"../../takabe/other/puppetik.h"
#include	"../thing/enemem.h"

//#include	"security.h"

#include	"../../shibata/t_headmark/headmark2.h"

/*重野追加 ムカデＩＫ用関数プロトタイプ*/
#ifdef PSX2
#include	"../../kano/ik/ik_sub.h"
#endif

#define HEAD_TO_TARGET	/* 頭から視界計算 */
#define	WARP_JIK		/* ワープ実験 */

#define GM_PlayerHzxID	(GM_PlayerControl->hzx_id)

#define NO_BLOOD_OUT (1)	/* 出血死なし */

/*----- データ名 --------------------------------------------------*/
/* MDL_の実体はg_define.hに定義 */
#define E_WP_AKS	MDL_AKS		/* (GV_StrCode( "aks" )) */
#define E_WP_M4_NM	MDL_M4B_GRD	
#define E_WP_SPS	MDL_SPS		/* (GV_StrCode( "sps" )) */
#define E_WP_AKS_SP	MDL_AKS_SP		/* (GV_StrCode( "aks_sp" )) */
#define E_WP_MKR	MDL_MKR		/* (GV_StrCode( "mkr" )) */
#define E_WP_P90	MDL_P90		/*p90*/
#define E_WP_KATANA	MDL_TNG_KANATA	/*tng_katana*/
#define E_WP_ABAKAN	MDL_ABAKAN		/*abk.kms*/
#define	E_GLASS_NAME (10982105)		/* (GV_StrCode( "glass" )) */

#define	E_DOGTAG_NAME (9666632)		/* (GV_StrCode( "dogtag_gps" )) */

#define	ENE_MDL_NAME_GBS	(6371217)	/* GV_StrCode( "gbs_def" ) */
#define	ENE_MDL_NAME_GBA	(13216431)	/* GV_StrCode( "gba_def_mt" ) */
#define	ENE_MDL_NAME_GPS	(14170799)	/* GV_StrCode( "gps_def_mt" ) */
#define	ENE_MDL_NAME_GPA	(14133935)	/* GV_StrCode( "gpa_def_mt" ) */
//#define	ENE_MDL_NAME_TNG	(10566376)	/* GV_StrCode( "tng_def" ) */
#define	ENE_MDL_NAME_TNG	(2715036)	/* GV_StrCode( "tng_high" ) */
#define	ENE_MDL_NAME_HTC	(16497327)	/* GV_StrCode( "htc_def_mt" ) */
/*シールズ*/
//#define	ENE_MDL_NAME_SEL	(15809174)	/* GV_StrCode( "sel_def" ) */

/*VR*/
//#define	ENE_MDL_NAME_VR		(13751595)	/* GV_StrCode( "vr_gbs_def_mt" ) */
#define	ENE_MDL_NAME_VR		(10381731)	/* GV_StrCode( "vr2_gbs1_sh_mt" ) */
#define	ENE_MDL_NAME_VR_ATK	(10381795)	/* GV_StrCode( "vr2_gbs2_sh_mt" ) */

#define	ENE_MOT_BASE		(108723)	/* GV_StrCode( "gbs" ) */
#define	ENE_MOT_STAGE		(7266994)	/* GV_StrCode( "gbsstage" ) */

/*----- 定数定義 --------------------------------------------------*/
#define	AVOID_TIME	COUNT_VMODE(60*20)	//回避モードの時間

#define ALERT_NOISE_TIME_DETECT_PLAYER	(6)	//危険モードＬノイズ数回でプレイヤー発見

#define MAX_MAGAZINE_MG (30+1)
#define MAX_MAGAZINE_AK (30+1)
#define MAX_MAGAZINE_P90 (50+1)
#define MAX_MAGAZINE_SPS 7	/*ショットガン*/
#define MAX_MAGAZINE_MKR 8	/*ハンドガン*/


#define	ENEMY_APPEAR_MAX (64)	/*最大存在人数（live+corp+oldcorp）*/
#define	ENEMY_MAX_RES 	(64)	/*最大復活人数*/
#define DEF_MAX_RES_IN_ALERT	16	/* 危険モード復活人数 */

#define UNDER_EYE_SIGHT	512		/* 下方向視野 */
#define UPPER_EYE_SIGHT	-512			/* 上方向視野 */

#define	DEF_EYE_SIGHT		6000	/* 標準視力 */
#define	DEF_AT_EYE_SIGHT	6000	/* 攻撃兵 */
#define	DEF_EYE_RANGE		512		/* 標準視野 */
#define DEF_GLASSES_DIS		15000	/* 双眼鏡装着時の＋視力 */
#define DEF_GLASSES_RANGE	128		/* 双眼鏡装着時の視界 */
//#define DEF_EYE_DELAY		12		/* 認識遅延時間 */
#define DEF_EYE_DELAY		2		/* 認識遅延時間 */
#define	DEF_HEARING			12000	/* 標準聴力 */
#define	DEF_SMELL			6000	/* 標準嗅覚 */
#define	EYE_NEARLENGTH		500		/* 近すぎる距離 */
#define ENE_HOLD_DIS		2500	/* ホールドアップ距離 */
#define	MAX_DAM_ENE			24		/* ダメージチェック人数 */
#define	NORINORI_HEARING	1000	/* ウォークマン兵聴力 */
#define EYE_LIGHT_DIS		(entk->sense.eye_s*3)	/* ライトが直接見える距離 */
#define EYE_LIGHT_NEARDIS	(entk->sense.eye_s*2)	/* 照らされれば気づく距離 */
#define EYE_LIGHT_RANGE		512	/* 適当！！ */
#define CROSS_DIS			250	/* 重なり判定距離 適当！！ */
#define	RUN_DISTANCE		(12000/250) /* 走って移動する距離（ゾーン間距離）*/
#define ENE_REGION_DETECT_COUNT	COUNT_VMODE(16)	/* 部位が発見される為の必要フレーム */
#define	ENE_INDISTINCT_ALERT_LEVEL	COUNT_VMODE(8) /* 朧モード閾値 */
#define COMMAND_WAKEUP_COUNT	COUNT_VMODE(180)	/* コマンダーが寝ている兵を起こす時間 */
#define	BOX_CHECK_STAND_DIS		(1000)	/* 近づく距離 */
#define	ACCIDENT_DELAY_TIME     COUNT_VMODE(900)   /* 確認に移るまでの時間 */
#define	ACCIDENT_POS_DELAY_TIME     COUNT_VMODE(600)   /* 異常発見確認に移るまでの時間 */
#define	ACCIDENT_REPO_DELAY_TIME     COUNT_VMODE(3600)   /* 定時連絡兵確認に移るまでの時間 */

#define	DEF_LIFE		3000	/* 標準体力値 */
#define	DEF_FAINT		9		/* 標準気絶値 */
#define	DEF_BLOOD		6000	/* 標準血液値 */

#define ENE_SUBWP_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE)

#define ENE_DEF_TARGET_CLASS	(TARGET_LOCKON|TARGET_CAPTURE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_CHILD)	/* 標準ターゲットクラス */
#define FAINT_TARGET_CLASS	(TARGET_CAPTURE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_CHILD|TARGET_DEAD)	/* 気絶ターゲットクラス */

#define CHILD_TRG_FLAG	(TARGET_ROTATE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)

#define	NEAR_ATK_DIS	1000
#define FAINT_TIME		COUNT_VMODE(60*30)
#define ZZZ_TIME		COUNT_VMODE(60*60*5)

/** 攻撃力 NORMAL LIFE LIMIT 100****/
#define ENE_PUNCH_DMG	(5)
#define ENE_GRENADE_DMG	(40)
#define ENE_M4GRD_DMG	(60)	/*グレネードランチャー強く*/
#define ENE_GRENADE_NO_AVOID_DMG	(80) /*回避不能グレネード*/

#define	BODY16_HEAD		6
#define	BODY16_RHAND	4
#define	BODY16_LHAND	9
#define	BODY16_RSHOULD	2
#define	BODY16_LSHOULD	7
#define	BODY16_WAIST	0
#define	BODY16_RFOOT	12
#define	BODY16_LFOOT	15

#define	BODY21_HEAD		12
#define	BODY21_RHAND	6
#define	BODY21_LHAND	10
#define	BODY21_RSHOULD	4
#define	BODY21_LSHOULD	8
#define	BODY21_WAIST	0
#define	BODY21_CHEST	2
#define	BODY21_RFOOT	15
#define	BODY21_LFOOT	19
#define	BODY21_RTOE		16
#define	BODY21_LTOE		20

/* ヘッドマーク */
#define HM_START	HMK2_TYPE_CALL

#define HEADMARK_BR		HMK2_TYPE_RED_AT	//赤！
#define HEADMARK_QR		HMK2_TYPE_RED_QE	//赤？
#define HEADMARK_BY		HMK2_TYPE_YLW_AT	//黄！
#define HEADMARK_QY		HMK2_TYPE_YLW_QE	//黄？
#define HEADMARK_BW		HMK2_TYPE_WHT_AT	//白！
#define HEADMARK_QW		HMK2_TYPE_WHT_QE	//白？
#define HEADMARK_BP		HMK2_TYPE_PNK_AT	//ピンク！
#define HEADMARK_ZZZ	HMK2_TYPE_ZZZ	//居眠り
#define HEADMARK_PIYO	HMK2_TYPE_PIYO	//気絶
#define HEADMARK_ANA	HMK2_TYPE_PIYO_A	//麻酔
#define HEADMARK_POWA	HMK2_TYPE_POWA		//放心 

#define HEADMARK_CLEAR	HMK2_TYPE_KILL	//クリア

#define HEADMARK_TARGET	HMK2_TYPE_TRGT	//壊せる

#define	FLAG_CLEAR 		0

/* 目アニメ */
enum {
	EYE_NORMAL,		/* 0通常 */
	EYE_MABATAKI,	/* 1瞬き */
	EYE_BIKKIURI,	/* 2瞬きあり見開き */
	EYE_DAMAGE,		/* 3ダメージ（つぶり） */
	EYE_SIROME,		/* 4白目 */
	EYE_TUBIRI,		/* 5つぶりっぱなし */
	EYE_MIHIRAKI,	/* 6開きっぱなし */
	EYE_KURURI,		/* 7白目へ */
} ;

/* 口パク */
enum {
	MOUTH_NORMAL,	/* 0通常 */
	MOUTH_I,		/* 「い」の口 */
	MOUTH_O,		/* 「お」の口 */
} ;

/* バレットフラグ */
#define	ENE_BULLET_NORMAL	0x00000000	/* ランダムずれ有り、狙いは頭、跳弾有り、軌跡有り、右手に銃 */
#define	ENE_BULLET_NORANDAM	0x00000001	/* ランダムずれなし */
#define	ENE_BULLET_NOWALL	0x00000002	/* 壁を突きぬける */
#define ENE_BULLET_NOATTACK	0x00000004	/* 攻撃判定無し */
#define	ENE_BULLET_NOSPARK	0x00000010	/* 跳弾無し */
#define	ENE_BULLET_NOLINE	0x00000020	/* 軌跡無し */
#define	ENE_BULLET_LEFT		0x00000040	/* 左手に銃 */

/*----- status ------*/
/* 初期設定 */
#define	ENE_STATUS_EVER_UNREAL	0x00000001	/* スーパー存在しない */
#define	ENE_STATUS_GUNLIGHT		0x00000002	/* 銃ライトセット */
#define	ENE_STATUS_BODYSHADOW	0x00000004	/* 体影セット */
#define	ENE_STATUS_BODYSPLASH	0x00000008	/* 体水飛沫セット */
#define	ENE_STATUS_SHADOW_OFF	0x00000010	/* 影ＯＦＦ */
#define	ENE_STATUS_NIGHT_SIGHT	0x00000020	/* 暗視ゴーグル */
#define	ENE_STATUS_NORINORI		0x00000040	/* ウォークマン兵 */
#define	ENE_STATUS_NIGHT_SIGHT2	0x00000080	/* 暗視ゴーグル常に付ける */
#define	ENE_STATUS_EVER_ZZZ		0x00000100	/* 一度寝ると起きない */
#define	ENE_STATUS_DEAD			0x00000200	/* 死体 */
#define	ENE_STATUS_NO_EYEANIM	0x00000400	/* 目アニメ無し */
#define	ENE_STATUS_NO_VANIM		0x00000800	/* 頂点アニメ無し */
#define	ENE_STATUS_NO_FINGER	0x00001000	/* 手モーフィング無し */
#define	ENE_STATUS_NO_HEADMARK	0x00002000	/* ヘッドマーク無し */
#define	ENE_STATUS_TRAPSHADOW	0x00004000	/* 投影モデルの切替えをトラップで行う */
#define	ENE_STATUS_NO_BLURR		0x00008000	/* 朧視界なし */
#define	ENE_STATUS_LONELY_FIGHT	0x00010000	/* 発見しても一人で戦う */
#define	ENE_STATUS_ENDLES_CHASE	0x00020000	/* ずっと追いかける（変装時） */
#define	ENE_STATUS_NO_CAPTURE	0x00040000	/* 捕まれ無い*/
#define	ENE_STATUS_BIG_SNORE	0x00080000	/* いびきがでかい */
#define ENE_STATUS_URBAN		0x00100000  /* 都市迷彩 */
#define ENE_STATUS_HOUNYOU		0x00200000  /* おしっこ兵 */
#define ENE_STATUS_NPC			0x00400000  /* NPC担当兵 */
#define ENE_STATUS_CONVERT		0x00800000  /* 警備兵から攻撃兵へ転向兵 */
#define ENE_STATUS_REPORT		0x01000000  /* 定時連絡兵 */
#define ENE_STATUS_TALK_SLEEP	0x02000000  /* 寝言で返事 */
#define ENE_STATUS_INVESTIGATED	0x04000000  /* 仲間に調査依頼 */
#define ENE_STATUS_ACCDNT_REPO	0x08000000  /* 発見後アクシデント報告 */
#define ENE_STATUS_NO_STEALTH	0x10000000  /* ステルスチェックなし */
#define ENE_STATUS_BLUFF		0x20000000  /* 強がり兵 */
#define ENE_STATUS_CONVERT2		0x40000000  /* 攻撃兵からサポート兵へ転向兵 */

/* フラグ */
#define	ENE_STATUS_TRAPSHADOW_ON	0x80000000	/* 投影モデルの切替えON */

/*----- status_status ------*/
#define	ENE_STST_OPTCAMOUFLAGE	0x00000001	/* 光学迷彩 */
#define ENE_STST_VRBODY			0x00000002	/* VR用のモデル */
#define ENE_STST_FAINT_EXIT		0x00000004	/* 気絶、居眠りで退場 */
#define ENE_STST_HOLD_EXIT		0x00000008	/* ホールドアップで退場 */
#define ENE_STST_DONT_KILL		0x00000010	/* 殺すとゲームオーバー */
#define ENE_STST_NO_OOZEBLOOD	0x00000020	/* にじみ血無し */

/*----- status2 ------*/
#define	ENE_STATUS_GLASSES			0x00000010	/* 双眼鏡 */
#define	ENE_STATUS2_GUNLIGHT		0x00000020	/* ガンライトＯＮ */
#define	ENE_STATUS2_AIM_WAIST		0x00000080	/* 腰から注視点に向ける */
#define	ENE_STATUS2_AIM_FACE_Y		0x00000100	/* 顔を注視点に向けるY方向 */
#define	ENE_STATUS2_AIM_FACE_X		0x00000200	/* 顔を注視点に向けるX方向 */
#define	ENE_STATUS2_AIM_GUN			0x00000400	/* 銃を注視点に向ける */
#define	ENE_STATUS2_AIM_DIR_FACE	0x00000800	/* 顔を指定方向に向ける */
#define	ENE_STATUS2_AIM_NO_WAIST	0x00001000	/* 腰を動かさない */
#define	ENE_STATUS2_AIM_TURN		0x00002000	/* ゆっくりと見渡す */
#define	ENE_STATUS2_AIM_GUNSHOOT	0x00004000	/* 銃を注視点に向ける（撃つときはこれ！） */
#define	ENE_STATUS2_AIM_RHAND		0x00008000	/* 右腕だけで銃を注視点に向ける */
#define	ENE_STATUS2_AIM_FACE_ONLY	0x00010000	/* 顔だけを注視点に向ける */
#define	ENE_STATUS2_AIM_NO_LIMIT	0x00020000	/* リミット無し */
#define	ENE_STATUS2_AIM_S_LIMIT		0x00040000	/* リミット小さい */


#define	ENE_STATUS2_AIM_FACE	(ENE_STATUS2_AIM_FACE_Y|ENE_STATUS2_AIM_FACE_X)
#define	ENE_STATUS2_AIM			(ENE_STATUS2_AIM_FACE|ENE_STATUS2_AIM_FACE_ONLY|ENE_STATUS2_AIM_DIR_FACE|ENE_STATUS2_AIM_GUN|ENE_STATUS2_AIM_TURN|ENE_STATUS2_AIM_RHAND)


/*----- thk_status（通信用） ------*/
#define	THK_STATUS_MOVE				0x00000001	/* 移動中 */
#define	THK_STATUS_WATCH			0x00000002	/* 確認中 */
#define	THK_STATUS_TALK				0x00000004	/* 話中 */
#define	THK_STATUS_RADIO			0x00000008	/* 無線中 */
#define	THK_STATUS_SIGHT_IN			0x00000010	/* 視認中 */
#define	THK_STATUS_CLE_SUPPORT		0x00000020	/* クリアリングサポート中 */
#define	THK_STATUS_DOWN_WATCH		0x00000040	/* 倒れている仲間確認中 */
#define	THK_STATUS_CORP_WATCH		0x00000080	/* 死体確認中 */
#define	THK_STATUS_CBOX_WATCH		0x00000100	/* 段ボール確認中 */
#define	THK_STATUS_ROOT_MOVE		0x00000200	/* 巡回ルート ルートからルートへ移動中 */
#define	THK_STATUS_ROOT_TRACE		0x00000400	/* 巡回ルート トレース中 */
#define	THK_STATUS_ROOT_ACTION		0x00000800	/* 巡回ルート ルートアクション中 */
#define	THK_STATUS_ROOT_RADIO		0x00001000	/* 巡回ルート 無線中 */
#define	THK_STATUS_ATTACK_PLAYER	0x00002000	/* プレイヤー攻撃中 */
#define	THK_STATUS_CLEARING			0x00004000	/* クリアリング中 */
#define	THK_STATUS_CHECKZONE		0x00008000	/* ゾーンチェック中 */
#define	THK_STATUS_HOLD_UP			0x00010000	/* ホールドアップ中 */


enum {	/* オーバーライド */
	PBREAK_OVER_NONE,
	PBREAK_OVER_STAND,
	PBREAK_OVER_MOVE,
	PBREAK_OVER_BASE,
	PBREAK_OVER_HOLDUP,
	PBREAK_OVER_HOLDUP_BASE,
	PBREAK_OVER_BASE_HAND_ONLY,
} ;

enum {	/* ターゲットサイズ enemy.c EneBodyTrgSize */
	ENE_BODYTRGSIZE_STAND,
	ENE_BODYTRGSIZE_SQUAT,
	ENE_BODYTRGSIZE_DOWN,
	ENE_BODYTRGSIZE_STAND_SHIELD,
	ENE_BODYTRGSIZE_SQUAT_SHIELD,
	ENE_BODYTRGSIZE_STAND_DEF,
	ENE_BODYTRGSIZE_TOUCH,
	ENE_BODYTRGSIZE_LIE,
	ENE_BODYTRGSIZE_WALLDOWN,
	ENE_BODYTRGSIZE_W25_SUPER_PEEP,
} ;

/*----- 敵兵共通メッセージ ------*/
#define ENE_MSG_TRAP_SHADOW_ON	(256)		/* 影ＯＮ */
#define ENE_MSG_TRAP_SHADOW_OFF	(257)		/* 影ＯＦＦ */
#define ENE_MSG_POINT_ACTION_START	(258)		/* ポイントアクションスタート */

#define ENE_MSG_EVER_DOWN_OFF	(259)		/* 永久ダウンオフ */
#define ENE_MSG_EVER_DOWN_ON	(260)		/* 永久ダウンオン */
#define ENE_MSG_SET_DOWN_TIME	(261)		/* ダウン時間セット */

#define ENE_MSG_UNREAL_ON		(262)		/* 永久アンリアルON */
#define ENE_MSG_UNREAL_OFF		(263)		/* 永久アンリアルOFF */

/*----- 警備兵メッセージ ------*/
#define ENE_MSG_ROOTCHANGE		(1)		/* ルート変更 */
#define ENE_MSG_ACTION_END		(2)		/* アクション終了 */
#define ENE_MSG_HOUNYOU_HOM_ON	(3)		/* 放尿ホーミング開始 */
#define ENE_MSG_HOUNYOU_HOM_OFF	(4)		/* 放尿ホーミング止め */

/***攻撃兵 メッセージリスト ***/
#define AT_MSG_DEFENSE_OFF	(0) /*守備終了*/
#define AT_MSG_ROOTCHANGE	(1)	/* ルート変更 */
#define AT_MSG_POS_SET		(2) /*守備位置指定*/
#define AT_MSG_DEFENSE_ON	(3) /*守備開始*/
#define AT_MSG_SEARCHCHANGE	(4)	/* 警戒ルート変更 */

/***長廊下兵 メッセージリスト ***/
#define EVENT_MSG_DEFENSE	(0)		/*守備開始*/
#define EVENT_MSG_ALERT		(1)		/*追跡開始*/
#define EVENT_MSG_WARP		(2)		/*座標移動*/
#define EVENT_MSG_ROOTCHANGE	(3)	/*ルート変更*/
#define EVENT_MSG_MODE		(4)	/*ルート変更*/
#define EVENT_MSG_RESET		(5)	/*初期配置に戻る*/

/***天狗兵Ａ メッセージリスト ***/
#define TNG_A_MSG_DEFENSE	(0)		/*守備開始*/
#define TNG_A_MSG_ALERT		(1)		/*追跡開始*/
#define TNG_A_MSG_WARP		(2)		/*座標移動*/
#define TNG_A_MSG_ROOTCHANGE	(3)	/*ルート変更*/
#define TNG_A_MSG_MODE		(4)	/*ルート変更*/
#define TNG_A_MSG_RESET		(5)	/*初期配置に戻る*/


/*----- ------*/
	/* 視覚情報 */
#define	EYE_INFO_SIGHT_IN		3		/* はっきりとみえる */
#define	EYE_INFO_SIGHT_BLURR	2		/* ぼやけてみえる	*/
#define	EYE_INFO_SIGHT_OUT_HZD	1		/* 間に障害物が有りみえない			*/
#define	EYE_INFO_SIGHT_OUT		0		/* みえない			*/

#define	EYE_INFO_FLAG_SKIP			0x00000001	/* スキップ */
#define	EYE_INFO_FLAG_BOX			0x00000002	/* ダンボール見つめ中 */

	/* ？（Notice）モード */
#define	ENE_NOTICE_NONE			0x00000000
#define	ENE_NOTICE_NOISE		0x00000001	/* 物音モード */
#define	ENE_NOTICE_SMELL		0x00000002	/* 臭いモード */
#define	ENE_NOTICE_FOOT			0x00000004	/* 足跡モード */
#define	ENE_NOTICE_BLOOD		0x00000008	/* 血痕モード */
#define	ENE_NOTICE_BOX			0x00000010	/* 段ボールモード */
#define	ENE_NOTICE_DAMAGE		0x00000020	/* ダメージモード */
#define	ENE_NOTICE_ENE_DAMAGE	0x00000040	/* 味方ダメージモード */
#define	ENE_NOTICE_CORP			0x00000080	/* 死体モード */
#define	ENE_NOTICE_DISCOVERY	0x00000100	/* プレイヤー発見モード */
#define	ENE_NOTICE_HOLDUP		0x00000200	/* ホールドアップモード */
#define	ENE_NOTICE_INDISTINCT	0x00000400	/* ぼんやりモード */
#define	ENE_NOTICE_NOISE_M		0x00000800	/* ノイズＭモード */
#define	ENE_NOTICE_NOISE_L		0x00001000	/* ノイズＬモード */
#define	ENE_NOTICE_LIGHT		0x00002000	/* ライトモード */
#define	ENE_NOTICE_REGION		0x00004000	/* 部位発見モード */
#define	ENE_NOTICE_FOUND		0x00008000	/* 不審物発見モード */
#define	ENE_NOTICE_TRACE		0x00010000	/* 不審物発見モード */
#define	ENE_NOTICE_NEAR_BOX		0x00020000	/* 近距離段ボールモード */
#define	ENE_NOTICE_ACCIDENT		0x00040000	/* アクシデントモード */
#define	ENE_NOTICE_COUNTER		0x00080000	/* 反撃 */
#define	ENE_NOTICE_ENDAMAGE_NOW	0x00100000	/* 今ダメージを受けている */
#define	ENE_NOTICE_TOUCH		0x00200000	/* タッチモード */
#define	ENE_NOTICE_UNIFORM		0x00400000	/* 変装モード */
#define	ENE_NOTICE_EMMA			0x00800000	/* エマ発見モード */
#define	ENE_NOTICE_INVESTIGATED	0x01000000	/* 調査モード */
#define	ENE_NOTICE_ACCDNT_REPO	0x02000000	/* アクシデント報告 */
#define	ENE_NOTICE_NEAR_TOUCH	0x04000000	/* ふれた */
#define	ENE_NOTICE_RADIO		0x08000000	/* 無線連絡 */
#define	ENE_NOTICE_RES			0x80000000	/* 再発生 */

#define MES_NOTICE_NONE			0x00000000	/* メッセージ無し */
#define MES_NOTICE_ZZZ			0x00000001	/* 眠ってました */
#define MES_NOTICE_GOOFY		0x00000002	/* 呆けてました */
#define MES_NOTICE_FAINT		0x00000004	/* 気絶してました */
#define MES_NOTICE_SMOKE		0x00000008	/* 煙かぶってました */
#define MES_NOTICE_BUMP			0x00000010	/* ぶつかってしまった */

	/* iknow フラグ */
#define IKNOW_DETECT			0x00000001	/* 発見している */
#define IKNOW_TARGET			0x00000002	/* 狙われている */
#define IKNOW_RADIO_BREAK		0x00000004	/* 無線機壊れている */
#define IKNOW_NSIGHT_BREAK		0x00000008	/* 暗視ゴーグル壊れている */
#define IKNOW_ZZZ				0x00000010	/* 眠っている */
#define IKNOW_HOLD_PLAYER		0x00000020	/* ホールドアップ中プレイヤー視認 */
#define IKNOW_HIDDEN_PLAYER		0x00000040	/* プレイヤーが隠れるのを見た */
#define IKNOW_CLEARING			0x00000080	/* クリアリング中 */
#define IKNOW_HELP_ME			0x00000100	/* 仲間の助けが必要 */
#define IKNOW_MUST_HOLD_UP		0x00000200	/* ホールドアップしなくてはならない */
#define IKNOW_AVOID_CLEARING	0x00000400	/* 回避モードにクリアリングする */
#define IKNOW_DRESSED			0x00000800	/* 仲間に変装している */
#define IKNOW_PLAYER_POS		0x00001000	/* プレイヤーを発見している */
#define IKNOW_NPC_POS			0x00002000	/* NPCを発見している */
#define IKNOW_HELP_ME_REPO		0x00004000	/* 倒れているのを報告された */
#define IKNOW_DOWNED			0x00008000	/* 倒れていた */
#define IKNOW_DETECT_SEARCH		0x00010000	/* 発見モードから警戒へ */
#define IKNOW_CLEARIMG_DAMAGE	0x00020000	/* クリアリング中ダメージ */
#define IKNOW_HIDDEN_PLAYER_CLEARING_INTRUDE	0x00040000	/* クリアリング中にインルード発見 */

	/* receive フラグ */
#define ENE_ORDER_RESURRECT		0x00000001	/* 復活して下さい */
#define ENE_ORDER_DIE_ERASE		0x00000002	/* 死んで消えて下さい */
#define ENE_ORDER_HOW_DO		0x00000004	/* どうしました？ */
#define	ENE_ORDER_ACTION_END	0x00000008	/* アクションエンド */
#define	ENE_ORDER_CALL_COMM		0x00000010	/* 司令室より応答せよ*/

/*----- ------*/
/*atavoid.c より移動 2000.02.04 重野*/
#define		CLE_BUFF_AREA	0
#define		CLE_BUFF_KIND	1
#define		CLE_BUFF_SCENE	2

enum {
	ENE_TH1_SNEAK,ENE_TH1_ALERT,ENE_TH1_AVOID,ENE_TH1_SEARCH,ENE_TH1_DAMAGE,ENE_TH1_RESURRECT,ENE_TH1_DEFENSE 
} ;

/*----- モーションシーケンス motion.h の EnemyMotionArrayと相互関 ------------------------*/
/*base gbs.mar */
enum {
	/* 基本モーション 32 */
	EM_stand,
	EM_walk,
	EM_run,
	EM_find_ply,
	EM_cau_mukade_idle,
	EM_ijiiji,
	EM_glasses,
	EM_glasses_p,
	EM_idle_holdup,
	EM_idle_holdup_surprise,	/* ホールドアップ中驚き */
	EM_call_break_l,
	EM_call_break_r,
	EM_turn_b_t,
	EM_turn_l_t,
	EM_turn_r_t,
	EM_akubi,
	EM_senobi,
	EM_zzz,
	EM_walk_light,
	EM_surprised_bomb, /* 物音びっくり */
	EM_gogle_off,		/* ゴーグルはずす */
	EM_gogle_on,		/* ゴーグル付ける */
	EM_walk_pass,		/* すれ違いモーション */
	EM_call_team_l_1start,	/* 無線構え */
	EM_call_team_l_2talk,	/* 無線使用中 */
	EM_call_team_l_3end,	/* 無線もどす */
	EM_call_team_gun_l_1_start,	/* 銃構えて無線連絡 */
	EM_call_team_gun_l_2_talk,	/* 銃構えて無線連絡 */
	EM_call_team_gun_l_3_end,	/* 銃構えて無線連絡 */
	EM_look_erotica,			/* エロ本発見 */
	EM_look_erotica_loop2,		/* エロ本鑑賞２ */
	EM_look_erotica_loop1,		/* エロ本鑑賞１ */
	EM_look_erotica_stand, 
	EM_look_erotica_stand_loop2, 
	EM_look_erotica_stand_loop1, 
	EM_call_team_gun_l_1_start_fast,/* 銃構えて無線連絡 早 */
	EM_call_team_l_1_start_fast,/* 無線構え 早 */
	EM_idle_holdup_item,	/* ホールドアップアイテム出し */
	EM_idle_holdup2idle,	/* ホールドアップ密かに戻る */
	EM_idle_holdup_bend,	/* ホールドアップ強がる */
	EM_box_look,		/* ダンボール見に行く */
	EM_box_look_idle,	/* ダンボール調べる */
	EM_box_kick,		/* ダンボール蹴る */
	EM_box_look2stand,	/* ダンボール調べ終わる */
	EM_box_lift,		/* ダンボール持上げる */
	EM_box_lift_p,		/* ダンボール持上げるアイドリング */
	EM_box_lift2throw,	/* ダンボール投げる */
	EM_see_patient,		/* 死体確認 */

	EM_arml_cle_call_break_r,			/* 右手無線壊れ */
	EM_arml_cle_call_team_r_1start,		/* 右手無線 */
	EM_arml_cle_call_team_r_2talk,		/* 右手無線 */
	EM_arml_cle_call_team_r_3end,		/* 右手無線 */

	EM_stair_run_down,	/* 階段走り降り */
	EM_stair_walk_down,	/* 階段歩き降り */
	EM_stair_run_up,	/* 階段走り上り */
	EM_stair_walk_up,	/* 階段歩き上り */

	EM_check_disguise_end,		/* 変装疑う */
	EM_check_disguise_idle,		/* 変装疑う */
	EM_check_disguise_start,	/* 変装疑う */
	EM_ak_nom_holdup_counter,			/* 攻撃兵ホールドアップ */

	/* VR */
	EM_ak_nom_cle_look_l_max,  	
	EM_ak_nom_cle_look_r_max,  	

	/* 攻撃基本モーション 24 */
	EM_run_atk,
	EM_kamae_gun_high,
	EM_reload,
	EM_squat_fire_reload,
	EM_slide_l_fire,
	EM_slide_r_fire,
	EM_tobidasi_l_t,
	EM_tobidasi_r_t,
	EM_squat_fire,
	EM_squat_hide_idle,
	EM_squat_roll_l,		/* しゃがみ転がり左 */
	EM_squat_roll_r,		/* しゃがみ転がり右 */
	EM_step_l,				/* ステップ左 */
	EM_step_r,				/* ステップ右 */
	EM_behind_idle,			/* 壁張り付き */
	EM_walk_b_atk,			/* 攻撃しながらさがる */
	EM_nom_stomp,			/* 踏み付け */
	EM_nom_signe_go_atk,	/* ゴーサイン */
	EM_gm_fire_high,		/* 手榴弾上投げ */
	EM_gm_fire_low,			/* 手榴弾下投げ */
	EM_gm_fire_korokoro,	/* 手榴弾転がし */
	EM_cle_gm_fire,			/* クリアリングスタン投げ */

	EM_sign_mukade_go,		/* ムカデから突入サイン */
	EM_attack_near,			/* 近距離攻撃 キック */

	EM_ak_attack_near_kick_r,	/*新 右キック */
	EM_ak_attack_near_kick_l,	/*左キック*/
	EM_ak_attack_near_punch_r,	/*右パンチ*/
	EM_ak_attack_near_punch_l,	/*左パンチ*/
	EM_snipe_fire_p,		/* 覗き込み撃ち */
	EM_nom_turn_out,		/* 寝ている兵起こす */
	EM_squat_fire_intrude,	/* 対イントルード射撃 */
	EM_ak_call_team_l,		/* 攻撃兵無線モーション */

	/* 警戒基本モーション 16 */
	EM_cle_stand02,
	EM_cle_call_team,
	EM_cle_call_team_r,
	EM_cle_signe_go,
	EM_cle_look_l,
	EM_cle_look_r,
	EM_cle_look_d,
	EM_cle_walk,
	EM_cle_walk_low,
	EM_cle_kaniwalk_l,
	EM_cle_kaniwalk_r,
	EM_cle_run,
	EM_cle_signe_clear,
	EM_cle_signe_move,
	EM_cle_signe_stop,
	EM_cle_locker_open,	/* ロッカー開くモーション */
	EM_ak_cle_go_ahead,

	/* ダメージモーション 36 */
	EM_dam_nokezori,
	EM_dam_gun_2,
	EM_dam_gun_3,
	EM_dam_out_f,
	EM_dam_out,
	EM_dam_out_pause,
	EM_nom_iya_escape,
	EM_nom_iya_hang,
	EM_nom_iya_idle,
	EM_iya_release,
	EM_iya_release_down,
	EM_nom_iya_resist,
	EM_nom_iya_tie,
	EM_nom_iya_walk,
	EM_dam_bomb_b,	/* 倒れるけどすぐ起き上がる場合のダウン */
	EM_dam_bomb_f,
	EM_dam_throw,
	EM_dam_kick_t,
	EM_dam_wall_down,
	EM_dam_wall_down_p,
	EM_dam_wall_move_d,
	EM_okiru_aomuke,
	EM_okiru_aomuke_slow,
	EM_okiru_utubuse,
	EM_okiru_utubuse_slow,
	EM_dam_punch_l,
	EM_dam_punch_r,
	EM_dam_fence_down,
	EM_dam_fence_fall,
	EM_dam_fence_start,
	EM_sleep_f,
	EM_sleep_b,
	EM_dam_bomb_fly_b,
	EM_dam_bomb_fly_f,
	EM_dam_elude,		/* 上から乗っかられてダウン */
	EM_dam_out_bleed,	/* 出血多量死 */
	EM_dam_okaji_end,	/* 呆けから復帰 */
	EM_dam_okaji,		/* 呆け */
	EM_dam_stun_far,	/* スタン、ファーダメージ */
	EM_dam_stun_near,	/* スタン、ニアダメージ */
	EM_dam_fire_ex,		/* 水蒸気ダメージ */
	EM_avoid_hang,		/* 首締め避け */
	EM_dam_punch2down,
	EM_dam_stair_start_b,	/* 階段転がり後ろ向きから */


	EM_dam_fence_f_fall,	/* 手摺り落ち中 */
	EM_dam_fence_f_down,	/* 手摺り落ちダウン */
	EM_dam_fence_f_start,	/* 手摺り落ちスタート */
	EM_dam_stair_start,		/* 階段落ちスタート */
	EM_dam_stair_roll_1,		/* 階段落ち転がり */
	EM_dam_stair_roll_2,		/* 階段落ち転がり */
	EM_dam_stair_end,		/* 階段落ち最後 */
	EM_push_ply_r,			/* スネーク押し右 */
	EM_push_ply_l,			/* スネーク押し左 */
	EM_squeezed_f,			/* 押され前 */
	EM_squeezed_b,			/* 押され後ろ */
	EM_dam_cliff_b,			/* 崖落下後ろ */
	EM_dam_cliff_f,			/* 崖落下前 */

	EM_dam_out_f_cap01,
	EM_dam_out_f_cap02,
	EM_dam_out_b_cap01,
	EM_dam_out_b_cap02,
	EM_dam_out_b_cap03,
	EM_dam_out_b_cap04,
	EM_dam_ball_cap01,

	/* 部位ダメージ */
	EM_dam_arm_l,
	EM_dam_leg_l,
	EM_dam_legs,
	EM_dam_ball,
	EM_arml_idle,
	EM_arml_run,
	EM_arml_walk,
	EM_arms_idle,
	EM_arms_run,
	EM_arms_walk,
	EM_legl_idle,
	EM_legl_run,
	EM_legl_walk,
	EM_legs_idle,
	EM_legs_okiru,
	EM_legs_dam_out,
	EM_armr_morph_arm,
	EM_legr_morph_legr,
	EM_legl_morph_legl,
	EM_legs_morph_arm,
	EM_legs_morph_leg,
	EM_dam_arm_r,
	EM_dam_leg_r,
	EM_armr_idle,
	EM_armr_run,
	EM_armr_walk,
	EM_legr_idle,
	EM_legr_run,
	EM_legr_walk,
	EM_arml_nom_idle_holdup,
	EM_armr_nom_idle_holdup,
	EM_arms_nom_idle_holdup,

	/* 死体モーション */
	EM_dead_carry_body_end,
	EM_dead_carry_body_idle,
	EM_dead_carry_body_start_b,
	EM_dead_carry_body_start_f,
	EM_dead_carry_body_walk,
	EM_dead_idle_f,
	EM_dead_carry_leg_end_b,
	EM_dead_carry_leg_end_f,
	EM_dead_carry_leg_idle_b,
	EM_dead_carry_leg_idle_f,
	EM_dead_carry_leg_start_b,
	EM_dead_carry_leg_start_f,
	EM_dead_carry_leg_walk_b,
	EM_dead_carry_leg_walk_f,

	EM_dead_carry_body_put_locker,
	EM_dead_carry_put_toilet,
	EM_dead_carry_locker_idle,
	EM_dead_carry_locker_surprise,
	EM_dead_carry_leg_put_locker_f,
	EM_dead_carry_leg_put_locker_b,

	EM_toilet_knock,	/* トイレノック */
	EM_toilet_peep_u,	/* トイレ上覗き込み */
	EM_toilet_peep_d,	/* トイレ下覗き込み */

	// おしっこ -DDEL_SHOUBEN=DUMMY
	EM_toilet_ready,
	EM_toilet_putback,
	EM_toilet_idle,

	/* ウォークマン -D DEL_WALKMAN */
	EM_nori_nori,
	EM_norinori_sleep,	/* ウォークマン聞きながら眠る */
	EM_norinori_idle,
	EM_norinori_walk,

	/* 盾兵モーション -D DEL_SHIELD */
	EM_shl_nom_dam_gun,
	EM_shl_nom_idle,
	EM_shl_nom_squat_fire,
	EM_shl_nom_walk,
	EM_shl_nom_run,
	EM_shl_nom_override,
	EM_shl_nom_defence_idle,
	EM_shl_nom_defence_squat,
	EM_shl_nom_attack_near,		/* キック攻撃 */
	EM_shl_nom_fire_side_p,
	EM_shl_nom_signe_go,
	EM_shl_nom_attack_near_shl,	/* 盾で攻撃 */
	EM_shl_nom_stomp,			/* 盾兵踏み付け */
	EM_shl_nom_mkr_reload,

	EM_shl_cle_walk,			/* 警戒歩き */
	EM_shl_cle_run,				/* 警戒走り */
	EM_shl_nom_cle_slidewalk_l,	/* 左横歩き */
	EM_shl_nom_cle_slidewalk_r,	/* 右横歩き */
	EM_shl_nom_walk_b_atk,		/* 後ろ歩き */
	EM_shl_nom_cle_look_l,		/* 左覗き込み */
	EM_shl_nom_cle_look_r,		/* 右覗き込み */
	EM_shl_cle_signe_idle,		/* リーダー待機モーション */
	EM_shl_nom_cle_seach_r2l,	/* 下覗き込み */
	EM_shl_nom_stair_run_down,	/* 階段走り降り */

	/*ショットガン -D DEL_SHOTGUN */
	EM_gbs_sps_nom_fire,
	EM_gbs_sps_nom_fire_p,
	EM_gbs_sps_nom_fire_reload ,
	EM_gbs_sps_nom_fire2,

	/*攻撃兵基本 覗き撃ち*/
	EM_gbs_ak_nom_peek_r_start,
	EM_gbs_ak_nom_peek_r_end,
	EM_gbs_ak_nom_peek_r_squat_end,
	EM_gbs_ak_nom_peek_r_squat_start,
	EM_gbs_ak_nom_peek_r_squat_fire,
	EM_gbs_ak_nom_peek_r_fire,
	EM_nom_fire_blind,
	EM_nom_fire_blind_squat,
	EM_gbs_ak_nom_peek_l_start,
	EM_gbs_ak_nom_peek_l_end,
	EM_gbs_ak_nom_peek_l_fire,

/* 削除モーション 後で削除 */
	EM_ottotto,		/* ステージ固有に */
	EM_poripori,	/* ステージ固有に */
	EM_break_seasick,		/* 船酔いステージ固有に */
	EM_ak_nom_guruguru,	/* 肩凝りほぐし */
	/* ステージ固有・keep */
	EM_watch_pose_1,
	EM_watch_pose_2,
	/* ステージ固有・OneTime */
	EM_watch_OneTime_1,
	EM_watch_OneTime_2,
	EM_watch_OneTime_3,

	/*天狗兵 p90装備*/
	EM_tng_p90_nom_fire, 			//EM_kamae_gun_high
	EM_tng_p90_nom_idle,			//EM_stand
	EM_tng_p90_nom_jump_down,
	EM_tng_p90_nom_jump_end,
	EM_tng_p90_nom_jump_peak,
	EM_tng_p90_nom_jump_start,
	EM_tng_p90_nom_jump_up,
	EM_tng_p90_nom_run,   			//EM_run_atk 戦闘中用
	EM_tng_p90_nom_squat_fire,
	EM_tng_p90_nom_walk,   			//EM_walk
	EM_tng_p90_nom_attack_near_spinkick_high,
	EM_tng_p90_nom_attack_near_shoutei,
	EM_tng_p90_nom_attack_near_spiraluper,
	EM_tng_p90_nom_attack_near_kneekick,
	EM_tng_p90_nom_attack_near_spinkick_low,
	EM_tng_p90_nom_attack_near_elbow,
	EM_tng_p90_nom_attack_near_somersault,
	EM_tng_p90_cle_walk,			//EM_cle_walk
	EM_tng_p90_nom_walk_b_atk,
	EM_tng_p90_nom_peek_r_start,
	EM_tng_p90_nom_peek_r_end,
	EM_tng_p90_nom_peek_l_start,
	EM_tng_p90_nom_peek_l_end,
	EM_tng_p90_nom_sliding,
	EM_tng_p90_nom_somersault,
	EM_tng_p90_nom_roll_l,
	EM_tng_p90_nom_slide_fire_r,
	EM_tng_p90_nom_slide_fire_l,
	EM_tng_p90_nom_somersault2,
	EM_tng_p90_nom_peek_r_fire,
	EM_tng_p90_nom_peek_l_fire,
	EM_tng_p90_nom_holdup,
	EM_tng_p90_nom_holdup_counterattack,
	EM_tng_p90_nom_stamp,
	EM_tng_p90_nom_holdup_counterattack2,
	EM_tng_p90_nom_fire_reload,
	EM_tng_p90_nom_squat_fire_reload ,
	EM_tng_p90_nom_leap ,

	EM_tng_tbl_nom_attack2,
	EM_tng_tbl_nom_attack1,
	EM_tng_tbl_nom_attack3,
	EM_tng_tbl_nom_idle,
	EM_tng_tbl_nom_walk_f,
	EM_tng_tbl_nom_walk_b,
	EM_tng_tbl_nom_walk_l,
	EM_tng_tbl_nom_walk_r,
	EM_tng_tbl_nom_run,
	EM_tng_tbl_guard_1,
	EM_tng_tbl_guard_2,
	EM_tng_tbl_guard_3,
	EM_tng_tbl_nom_leap_attack,
	EM_gbs_ak_nom_fire_peep_w25, //窓から乗り出し 常駐で使うかも
	EM_sna_non_hang_idle,	/*首絞め 仮スネーク用*/
	EM_sna_non_hang_miss,	/*首絞め 仮スネーク用*/
	EM_sna_non_hang_tie,	/*首絞め 仮スネーク用*/
	EM_gbs_shl_nom_gm_fire_high,	/*盾GRD*/

	/*　ハイテク兵　新規モーション*/
	EM_htc_m4_nom_attack_near,			//EM_ak_attack_near_kick_r近接攻撃
	EM_htc_m4_nom_squat_fire,			//EM_squat_fire		しゃがみ撃ち
	EM_htc_m4_nom_fire_reload,			//EM_reload			立ちリロード
	EM_htc_m4_nom_squat_fire_reload,	//EM_squat_fire_reloadしゃがみリロード
	EM_htc_m4_nom_walk_b_atk,			//EM_walk_b_atk		戦闘歩き　後
	EM_htc_m4_nom_walk_atk,				//EM_cle_walk		戦闘歩き　前
	EM_htc_m4_nom_slide_fire_r,			//EM_slide_r_fire	戦闘歩き　右
	EM_htc_m4_nom_slide_fire_l,			//EM_slide_l_fire	戦闘歩き　左
	EM_htc_m4_nom_run_atk,				//EM_run_atk	戦闘走り
	EM_htc_m4_nom_idle,					//EM_stand		基本静止姿勢
	EM_htc_m4_nom_fire_p_high,			//EM_kamae_gun_high	上段撃ち
	EM_htc_m4_nom_fire_grenade,			//NEW!! グレネードランチャー発射
	EM_gbs_shl_nom_fire_intrude,		//盾兵イントルード攻撃
	EM_gbs_ak_nom_idle_lower,			//壁めりこみ防止用 銃おろし姿勢
	EM_tng_p90_cle_run,					//天狗クリアリング走り
	/*　ハイテク兵用　調整モーション*/
	EM_htc_ak_cle_signe_go,			//EM_cle_signe_go
	EM_htc_ak_nom_cle_look_l,		//EM_cle_look_l
	EM_htc_ak_nom_cle_look_r,		//EM_cle_look_r
	EM_htc_ak_cle_signe_idle,		//EM_cle_stand02
	EM_htc_ak_nom_walk,				//EM_walk
	EM_htc_ak_nom_run,				//EM_run
	EM_htc_ak_nom_fire_intrude,		//EM_squat_fire_intrude
	EM_htc_ak_nom_snipe_fire_p,		//EM_snipe_fire_p
	EM_htc_ak_nom_walk_light,		//EM_walk_light
	EM_htc_ak_nom_zzz	,			//EM_zzz
	EM_htc_ak_nom_peek_l_fire,		//EM_gbs_ak_nom_peek_l_fire
	EM_htc_ak_nom_peek_l_end	,	//EM_gbs_ak_nom_peek_l_end
	EM_htc_ak_nom_peek_l_start	,	//EM_gbs_ak_nom_peek_l_start
	EM_htc_ak_nom_peek_r_fire	,	//EM_gbs_ak_nom_peek_r_fire
	EM_htc_ak_nom_peek_r_end	,	//EM_gbs_ak_nom_peek_r_end
	EM_htc_ak_nom_peek_r_start	,	//EM_gbs_ak_nom_peek_r_start
	EM_htc_ak_nom_dam_stun_light,	//EM_dam_stun_far
	EM_htc_ak_nom_okiru_f_slow	,	//EM_okiru_aomuke_slow
	EM_htc_ak_nom_cle_seach_r2l	,	//EM_cle_look_d
	EM_htc_ak_nom_stair_run_dowm,	//EM_stair_run_down
	EM_gbs_shl_nom_find_ply,
	EM_gbs_ak_nom_peek_l_end_w25,
	EM_gbs_ak_nom_peek_l_fire_w25,
	EM_gbs_ak_nom_peek_l_start_w25,
	EM_STANDARD_MAX
} ;


/*
	base 以外のモーションは
	ENE_SetActionPBreak( act, n_layer, data, m_time, mask, interp, over )
	ではなく
	ENE_SetMarActionPBreak( act, n_layer, data, m_time, mask, interp, over, mar )
	を、
	
	SetMarAction( act, n_layer, data, m_time, mask, interp )
	ではなく
	SetMarAction( act, n_layer, data, m_time, mask, interp, mar )
	を、使用すること
*/

/* 削除モーション 後で削除 */
enum {
//	EM_mukade_run1,			/* ムカデ走り１ */
//	EM_mukade_run4,			/* ムカデ走り４ */
//	EM_mukade_run2,			/* ムカデ走り２ */
//	EM_mukade_run3,			/* ムカデ走り３ */

	EM_near_shoulder,		/* 近距離攻撃、ショルダー */
	EM_near_punch,			/* 近距離攻撃 パンチ */
} ;


/* 立ち位置情報付きモーション */
enum {
	MOT_RINTRPT_LOCKER_OPEN ,	/* ロッカーオープンモーション */
} ;



/* 左右反転モーション */
/*
enum {
	EM_dam_arm_r = ENE_MOT_FLAG_REVERSAL_U + EM_rev_dam_arm_l,
	EM_dam_leg_r = ENE_MOT_FLAG_REVERSAL_D + EM_rev_dam_leg_l,
	EM_armr_idle = ENE_MOT_FLAG_REVERSAL_U + EM_rev_arml_idle,
	EM_armr_run = ENE_MOT_FLAG_REVERSAL_U + EM_rev_arml_run,
	EM_armr_walk = ENE_MOT_FLAG_REVERSAL_U + EM_rev_arml_walk,
	EM_legr_idle = ENE_MOT_FLAG_REVERSAL_D + EM_rev_legl_idle,
	EM_legr_run = ENE_MOT_FLAG_REVERSAL_D + EM_rev_legl_run,
	EM_legr_walk = ENE_MOT_FLAG_REVERSAL_D + EM_rev_legl_walk,
} ;
*/
/*----- 構造体 --------------------------------------------------*/
/** 重野 追加 /user/sigeno**/
/** ひとつのゾーンに対して確保する安全地帯数  **/
#define	SAFE_NUM	(HZX_MAX_SAFEZONE_NUM)
//#define	ATUNIT_MAX	2 /** 攻撃隊最大数 **/
#define	ATUNIT_MAX	1 /** 攻撃隊最大数 **/
//#define	ATENEMY_MAX	8 /** 攻撃隊 １チーム人数 **/
#define	ATENEMY_MAX	16 /** 攻撃隊 １チーム人数 **/

#define GOSIGN_TIME	COUNT_VMODE(240) /** 号令有効時間 **/

#define	AT_COV_MAX	(2)	/*遮蔽物最大数*/
/**安地の属性フラグ**/
/** type に格納 **/
#define	SAFE_LOW		(0x0001) /** 低い遮蔽物 **/
	/*張り付かないとゾーン内に入れない場合*/
#define	SAFE_BEHIND1	(0x0002) /*壁1*/
#define	SAFE_BEHIND2	(0x0004) /*壁2*/
#define	SAFE_BEHIND3	(0x0008) /*壁3*/
#define	SAFE_BEHIND4	(0x0010) /*壁4*/
/*どの点が死角か*/
/*注意 SAFE_LOWゾーンの場合 中心のみチェックしている*/
#define	SAFE_POINT1		(0x0020) /*-x -z点*/
#define	SAFE_POINT2		(0x0040) /*+x -z点*/
#define	SAFE_POINT3		(0x0080) /*-x +z点*/
#define	SAFE_POINT4		(0x0100) /*+x +z点*/
#define	SAFE_CENTER		(0x0200) /*ゾーン中心*/


#define BEHIND_FLGS (SAFE_BEHIND1|SAFE_BEHIND2|SAFE_BEHIND3|SAFE_BEHIND4)
/*安地ゾーン距離の上限*/
#define	SAFE_MAX_DIS	15000


/** 張り付き時のアタリ**/
#define ENE_BEHIND_SPHERE (100)
/*仮設定 張り付き時間*/
#define ENE_BEHIND_TIME COUNT_VMODE(120)

/*攻撃兵装備*/
enum {
	AT_EQUIP_NORMAL,
	AT_EQUIP_SHIELD,
	AT_EQUIP_LIGHT_SHIELD, /*照明付き盾*/
	AT_EQUIP_SHOTGUN,
	AT_EQUIP_HITECH_1,
	AT_EQUIP_ABAKAN
};


/*対称モード用変数群*/
typedef	struct	{
	FVECTOR		cov_box[2] ;/*バウンディング*/
	FVECTOR		cov_cent_pos ;
	int			cov_map ;
}AT_COV ;

	/* ナビゲーション */
typedef	struct	{
	FVECTOR		flore_pos ;			/* 床設置場所		*/
	int			this_addr ;			/* 現在のアドレス	*/
	int			next_addr ;			/* 一時目標アドレス	*/
	int			next_map ;			/* 一時目標マップ */
	int			going_addr ;		/* 最終目標アドレス	*/
	int			going_map ;			/* 最終目標マップ */
	FVECTOR		next_zonepos ;		/* 一時目標地点		*/
} ZONENAVI ;

#include "route.h"

	/* 目標データ */
typedef	struct	{
	FVECTOR		pos ;	/* 場所 */
	short		dir ;	/* 方向 */
	short		tmp_dirbuff ;	/* 方向バッファ */
	int			h_dis ;	/* 平面上での距離 */
	int			addr ;	/* ゾーンアドレス */
	int			map ;	/* マップ */
} TRGPOINT ;

/* trgflag */
#define EYEI_TRGFLAG_INDISTINCT		/* はっきりと見えない */

typedef	struct{
	FVECTOR	*pos ;			/* ターゲットの位置 */
	int		*addr ;			/* ターゲットのアドレス */
	int		*status ;		/* ターゲットのステイタス */
	int		*map ;			/* ターゲットのマップ */
	int		trgflag ;		/* ターゲットのフラグ */
	int		dis ;			/* 距離 */
	short	dir ;			/* 方向 */
	short	sight ;			/* ターゲットが見えている状態 */
	short	flag ;			/* 各種フラグ */
	short	sight_real ;	/* 補正前の結果 */
	int		sight_dis ;		/* 現在の視認 */
	int		blurr_dis ;		/* 現在の朧下に見える距離 */
} EYEINFO ;

	/* 感覚パラメータ */
typedef	struct{
	RADAR_CTRL	rctrl ;

	short	facedir ;	/* 顔の方向 */
	short	facedir_x ;	/* 顔の上下方向 */
	short 	eye_r ; 	/* 顔の向きを中心とした片側視野角度 */
	u_short 	glasses_dis ; 	/* 双眼鏡装着時の＋視力*/
	int 	eye_s ;		/* 視力 */
	int		hearing ;	/* 聴力 */
	int		smell ;		/* 嗅覚 */
	int		status ;	/* ステータス（レーダーの色） */

	int 	eye_s_s[4] ;		/* 視力 */
} SENSEPARAM ;

	/* 回避モード情報 */
typedef	struct{
	int	check_num ;	/* 確認ゾーン数 */
	short check_count ;
	short check_zone[ SAFE_NUM ] ;
	int	check_map ;
	u_short	root_chg_message ;	/* ルートチェンジメッセージフラグ */
	u_short	mess_root ;			/* 回避に利用するルート番号 */
	u_short	mess_point ;		/* 回避に利用するポイント番号 */
	u_short	padddddddddd ;		/*  */
} AVOIDINFO ;

	/* プロック */
typedef	struct{
	int	proc ;
	int	argc ;
	int	argv[ GCL_MAX_ARGS/2 ] ;	/* 節約 */
} ENE_ARGS ;

	/* 敵兵思考 */
typedef	ALIGN16_DECL(struct)	_enethink__{
	void		*w ;
	CONTROL		*ctrl ;
	ROUTENAVI	*rnavi ;
	ZONENAVI	*znavi ;
	ACTION		*act ;
	FMATRIX		*lights ;
	TRGPOINT	trgpoint ;
	TRGPOINT	trgpoint_cover ;
	ROUTENAVI	*rnavi2 ;
	HOMING_TRG	hom ;
	void		*shadow ;

	NAME_ID		name_id ;
	SWITCH		sw ;
	short		g_id ;		/* 所属グループ */
	short		u_id ;		/* 所属ユニット */
	short		id ;		/* 番号 */
	short		se_tableID ;	/* ＳＥ変換テーブル番号 */
	int			uniq_id ;	/* ユニークＩＤ */
	int			old_uniq_id ;	/* 前世のユニークＩＤ */
	int			before_inzone[4] ;	/* 前にいたゾーン ４つまで */
	OBJECT		*weapon ;	/* 武器 */
	OBJECT		*sub_weapon ;	/* 予備武器 */
	OBJECT		*sling ;	/* スリング */
	void		*slinggun ;	/* スリング銃用 */
	DG_OBJS		*lowshadow ;	/* 影用ローポリモデル */
	void		*d_name ;	/*ドッグタグ名前 */
	int			alive_name ;	/* 死体なら生前の名前*/

	EYEINFO		pl_eyei ;	/* プレイヤー */
	EYEINFO		npc_eyei ;	/* サブNPCキャラ */

	int			bd_uniq_id ;	/* 相棒ユニークID */
	struct	_enethink__	*spbudy ;	/* 相棒 */
	struct	_enethink__	*buddy ;	/* 相棒 */
	EYEINFO		bd_eyei ;	/* 相棒視覚情報 */
	FVECTOR		last_pos ;		/* 最後に見えた場所 */
	int			last_map ;	/* 最後のマップ */

	SENSEPARAM	sense ;
	int			notice ;	/* ？モードフラグ */
	int			c_notice ;	/* 現在のモード */
	int			mess_notice ;	/* 前のモードからのメッセージ */
	short		alert ;		/* 危険値 */
	short		avoid ;		/* 回避値 */
	short		search ;	/* 探索値 */
	u_short 	ef_id ;		/* 跡ID */
	u_short 	efl_id ;	/* 跡リストID */
	u_short		found_ef_id ;	/* 不審物ID */
	short 		search_route ;	/* 探索モードのルート */
	short 		region_count ;	/* 部位発見カウンタ */
	short		voice_chara ;	/* 声優さん */
	short		notice_retry ;	/* 連続カウント */
	short		seethrough_time ;	/* 見破れる時間 （未使用） */
	short		waiting_pos_num ;
	u_short		now_found_ef_id ;	/* 現在の不審物ID */
	short		checkzone ;			/* 現在チェックしているゾーン */

	int 		corp_id ;				/* 発見死体ＩＤ */
	int 		corp_alive_name ;		/* 発見死体生前の名前 */
	int 		accident_id ;	/* 異常事態ＩＤ */
	int 		enedam_id ;		/* 仲間ダメージＩＤ */
	int			bullet ;	/* マガジン内の弾使用量 */
	int			max_bullet ;	/* マガジンの最大装填数 */
	FVECTOR		def_pos ;	/* 守備位置 */
	int			def_mapbit ;	/* 守備位置のマップビット */
	AVOIDINFO	avoinfo ;
	FVECTOR		last_radio_pos ;	/* 最後に連絡した場所 */
	int			last_radio_map ;

	struct	_enethink__	*dam_entk ;	/* ダメージを受けている敵兵 */
	FVECTOR		tmp_pos ;		/* 汎用ＰＯＳ */
	int			tmp_buff[2] ;	/* 汎用バッファ(think内 で使用) */
	int			tmp_buff2[2] ;	/* 汎用バッファ(thinkでは書き換え不可) */
	void		( *tmp_proc[2] )(struct _enethink__ * ) ;	/* 汎用関数ポインタ(think内 で使用) */
#if 0
	char		think1 ;
	char		think2 ;
	char		think3 ;
	char		next_think_buff[ 5 ] ;
#else
	int			think1 ;
	int			think2 ;
	int			think3 ;
#endif
	int			count1 ;
	int			count3 ;
	int			tmp_time ;		/* 汎用タイム */
	int			tmp_count ;		/* 汎用カウンタ */
	int			thk_status ;	/* 思考ステータス（通信用） */
	int			iknow_flag ;	/* 既知フラグ */
	int			receive ;	/* 通信フラグ */

	int			status ;
	int			status_status ;	/* status足らなくなったのでもう一つ */
	int			status2 ;

	int			scene ;		/* 各コマンダーとの状況連絡用 */
	ENE_ARGS	death_proc ;	/* エンドプロック */

	int			sw_shadow ;	/* 影のＯＮＯＦＦスイッチ */
	int			sw_light ;	/* ライトのＯＮＯＦＦスイッチ */
	int			sw_gun ;	/* 銃スイッチ */

    /* 頂点アニメ */
    int			vanime_flag ;
    HUMANMA_WORK	*vanime_head ;

	/* ゾーンインタラプト */
	Z_INTRPT	z_intrpt ;

	struct	_commander 	*com ;

	/* 固有構造体 */
	void		*character ;
	/** 重野 追加 sigeno**/
	/*イベント兵専用*/
	void	*eve_a ; /*特種イベント兵独自メンバ*/ 
	struct	_at_com 	*at_com ; /** 攻撃兵コマンダ情報**/
	void	*arm_ik ; /* ムカデ時ＩＫワーク */
	void	*attachment	; /*装備品群*/
	int		lockon ;	/*VR ロックオンカーソル管理用*/
/*以下デバッグ用*/
	int		head_num ;

} ENETHINK /* BP  */;





typedef	void	( *ENETHINKPROC )( ENETHINK * ) ;

/*----- コマンダー --------------------------------------------------*/
#if 1
	#define	MAX_ENEMY		64
	#define	MAX_UNIT		4
	#define	MAX_GROUP		4
//	#define	MAX_SECURITY	64
#else
	#define	MAX_UNIT	8
	#define	MAX_ENEMY	128
	#define	MAX_GROUP	4
	#define	MAX_SECURITY	64
#endif

#define	MAX_ALERT_LEVEL	ALERT_LEVEL_MAX
#define	MAX_AVOID_LEVEL	AVOID_LEVEL_MAX


/*----- コマンダーステイタス --------------------------------------------------*/
#define	CMST_ENEMY_SIGHT_OFF		0x00000001
#define	CMST_ENEMY_ACCIDENT			0x00000002
#define	CMST_ENEMY_GAME_OVER		0x00000004
#define	CMST_ENEMY_ENDLES_ALERT		0x00000008
#define	CMST_ENEMY_ENDLES_SEARCH	0x00000010
#define	CMST_ENEMY_DESTROY_ALERT	0x00000020
#define	CMST_ENEMY_TMP_LIGHT1		0x10000000
#define	CMST_ENEMY_TMP_LIGHT2		0x20000000
#define	CMST_ENEMY_GUNLIGHT_OFF		0x40000000
#define	CMST_ENEMY_BODY_LIGHT		0x80000000
#define	CMST_ENEMY_IK_STOP			0x01000000
#define	CMST_ENEMY_GUN_LINE			0x02000000
#define	CMST_ENEMY_GUN_CUT			0x04000000
#define	CMST_ENEMY_SIGHT_VIEW		0x08000000
#define	CMST_ENEMY_HEARING_OFF		0x00100000
#define	CMST_ENEMY_VR_CLEAR			0x00200000
#define	CMST_ENEMY_VR_GAMEOVER		0x00400000

/*----- コマンダー1フレームフラグ --------------------------------------------------*/
#define CMFLAG_DETECT			0x00000001	/* 発見～危険まで */
#define CMFLAG_BIG_SNORE		0x00000002	/* でっかいイビキが出てる */

/*-----  --------------------------------------------------*/
#define MAX_PLAYER_POS	3
#define MAX_RES_POS	8
#define MAX_WAIT_POS 8
#define PL_POS_CENTER	0
#define PL_POS_HEAD		1
#define PL_POS_FOOT		2

typedef	struct	{
	int			unit_id ;
	int			enemy_num ;
	int			die_num ;
	int			u_buff[8] ;
	ENETHINK	*entk[ MAX_ENEMY ] ;
} E_UNIT ;
typedef	struct	{
	int			group_id ;
	int			unit_num ;
	E_UNIT		*unit[ MAX_UNIT ] ;
} E_GROUP ;
typedef	struct	{
	int			group_num ;
	E_GROUP		*group[ MAX_GROUP ] ;
} ENEMYS ;

/*
typedef	struct	{
	int			secur_num ;
	SECURITY	*secur[ MAX_SECURITY ] ;
} SECURITIES ;
*/

#define MAX_CLE_PROC	32
#define MAX_ACCIDENT	8
#define MAX_ROUTE_PROC	16
#define MAX_CLE_AREA	(32)
#define MAX_COM_PROC	(8)

enum {
	ComProcsCyperAllDestroy,
} ;


typedef	struct	{
	short	area ;
	short	route ;
	short	point ;
	short	padddd ;
	int		proc ;
} CLEARING_PROC ;

typedef	struct	{
	short	route ;
	short	point ;
	int		proc ;
} ROUTE_PROC ;

typedef	ALIGN16_DECL(struct) _commander{
	ENEMYS		enemys ;
	HZX_HDL		*hzx ;

	int			name ;

	int			root_ofset ;
	int			alertlevel ;	/* 危険値 */
	int			avoidlevel ;	/* 回避値 */
	int			searchlevel ;	/* 探索値 */
	int			maxalert ;		/* マックス危険値 */
	int			maxavoid ;		/* マックス回避値 */
	int			maxsearch ;		/* マックス探索値 */
	int			security_alert ;/* セキュリティーシステム危険値 */
	int			time ;			/* */
	int			uniq_id ;
	short		wakeup_count ;	/* 起こしカウント */
	short		noise_alert_timer ;
	short		noise_time ;		/* 危険モード連続爆発でプレイヤー発見*/
	short		noise_padddddddddd ;
	int			alertstatus ;	/* 危険モード時のステータス */
	int			avoidstatus ;	/* 回避モード時のステータス */
	int			com_procs[ MAX_COM_PROC ] ;

	short		noise ;			/* 物音種類 */
	short		noise_padddd ;
	u_int		noise_dis ;		/* 物音までの距離 */
	char		noise_near_id[4] ;		/* 物音に一番近い兵の番号0:gp 1:un 2:id */
	ENETHINK	*dam_entk[MAX_DAM_ENE] ;	/* ダメージを受けた敵 */
	short		dam_en_num ;		/* ダメージ受けている敵の数 */
	short		live_num ;			/* 生存人数 */
	short		enemy_count ;		/* 出現人数 */
	short		enemy_appear_max ;	/* 最大出現人数 */
	short		res_count ;			/* 復活人数 */
	short		max_res_num ;		/* 最大復活人数 */
	short		res_count_in_alert ;/* 危険モード復活人数 */
	short		max_res_in_alert ;	/* 危険モード最大復活人数 */

	short		cyp_res_count ;			/* サイファー復活数 */
	short		cyp_max_res_num ;		/* サイファー最大復活数 */
	short		cyp_res_count_in_alert ;/* サイファー危険モード復活数 */
	short		cyp_max_res_in_alert ;	/* サイファー危険モード最大復活数 */

	short		cyp_kill_count ;		/* サイファー破壊数 */
	short		cyp_num ;				/* サイファー数 */


	ROUTE_PROC	route_proc[ MAX_ROUTE_PROC ] ;

	int			accident_bit ;
	int			accident_uniq_id[ MAX_ACCIDENT ] ;
	short		accident_delay[ MAX_ACCIDENT ]  ;
	FVECTOR		accident_pos[ MAX_ACCIDENT ]  ;
	int			accident_map[ MAX_ACCIDENT ]  ;
	int			accident_status[ MAX_ACCIDENT ]  ;
	int			free_accident_bit ;	/* アクシデント解放予約 */

	int			investigated ;			/* 調査 */
	FVECTOR		pos_investigated ;		/* 調査場所 */
	int			groupid_investigated ;	/* 調査グループＩＤ */
	int			uniqid_near_investigated ;	/* 調査場所に一番近い敵兵 */
	u_int		dis_investigated ;			/* 調査場所に一番近い敵兵までの距離 */

	int			res_num ;
	FVECTOR		res_pos[ MAX_RES_POS ] ;	/*復活場所 */
	int			res_map[ MAX_RES_POS ] ;
	int			waiting_num ;
	int			waiting_cur ;
	FVECTOR		waiting_pos[ MAX_WAIT_POS ] ;		/*待機場所 */
	int			wait_map[ MAX_WAIT_POS ] ;

	int			status ;		/* デバッグモード */
	int			stage_kind ;		/* ステージ種類 */
	FVECTOR		player_lastpos ;	/* 回避モードになった時点のプレイヤーの位置 */
	int			player_lastmap ;	/* 回避モードになった時点のプレイヤーの位置 */
	int			flame_flag ;			/* 毎フレームクリアのフラグ */
	CONTROL		*npc_ctrl ;			/* NPC （エマ）コントロール */

	int			pl_pos_num ;
	int			pl_in_zone[ MAX_PLAYER_POS ] ;		/* ゾーン内かどうかのチェック */
	int			plzone_in_zone[ MAX_PLAYER_POS ] ;	/* 最後のゾーン */
	FVECTOR		plpos_in_zone[ MAX_PLAYER_POS ] ;		/* 最後のゾーン内のプレイヤー位置 */
	int			plmap_in_zone[ MAX_PLAYER_POS ] ;		/* 最後のゾーンでのマップ */
	FVECTOR		plpos_box ;		/* 最後見た段ボール位置 */

	int			atk_goalzone ;

	int			cle_area_status[ MAX_CLE_AREA ] ;
	int			cle_proc_num ;
	CLEARING_PROC	cle_proc[ MAX_CLE_PROC ] ;
	int			tmp_cle_proc_num ;
	CLEARING_PROC	tmp_cle_proc[ MAX_CLE_PROC ] ;
	short		cle_route[ ATENEMY_MAX ] ;		/* クリアリングルート */
	short		cle_point[ ATENEMY_MAX ] ;		/* クリアリングポイント */
//	struct	_at_com 	*at_com ; /** 攻撃兵コマンダ情報**/
} COMMANDER  ;
/*----- alertstatus --------------------------------------------------*/
#define COM_ALERT_PLAYER_DETECT		0x00000001	/* プレイヤーを見つけている */
#define COM_ALERT_NPC_DETECT		0x00000002	/* NPC(エマ)を見つけている */
#define COM_ALERT_ATK_ATTACK		0x00000004	/* 攻撃兵、プレイヤーに攻撃開始 */
#define COM_ALERT_ATK_NPC			0x00000008	/* NPC担当兵、NPCに攻撃開始 */
#define COM_ALERT_ATK_TOTAL_DEFEAT	0x00000010	/* 攻撃兵全滅 */
#define COM_ALERT_WTC_TOTAL_DEFEAT	0x00000020	/* 警備兵全滅 */
#define COM_ALERT_SUP_TOTAL_DEFEAT	0x00000040	/* サポート兵全滅 */

/*----- avoidstatus --------------------------------------------------*/
#define COM_AVOID_CLEARING			0x00000001	/* クリアリング */

/*----- ENE_GameStatus --------------------------------------------------*/
#define ENE_GMSTATUS_PLAYER_FOUND	0x00000001	/* プレイヤーは一度発見されている */
#define ENE_GMSTATUS_TOILET			0x00000002	/* ロッカーではなくトイレステージ */
#define ENE_GMSTATUS_NPC_EMMA		0x00000004	/* ＮＰＣエマが出るステージ */
#define ENE_GMSTATUS_NPC_SNK		0x00000008	/* ＮＰＣスネークが出るステージ */
#define ENE_GMSTATUS_NO_STRM		0x00000020	/* 音声無しステージ  */
#define ENE_GMSTATUS_CALL_BIKKURI	0x00000040	/* 発見びっくり音なった  */

/*----- stage_kind --------------------------------------------------*/
#define ENE_STAGE_TOILET			0x00000001	/* ロッカーではなくトイレステージ */
#define ENE_STAGE_NPC_EMMA			0x00000002	/* NPCエマ出現ステージ */
#define ENE_STAGE_NPC_SNAKE			0x00000004	/* NPCスネークステージ */
#define ENE_STAGE_GPS				0x00000010	/* 都市迷彩ゴル兵ステージ */
#define ENE_STAGE_NO_STRM			0x00000020	/* 音声無しステージ */
#define ENE_STAGE_ALERT2SNEAK		0x00000040	/* 危険モード後、潜入モード */
#define ENE_STAGE_SEARCH_MAX		0x00000080	/* 警戒モード、ＭＡＸスタート */
#define ENE_STAGE_GOVER_STOPSTR		0x00000100	/* ゲームオーバーフラグが立つと台詞カット */
#define ENE_STAGE_NO_ACCIDENT		0x00000200	/* アクシデント無し */
#define ENE_STAGE_NO_COMMANDER		0x00000400	/* コマンダー無し */
#define ENE_STAGE_NO_WC_GOVER		0x00000800	/* 発見即ゲームオーバーフラグ無視 */
#define ENE_STAGE_CROWD_PEOPLE		0x00001000	/* 大人数ステージ */
#define ENE_STAGE_VR_ANOTHER		0x00002000	/* VRステージ */
#define ENE_STAGE_NO_NOTICE			0x00004000	/* 敵反応無し 注！潜入モードのみ！！ */
#define ENE_STAGE_NO_APPER_EFFE		0x00008000	/* 出現エフェクトなし！！ */
#define ENE_STAGE_NO_RUN_SMOOTH		0x00010000	/* 特殊移動のとき立ち止まる（本編はこうだった）*/
#define ENE_STAGE_ROOT_NO_LOOP		0x00020000	/* ルートの終点まできたら終わり */
#define ENE_STAGE_FAINT_EXIT		0x00040000	/* 倒れたら退場 */
#define ENE_STAGE_VRBOM_BIGDAM		0x00080000	/* ボム系大ダメージ */

/*----- accidednt_status --------------------------------------------------*/
#define ENE_ACCIDENT_RADIO			0x00000001	/* 無線連絡が途切れた */
#define ENE_ACCIDENT_REPORT			0x00000002	/* 定時連絡兵に異常があった */
#define ENE_ACCIDENT_DIRECT			0x00000004	/* 直接調査依頼 */
#define ENE_ACCIDENT_POS			0x00000008	/* カメラ系が異常発見、場所指定 */
#define ENE_ACCIDENT_MECA_BREAK		0x00000010	/* カメラ系から故障 */
#define ENE_ACCIDENT_MECA_ENEDOWN	0x00000020	/* カメラ系が倒れている兵を発見 */
#define ENE_ACCIDENT_MECA_PLAYER	0x00000040	/* 変装プレイヤーのおかしな行動発見 */

/*----- クリアリング関係 分離予定 --------------------------------------------------*/
/* cle_area_status */
#define CLE_AREA_ST_SKIP		0x00000001	/* エリアスキップ */
#define CLE_AREA_ST_ENECHK_SKIP	0x00000002	/* 敵兵がいる場合のクリアリングスキップをスキップ*/


enum {
	CLE_TH1_NONE ,
	CLE_TH1_ALERT ,
	CLE_TH1_AVOID
} ;

enum {
	CLE_TH2_CLEARING ,
	CLE_TH2_SEARCH
} ;

enum {
	CLE_TH3_SCENE_MANAGE,
	CLE_TH3_ASSEMBLE,
	CLE_TH3_GOSIGN,
	CLE_TH3_DYNAMICENTRY,
	CLE_TH3_D_TO_S,
	CLE_TH3_2CLEARING,
	CLE_TH3_COMP,
	CLE_TH3_NOISE,
	CLE_TH3_DAMAGE,
	CLE_TH3_BLOOD,
	CLE_TH3_WITHDROW
} ;

enum {
	CLE_ENE_MOVE_START,
	CLE_ENE_WAIT_START,
	CLE_ENE_GOSIGN,
	CLE_ENE_WAIT_GOSIGN,
	CLE_ENE_DYNAMIC_ENTRY,
	CLE_ENE_WAIT_DENTRY,
	CLE_ENE_D_TO_S,
	CLE_ENE_WAIT_D2S,
	CLE_ENE_CLEARING,
	CLE_ENE_WAIT_CLEARING,
	CLE_ENE_COMPLETE,
	CLE_ENE_WAIT_COMPLETE,
} ;

#define	CLE_IKNOW_BLOOD		0x00000001	/* 血の跡は発見した */
#define	CLE_IKNOW_EXCEPT	0x00000002	/* クリアリング以外の調査 */
#define	CLE_IKNOW_CANCEL	0x00000004	/* クリアリングキャンセル */
#define	CLE_IKNOW_GO_NORMAL	0x00000008	/* ノーマル回避モードへ変更 */

#define CLE_NOTICE_SUPPORT (ENE_NOTICE_NOISE|ENE_NOTICE_BOX|ENE_NOTICE_TRACE|ENE_NOTICE_ENE_DAMAGE)
#define	CLE_NOISE_ALERT_NUM	(1)			/* CLE_NOISE_ALERT_NUM以上で発見 */

#define MAX_CLE_ENE	(16)
typedef	struct	{
	/* クリアリング関係 */
	E_UNIT		*unit ;
	HZX_HDL		*hzx ;

	short		cle_think1 ;
	short		cle_think2 ;
	short		cle_think3 ;
	short		cle_count3 ;
	short		clear_area ;	/* カレントクリアリングエリア */
	short		clear_gotime ;	/* 突入カウント */
	short		cle_area_inplayer ;	/* プレイヤーが入っているクリアリングエリア */
	short		scene ;		/* 全体の進行度 */

	int			c_notice ;	/* 現在のモード */
	int			ene_num ;	/* 各モード実行中の兵 */
	FVECTOR		trg_pos ;	/* 各モードでのターゲット */
	int			flam_flag ;
	int			scene_flag ;	/* 体験版後 scene を消す */
	int			iknow_flag ;	/* 既知フラグ */
	int			noise_num ;	/* 物音モード回数 */
} CLEARING ;






typedef	struct _at_com{
	FVECTOR	at_bound[2]; /*攻撃兵展開エリア*/
	FVECTOR	tmptrg ;	/* なにか発見したときなど */
	E_GROUP	group ;
	E_UNIT		unit[ ATUNIT_MAX ] ;
	CLEARING	clearing[ ATUNIT_MAX ] ;
	COMMANDER	*com ;
	int			*teamaddr[ATUNIT_MAX][ATENEMY_MAX];
	int			feel_flag ;	/*全体の気配レベル*/
	int			think1 ;
	int		pl_weapon;	/* プレイヤ武器状態*/
	u_int		watch_status;	/*監視状態*/
	int		berserk ;			/*突撃モード*/
	int		wait_cnt ;		/*出動までの待機時間*/
	int		avoid_wait_cnt ;
/*このあたり廃止予定*/
	u_char		shield_num; /*現在の盾人数*/
	u_char		shield_max; /*最大盾人数*/
	u_char		shield_start; /*追加時のやられ人数*/
	u_char		shield_rate; /*何人毎に追加するか*/

/*このあたりも不要変数あり*/
	u_char		siege[ ATUNIT_MAX ] ;
	u_char		level;		/* パターン変化の基準 */
	u_char		shoot_delay;	/*発砲ためらい時間*/
	u_char		attack_num;	/* 同時攻撃人数*/

	u_char		siege_num;	/* 包囲人数 */
	u_char		esctime;	/* 逃げタイミング*/
	u_char		com_sight;	/** 共有視界情報 **/
	u_char		pad;		/**/

	u_int		rollout;	/* 転がりタイミング*/

	u_char		watch_status_req;	/*statusリクエスト*/
	u_char		gosign;		/* 号令情報 */
	u_short		scn_status ;	/*シナリオステータス*/

	u_int		Pl_StayTime;	/*プレイヤの静止時間*/

//	u_short		minlen;		/* 最接近距離 */
	u_int		chasedis;	/* 追跡距離可変*/

	u_short		esc_dis;	/* 逃げ距離限界*/
	u_short		alert_time;	/* 危険モード経過時間 */

	int			OldPlAddr;	/*前フレームでのプレイヤアドレス待避*/
	int			player_lastaddr; /*回避モード時点のアドレス*/

	int			surprised;		/*ビックリ間隔*/
	long64		pl_status_old;	/*前フレームでのプレイヤ状態*/

	/*対称モード用変数群*/
	short		cov_num ;	/*領域数*/
	short		now_cov ;	/**/
	FVECTOR		cov_pos ;	/*目標地点*/
	FVECTOR		side_pos[2] ;
	int			cov_addr ;	/*目標ゾーン*/

	AT_COV		*at_cov ;

	/** 使用中ゾーン管理 **/
	int		usezones[ATUNIT_MAX][ATENEMY_MAX];
	CONTROL		*npc_ctrl ;

	int		bound_last ; /*攻撃中バウンド内の最後尾兵ID*/
	/*戦闘中音声管理*/
	int		called_se_code ;
	int		called_se_count ;
	int		called_se_pl_addr ;
	int		at_trg_addr; /*チームの目標とするaddr*/
#ifdef DEBUG_MODE

	/*以下デバッグ用*/
	int			video; /*ビデオ録り用 嘘モード*/
	int			debug_count; /*デバッグ表示用*/
//	int			fix_level;	/*レベル固定*/
#endif
} AT_COM ;

/*----- 死体管理 --------------------------------------------------*/

typedef struct _Corp{
	CONTROL	*ctrl ;		
	int		*zadd ;
	int		uniq_id ;
	int		id ;
	int		count ;
	int		flag ;
	ENETHINK *entk ;

	struct _Corp	*before ;
	struct _Corp	*next ;
} NEWCORP ;

typedef struct _OldCorp{
	FVECTOR	pos ;
	int		id ;
	int		status ;

	struct _OldCorp	*before ;
	struct _OldCorp	*next ;
} OLDCORP ;

#define	CORPS_ST_ACT_CTRL		0x00000001	/* 動かされている */
#define	CORPS_ST_ROTTEN_START	0x00000002	/* 腐りはじめている */
#define	CORPS_ST_WATCH			0x00000004	/* 発見された */
#define	CORPS_ST_ROTTEN			0x00000008	/* 腐った */
#if 1
#define	CORPS_ST_KILL			0x00000100	/* 強制終了 */
#define	CORPS_ST_CAPTURE		0x00000200	/* 捕まえられている */
#define	CORPS_ST_REPORT			0x00000400	/* 報告済み */
#else
#define	CORPS_ST_KILL			0x00000010	/* 強制終了 */
#define	CORPS_ST_CAPTURE		0x00000020	/* 捕まえられている */
#define	CORPS_ST_REPORT			0x00000040	/* 報告済み */
#endif

/*----- 死体フラグ  --------------------------------------------------*/
#define	CORPS_FLAG_HEADBLOOD	0x00000001	/* ハイテク部隊兵、顔血付き */

/*-----  --------------------------------------------------*/
/*-----  --------------------------------------------------*/

#include	"enemy.x"

#define	COMMANDER_PRIO	0x10
#define	SUBCOMM_PRIO	0x20
#define	ENEMY_PRIO		0x30
#define	AFTER_ENEMY_PRIO	0x40
#endif

