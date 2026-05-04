/*
   player.h
   プレイヤー関係定義ファイル

   2000/02/01 M.Sonoyama
   $Id: player.h,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $
*/

#ifndef _player_h_
#define	_player_h_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------

  基本設定値

---------------------------------------------------------------*/

/* 装備品管理 */
#define	PL_EQUIPMENT_MANAGER_NAME	(GV_StrCode( "ライデン装備品管理" ))
/* 体炎 */
#define	GM_PLAYER_CHAR_BODY_FLAME  	(14277024)

/* メニューのクイックチェンジ期間 */
#define	GM_MENU_QUICK_TIME	(12)

/* ズームカメラの名前 */
#define	PL_PSG1_ZOOMCAMERA_NAME			(GV_StrCode( "ズームカメラＰＳＧ１"))
#define	PL_SCOPE_ZOOMCAMERA_NAME		(GV_StrCode( "ズームカメラ双眼鏡"))
#define	PL_CAMERA_ZOOMCAMERA_NAME		(GV_StrCode( "ズームカメラデジタルカメラ"))
#define	PL_TNKCAM_ZOOMCAMERA_NAME		(GV_StrCode( "ズームカメラタンカーカメラ"))

/* レーション回復値 */
static	inline	int		GM_RationHeal( void )
{
	if ( GM_GameLevel == GM_LEVEL_VERYEASY ) return GM_VitalityMax ;
	else									 return GM_VitalityMax / 2 ;
}
#define	GM_RATION_HEAL		GM_RationHeal()

/* ジアゼパム有効時間 */
#define	GM_DIAZEPAM_COUNT	(DIRECT_TICK(1800))

/* 武器の１ステージセットＭＡＸ */
#define	GM_C4_MAX					(16)
#define	GM_CLAYMORE_MAX				(16)
#define	GM_BOOK_MAX					(16)
#define	GM_CURRENT_GRENADE_MAX 		(16)
#define	GM_CURRENT_RGB6_MAX			(4)	


/* 影チェック位置 */
#define	GM_MAX_PLAYER_SHADOW_POS	(4)

/* ニキータ存在フラグ */
enum {
    NKT_NONE = 0,		/* 飛んでいない */
    NKT_NORMAL,			/* 通常飛行中 */
    NKT_BLAST,			/* 爆発処理中 */
    NKT_CANCEL,			/* プレイヤーの状況により操作不能 */
	NKT_JAMMING,		/* ジャミングにより操作不能 */
} ;

/* グレネードフラグ定義 */
/* PL_Bul_Grenade_Flag */
enum {
	BGF_NONE =				0x0000,
	BGF_TRAP_CHECK =		0x0001,		/* トラップチェックする */
	BGF_QUEUE_CONTROL =		0x0002,		/* コントロールを登録する */
	BGF_EXIST_BOMBLOST =	0x0004,		/* bomblostトラップあり */
} ;

/* 移動床関数 */
typedef	int		( *PL_MOVE_FLOOR_FUNC )( FVECTOR *, HZX_GROUP_ID, FVECTOR *, HZX_EVT *evt, int name ) ;

/* 落下弾関数 */
typedef	void*	( *PL_FALLBULLET_FUNC )( FMATRIX *,FVECTOR *,FVECTOR *,int,float,float,int ) ;

/* エルード妨害物チェック */
typedef	int		( *PL_ELUDEDISTURB_FUNC )( FVECTOR *, int ) ;

/* 弾水飛沫 */
typedef	void*	( *PL_BULLETSPLASH_FUNC )( FVECTOR *, float, float, int ) ;
typedef	void*	( *PL_BLASTWATER_FUNC )( FVECTOR *, int, int, int, int, int, int, int ) ;

/* 落下弾関数用 */
enum {
	FALLBUL_TYPE_ZFRONT 	= 0x0000,
	FALLBUL_TYPE_YDOWN		= 0x0001,
	FALLBUL_TYPE_RANDOM		= 0x0002,
	FALLBUL_TYPE_ENDDESTROY	= 0x0004,
	FALLBUL_TYPE_ENDXROTZERO =	0x0008,
	FALLBUL_TYPE_CHANL0ONLY =	0x0010,
	FALLBUL_TYPE_NONEARCHECK =	0x0020,
} ;

/* アクタープライオリティ */
/* ACTOR_PLAYER */
enum {
	PLAYER_BODY_ACTOR_PRIO = 8,
	PLAYER_ARM_ACTOR_PRIO = 16,
	PLAYER_WEAPON_ACTOR_PRIO = 24,
	PLAYER_ITEM_ACTOR_PRIO = 32,
	PLAYER_INVISIBLE_ACTOR_PRIO = 35,	/* プレイヤーを消す */
	PLAYER_CHECK_ACTOR_PRIO = 40,
} ;

/* ACTOR_AFTER2 */
enum {
	PLAYER_WPMNG_ACTOR_PRIO = 32,	/* 武器管理者 */
	PLAYER_SUNGLASSES_ACTOR_PRIO = 48,	/* サングラス管理 */
	PLAYER_UNIFORM_ACTOR_PRIO = 64,	/* ゴル兵制服 */
	PLAYER_STEALTH_ACTOR_PRIO = 96,	/* ステルス迷彩 */
	PLAYER_MULTI_ACTOR_PRIO = 128,	/* マルチウェイト */
	PLAYER_MENU_ACTOR_PRIO = 160,
	PLAYER_GAGE_DAEMON_PRIO = 248,
	PLAYER_LODCTRL_ACTOR_PRIO = 250, /* ＬＯＤコントロール */
	PLAYER_ACTIONBLUR_ACTOR_PRIO = 251,	/* モーションブラー */
	PLAYER_SUBJECTCAM_ONOFF_PRIO = 254,	/* 主観カメラ制御 */
	PLAYER_PADDEMO_ACTOR_PRIO = 255, /* パッドデモ */
} ;

/* ACTOR_PREV2 */
enum {
	PLAYER_LODCTRL_ACTOR2_PRIO = 16,
	PLAYER_MULTI_ACTOR2_PRIO = 24,
	PLAYER_STEALTH_ACTOR2_PRIO = 28,
	PLAYER_UNIFORM_ACTOR2_PRIO = 32,
	PLAYER_INVISIBLE_ACTOR2_PRIO = 36,	/* プレイヤーを消す */

} ;

/*---------------------------------------------------------------

  カメラ優先度

---------------------------------------------------------------*/

