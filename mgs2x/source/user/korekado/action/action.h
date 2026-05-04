/*
	action.h
	アクションコントロールヘッダ

	1997/07/07 Y.Korekado
	$Id: action.h,v 1.1.1.3 2002/11/19 11:43:57 Yoshizawa1 Exp $
	
*/
#ifndef __ACTION___
#define	__ACTION___ 1
#include	"libutl.h"
#include	"../../skoba/weapon_old/matsu.h"
#include	"../thing/item.h"

#define MOTION_PLUGIN	(1)
/*----- 基本モーション番号 -----------------------------------*/
/* 移動方法 */
enum {
	MoveWalk,			/* 歩く */
	MoveWalkGun,		/* 銃を前に構え歩く */
	MoveSideL,			/* 左横移動 */
	MoveSideR,			/* 右横移動 */
	MoveBack,			/* バック */
	MoveCautionWalk,	/* 警戒歩き */
	MoveCautionWalkLow,	/* 警戒歩き低姿勢 */
	MoveRun,			/* 走る */
	MoveCautionRun,		/* 警戒走り */
	MoveAttackRun,		/* 攻撃兵走り */
	MoveNoriNoriWalk,	/* ウォークマン兵 */
	MoveStairRunD,		/* 階段走り降り */
	MoveStairRunU,		/* 階段走り上り */
	MoveStairWalkD,		/* 階段歩き降り */
	MoveStairWalkU,		/* 階段歩き上り */
} ;

/* ダウン方法 */
enum {
	DownBack,	/* うつぶせ */
	DownFront,	/* 仰向け */
	DownWall	/* 壁もたれ */
} ;

/*----- 定数定義 -----------------------------------*/
#define MAX_ACT_TIME	COUNT_VMODE(180)
#define MAX_VOL_TIME	COUNT_VMODE(600)

#define	MOTION_MAX	128
#define	MOTION_MASK_FULL	0xffffff
#define	MOTION_MASK_UPPER	0x1ffe
#define	MOTION_MASK_LOWER	0x1fe001

#define MOTION_MASK_ARM_R	0x78
#define MOTION_MASK_ARM_L	0x780
#define MOTION_MASK_ARMS	0x7f8
//#define MOTION_MASK_ARM_R	0x7e
//#define MOTION_MASK_ARM_L	0x786
//#define MOTION_MASK_ARMS	0x7fe

#define MOTION_MASK_LEG_R	0x1e000
#define MOTION_MASK_LEG_L	0x1e0000

#define	ACT_INTERP_DEF		8*5	/* 1/300単位 */
#define	ACT_INTERP_M		60*1 /* 1/300単位 */
#define	ACT_INTERP_SLOWLY	60*5 /* 1/300単位 */
#define	ACT_INTERP_VERY_SLOWLY	60*30 /* 1/300単位 */
#define	LAYER_BASE	0
#define	LAYER_OVER1	1
#define	LAYER_OVER2	2
#define	ACT_CTRL_INTERP_SLOWLY		90 /* 悠々と回転 */