/* 主観カメラ */
enum {
	ZOOM_CTRL_CAMERA_PRIO = 16,
	ELEVATOR_CAMERA_PRIO =	24,
	WEAPON_CAMERA_PRIO = 	32,
	SCOPE_CAMERA_PRIO =		96,
	SUBJECT_CAMERA_PRIO = 	128,
} ;

/*---------------------------------------------------------------

  プレイヤー状態

---------------------------------------------------------------*/

#define    PLAYER_NORMAL   	I64(0x0000)		/* 通常状態 */
#define    PLAYER_WATCH    	I64(0x0001)		/* 主観 */
#define    PLAYER_INTRUDE  	I64(0x0002)		/* イントルード */

#define    PLAYER_SQUAT    	I64(0x0010)		/* しゃがみ */
#define    PLAYER_GROUND   	I64(0x0020)		/* 匍匐、ダウン */
#define    PLAYER_CAUTION  	I64(0x0040)		/* 張り付き */
#define    PLAYER_LOCKER  	I64(0x0080)		/* ロッカーの中 */

#define    PLAYER_ATTACK   	I64(0x0100)		/* 攻撃中（未使用） */
#define    PLAYER_DAMAGED  	I64(0x0200)		/* ダメージモード中 */
#define    PLAYER_DOWNED      I64(0x0400)		/* ダウン中 */
#define    PLAYER_HOLD	   	I64(0x0800)		/* 武器構え状態 */

#define    PLAYER_BEYOND   	I64(0x1000)		/* ビヨンドモード */
#define    PLAYER_FORCE    	I64(0x2000)		/* 強制モーション */
#define    PLAYER_CB_BOX   	I64(0x4000)		/* ダンボールモード */
#define    PLAYER_DEAD	   	I64(0x8000)		/* ゲームオーバー */

#define    PLAYER_LADDER   I64(0x00010000)		/* はしごモード */
#define	PLAYER_ENEMY_PULL  I64(0x00020000)	/* 引き摺り中 */
#define	PLAYER_CB_BOX_STAND  I64(0x00040000)	/* ダンボールかぶって立ち */

    /* 以下、通常状態とみなす。 */
#define    PLAYER_BLOOD_DROP  I64(0x01000000)		/* ぽたぽた血中 */
#define    PLAYER_WEAPON_DISABLE  I64(0x02000000)	/* 武器メニュー不可 */
#define    PLAYER_ITEM_DISABLE	   I64(0x04000000)	/* アイテムメニュー不可 */
#define    PLAYER_MENU_OPEN	   I64(0x08000000)	/* メニューが開いている */

#define    PLAYER_STOP	       I64(0x10000000)		/* 処理停止 */
#define    PLAYER_BEHIND_CAMERA_ENABLE  I64(0x20000000)	/* ビハインドカメラになってもいいよ */
#define    PLAYER_WEAPON_INVISIBLE  I64(0x40000000)	/* 武器非表示 */
#define    PLAYER_DEBUG   	I64(0x80000000)		/* デバグモード */

#define    PLAYER_WEAPON_QUICK_ONLY 	I64(0x0000000100000000) 	/* 武器変更クイックのみ可 */
#define    PLAYER_ITEM_QUICK_ONLY    	I64(0x0000000200000000) 	/* アイテム変更クイックのみ可 */
#define    PLAYER_NEED_NEW_PRESS     	I64(0x0000000400000000)	/* パッド押し直しが必要 */
#define	PLAYER_ENEMY_HANG 			I64(0x0000000800000000)	/* 敵つかみ中 */

#define    PLAYER_SIGHT_LOCKON  I64(0x0000001000000000)		/* サイトが敵にロックしている */
#define    PLAYER_SNAKE  	  I64(0x0000002000000000)			/* プレイヤーはスネーク */
#define    PLAYER_DARK_AREA  	  I64(0x0000004000000000)		/* 暗い所にいる */
#define    PLAYER_ENEMY_HIDDEN  I64(0x0000008000000000)		/* 敵からは見えないエリア */

#define    PLAYER_INVINCIBLE  I64(0x0000010000000000)		/* 無敵 */
#define    PLAYER_PAD_OFF  	I64(0x0000020000000000)		/* パッドオフ */
#define    PLAYER_BEHIND  	I64(0x0000040000000000)		/* ビハインド */
#define    PLAYER_MOVE     	I64(0x0000080000000000)		/* 移動中 */

#define	PLAYER_EVENT_ENABLE  I64(0x0000100000000000)		/* イベント突入可能 */
#define	PLAYER_IN_THE_WATER  I64(0x0000200000000000)		/* 水中 */
#define	PLAYER_ON_CORPSE	 I64(0x0000400000000000)		/* 死体の上 */
#define	PLAYER_CROSS 		  I64(0x0000800000000000)		/* 重なり中 */

#define	PLAYER_INVINCIBLE_SCN  I64(0x0001000000000000)	/* シナリオから無敵にされた */
#define	PLAYER_INVINCIBLE_PRG  I64(0x0002000000000000)	/* 外部から無敵にされた */
#define	PLAYER_BEHIND_ATTACK 	I64(0x0004000000000000)	/* 飛び出し撃ち */
#define	PLAYER_CB_BOX_CANCELED  I64(0x0008000000000000)	/* ダンボール取られた */

#define	PLAYER_ROLLING 		 I64(0x0010000000000000)	/* 転がり中 */
#define	PLAYER_NARROW 			 I64(0x0020000000000000)	/* 狭いところにいる */
#define	PLAYER_CB_BOX_HIDDEN 	 I64(0x0040000000000000)	/* ダンボール被っていれば見つからない */
#define	PLAYER_NORECOVER 		 I64(0x0080000000000000)	/* 回復不能状態 */

#define	PLAYER_WALK  			 I64(0x0100000000000000)	/* 歩き */
#define	PLAYER_DASH  			 I64(0x0200000000000000)	/* ダッシュ */
#define	PLAYER_CBBOX_RUN 		 I64(0x0400000000000000)	/* ダンボール移動中 */
#define	PLAYER_COLD 			 I64(0x0800000000000000)	/* 風邪引き中 */

#define	PLAYER_WATER_SURFACE 	 I64(0x1000000000000000)	/* 泳ぎ位置水面付近 */
#define	PLAYER_NO_BREATH 		 I64(0x2000000000000000)	/* 息ができない */
#define	PLAYER_STEALTH 		 I64(0x4000000000000000)	/* ステルス迷彩 */

#define	PLAYER_STATUS_EX 		 I64(0x8000000000000000)	/* GM_PlayerStatus2で判定 */