#define	ACT_STATUS_STAND		I64(0x00000001)		/* 立ち状態 */
#define	ACT_STATUS_MOVE			I64(0x00000002)		/* 移動中 */
#define	ACT_STATUS_DOWN			I64(0x00000004)		/* 倒れ状態 */
#define	ACT_STATUS_DAMAGE		I64(0x00000008)		/* ダメージ中 */
#define	ACT_STATUS_DAM_DIR		I64(0x00000010)		/* どこからダメージを受けたかわかる */
#define	ACT_STATUS_ADJ_X		I64(0x00000100)		/* 上下アジャストＯＮ */
#define	ACT_STATUS_ADJ_Y		I64(0x00000200)		/* 左右アジャストＯＮ */
#define	ACT_STATUS_HOMING_SKIP	I64(0x00000400)		/* ホーミングスキップ */
#define	ACT_STATUS_TARGET_SKIP	I64(0x00000800)		/* ターゲットスキップ */
#define	ACT_STATUS_EYE_CLOSE	I64(0x00001000)		/* 視力０ */
#define	ACT_STATUS_CAPTURE		I64(0x00002000)		/* 捕まえられている */
#define	ACT_STATUS_FAINT		I64(0x00004000)		/* 気絶中 */
#define	ACT_STATUS_IK_DOWN		I64(0x00008000)		/* ダウン時IK発動 */
#define	ACT_STATUS_IK_HAND		I64(0x00010000)		/* ひづられ時腕IK発動 */
#define	ACT_STATUS_IK_FOOT		I64(0x00020000)		/* ひづられ時足IK発動 */
#define	ACT_STATUS_IK_MUKADE	I64(0x00040000)		/* ムカデ移動IK発動 */
#define	ACT_STATUS_GUN_FREE		I64(0x00080000)		/* 銃放す */
#define	ACT_STATUS_WAKEUP		I64(0x00100000)		/* 起き上がり中 */
#define	ACT_STATUS_GUNLIGHT_OFF	I64(0x00200000)		/* ガンライトオフ */
#define	ACT_STATUS_FLR_OFF		I64(0x00400000)		/* フロアチェックオフ */
#define	ACT_STATUS_SEG_OFF		I64(0x00800000)		/* 壁あたりチェックオフ */
#define	ACT_STATUS_TURN_SLOW	I64(0x01000000)		/* ゆっくり方向転換 */
#define	ACT_STATUS_FACE_TURN	I64(0x02000000)		/* 顔アジャストＯＮ */
#define	ACT_STATUS_IK_PIKU		I64(0x04000000)		/* IKダメージぴく */
#define	ACT_STATUS_SHOT_PIKU	I64(0x08000000)		/* 銃反動ぴく */
#define	ACT_STATUS_INVISIBLE	I64(0x10000000)		/* 消える */
#define	ACT_STATUS_RESURRECT	I64(0x20000000)		/* 復活の瞬間 */
#define	ACT_STATUS_GHOST		I64(0x40000000)		/* 幽霊 */
#define	ACT_STATUS_DEATH		I64(0x80000000)		/* 死亡 */

#define	ACT_STATUS_MOVE_START	I64(0x0000000100000000)	/* 目標をねらう */
#define	ACT_STATUS_GLASS		I64(0x0000000200000000)	/* 双眼鏡をかける */
#define	ACT_STATUS_MASUI_KIKU	I64(0x0000000400000000)	/* 麻酔がきいた */
#define	ACT_STATUS_MASUI_SASARU	I64(0x0000000800000000)	/* 麻酔弾が刺さった */
#define	ACT_STATUS_VANIME_HEAD	I64(0x0000001000000000)	/* 頂点アニメ頭 */
#define	ACT_STATUS_ADJ_PIKU		I64(0x0000002000000000)	/* ピク */
#define	ACT_STATUS_SHAGAMI		I64(0x0000004000000000)	/* しゃがみ状態 */
#define	ACT_STATUS_MECABREAK	I64(0x0000008000000000)	/* 機会が壊れた（軽い衝撃) */
#define	ACT_STATUS_GUNLIGHT_D	I64(0x0000010000000000)		/* ガンライト下照らす */
#define	ACT_STATUS_GUN_LEFTHAND	I64(0x0000020000000000)	/* 銃左手持ち */
#define	ACT_STATUS_TRG_THROUGH	I64(0x0000040000000000)	/* あたりにTHROUGHフラグを立てる */
#define	ACT_STATUS_TRG_PLAYER	I64(0x0000080000000000)	/* ターゲットサイドをプレイヤーに変更 */
#define	ACT_STATUS_UNREAL		I64(0x0000100000000000)	/* 存在しない */
#define	ACT_STATUS_PUSHT_SKIP	I64(0x0000200000000000)	/* プッシュターゲットスキップ */
#define	ACT_STATUS_BLEDTODEATH	I64(0x0000400000000000)	/* 出血多量 */
#define	ACT_STATUS_DAMAGE_NOW	I64(0x0000800000000000)	/* 今ダメージを受けている */
#define	ACT_STATUS_FALL			I64(0x0001000000000000)	/* 倒れている最中 */
#define	ACT_STATUS_SLEEP		I64(0x0002000000000000)	/* 眠っている */
#define	ACT_STATUS_HOLD_UP		I64(0x0004000000000000)	/* 降参している */
#define	ACT_STATUS_LOCKER		I64(0x0008000000000000)	/* ロッカーにはいっている */
#define	ACT_STATUS_GUNLIGHT_LR	I64(0x0010000000000000)	/* 左右覗き込み */
#define	ACT_STATUS_SPHERE_100	I64(0x0020000000000000)	/* r_sphereを１００に */
#define	ACT_STATUS_SPHERE_200	I64(0x0040000000000000)	/* r_sphereを２００に */
#define	ACT_STATUS_GURAD		I64(0x0080000000000000)	/* 防御中 */
#define	ACT_STATUS_HOUNYOU		I64(0x0100000000000000)	/* おしっこ */
#define	ACT_STATUS_STAND_ZZZ	I64(0x0200000000000000)	/* 立ち寝 */
#define	ACT_STATUS_MEDICATION	I64(0x0400000000000000)	/* 治療中 */
#define	ACT_STATUS_SPHERE_300	I64(0x0800000000000000)	/* r_sphereを3００に */
#define	ACT_STATUS_STANDSTILL	I64(0x1000000000000000)	/* 基本モーション */