#define	PLAYER2_ELUDE_FALL			I64(0x8000000000000001)	/* エルード落下中 */
#define	PLAYER2_NUDE 				I64(0x8000000000000002)	/* はだか中 */
#define	PLAYER2_MENU_DISABLE 		I64(0x8000000000000004)	/* ポーリングでメニューを禁止するとき	
														   専用 */
#define	PLAYER2_NO_MOTION_STEP_XZ 	I64(0x8000000000000008)	/* ポーリングでＸＺ移動禁止専用 */

#define	PLAYER2_NO_KICK 			I64(0x8000000000000010)	/* コンボキック禁止 */
#define	PLAYER2_JUMP 				I64(0x8000000000000020)	/* ジャンプ中 */
#define	PLAYER2_LOD_HIGH 			I64(0x8000000000000040)	/* ＬｏＤ中 */
#define	PLAYER2_POWDER 			I64(0x8000000000000080)	/* 粉エリア中 */

#define	PLAYER2_ARM_INVISIBLE 	 	I64(0x8000000000000100)	/* 主観腕の強制非表示 */
#define	PLAYER2_SUBJECT_DEMO 		I64(0x8000000000000200)	/* 主観デモキャラ起動中 */
#define	PLAYER2_COMBO 				I64(0x8000000000000400)	/* コンボ中 */
#define	PLAYER2_KNOCK_WALL 		I64(0x8000000000000800)	/* 壁コンコン中 */

#define	PLAYER2_ON_CORPSE_STAND 	I64(0x8000000000001000)	/* 死体の上なら立ち上がる（ファットマン専用） */
#define	PLAYER2_WATERMINE_NEAR 	I64(0x8000000000002000)	/* 水中機雷に近い（エマ専用） */
#define	PLAYER2_NARROW_HIDDEN 		I64(0x8000000000004000)	/* 狭いところで、敵からは見えない */
#define	PLAYER2_BLAST_IN_HAND 		I64(0x8000000000008000)	/* グレネード手中爆発 */

#define	PLAYER2_DIVE_GOGGLES 		I64(0x8000000000010000)	/* 潜水ゴーグル付き */
#define	PLAYER2_SQUAT_ONLY 		I64(0x8000000000020000)	/* しゃがみＯＮＬＹエリアに居る */
#define	PLAYER2_DEEP_STEALTH 		I64(0x8000000000040000)	/* 濃いステルス */
#define	PLAYER2_SPRAY_CHECK_WIDE 	I64(0x8000000000080000)	/* スプレー当たり判定拡大 */

#define	PLAYER2_KAGESHIBARI_NOW 	I64(0x8000000000100000)	/* 影縛り中 */

#define	PLAYER_NORMAL_MASK	(I64(0x0000000000ffffff) | PLAYER_BEHIND_CAMERA_ENABLE | \
							 PLAYER_IN_THE_WATER | PLAYER_BEHIND | PLAYER_ROLLING | \
							 PLAYER_WATER_SURFACE)

#define	PLAYER_MENU_DISABLE		(PLAYER_WEAPON_DISABLE|PLAYER_ITEM_DISABLE)
#define	PLAYER_INVINCIBLE_OUT	(PLAYER_INVINCIBLE_SCN|PLAYER_INVINCIBLE_PRG)
#define	PLAYER_INVINCIBLE_ALL	(PLAYER_INVINCIBLE|PLAYER_INVINCIBLE_OUT)

#define	PLAYER2_NORMAL_MASK		(PLAYER2_ELUDE_FALL)

/*----------------------------------------------------

  武器・アイテム設定

-----------------------------------------------------*/

/* 武器タイプ */
enum {
	WP_TYPE_NORMAL	= 0x0000,
    WP_TYPE_PRESS	= 0x0001,	/* ボタン”押し”で攻撃 */
    WP_TYPE_INTRUDE_OK	= 0x0002,	/* イントルード中も可 */
    WP_TYPE_HOMING	= 0x0004,	/* ホーミングする */
    WP_TYPE_MAGAZINE	= 0x0008,	/* マガジン武器 */

    WP_TYPE_CONSECUTIVE = 0x0010,	/* 連射武器 */
    WP_TYPE_CALLFUNC	= 0x0020,	/* 装備時関数呼びタイプ */
    WP_TYPE_SUBJECT	= 0x0040,	/* 主観武器 */
    WP_TYPE_PRESSURE	= 0x0080,	/* 攻撃判定アナログ敷居値あり */

    WP_TYPE_RUNNING	= 0x0100,	/* 構えたまま走れる */
    WP_TYPE_NIKITA	= 0x0200,	/* ニキータ特殊 */
    WP_TYPE_MODE1FUNC	= 0x0400,	/* 攻撃時、モード１で関数コール */
    WP_TYPE_RELEASE_MOTION = 0x0800,	/* 撃ち終わりモーションあり */

	WP_TYPE_TACTICAL =	0x1000,	/* タクティカルリロード可 */
	WP_TYPE_GRENADE = 	0x2000,	/* グレネード系 */
	WP_TYPE_BODY =		0x4000,	/* 本体と弾タイプ */
	WP_TYPE_ONLY_ONE =	0x8000,	/* 一個のみ */

	WP_TYPE_PUNCH_S	=	0x00010000,	/* パンチ弱 */
	WP_TYPE_PUNCH_M =	0x00020000,	/* パンチ中 */
	WP_TYPE_PUNCH_L =	0x00040000,	/* パンチ強 */
	WP_TYPE_PUNCH_LL =	0x00080000,	/* パンチ強大 */

	WP_TYPE_BH_ATTACK =		0x00100000,	/* ビハインド覗きから飛び出し攻撃可 */
	WP_TYPE_BH_JUMPOUT =	0x00200000,	/* ビハインド通常から飛び出し撃ち可 */
	WP_TYPE_BLADE =			0x00400000,	/* ブレード特殊 */
	WP_TYPE_STICK =			0x00800000,	/* くっつけ武器 */

	WP_TYPE_MAXCHECK =		0x01000000,	/* 設置最大数あり */
	WP_TYPE_ZERO_TO_NONE =	0x02000000,	/* 所持数０時素手に戻る */
	WP_TYPE_SHOOTHEIGHT1 =	0x04000000,	/* 撃ち高さ１ （950）*/
	WP_TYPE_SHOOTHEIGHT2 =	0x08000000,	/* 撃ち高さ２ （750）*/
} ;

#define	NONE_TYPE	(WP_TYPE_PRESS|WP_TYPE_ONLY_ONE)
#define	M92_TYPE	(WP_TYPE_INTRUDE_OK|WP_TYPE_HOMING|WP_TYPE_MAGAZINE|\
					 WP_TYPE_PRESSURE|WP_TYPE_BODY|WP_TYPE_PUNCH_S|\
					 WP_TYPE_SHOOTHEIGHT1|\
					 WP_TYPE_BH_ATTACK|WP_TYPE_BH_JUMPOUT)	/* タクティカルではない */
#define	USP_TYPE	(WP_TYPE_INTRUDE_OK|WP_TYPE_HOMING|WP_TYPE_MAGAZINE|\
					 WP_TYPE_PRESSURE|WP_TYPE_BODY|WP_TYPE_TACTICAL|WP_TYPE_PUNCH_S|\
					 WP_TYPE_SHOOTHEIGHT1|\
					 WP_TYPE_BH_ATTACK|WP_TYPE_BH_JUMPOUT)
#define	SOCOM_TYPE	(WP_TYPE_INTRUDE_OK|WP_TYPE_HOMING|WP_TYPE_MAGAZINE|\
					 WP_TYPE_PRESSURE|WP_TYPE_BODY|WP_TYPE_TACTICAL|WP_TYPE_PUNCH_S|\
					 WP_TYPE_SHOOTHEIGHT1|\
					 WP_TYPE_BH_ATTACK|WP_TYPE_BH_JUMPOUT)
#define	FAMAS_TYPE	(WP_TYPE_INTRUDE_OK|WP_TYPE_HOMING|WP_TYPE_MAGAZINE| \
					 WP_TYPE_CONSECUTIVE|WP_TYPE_RELEASE_MOTION|WP_TYPE_BODY|WP_TYPE_TACTICAL|\
					 WP_TYPE_PUNCH_L|\
					 WP_TYPE_SHOOTHEIGHT2|\
					 WP_TYPE_BH_ATTACK|WP_TYPE_BH_JUMPOUT)
#define	PSG_TYPE	(WP_TYPE_INTRUDE_OK|WP_TYPE_MAGAZINE|WP_TYPE_CALLFUNC| \
					 WP_TYPE_SUBJECT|WP_TYPE_BODY)
#define	STG_TYPE	(WP_TYPE_CALLFUNC|WP_TYPE_SUBJECT|WP_TYPE_BODY)
#define	SPP_TYPE	(WP_TYPE_INTRUDE_OK|WP_TYPE_HOMING|WP_TYPE_MAGAZINE|\
					 WP_TYPE_PRESSURE|WP_TYPE_BODY|WP_TYPE_TACTICAL)
#if 0
#define	RGB_TYPE	(WP_TYPE_CONSECUTIVE|WP_TYPE_BODY|WP_TYPE_PUNCH_L|\
					 WP_TYPE_BH_ATTACK|WP_TYPE_BH_JUMPOUT|WP_TYPE_MAGAZINE)
#endif
#define	RGB_TYPE	(WP_TYPE_PRESSURE|WP_TYPE_BODY|WP_TYPE_PUNCH_L|\
					 WP_TYPE_SHOOTHEIGHT2|WP_TYPE_INTRUDE_OK|\
					 WP_TYPE_BH_ATTACK|WP_TYPE_BH_JUMPOUT|WP_TYPE_MAGAZINE)
#define	NIKITA_TYPE	(WP_TYPE_NIKITA|WP_TYPE_PRESSURE|WP_TYPE_BODY|WP_TYPE_PUNCH_LL|\
					 WP_TYPE_SHOOTHEIGHT2|WP_TYPE_INTRUDE_OK|\
					 WP_TYPE_MODE1FUNC)
#define	CLAY_TYPE	(WP_TYPE_PRESS|WP_TYPE_MODE1FUNC|WP_TYPE_STICK|WP_TYPE_MAXCHECK|\
					 WP_TYPE_ZERO_TO_NONE|WP_TYPE_PUNCH_M)
#define	C4_TYPE		(WP_TYPE_PRESS|WP_TYPE_MODE1FUNC|WP_TYPE_STICK|WP_TYPE_MAXCHECK|\
					 WP_TYPE_ZERO_TO_NONE)
#define	CHAFF_TYPE	(WP_TYPE_PRESS|WP_TYPE_RUNNING|WP_TYPE_PUNCH_M|\
					 WP_TYPE_GRENADE|WP_TYPE_BH_ATTACK|WP_TYPE_ZERO_TO_NONE|\
					 WP_TYPE_INTRUDE_OK)
#define	STUN_TYPE	(WP_TYPE_PRESS|WP_TYPE_RUNNING|WP_TYPE_PUNCH_M|\
					 WP_TYPE_GRENADE|WP_TYPE_BH_ATTACK|WP_TYPE_ZERO_TO_NONE|\
					 WP_TYPE_INTRUDE_OK)
#define	MAGAZINE_TYPE	(WP_TYPE_PRESS|WP_TYPE_RUNNING|WP_TYPE_PUNCH_S|\
						 WP_TYPE_GRENADE|WP_TYPE_BH_ATTACK|WP_TYPE_ZERO_TO_NONE|\
						 WP_TYPE_INTRUDE_OK)
#define	GRENADE_TYPE	(WP_TYPE_PRESS|WP_TYPE_RUNNING|WP_TYPE_PUNCH_M|\
						 WP_TYPE_GRENADE|WP_TYPE_BH_ATTACK|WP_TYPE_ZERO_TO_NONE|\
						 WP_TYPE_INTRUDE_OK)
#define	SPRAY_TYPE		(WP_TYPE_SUBJECT|WP_TYPE_CALLFUNC|WP_TYPE_INTRUDE_OK|WP_TYPE_ONLY_ONE)
#define	BLADE_TYPE		(WP_TYPE_NORMAL|WP_TYPE_ONLY_ONE|WP_TYPE_BLADE|WP_TYPE_PUNCH_M)

#define	MIC_TYPE		(WP_TYPE_INTRUDE_OK|WP_TYPE_ONLY_ONE|WP_TYPE_SUBJECT|\
						 WP_TYPE_CALLFUNC)

#define	AKS_TYPE		FAMAS_TYPE
#define	M4_TYPE			FAMAS_TYPE
#define	BOOK_TYPE		CLAY_TYPE

/* アイテムタイプ */