//#define	ACT_STATUS_INTRUDE		I64(0x0000100000000000)	/* イントルードも見える */
//#define	ACT_STATUS_TRG_HEAD		I64(0x0000000100000000)	/* 目標に頭を向ける */
//#define	ACT_STATUS_AIM			I64(0x0000000200000000)	/* 目標をねらう */


/* status_status */
#define	ACT_STST_PIYOPIYO			I64(0x00000001)		/* 気絶している */
#define	ACT_STST_ZZZ				I64(0x00000002)		/* 眠っている */
#define	ACT_STST_HANG				I64(0x00000004)		/* 首締められている */
#define	ACT_STST_DRAG				I64(0x00000008)		/* ひきづられている */
#define	ACT_STST_ADJ_AIM			I64(0x00000010)		/* 注視点を向いて攻撃 */
#define	ACT_STST_DOWN_DAM			I64(0x00000020)		/* ダウン中ダメージ */
#define	ACT_STST_FALL_DOWN			I64(0x00000040)		/* 落下中 */
#define	ACT_STST_CHANGE_UNIQ_ID		I64(0x00000080)		/* ユニーくID変更（要注意） */
#define	ACT_STST_DROP				I64(0x00000100)		/* 落下中 */
#define	ACT_STST_PEEP_TOILET		I64(0x00000200)		/* トイレ覗き込み */
#define	ACT_STST_UNDER_NEARCHECK	I64(0x00000400)		/* 低い床チェック */
#define	ACT_STST_NOW_DAMAGE			I64(0x00000800)		/* 今まさにダメージを受けている */
#define	ACT_STST_KAITAIC4_BOMB		I64(0x00001000)		/* 解体C4ついていたら爆発or床に落ちる */
#define	ACT_STST_ERO_GETCHU			I64(0x00002000)		/* エロ本ゲッチュー */

/* pas_status */
#define P_STATUS_AIM	0x00000001		/* 注視点を見ながら */

/* ヘッドマーク */
enum {
	ACT_HEADMARK2_NONE,
	ACT_HEADMARK2_FAINT,
	ACT_HEADMARK2_ANES,
	ACT_HEADMARK2_POWA,
} ;

#define MAX_BANDAGE	9
#define BANDAGE_BODY	0x00000001
#define BANDAGE_RH_1	0x00000002
#define BANDAGE_RH_2	0x00000004
#define BANDAGE_LH_1	0x00000008
#define BANDAGE_LH_2	0x00000010
#define BANDAGE_RL_1	0x00000020
#define BANDAGE_RL_2	0x00000040
#define BANDAGE_LL_1	0x00000080
#define BANDAGE_LL_2	0x00000100


/* 部位ダメージ情報 */
enum {
	PTARGET_LEVEL0,	/* 体以外のターゲット */
	PTARGET_LEVEL1,	/* 急所 */
	PTARGET_LEVEL2,	/* 手足 */
	PTARGET_LEVEL3,	/* 体 */

	PTARGET_FALL,	/* 通常ダウン */
	
	MAX_PTARGET
};

typedef	struct{
	u_int 	dam_motion ;	/* ダメージモーション番号 */
	u_char obj_num ;	/* オブジェ番号 */
	u_char down_s ;		/* ダウン時の体の向き */
	u_char paddddd ;
	u_char paddddd2 ;
}PTARGET_INFO ;

typedef	struct{
	int num[MAX_PTARGET] ;
	PTARGET_INFO *info[MAX_PTARGET] ;
}PTARGET ;

#define	PTARGET_LEVEL0_NUM	3	/* 体以外のターゲット */
#define	PTARGET_LEVEL1_NUM	3	/* 急所 */
#define	PTARGET_LEVEL2_NUM	8	/* 手足 */
#define	PTARGET_LEVEL3_NUM	1	/* 体 */
#define	PTARGET_FALL_NUM	5	/* 通常ダウン */