enum {
    IT_TYPE_NORMAL	= 0x0000,
    IT_TYPE_PRESS	= 0x0001,	/* ボタン押しで発動 */	
    IT_TYPE_INTRUDE_OK	= 0x0002,	/* イントルード中も可 */
    IT_TYPE_SUBJECT	= 0x0004,	/* 主観アイテム */
	IT_TYPE_CBBOX =	  0x0008,	/* ダンボール */
    IT_TYPE_CALLFUNC	= 0x0020,	/* モード変更 */

	IT_TYPE_FLAMEVANISH = 0x0100,	/* 装備で体炎が消える */
	IT_TYPE_MUGEN =		  0x0200,	/* 無限系 */

	IT_TYPE_ONLY_ONE	= 0x8000,	/* 一個のみ */
} ;

#define	SCOPE_TYPE	(IT_TYPE_CALLFUNC | IT_TYPE_SUBJECT | IT_TYPE_ONLY_ONE)
#define	C_BOX_TYPE	(IT_TYPE_CALLFUNC | IT_TYPE_CBBOX | IT_TYPE_FLAMEVANISH)
#define	CIGAR_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE)
#define	DGCAMERA_TYPE	(IT_TYPE_CALLFUNC | IT_TYPE_SUBJECT | IT_TYPE_ONLY_ONE)
#define	MINEDT_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE)
#define	BSENSA_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE)
#define	BSENSB_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE)
#define	JACKET_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE)
#define	UNIFORM_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE | IT_TYPE_FLAMEVANISH)
#define	GOGGLES_TYPE	(IT_TYPE_NORMAL | IT_TYPE_ONLY_ONE)
#define	BANDANA_TYPE	(IT_TYPE_ONLY_ONE|IT_TYPE_MUGEN)
#define	MUGENWIG_TYPE	BANDANA_TYPE
#define	WIG_TYPE		(IT_TYPE_ONLY_ONE)

#define	IT_TYPE_ABNORMAL	(IT_TYPE_SUBJECT | IT_TYPE_CBBOX)

/*----------------------------------------------------

  発射トリガー 

-----------------------------------------------------*/

enum {
    TRIG_YET = 0,			/* 通常 */
    TRIG_SET,				/* 構え */
    TRIG_FIRE,				/* 発射、設置 */
	TRIG_FALL,				/* 落とす */
	TRIG_MAG_FALL,			/* マガジン落とす */
	TRIG_MAG_RIGHT_HAND,	/* マガジン右手もち */
	TRIG_MAG_LEFT_HAND,		/* マガジン左手もち */
	TRIG_MAG_INVISIBLE,		/* マガジン不可視 */
	TRIG_MAG_LEFT_HAND_AND_FALL,
	TRIG_MAG_LEFT_HAND2,
	TRIG_FIRE_C4ENEMY,		/* 敵にＣ４設置特殊 */
	TRIG_FIRE_C4WALL,		/* 壁にＣ４設置特殊 */
	TRIG_FIRE_LEVEL0,		/* レベルつき発射 */
	TRIG_FIRE_LEVEL1,
	TRIG_FIRE_LEVEL2,
	TRIG_FIRE_LEVEL3,
	TRIG_FIRE_LEVEL4,
	TRIG_OPEN_START,		/* 雑誌開き開始 */
	TRIG_RELOAD_START,		/* リロード開始（ＲＧＢ専用） */
	TRIG_RELOADING,			/* リロード中（ＲＧＢ専用） */
	TRIG_RGB_AMOGRIP_INVISIBLE,	/* マガジングリップ非表示（ＲＧＢ専用） */
} ;

/* ブレード用 */
enum {
	BLD_TRIG_NONE =			0,
	BLD_TRIG_PULLOUT =		1,
	BLD_TRIG_PUTBACK =		2,
	BLD_TRIG_SLASH =		3,
	BLD_TRIG_GUARD_SLASH =	4,
	BLD_TRIG_CHANGE =		5,
	BLD_TRIG_THRUST =		6,
	BLD_TRIG_SLASH_STRONG =	7,

	BLD_TRIG_FIRE		   =			0x4000,
	BLD_TRIG_FIRE_NO_GUARD =			0x8000,
} ;

#define BLD_TRIG_FIRE_ALL	(BLD_TRIG_FIRE|BLD_TRIG_FIRE_NO_GUARD)

/*----------------------------------------------------

  プレイヤーデバッグモード GM_PlayerDebugMode

-----------------------------------------------------*/

enum {
    GM_PDM_WEAPON_CAMERA = 0,
    GM_PDM_DEBUG_ARM,
//    GM_PDM_GUN_FAIRY_COLOR,
//    GM_PDM_GUN_FAIRY_RANGE,
//    GM_PDM_CH_CAMERA,
    GM_PDM_NOBEHIND,
    GM_PDM_NOGAGE,
//    GM_PDM_TRAPVIEW,
//    GM_PDM_BEHINDVIEW,
//    GM_PDM_SEGMENTVIEW,
//	GM_PDM_RSEGMENTVIEW,
//    GM_PDM_TARGETVIEW,
//    GM_PDM_ZONEVIEW,
//	GM_PDM_ONLINEINFO,
//	GM_PDM_FINDPOSVIEW,
    GM_PDM_ADJUST,
	GM_PDM_NOTHING,
    MAX_PLAYER_DEBUG_MODE,
} ;

/* 昔あったのよ */
#define	GM_PDM_GUN_LIGHT_COLOR	GM_PDM_DEBUG_ARM

/*----------------------------------------------------

  新デバッグメニュー PlayerDebugMenuStatus

-----------------------------------------------------*/
enum {
	PDMS_NONE			=		0x00000000,
	PDMS_TRAPVIEW		=		0x00000001,
	PDMS_BEHINDVIEW		=		0x00000002,
	PDMS_HAZARDVIEW		=		0x00000004,
	PDMS_ZONEVIEW		=		0x00000008,
	PDMS_ONLINEINFO		=		0x00000010,
	PDMS_TARGETVIEW		=		0x00000020,
	PDMS_FINDPOSVIEW	=		0x00000040,	
	PDMS_STATUSVIEW		=		0x00000080,
	PDMS_GSTATUSVIEW	=		0x00000100,
	PDMS_MUTEKIMODE1	=		0x00000200,
	PDMS_MUTEKIMODE2	=		0x00000400,
	PDMS_DISPLAY_OFF	=		0x00000800,
	PDMS_HAZARDCHECK	=		0x00001000,
	PDMS_HAZARDCHECK_R	=		0x00002000,
	PDMS_BLOODFLOORCHK	=		0x00004000,
	PDMS_BLOODEVERY		=		0x00008000,
	PDMS_RESULTVIEW		=		0x00010000,
	PDMS_DEEPSTEALTH	=		0x00020000,
} ;

/*----------------------------------------------------------------

  構造体 
 
----------------------------------------------------------------*/

/* ボム系管理 */
typedef	struct _PL_BOMB	{
	struct _PL_BOMB		*next ;
	int					weapon ;
	int					flag ;
	FVECTOR				*mov ;
	FVECTOR				*step ;
} GM_BOMB ;

enum {
	GM_BMB_FLAG_NOTHING =		0x0000,
	GM_BMB_FLAG_DESTROY =		0x0001,		/* 終了命令 */
	GM_BMB_FLAG_INVISIBLE =		0x0002,		/* 非表示命令 */
	GM_BMB_FLAG_NOBLAST =		0x0004,		/* 爆発禁止 */
	GM_BMB_FLAG_FALL =			0x0008,		/* その場で床につくまで落下 */
	GM_BMB_FLAG_BLAST =			0x0010,		/* 強制爆破 */
	GM_BMB_FLAG_NOCONTROL =		0x0020,		/* 操作不能（ニキータ特殊） */
	GM_BMB_FLAG_MGN_IN =		0x0040,		/* メカゲノラ体内 */
} ;

/* 設置武器ワーク */
typedef	struct	{
	u_int			trigger ;		/* トリガー */
	TARGET			*target ;		/* 設置ターゲット */
	CAPTURE_TARGET	*capture ;		/* 設置つかみターゲット */
	int				reserved ;
	HZX_HZD			hzd ;			/* 設置ハザード（実体） */
} PL_StickWPWork ;

/* Ｃ４設置タイプ */
enum {
	C4_TYPE_NONE = -1,
    C4_TYPE_SEGMENT = 0,	
    C4_TYPE_FLOOR,
	C4_TYPE_CHARA,
	C4_TYPE_CHARA2,	/* cpture target 無いキャラ */
} ;

/*------------------------------------------------------------------

  外部変数宣言 

------------------------------------------------------------------*/

/* user/sonoyama/raiden/rai_init.c */
extern	int	GM_PlayerDebugMode ;	/* デバッグ用 */
extern	CONTROL	*GM_PlayerControl ;	/* ＣＯＮＴＲＯＬ */
extern	OBJECT	*GM_PlayerBody ;	/* ＯＢＪＥＣＴ */
extern	OBJECT	*GM_PlayerArmBody ;	/* 主観腕のＯＢＪＥＣＴ */
extern	OBJECT	*GM_PlayerWeaponBody ;	/* 武器のＯＢＪＥＣＴ */
extern	OBJECT	*GM_PlayerSubWeaponBody ;	/* 主観武器のＯＢＪＥＣＴ */
extern	OBJECT	*GM_PlayerEquipBody ;	/* 装備品オブジェクト（ダンボール以外） */
extern	OBJECT	*GM_PlayerBoxBody ;		/* ダンボールＯＢＪＥＣＴ */
extern	int		GM_PlayerWeaponModel ;	/* 武器のモデル名 */

extern	long64	GM_PlayerDamagedWeaponType ;

extern	TARGET	*GM_PlayerTarget ;	/* ＴＡＲＧＥＴ */
extern	FVECTOR	GM_PlayerPosition ;	/* == control->mov */
extern	FVECTOR	GM_PlayerFirstPosition ;	/* ステージ初期位置 */
extern	FVECTOR	GM_PlayerFindPos ;	/* 敵が発見する為の基本ポイント */
extern	u_int	GM_PlayerFindObj ;	/* 敵がチェックする体の部位（ビットフラグ） */
extern	int	GM_PlayerAddress ;	/* アドレス */
extern	long64	_GM_PlayerStatus ; 	/* プレイヤー状態 */
extern	long64	_GM_PlayerStatus2 ; 	/* プレイヤー状態（拡張） */
extern	short	GM_PlayerCautionDirs[] ;/* 張り付き時移動方向 */
extern 	int	GM_PlayerMap ; 		/* プレイヤーのいるマップ */
extern	int	GM_WeaponChanged ;	/* 武器変更があった */
extern	int	GM_ItemChanged ;	/* アイテム変更があった */
extern	int	GM_MagazineMax ;	/* マガジン最大値 */
extern	int	GM_Magazine ;		/* マガジン内弾数 */

extern	signed char GM_WeaponPrevChangedScn ;	/* シナリオで前武器を変更 */
extern	signed char	GM_ItemPrevChangedScn ;	/* シナリオで前装備を変更 */

extern	int	PL_EquipWeaponCode ;	/* 現在装備している武器のＳＴＲＣＯＤＥ（素手は１） */

extern	int	GM_VitalityAdjust ;	/* プレイヤーの体力を攻撃以外の外部要因で
								   変更する場合、これに加減算する */
extern	int	GM_VitalityAdjustNoDead ;	/* 上と同じだが、これによって死ぬことはない */
extern	int	GM_DiazepamCount ;	/* ジアゼパム有効時間 */
extern	int	GM_SubjectVMax[] ;	/* 主観上下幅最大 */
extern	int	GM_SubjectHMax[] ;	/* 主観左右幅最大 */
extern	int	GM_SubjectVStep ;	/* 主観上下回転速度 */
extern	int	GM_SubjectHStep ;	/* 主観左右回転速度 */

extern	int	GM_SubjectVMaxTmp[] ;	/* 主観上下幅最大（一時） */
extern	int	GM_SubjectHMaxTmp[] ;	/* 主観左右幅最大（一時） */
extern	int	GM_SubjectVStepTmp ;	/* 主観上下回転速度（一時） */
extern	int	GM_SubjectHStepTmp ;	/* 主観左右回転速度（一時） */

extern	float	GM_SubjectAngleTmp ;	/* 主観時カメラ画角（一時） */

extern	FVECTOR	PL_SubjectCameraShift ;	/* 主観カメラ位置シフト */

extern	u_char	GM_C4_Blast ;		/* Ｃ４爆破スイッチ */
extern	u_char	GM_C4_BlastAll ;	/* Ｃ４全消去スイッチ */
extern	u_char	GM_N_C4Bombs ;		/* 設置されたＣ４数 */
extern	u_char	GM_N_Claymores ;	/* クレイモア設置数 */
extern	u_char	GM_N_Books ;		/* 雑誌設置数 */
extern	u_char	GM_N_Grenades ;		/* 投擲グレネード数 */

extern	int	GM_CurrentCameraChanl ;	/* プレイヤーのいる画面（対戦用） */