#define	PTARGET_RADIO		0	/* 無線機 */
#define	PTARGET_NVISION1	1	/* 暗視ゴーグルレンズ部分 */
#define	PTARGET_NVISION2	2	/* 暗視ゴーグルバッテリー部分 */

#define	PTARGET_HEAD	0
#define	PTARGET_HART	1
#define	PTARGET_GOLD	2

#define	PTARGET_ARMR1	0
#define	PTARGET_ARMR2	1
#define	PTARGET_ARML1	2
#define	PTARGET_ARML2	3
#define	PTARGET_LEGR1	4
#define	PTARGET_LEGR2	5
#define	PTARGET_LEGL1	6
#define	PTARGET_LEGL2	7

#define	PTARGET_BODY	0

#define PBREAK_ARM_R	0x0001						/* 右腕故障 */
#define PBREAK_ARM_L	0x0002						/* 左腕故障 */
#define PBREAK_LEG_R	0x0004						/* 右足故障 */
#define PBREAK_LEG_L	0x0008						/* 左足故障 */

#define PBREAK_ARMS		(PBREAK_ARM_R|PBREAK_ARM_L)	/* 両腕故障 */
#define PBREAK_LEGS		(PBREAK_LEG_R|PBREAK_LEG_L)	/* 両足故障 */
#define PBREAK_ARMLEG	(PBREAK_ARMS|PBREAK_LEGS)	/* 両腕両足故障 */

/* 各防具部位 */
enum{
	DURABLE_AREA0,	/* 頭部 */
	DURABLE_AREA1,	/* 胸、腰部 */
	DURABLE_AREA2,	/* 四肢 */
	DURABLE_AREA3,	/* 未使用 */
	
	MAX_DURABLE_AREA
} ;

/* タイプ */
#define ENE_TYPE_NORMAL			0x00000000
#define ENE_TYPE_WATCHER		0x00000001
#define ENE_TYPE_ATTACKER		0x00000002
#define ENE_TYPE_SUPPORT		0x00000004
#define ENE_TYPE_SHIELD			0x00000008
#define ENE_TYPE_SHOTGUN		0x00000010
#define ENE_TYPE_EVENT_A		0x00000020 /*長廊下イベント兵*/
#define ENE_TYPE_TNG_A			0x00000040 /*天狗兵Ａ*/
#define ENE_TYPE_DMG_HAND_LEG	0x00000080 /*手足ダメージでライフ減る*/
#define ENE_TYPE_KATANA			0x00000100 /*刀兵*/
#define ENE_TYPE_NO_HEAR		0x00000200 /*完全に耳無し 爆発音も聞かない*/
#define ENE_TYPE_NO_BLURR		0x00000400 /*おぼろ視界無し*/
#define ENE_TYPE_DMG_MUTEKI		0x00000800 /*ダメージ中無敵*/
#define ENE_TYPE_EVENT_32		0x00001000 /*エマ援護イベント兵*/
#define ENE_TYPE_ABAKAN			0x00002000 /*アバカン装備*/
#define ENE_TYPE_HITECH			0x00004000 /*ハイテク装備*/
#define ENE_TYPE_M4				0x00008000 /*M4装備*/
#define ENE_TYPE_URBAN			0x00010000 /* 都市迷彩 */
#define ENE_TYPE_CONVERT		0x00020000 /* 普段は警備兵　発見後はアタッカー */
#define ENE_TYPE_NO_PBREAK		0x00040000 /* 部位ダメージ残らない*/
#define ENE_TYPE_NO_ROTTEN		0x00080000 /* 死体になっても腐らない */
#define ENE_TYPE_CONVERT2		0x00100000 /* 普段はアタッカー警戒モードはサポート兵*/
#define ENE_TYPE_USHOLD			0x00200000 /* 船倉兵 */
#define ENE_TYPE_CAMERA_USHOLD	0x00400000 /* カメラマン船倉兵 */
#define ENE_TYPE_FAINT_EXIT		0x00800000 /* 居眠り気絶で退場 */
#define ENE_TYPE_HOLD_EXIT		0x01000000 /* ホールドアップで退場 */
#define ENE_TYPE_DONT_KILL		0x02000000 /* 死ぬとゲームオーバー */
#define ENE_TYPE_VR				0x04000000 /* VR兵 */