extern	int	GM_NikitaAlive[] ;	/* ニキータ存在フラグ */
extern	short		GM_NikitaLife ;		/* ニキータ爆発までのカウンター */
extern	short		GM_NikitaLifeMax ;		/* ニキータ爆発までのカウンター最大 */
extern	FVECTOR		GM_NikitaPosition[] ;	/* ニキータ飛行位置 */
extern	void		*GM_NikitaMapConnection ;	/* ニキータ用マップ接合 */

extern	FVECTOR		GM_StingerPosition ;	/* スティンガー関係 */
extern	int			GM_N_RGB6S ;
extern	FVECTOR		GM_RGB6Position[ GM_CURRENT_RGB6_MAX ] ;  		/* ＲＧＢ−６ */

extern	int			GM_WeaponAlive ;	/* 特殊武器の存在フラグ */
extern	signed char	GM_WeaponFire ;		/* 武器発射トリガー */

extern	float	PL_PlayerFloorHeight ;

extern	u_char		PL_PluginPreCheck ;

extern	FVECTOR		GM_MissileBoundMin ;
extern	FVECTOR		GM_MissileBoundMax ;

/* 特殊武器存在フラグのenum */
/* 発射不可フラグにも使ってます */
enum {
	WP_ALIVE_NONE =			0x0000,
	WP_ALIVE_STINGER =		0x0001,
	WP_ALIVE_RGB6 =			0x0002,
	WP_ALIVE_NIKITA =		0x0004,

	WP_CANNOT_FIRE_RGB6 =		0x0100,
	WP_CANNOT_FIRE_STINGER =	0x0200,
} ;

extern	int		GM_PadDemoVersion ;		/* パッドデモバージョン */

extern	int			GM_N_PlayerShadowPos ;	/* 影チェック位置数 */
extern	FVECTOR		GM_PlayerShadowPos[] ; 	/* 影チェック位置 */

#define	GM_PlayerCautionDir 	GM_PlayerCautionDirs[ 0 ]

extern	int	   	PL_DGCam_N_CheckSquares ;
extern	int		PL_DGCam_N_CheckCharas ;
extern	int		PL_DGCam_ShutterProc ;

extern	int		PL_USP_LightOn ;

extern	int		PL_Bul_Grenade_Flag ;

extern	GM_BOMB		GM_BombList ;

extern	PL_MOVE_FLOOR_FUNC		PL_MoveFloorFunc ;
extern	PL_FALLBULLET_FUNC		PL_FallBulletFunc ;
extern	PL_ELUDEDISTURB_FUNC	PL_EludeDisturbFunc ;

extern	PL_BULLETSPLASH_FUNC	PL_BulletSplashFunc ;
extern	PL_BLASTWATER_FUNC		PL_BlastWaterFunc ;

extern	long64	PL_MenuNoUseWeapon ;
extern	long64	PL_MenuNoUseItem ;
extern	long64	PL_MenuNoUseWeaponScn ;
extern	long64	PL_MenuNoUseItemScn ;
extern	int		PL_MenuNoUseWeaponType ;
extern	int		PL_MenuNoUseItemType ;

extern	float	PL_JumpGravity ;

extern	DG_OBJS	*PL_GolUniformObjs ;
extern	DG_OBJS	*PL_GolCapObjs ;
extern	DG_OBJS	*PL_ShadowHairObjs ;

/* モーションブラー用DG_OBJS */
extern	DG_OBJS		*PL_ActionBlurObjs ;
extern	void		*PL_ActionBlurParent ;

/* 保険 */
#define	GM_PlayerStatus		(_GM_PlayerStatus)

/*------------------------------------------------------------------

  インライン

------------------------------------------------------------------*/

EXTERN_INLINE	void	GM_InitPlayerStatus( void )
{
	_GM_PlayerStatus = I64(0) ;
	_GM_PlayerStatus2 = I64(0) ;
}

EXTERN_INLINE	long64	GM_CheckPlayerStatusEX( long64 state1, long64 state2 )
{
	ASSERT( !( state1 & PLAYER_STATUS_EX ) && ( state2 == 0 || ( state2 & PLAYER_STATUS_EX ) ) ) ;
	return ( ( _GM_PlayerStatus & state1 ) | ( _GM_PlayerStatus2 & state2 ) ) ;
}

#define	GM_CheckPlayerStatus( _s )	GM_CheckPlayerStatusEX( _s, I64(0) )

EXTERN_INLINE	void	GM_SetPlayerStatusEX( long64 state1, long64 state2 )
{
	ASSERT( !( state1 & PLAYER_STATUS_EX ) && ( state2 == I64(0) || ( state2 & PLAYER_STATUS_EX ) ) ) ;
	_GM_PlayerStatus |= state1 ;
	_GM_PlayerStatus2 |= ( state2 & ~PLAYER_STATUS_EX ) ;
}

#define	GM_SetPlayerStatus( _s )	GM_SetPlayerStatusEX( _s, I64(0) ) 

EXTERN_INLINE	void	GM_ResetPlayerStatusEX( long64 state1, long64 state2 )
{
	ASSERT( !( state1 & PLAYER_STATUS_EX ) && ( state2 == I64(0) || ( state2 & PLAYER_STATUS_EX ) ) ) ;
	_GM_PlayerStatus &= ~state1 ;
	_GM_PlayerStatus2 &= ~( state2 & ~PLAYER_STATUS_EX ) ;
}

#define	GM_ResetPlayerStatus( _s )	GM_ResetPlayerStatusEX( _s, I64(0) ) 

#ifdef PAL
static	inline	float	PL_PalAdj( float y )
{
	return ( y * 256.0F / 224.0F ) ;
}

static	inline	float	PL_PalAdjR( float y )
{
	return ( y * 224.0F / 256.0F ) ;
}

static	inline	void	PL_PalAdjV( FVECTOR *v )
{
	v->vx = PL_PalAdj( v->vx ) ;
	v->vy = PL_PalAdj( v->vy ) ;
	v->vz = PL_PalAdj( v->vz ) ;
}

static	inline	void	PL_PalAdjVR( FVECTOR *v )
{
	v->vx = PL_PalAdjR( v->vx ) ;
	v->vy = PL_PalAdjR( v->vy ) ;
	v->vz = PL_PalAdjR( v->vz ) ;
}
#else
#define	PL_PalAdj( _y )
#define	PL_PalAdjR( _y )
#define	PL_PalAdjV( _v )
#define	PL_PalAdjVR( _v )
#endif