#define ENE_TYPES_NO_CORP	(ENE_TYPE_USHOLD) /* 死体にならない */
#define ENE_TYPES_NO_GLASS	(ENE_TYPE_USHOLD) /* 双眼鏡なし */
#define ENE_TYPES_NO_STRM	(ENE_TYPE_USHOLD) /* ストリーミング音声なし */
#define ENE_TYPES_EAR_NARROW	(ENE_TYPE_USHOLD) /* 聴力幅せまい */
#define ENE_TYPES_NO_STUN	(ENE_TYPE_EVENT_A|ENE_TYPE_TNG_A|ENE_TYPE_EVENT_32) /* スタン効き難い */
#define ENE_TYPES_NO_STUN_VR	(ENE_TYPE_EVENT_32) /* スタン効き難い */
#define ENE_TYPES_NO_STUN_TALES	(ENE_TYPE_EVENT_32) /* スタン効き難い */
#define ENE_TYPES_NO_STUN_ANOTHER	(ENE_TYPE_EVENT_A|ENE_TYPE_EVENT_32) /* スタン効き難い */
#define ENE_TYPES_NO_STUN_BOSS_SURVIVAL	(ENE_TYPE_EVENT_32) /* スタン効き難い */

/*可動部分のある武器*/
#define	ENE_EQUIP_MOVABLE (ENE_TYPE_ABAKAN|ENE_TYPE_SHOTGUN)

/* その他 */
#define	ADJ_PIKU_TIME	COUNT_VMODE(6)

/* 左右反転モーション */
#define	ENE_MOT_FLAG_REVERSAL_U	(0x40000000)	/* 上半身 */
#define	ENE_MOT_FLAG_REVERSAL_D	(0x80000000)	/* 下半身 */

/* 心音間隔 */
extern int BP_AdjustTick(int);
#define ENE_HEART_BEAT_NORMAL	(BP_AdjustTick(120))
#define ENE_HEART_BEAT_HIGH		(BP_AdjustTick(60))
#define ENE_HEART_BEAT_LOW		(BP_AdjustTick(180))




/* 薬室状態 */
/** switch.chamber **/
#define CBR1_EMPTY		0
#define CBR1_BULLET		1
#define CBR1_CTRG		2
#define CBR2_NORAML		0
#define CBR2_OPEN		1
#define CBR2_CLOSE		2

/*-----  -----------------------------------*/

typedef	struct{
	int	body ;
	int	weapon ;
	int	sub_weapon ;
	int	motion ;
	int	name ;

	int	mot_stage ;		/* ステージ固有モーション名 */

	short		g_id ;		/* 所属グループ */
	short		u_id ;		/* 所属ユニット */
	short		id ;		/* 番号 */
	short		voice ;		/* 声優番号 */
	int			uniq_id ;	/* 個別ＩＤ */
} NAME_ID ;

enum {
	DAM_MODE_NONE,
	DAM_MODE_BULLET,
	DAM_MODE_MASUI,
	DAM_MODE_MASUI_DOWN,
	DAM_MODE_BOMB,
	DAM_MODE_THROW,
	DAM_MODE_HANG,
	DAM_MODE_GOOFY,
	DAM_MODE_BLEDTODEATH,
	DAM_MODE_SMOKE,
	DAM_MODE_DROP,
	DAM_MODE_EXIT,
} ;

	/* 体調パラメータ */
typedef	struct{
    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まり情報 */
    POWER_TARGET	power ;	/* 防御属性 */
    TARGET			def_child0[ PTARGET_LEVEL0_NUM ] ;
    POWER_TARGET	power_child0[ PTARGET_LEVEL0_NUM ] ;	/* 防御属性 */
    TARGET			def_child1[ PTARGET_LEVEL1_NUM ] ;
    POWER_TARGET	power_child1[ PTARGET_LEVEL1_NUM ] ;	/* 防御属性 */
    TARGET			def_child2[ PTARGET_LEVEL2_NUM ] ;
    POWER_TARGET	power_child2[ PTARGET_LEVEL2_NUM ] ;	/* 防御属性 */
    TARGET			def_child3[ PTARGET_LEVEL3_NUM ] ;
    POWER_TARGET	power_child3[ PTARGET_LEVEL3_NUM ] ;	/* 防御属性 */
    TARGET			pushtrg ;	/* ぶつかりターゲット */

	TARGET			*damtrg ;	/* ダメージを受けたターゲット */
	int				n_damobj ;	/* ダメージを受けたオブジェクト番号 */
	FVECTOR			off_center ;	/* ダメージの発生源 */

    short			dammode ;	/* ダメージの種類 */
    short			heart_beat ;	/*	心音間隔 */
    short			m_life ;			/* ＭＡＸ体力値 */
    short			m_faint ;			/* ＭＡＸ気絶値 */
    short			m_blood ;			/* ＭＡＸ血液値 */
    short			m_anesthesia ;	/* ＭＡＸ麻酔耐久度 */

    short			life ;			/* 体力値 */
    short			faint ;			/* 気絶値 */
    short			blood ;			/* 血液値 */
    short			anesthesia ;	/* 麻酔耐久度 */
    short			stand ;			/* 立ち耐久度 */
    short			pbreak ;		/* 部位ダメージ情報 */
    short			loss_blood ; 	/* 出血多量 */
    short			faint_time ;	/* 気絶時間 */
    u_char			dam_level_num[4] ;	/* 各部位のダメージの数 */
    u_char			ane_level_num[4] ;	/* 各部位の麻酔の数 */

    char			durable[4] ;		/* 各防具個所の耐久値 */
    char			max_durable[4] ;	/* 各防具個所の最大耐久値 */

	int				type ;
	long64			last_weapon ;
} BODYPARAM ;