/* メニューＮＯ＿ＵＳＥ */
static	inline	void	PL_SetNoUseWeapon( int weapon )
{
	PL_MenuNoUseWeapon |= ( I64(1) << weapon ) ;
}

static	inline	void	PL_SetNoUseItem( int item )
{
	PL_MenuNoUseItem |= ( I64(1) << item ) ;
}

static	inline	void	PL_SetNoUseWeaponType( int type )
{
	PL_MenuNoUseWeaponType |= type ;
}

static	inline	void	PL_SetNoUseItemType( int type )
{
	PL_MenuNoUseItemType |= type ;
}

static	inline	void	PL_ResetNoUseWeapon( int weapon )
{
	PL_MenuNoUseWeapon &= ~( I64(1) << weapon ) ;
}

static	inline	void	PL_ResetNoUseItem( int item )
{
	PL_MenuNoUseItem &= ~( I64(1) << item ) ;
}

static	inline	void	PL_ResetNoUseWeaponType( int type )
{
	PL_MenuNoUseWeaponType &= ~type ;
}

static	inline	void	PL_ResetNoUseItemType( int type )
{
	PL_MenuNoUseItemType &= ~type ;
}

/*------------------------------------------------------------------

  外部関数宣言 

------------------------------------------------------------------*/

extern	int	PlayerDebugMenuStatus ;

extern	int		PL_CheckNoUseWeapon( int weapon ) ;
extern	int		PL_CheckNoUseItem( int item ) ;

extern	int	PL_GetPlayerItem( void ) ;	/* 現在装備中のアイテム */
extern	int	PL_GetPlayerWeapon( void ) ;	/* 現在装備中の武器 */
extern	int	PL_PadEnable( void ) ;		/* パッドが操作可能かチェック */
extern	int	PL_AttackDisable( void ) ;	/* 攻撃の無効状態チェック */
extern	int	PL_PlayerSetInvincible( void ) ;	/* 無敵にする */
extern	int	PL_PlayerResetInvincible( void ) ;	/* 無敵を解除する */
extern	void	PL_SetMagazine( int, int ) ;	/* マガジンセット */
extern	int		PL_DecrementMagazine( void ) ;
//extern	int	PL_CheckIntoPlayerCamera( DG_OBJS *, float ) ;
extern	void		PL_ShukanReverse( float *dy, short *rx ) ;

extern	void	*PL_BombListControl( void ) ;
extern	int		PL_CheckBulletSplash( FVECTOR *mov0, FVECTOR *step0, float, float ) ;
extern	void	GM_InitBombList( GM_BOMB *this_bomb, int weapon, FVECTOR *mov, FVECTOR *step ) ;
extern	void	GM_AddBombList( GM_BOMB	*this_bomb ) ;
extern	int		GM_RemoveBombList( GM_BOMB *this_bomb ) ;

extern	void	PL_GetNextFV( FVECTOR * ) ;
extern	void	PL_GetNextSV( SVECTOR * ) ;

extern	int		PL_GetOptionFV( char , FVECTOR * ) ;
extern	int		PL_GetOptionSV( char , SVECTOR * ) ;

extern	void	PL_InvisiblePlayer( void ) ;
extern	void	PL_VisiblePlayer( void ) ;

extern	void	PL_VisibleObjsAll( DG_OBJS *objs, int chanl ) ;
extern	void	PL_InvisibleObjsAll( DG_OBJS *objs, int chanl ) ;

/* 移動床関数実行 */
extern	int		PL_ExecMoveFloorFunc( FVECTOR *npos, HZX_GROUP_ID hzx_id, FVECTOR *pos, 
									  HZX_EVT *evt, int name ) ;

/*ゲージ関連 */
extern	void	GM_AppendGageSet( GM_GageSet * ) ;
extern	void	GM_RemoveGageSet( GM_GageSet * ) ;
extern	void	GM_InitGageSet( GM_GageSet *, char *, int, int, int, int,
			        int, int, int, int ) ;
extern	void	GM_InitGageSet2( GM_GageSet *, char *, int, int,
			        int, int, int, int ) ;
extern	void	GM_SetGageColor( GM_GageSet *, u_char, u_char, u_char, u_char,
				 u_char, u_char, u_char, u_char,
				 u_char, u_char, u_char, u_char ) ;
extern	void	GM_SetGageColorType( GM_GageSet *gs, int type ) ;

extern	void	GM_VisibleGage( GM_GageSet * ) ;
extern	void	GM_InvisibleGage( GM_GageSet * ) ;

extern	void	GM_InitGageM9( GM_GageSet *gs, int value, int max, int min, int delay ) ;
extern	void	GM_SetGageColorM9( GM_GageSet *gs, 
								  u_char left_r, u_char left_g, u_char left_b, 
								  u_char right_r, u_char right_g, u_char right_b, 
								  u_char delay_r, u_char delay_g, u_char delay_b ) ;

/*武器発射ステートをセット */
extern	void	GM_SetWeaponFire( int wp ) ;
extern	void	GM_ResetWeaponFire( void ) ;

extern	void	PL_VisibleHead( int mode, DG_OBJS *objs, int chanl ) ;

/* 汎用ブッブー */
static	inline	void	GM_Buzzer( void )
{
	GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_BUZZER01 ) ;
}

static	inline	void	GM_BuzzerPos( FVECTOR *pos )
{
	//GM_SeSetMode( SD_S_BUZZER01, pos, GM_SEMODE_BOMB ) ;
   int pan;
   float bp_angle;
   pan = GM_SeGetPan( pos, GM_SEMODE_BOMB, &bp_angle );
	GM_SeSet3D( pan, GM_MAX_VOL, SD_S_BUZZER01, bp_angle ) ;
}

/* Ｃ４つけかえ */
extern	void		GM_ChangeC4Body( TARGET *old, TARGET *new_trg ) ;

#define	Ply_GetPlayerItem() 	PL_GetPlayerItem()
#define	Ply_GetPlayerWeapon() 	PL_GetPlayerWeapon()
#define	PL_CurrentItem()		PL_GetPlayerItem()
#define	PL_CurrentWeapon()		PL_GetPlayerWeapon()

/* パッド振動 */
extern	void		*NewPadVibration2( int file, int playtime ) ;
extern	void		*NewPadVibration3( u_char *script, int *ctrl ) ;
//extern	void		*NewPadVibration( char *script, int type ) ;

/* 主観時はカメラ位置から出るＳＥセット */
extern	void	PL_SeSetSubject( int se, FVECTOR *pos, int mode ) ;

#ifdef __cplusplus
}
#endif
	
#endif