typedef	struct{
	int vibration ;
	int magg ;
	int radio ;
	int n_sight ;
	int eye_anim ;
	int finger ;
	int mouth ;
	int sub_weapon ;
	int shield ;
	int headmark ;
	int splash ;
	int hounyou ;
	int dogtag ;
	short	chamber1 ; /*薬室の状態*/
	short	chamber2 ; /*薬室の開閉制御*/
	int *bullet ;
	WEAPON_EF_CTRL 	wctrl ;		/* マカロフ... */
	WEAPON_EF_CTRL 	wctrl2 ;	/* ショットガン... */
} SWITCH ;

typedef	struct{
	short		gun_pad ;			/* 銃パッド */
	short		gun_brank ;			/* 銃 */
} ACTGUN ;

#define ACTGUN_PULL_TRIGGER		(0x00000001)
#define ACTGUN_TARGET_AIMPOS	(0x00000002)

typedef	struct	_Action{
	void		*w ;	/* 親子関係用各兵のワークのポインタ */
	BODYPARAM	bodyp ;
	CONTROL		*ctrl ;
	OBJECT		*body ;
	NAME_ID		*name_id ;
	SWITCH		*sw ;
	FMATRIX		*lights ;
	float		old_body_height ;	/* １フレーム前のbody height */
	short		dir ;			/* 体の向き */
	short		motion_num ;	/* モーション番号（bodyにセット) */
	short		act_end ;		/* アクションエンドフラグ */
	short		move_s ;		/* 移動手段 */
	short		down_s ;		/* ダウン種類 */
	short		tmp_dir ;		/* 汎用方向バッファ */
	short		body_dir ;			/* 体の向き */
	short		before_dir ;	/* １フレーム前の体の向き */
	short		adj_piku_time ;
   short    adj_piku_value ;
	short		aim_dir ;		/* 目をむける方向 */
	FVECTOR		aim_pos ;		/* 目を向ける位置 */
	FVECTOR		target_pos ;	/* 目標 */
	int			adj_flag_old ;	/* 最終アジャストフラグ */
	long64		status ;		/* アクションステータス */	
	long64		old_status ;		/* 1フレーム前のアクションステータス */	
	int			*ene_status ;	/* 敵兵のステータス */	
	int			*thk_status ;	/* 思考のステータス */	
	int			status_status ;	/* 拡張ステータス（ビット不足の為） */
	u_int		time ;			/* 経過時間 */
	u_int		tmp_time ;		/* 時間 */
	int			c_motion_num[3] ;	/* 使用中のモーション番号 */
	int			c_motion_mar[3] ;	/* 使用中のmar名 */

	int			( *CheckDamage)( struct _Action * ) ;	/* ダメージチェック関数 */
	int			( *CheckPad )( struct _Action * ) ;	/* パッドチェック関数 */
	int			pad ;			/* アクションパッド */
	int			pad_status ;	/* アクションステータス なになにしながら情報 */
	ACTGUN		actgun ;

	int			keep_mot ;		/* 継続モーション */
	int			keep_pad ;		/* 継続パッド */
	int			keep_mar ;		/* 継続mar */
	void		*h_mark ;		/* ヘッドマーク アニメで消える */
	void		*h_mark2 ;		/* ヘッドマーク 時間で消える */
	short		headmark ;
	short		headmark2 ;
	short		headmark2_num ;
	short		headmark_paddddddddddd ;
	void		*headmarkwork ;		/* ヘッドマークワーク*/
	void		*oozeblood ;		/* にじみ血用 */
	void		*head_blood ;	/* ハイテク兵、顔血用 */
	OBJECT		bandage[MAX_BANDAGE] ;
	int			bandage_parts ;
	OBJECT		*sub_obj ;		/* 盾用 */
	void		*tmp_item ;	/* 一時使用アイテム */
	int			sw_tmp_item ;	/* 一時使用アイテムSW */

	TARGET		offense ;		/* 攻撃用ターゲット */
    POWER_TARGET	off_pow ;	/* 攻撃用 */
	ITEM_PROC	item ;
	ITEM_PROC	hold_item ;		/* ホールドアップの時に出すアイテム */
	ITEM_PROC	dogtag_item ;	/* ドッグタグアイテム */
	int			dogtag_id ;		/* ドッグタグＩＤ */

	int			*se_num ;				/* ＳＥ番号バッファ */
	short		*se_time ;				/* ＳＥタイムバッファ */
	short		se_next ;				/* 次のＳＥバッファ番号 */

	short		bgm_track ;			/* 個別ＢＧＭトラック番号 */
	short		bgm_paddd ;

	int			*motion ;
	int			*motion_table ;
	SVECTOR		adj_rot ;

	void		( *action )( struct _Action *, int );
	VERTEX_ANIME_WORK	*vanime ;
	void		*ik ;		/* ＩＫ用 */
	int			ik_time ;	/* ＩＫタイマー */
	void		*new_ik ;		/* 高部ＩＫ用 */


	/*重野追加*/
//	int			mukade_time;	/*ムカデ制御タイマ*/
	float		mot_speed_correct; /*モーション移動量を加工*/
	int			mot_dir_correct;	/*モーション移動方向を加工*/
	TARGET		*the_target;	/*汎用目標*/
	CAPTURE_TARGET	*cap_attack ;	/*つかみ攻撃するキャラのみ使用*/
} ACTION ;

typedef	void	( *ACTIONMODE )( ACTION *, int ) ;

#if 0 /* XBOX版では、u_long64がきちんと取れないため yano add 2002.03.11 */
static inline void SetMarAction( act, n_layer, data, m_time, mask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
int		mar ;
#else
static inline void SetMarAction( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp, int mar )
#endif
{
	int	mot ;


	if ( act->c_motion_mar[n_layer] != mar ) {
//printf("enemy:mar change [%d]\n",mar ) ;
		act->c_motion_mar[n_layer] = mar ;
		act->body->m_ctrl->motion_arc = MT_GetMotionArchives( mar ) ;
		MT_FreeSequence( act->body->m_ctrl->sar_ctrl ) ;
		act->body->m_ctrl->sar_ctrl = MT_InitSequence( act->body->m_ctrl->n_layer, mar, 0 ) ;
		act->c_motion_num[n_layer] = -1 ;
	}

	mot = data & ~(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

	if ( act->c_motion_num[n_layer] == mot ) {
		return ;
	}
	act->c_motion_num[n_layer] = data ;
	GM_ConfigObjectAction( act->body, n_layer, act->motion_table[mot], m_time, mask, interp ) ;

	if ( n_layer == 0 ) act->body->m_ctrl->flag &= ~(MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2) ;

	if ( data & ENE_MOT_FLAG_REVERSAL_U ) act->body->m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
	if ( data & ENE_MOT_FLAG_REVERSAL_D ) act->body->m_ctrl->flag |= MT_FLAG_REVERSAL2 ;

	MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE ) ;

	/* マージフラグクリア */
//	act->body->m_ctrl->mt3_ctrl[ n_layer ].merge_flag = 0 ;

if ( act->motion_table[mot] < 0 ) {
//printf( " motion err num data[%d]->[%d] \n",data, act->motion_table[mot] ) ;
}
}

#if 0 /* yano 2002.03.11 */
static inline void SetAction( act, n_layer, data, m_time, mask, interp )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
#else
static inline void SetAction( ACTION *act, int n_layer, int data, int m_time, u_long64 mask, int interp )
#endif
{

#ifdef MOTION_PLUGIN
	SetMarAction( act, n_layer, data, m_time, mask, interp, act->name_id->motion ) ;
#else
	int	mot ;

	mot = data & ~(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
	if ( act->c_motion_num[n_layer] == mot ) return ;
	act->c_motion_num[n_layer] = data ;
	GM_ConfigObjectAction( act->body, n_layer, act->motion_table[mot], m_time, mask, interp ) ;

	if ( n_layer == 0 ) act->body->m_ctrl->flag &= ~(MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2) ;

	if ( data & ENE_MOT_FLAG_REVERSAL_U ) act->body->m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
	if ( data & ENE_MOT_FLAG_REVERSAL_D ) act->body->m_ctrl->flag |= MT_FLAG_REVERSAL2 ;

	MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE ) ;

	/* マージフラグクリア */
//	act->body->m_ctrl->mt3_ctrl[ n_layer ].merge_flag = 0 ;

if ( act->motion_table[mot] < 0 ) {
//printf( " motion err num data[%d]->[%d] \n",data, act->motion_table[mot] ) ;
}
#endif
}

#if 0 /* yano 2002.03.11 */
static inline void ReSetMarAction( act, n_layer, data, m_time, mask, interp, mar )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
int		mar ;
#else
static inline void ReSetMarAction( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp, int mar )
#endif
{
	act->c_motion_num[n_layer] = -1 ;
	SetMarAction( act, n_layer, data, m_time, mask, interp, mar ) ;
}

#if 0 /* yano 2002.03.11 */
static inline void ReSetAction( act, n_layer, data, m_time, mask, interp )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
#else
static inline void ReSetAction( ACTION	*act, int n_layer, int data, int m_time, u_long64 mask, int interp )
#endif
{
#ifdef MOTION_PLUGIN
	act->c_motion_num[n_layer] = -1 ;
	SetMarAction( act, n_layer, data, m_time, mask, interp, act->name_id->motion ) ;
#else
	int	mot ;

	mot = data & ~(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
	act->c_motion_num[n_layer] = data ;
	GM_ConfigObjectAction( act->body, n_layer, act->motion_table[mot], m_time, mask, interp );

	/* レイヤー０からモーションをセットして行くこと */
	if ( n_layer == 0 ) act->body->m_ctrl->flag &= ~(MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2) ;

	if ( data & ENE_MOT_FLAG_REVERSAL_U ) act->body->m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
	if ( data & ENE_MOT_FLAG_REVERSAL_D ) act->body->m_ctrl->flag |= MT_FLAG_REVERSAL2 ;

	MT_SetMotionSpeed( act->body->m_ctrl, ( float )TIME_BASE ) ;
	/* マージフラグクリア */
//	act->body->m_ctrl->mt3_ctrl[ n_layer ].merge_flag = 0 ;
#endif
}

static	inline	void	SetMode( act, action )
ACTION			*act ;
ACTIONMODE		action ;
{
	act->action = action ;
	act->time = 0 ;
}

/*----- プロトタイプ ----------------------------------------------*/
extern void AT_Action( ACTION	* ) ;
extern void AT_InitAction( ACTION *, CONTROL *, OBJECT * ) ;
extern void AT_SetType( ACTION	*, int ) ;
extern void AT_SetBodyParam( BODYPARAM *, short, short, short ) ;
extern void	AT_SetDurable( BODYPARAM *, int, int, int, int ) ;
extern void AT_SetActionMotion( ACTION *, int *, int * ) ;
extern void AT_SetMode( ACTION *, ACTIONMODE ) ;
extern void AT_SetModeFromPad( ACTION *, ACTIONMODE, int, int ) ;
extern void AT_SetModeFromPadMar( ACTION *, ACTIONMODE, int, int, int ) ;
extern void AT_SetMarAction( ACTION *, int, int, int, u_long64, int, int ) ;
extern void AT_SetAction( ACTION *, int, int, int, u_long64, int ) ;
extern void AT_ReSetMarAction( ACTION *, int, int, int, u_long64, int, int ) ;
extern void AT_ReSetAction( ACTION *, int, int, int, u_long64, int ) ;
extern void AT_SetTargetClass( ACTION *, int ) ;
extern void AT_UnSetTargetClass( ACTION *, int ) ;
extern void AT_SetAllChildTargetClass( ACTION *, int flag ) ;
extern void AT_UnSetAllChildTargetClass( ACTION *, int flag ) ;
extern void AT_SetActStatus( ACTION *, long64 ) ;
extern void AT_SetActStSt( ACTION *act, int flag ) ;
extern void	AT_SetHeartBeat( ACTION *act, int beat ) ;

#endif
