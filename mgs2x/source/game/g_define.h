/*
   g_define.h
	定数宣言
 	

   1999/07/07 M.Sonoyama
   $Id: g_define.h,v 1.3 2003/01/05 05:02:09 Yoshizawa1 Exp $

   *** PROJECT OF METAL GEAR SOLID2 ***
*/

#ifndef _g_define_h_
#define _g_define_h_

/* π */
#define		PI	(3.141593653589793F)
#define		TPI	(2.0F * PI)

/*---------------------------------------------------------------*/

/* ＳＴＲＣＯＤＥ */
#define	GM_STRCODE_ON		(3662)		/* "on" */
#define	GM_STRCODE_OFF		(117030)	/* "off" */

#define	GM_STRCODE_ENTER	(6753643)	/* 入る */
#define	GM_STRCODE_LEAVE	(6411627)	/* 出る */
#define	GM_STRCODE_INSIDE	(5547371)	/* いる */

/*---------------------------------------------------------------*/

/* マップ関係 */

#define GM_MAP_CHANL_MAX	4	// map.cで管理するDGのチャンネルの最大数

#define GM_MAP_CHANL_MAIN	0	// プレイヤーが使うチャンネル
#define GM_MAP_CHANLSUBWIN	1	// 子画面で使用するチャンネル

#define GM_INVALID_ADDR		0xffffffff

/*---------------------------------------------------------------*/

#define GM_MAX_RESULT_COUNT	(30000)	// リサルトカウント系のＭＡＸ

/* GM_AlertLevel */
#define	ALERT_LEVEL_MAX	DIRECT_TICK(1024)
#define	AVOID_LEVEL_MAX	DIRECT_TICK(1024)
#define SEARCH_LEVEL_MAX DIRECT_TICK(60*60)

/* GM_JammingLevel */
#define	JAMMING_LEVEL_MAX	1024

/* GM_AlertMode */
enum {
	ALERT_MODE_SNEAK = 0,	/* 潜入モード */
	ALERT_MODE_ALERT,		/* 危険モード */
	ALERT_MODE_AVOID,		/* 回避モード */
	ALERT_MODE_SEARCH		/* 探索モード */
} ;

/* GM_NoisePower */
enum {
	NOISE_ZERO ,
	NOISE_LOCKER_CLOSE ,	/* ロッカー閉まる音 */
	NOISE_MIC_QUEST ,		/* マイクで質問 */
	NOISE_HOLD ,
	NOISE_DEC ,	/* 拳銃デコック */
	NOISE_SS ,	/* 至近距離なら反応で白？ */
	NOISE_S,
	NOISE_CHOUDAN,	/* 跳弾御 */
	NOISE_SCREAM,
	NOISE_MM,
	NOISE_M,
	NOISE_L,
	NOISE_LL
} ;

/*ノイズマップ修正01.8.24*/
#define GMDEF_NOISE_MAP_RENEW	(1)

/* GM_NoiseStatus */
enum {
	NOISE_STATUS_NONE ,
	NOISE_STATUS_MAG ,	/*  マガジンの音 */
} ;
/*---------------------------------------------------------------*/

/* 言語 ( GM_Language ) */
/* あえてアメリカとイギリスを分けてみた */
/* （今は中身は一緒） */
enum {
//	GM_LANG_JAPANESE			= 0,	/* 日本 */
	GM_LANG_DEFAULT				= 0,	/* デフォルト */
	GM_LANG_ENGLISH				= 1,
	GM_LANG_ENGLISH_USA			= 1,	/* 亜米利加 */
	GM_LANG_ENGLISH_ENGLAND		= 1,	/* 英吉利 */
	GM_LANG_FRENCH				= 2,	/* 仏蘭西 */
	GM_LANG_GERMANY				= 3,	/* 独逸 */
	GM_LANG_ITALY 				= 4,	/* 伊太利亜 */
	GM_LANG_SPANISH				= 5,	/* 西班牙 */
	GM_LANG_KOREAN				= 6,	/* 韓国 */
	GM_LANG_JAPANESE			= 7,	/* 日本 */
} ;

/* ゲームレベル GM_GameLevel */
enum {
	LEVEL_VERY_EASY_JPN 	= 10,
	LEVEL_EASY_JPN      	= 20,
	LEVEL_NORMAL_JPN    	= 30,
	LEVEL_HARD_JPN      	= 40,
	LEVEL_EXTREME_JPN   	= 50,
} ;

enum { 
	GM_LEVEL_VERYEASY 	= LEVEL_VERY_EASY_JPN, 
	GM_LEVEL_EASY 	  	= LEVEL_EASY_JPN,      
	GM_LEVEL_NORMAL		= LEVEL_NORMAL_JPN,
	GM_LEVEL_HARD		= LEVEL_HARD_JPN,      
	GM_LEVEL_EXTREME	= LEVEL_EXTREME_JPN,
	GM_LEVEL_E_EXTREME	= 60,
	GM_MAX_LEVELS
} ;

/*---------------------------------------------------------------*/

/* システムコンフィグ GM_Configuration */
enum {
	GM_CONFIG_VIBRATION_OFF 	= 	0x0001,	/* 振動ＯＦＦ */
	GM_CONFIG_CAPTION_OFF  		= 	0x0002,	/* 字幕ＯＦＦ */
	GM_CONFIG_RADAR_OFF 		= 	0x0004,	/* レーダーＯＦＦ */
	GM_CONFIG_BLOOD_OFF			= 	0x0008,	/* 血ＯＦＦ */
	GM_CONFIG_CUTSCENES_LETTERBOXED	= 	0x0010,	/* モノラル */  // BP CHANGE - was SOUND_MONAURAL
	GM_CONFIG_RADAR_OFF_INTRUDE = 	0x0020,	/* イントルード時レーダーＯＦＦ */
	GM_CONFIG_SHUKAN_REVERSE 	=	0x0040,	/* 主観操作反転 */
	GM_CONFIG_OLD_TYPE_MENU		=	0x0080,	/* 旧式メニュー */
	GM_CONFIG_WATERUD_REVERSE 	=	0x0100,	/* 水中上下操作反転 */
	GM_CONFIG_MENU_QCHANGE_EX	=	0x0200,	/* 拡張クイックチェンジ */
	GM_CONFIG_SOUND_5_1CHANL	=	0x0400,	/* 5.1chanlサウンド対応 */
	GM_CONFIG_END_IF_FOUND		=	0x0800,	/* 見つかったらゲームオーバー */

	GM_CONFIG_STORY_TANKER 	 	= 	0x1000,	/* タンカー編。ビットが立ってなければプラント編 */
	GM_CONFIG_TANKER_CLEARED	=	0x2000, /* このプレイでタンカー編をクリアしている */
	GM_CONFIG_PLAYTIME_STOP		=	0x4000, /* PLAYTIMEカウンタを止める */
} ;
/* システムコンフィグ GM_Configuration2 */
enum {
	GM_CONFIG_SE_VOLUME 	= 	0x0000000F,	/* 4 *//* SEボリューム 16段階(0:MAX->15:MINまで)(XBOX専用) */
	GM_CONFIG_MUSIC_VOLUME 	= 	0x000000F0,	/* 4 *//* MUSICボリューム 16段階(0:MAX->15:MINまで)(XBOX専用) */
	GM_CONFIG_CONTROLS		=	0x00000300, /* 2 *//* キーコンフィグ 4種類(XBOX専用)*/
	GM_CONFIG_PLATFORM		=	0x00000c00,	/* 2 *//* プラットフォーム 一応４種類 */
	GM_CONFIG_REGION		=	0x00007000,	/* 3 *//* 発売リージョン 一応８種類 */
	GM_CONFIG_DOGTAGS_2002	=	0x00008000, /* 1 *//* 新ドックタグ */
};

enum {
	GM_CONFIG_PLATFORM_PS2	= 0,
	GM_CONFIG_PLATFORM_XBOX	= 1,
	GM_CONFIG_PLATFORM_PC	= 2 
};

enum {
	GM_CONFIG_REGION_JAPAN	= 0,
	GM_CONFIG_REGION_US		= 1,
	GM_CONFIG_REGION_EU		= 2,
}; 

#define GM_CONFIG_CONTROLS_SHIFT	8
#define GM_CONFIG_PLATFORM_SHIFT	10
#define GM_CONFIG_REGION_SHIFT		12

#define GM_GET_CONFIG_CONTROLS()	( ( GM_Configuration2 & GM_CONFIG_CONTROLS ) >>  GM_CONFIG_CONTROLS_SHIFT )
#define GM_SET_CONFIG_CONTROLS( type )	( GM_Configuration2 = ( GM_Configutation2 & ~GM_CONFIG_CONTROLS ) | ( type << GM_CONFIG_CONTROLS_SHIFT ) )

/* クリアコードフラグ GM_ClearCodeFlag */
enum {
	GM_CLEAR_GET_MUGENBANDANA	=	0x0001,		/* 無限バンダナゲット */
	GM_CLEAR_GET_MUGENWIG		=	0x0002,		/* 無限カツラゲット */
	GM_CLEAR_GET_DGCAMERA		=	0x0004,		/* デジタルカメラゲット */
	GM_CLEAR_GET_O2WIG			=	0x0008,		/* Ｏ２無限カツラゲット */
	GM_CLEAR_GET_ELUDEWIG		=	0x0010,		/* エルード握力無限カツラゲット */
	GM_CLEAR_SPECIAL_ITEM_USED 	=	0x0020,		/* 特殊アイテム使ってクリア */
	GM_CLEAR_MUGENBANDANA_USED 	=	0x0100,		/* 無限バンダナ使った */
	GM_CLEAR_MUGENWIG_USED		=	0x0200,		/* 無限かつら使った */
	GM_CLEAR_WIG_A_USED			=	0x0400,		/* Ｏ２かつら使った */
	GM_CLEAR_WIG_B_USED			=	0x0800,		/* 握力かつら使った */
	GM_CLEAR_STEALTH_USED		=	0x1000,		/* ステルス使った */
	GM_CLEAR_RADAR_USED 		=	0x2000,		/* レーダー使った */
} ;

/* クリア後フラグ GM_ClearFlag */
enum {
	GM_CLEARED_TANKER				=	0x0001,		/* タンカークリア */	
	GM_CLEARED_PLANT				=	0x0002,		/* プラントクリア */
	GM_CLEARED_TANKER_PLANT			=	0x0004,		/* タンカー＆プラントクリア */
	GM_CLEARED_SUNGLASSES_PLAYING	=	0x0010,		/* サングラスモードプレイ */
	GM_CLEARED_SUNGLASSES_PLAYED	=	0x0020,		/* サングラスモードプレイ済 */
} ;

/* Ｗドッグタグリソース数 */
#define	DOGTAG_RES_NUM	(3)
/*---------------------------------------------------------------*/

/* ゲームステータス GM_GameStatus */
enum {
    STATE_DETECT	=		0x00000001,	/* 発見～危険モードの間 enemy/command.c のみ操作する */
    STATE_CLEARING	=		0x00000002,	/* 発見～危険モードの間 enemy/command.c のみ操作する */
    STATE_BIG_SNORE	=		0x00000004,	/* でかいいびき enemy/command.c のみ操作する */
    STATE_ENE_SIGHTIN	=	0x00000008,	/* 敵兵に見られている enemy/command.c のみ操作する */
	STATE_CHAFF		=		0x00000010,		/* チャフ発動中 */
	STATE_STUN		=		0x00000020,		/* スタングレネード発動中 */
	STATE_CUT_IN	=		0x00000040,		/* カットインカメラ中 */
	STATE_RADAR_JAMMING	=	0x00000080,		/* レーダー電波障害 */
	STATE_PAUSE_DISABLE	=	0x00000200,	/* ポーズできない */
	STATE_VIB_PAUSE0	=	0x00001000,		/* ０レベル振動ＯＦＦ */
	STATE_VIB_PAUSE1	=	0x00002000,		/* １レベル振動ＯＦＦ */
	STATE_DISP_GAMEOVER	=	0x00004000,	/* ゲームオーバー画面中 */
	STATE_VR_ONLY		=	0x00400000,	/*VR中であるアナザーではない*/
	STATE_VR_ANOTHER	=	0x00800000,	/*VRまたはアナザーミッション中である*/
	STATE_BOSS_SURVIVAL	=	0x01000000,	/* ボスサバイバルモード中 */
	STATE_GLL			=	0x02000000,	/* VSゴルルゴン中 */
	STATE_GNO			=	0x04000000,	/* VSゲノラ中 */
	STATE_SCN_DEMO	=		0x08000000,		/* シナリオデモ */
    STATE_DEMO	=			0x10000000,		/* ポリゴンデモ中		*/
	STATE_PRG_DEMO	=		0x20000000,		/* プログラムデモ中 	*/
	STATE_PAD_DEMO	=		0x40000000,		/* パッドデモ中 */
	STATE_GAMEOVER	=		0x80000000,		/* ゲームオーバー処理中 */
} ;

#define	GM_STATUS_DETECT	STATE_DETECT
#define	STATE_PLAY_DEMO		(STATE_DEMO|STATE_PRG_DEMO|STATE_PAD_DEMO|STATE_SCN_DEMO)

#define	STATE_JAMMING		(STATE_CHAFF|STATE_RADAR_JAMMING)

/* 廃止 */
#define	STATE_MENU_INVISIBLE_PRG	(0)
#define	STATE_MENU_INVISIBLE_SCN	(0)
#define	STATE_RADAR_INVISIBLE		(0)
#define	STATE_MENU_DISABLE			(0)
#define	STATE_GAGE_INVISIBLE		(0)

/*---------------------------------------------------------------*/

/* メニューステータス GM_MenuStatus */
enum {
	MENU_NORMAL 				= 0x00000000,	
	/* 非表示命令系 */
	MENU_WEAPON_OFF 			= 0x00000001,	/* 武器メニュー非表示 */
	MENU_ITEM_OFF				= 0x00000002,	/* アイテムメニュー非表示 */
	MENU_RADAR_OFF				= 0x00000004,	/* レーダー非表示 */
	MENU_GAGE_OFF				= 0x00000008,	/* ゲージ非表示 */
	MENU_CAPTION_OFF			= 0x00000010,	/* 字幕非表示 */
	MENU_SUBWIN_OFF				= 0x00000020,	/* 子画面非表示 */
	/* 状態チェック系 */
	MENU_WEAPON_OPEN			= 0x00000100,	/* 武器メニューが開いている */
	MENU_ITEM_OPEN				= 0x00000200,	/* アイテムメニューが開いている */
	MENU_RADIO_ON				= 0x00000400,	/* 無線機中 */
	MENU_RADAR_ON				= 0x00000800,	/* レーダーが表示されている */
	MENU_GAGE_ON				= 0x00001000,	/* ゲージが表示されている */
	MENU_SUBWIN_ON				= 0x00002000,	/* 子画面表示中 */
	MENU_NODE_ACCESSED			= 0x00004000,	/* ノードアクセス済 */
	MENU_NODE_ON				= 0x00008000,	/* ノード表示中 */
	/* 不許可系 */
	MENU_WEAPON_DISABLE			= 0x00010000,	/* 武器メニュー開き不可 */
	MENU_ITEM_DISABLE			= 0x00020000,	/* アイテムメニュー開き不可 */
	MENU_RADIO_DISABLE			= 0x00040000,	/* 無線不可 */
	MENU_VIBRATE_DISABLE 		= 0x00080000,	/* 振動不可 */
	/* メニューに影響を与える状態系 */
	MENU_STREAM_CH_0			= 0x00100000,	/* STREAM ch0 */
	MENU_STREAM_CH_1			= 0x00200000,	/* STREAM ch1 */
//	MENU_SCN_DEMO				= 0x00800000,	/* シナリオデモ */
	/* その他 */
	MENU_MENU_NEWPRESS			= 0x01000000,	/* メニューを開くのに押し直しが必要 */
} ;

#define	MENU_MENU_OFF			(MENU_WEAPON_OFF|MENU_ITEM_OFF)
#define	MENU_MENU_OPEN			(MENU_WEAPON_OPEN|MENU_ITEM_OPEN)
#define	MENU_MENU_DISABLE		(MENU_WEAPON_DISABLE|MENU_ITEM_DISABLE)

/*---------------------------------------------------------------*/
/* タイトルメニュー設定 GM_TitleMenuStatus */

enum {
	TITLE_MENU_STORY_SEL_MASK      = 0x0003, /* タイトルで選ばれたものを記録しておく */
	TITLE_MENU_STORY_SEL_1ST       = 0x0000, /* タイトルで"1ST TIME"が選ばれた */
	TITLE_MENU_STORY_SEL_TANKER    = 0x0001, /* タイトルで"TANKER"が選ばれた */
	TITLE_MENU_STORY_SEL_PLANT     = 0x0002, /* タイトルで"PLANT"が選ばれた */
	TITLE_MENU_STORY_SEL_TANDP     = 0x0003, /* タイトルで"TANKER & PLANT"が選ばれた */

	TITLE_MENU_TANKER_CLEARD       = 0x0004, /* タンカー編をクリア済み */
	TITLE_MENU_PLANT_CLEARD        = 0x0008, /* プラント編をクリア済み */

	TITLE_MENU_LOOKED_PREV_STORY_1 = 0x0010, /* 'Previous Story'の一つ目を既にみた */
	TITLE_MENU_LOOKED_PREV_STORY_2 = 0x0020, /* 'Previous Story'の二つ目を既にみた */

	TITLE_MENU_OPTION_CHANGED      = 0x0100, /* オプションが変更された */
	TITLE_MENU_SCRADJ_CHANGED      = 0x0200, /* スクリーンアジャストが変更された */
	TITLE_MENU_START_FROM_LOAD     = 0x0400, /* ロードして始めた場合のフラグ */
	TITLE_MENU_DISPED_MCWARNING    = 0x0800, /* タイトルの最初でメモリーカードのチェックをした */
	TITLE_MENU_DISPED_FIRST_MESSAGE = 0x1000, /* タイトルで哀悼文を表示済み */
	TITLE_MENU_DISPED_WARNING_VIOLENCE = 0x1000, /* タイトルで暴力表現の警告を表示済み */
	TITLE_MENU_SHORT_STORAGE = 0x2000, 		/* セーブ領域が足りない */
	TITLE_MENU_OPTION_CLEAR_MASK   = 0xff00, /* 上記五つのフラグをクリアするフラグ */
};


/*---------------------------------------------------------------*/

/* 弾丸タイプ */
enum {
    BUL_TYPE_VISIBLE = 0x0001,		/* 可視 */
    BUL_TYPE_SPARK 	 = 0x0002,		/* 跳弾する */
    BUL_TYPE_SPARK_SMOKE = 0x0004,	/* 跳弾煙でる */
    BUL_TYPE_BLAST = 0x0008,		/* 爆発 */
    BUL_TYPE_NO_HZD = 0x0010,		/* 当たり見ない */
    BUL_TYPE_SCAR = 0x0020,			/* 弾痕残る */
    BUL_TYPE_GREEN = 0x0040,		/* 軌跡緑色 */
    BUL_TYPE_NO_ATTACK = 0x0080,	/* 攻撃判定無し */
	BUL_TYPE_HIT_CHILD = 0x0100,	/* 子ターゲットに当たる */
	BUL_TYPE_NO_PLAYER = 0x0200,	/* この弾はプレイヤー以外が撃った */
    BUL_TYPE_RED =		 0x0400,	/* 軌跡赤色 */
    BUL_TYPE_NOT_XROT = 0x1000,		/* Ｘ回転なし */
	BUL_TYPE_NOISE_S = 0x2000,		/* 跳弾時ノイズＳ */
	BUL_TYPE_NOISE_SS = 0x4000,		/* 跳弾時ノイズＳＳ */	
} ;

#define	BUL_TYPE_NOISE	(BUL_TYPE_NOISE_S|BUL_TYPE_NOISE_SS)

/* 爆発タイプ */
enum {
	BLAST_TYPE_NORMAL = 	0x0000,
	BLAST_TYPE_NO_NOISE = 	0x0001,	/* ノイズ発生なし */
	BLAST_TYPE_NO_FIRE = 	0x0002,	/* 爆発エフェクトなし */
	BLAST_TYPE_ONETARG =	0x0004,	/* １つしかターゲットを発生しない */
	BLAST_TYPE_NO_SE =		0x0008,
	BLAST_TYPE_WALLTHROUGH=	0x0010,	/* 壁貫通タイプ */
	BLAST_TYPE_NO_PLAYER =	0x0020,
} ;

/*---------------------------------------------------------------*/

enum {
	GM_ITEM_BOX = 0,
	GM_WEAPON_BOX,
	GM_WEAPON_BODY_BOX,
} ;

enum {
	GM_ITEMGET_OK = 0,
	GM_ITEMGET_FULL,
	GM_ITEMGET_GET_WEAPON_FIRST,
	GM_ITEMGET_ALREADY,
} ;

/*---------------------------------------------------------------*/

/* 武器番号 （攻撃方法 ）*/
enum {
    WP_None = 0,
    WP_m92,			/* ベレッタＭ９（麻酔銃） */
    WP_Usp,			/* ＵＳＰ */
    WP_Socom,		/* ソーコム */
    WP_Psg1,		/* ＰＳＧ－１ （スナイパーライフル）*/
    WP_Rgb6,		/* ＲＧＢ＿６（Ｇランチャー） */
    WP_Nikita,		/* ニキータ */
    WP_Stinger,		/* スティンガー */
    WP_Claymore,	/* クレイモア地雷 */
    WP_C4Bomb,		/* Ｃ４爆弾 */
	/* 10 */
    WP_ChaffGrenade,	/* チャフグレネード */
    WP_StunGrenade,	/* スタングレネード */
    WP_Mic,			/* 指向性マイク */
    WP_Blade,		/* 高周波ブレード */
    WP_ColdSpray,	/* 爆弾凍結スプレー */
    WP_Aks,			/* AKS-74U */
    WP_Magazine,	/* マガジン */
    WP_Grenade,		/* グレネード */
	WP_m4,			/* Ｍ４ */
	WP_Psg1T,		/* ＰＳＧ１－Ｔ（麻酔） */
	/* 20 */
	WP_DemoMic,		/* 特殊マイク */
	WP_Book,		/* 雑誌 */
    MAX_WEAPONS,
	WP_Player,		/* プレイヤーの攻撃である事を明示 */
	WP_BladeFaint,	/* ブレード嶺撃ち */
	WP_BladeStab,	/* ブレード突き刺し */	
	WP_BladeGuard,	/* 敵用。ブレードで防御される攻撃 */
	WP_BoxRemove,	/* ダンボールをはがす攻撃 */
	WP_Kick1,		/* キック一発目（ライデン専用） */
} ;	

/* 武器以外の攻撃方法 */
enum {
    WP_Punch = 32,
    WP_Kick,
    WP_Throw,
    WP_Hang,
    WP_Gus,
    WP_Tumble,		/* 転ばし */
    WP_Body,
    WP_OnCorpse,
	/* 40 */
    WP_PunchR,
    WP_PunchL,
    WP_LaserSight,
    WP_ThrowG,
    WP_NoBlood,
	WP_PutC4,
    WP_StunFar,
    MAX_PLAYER_ATTACKS
} ;

/* プレイヤー以外の攻撃方法 */
enum {
    WP_Mkr = MAX_PLAYER_ATTACKS,
    WP_Stamp,
    WP_MecaBreak,
	/* 50 */
	WP_ShotGun_Near,
	WP_ShotGun_Far,
	WP_HeadMark,
	WP_BoxKick,
	WP_WallCrash,
	WP_Extinguisher,	/* 消化器破裂 */
	WP_NoGuard,			/* 無敵でも当たる */
	WP_NoRecover,		/* レーション装備でも回復不能 */
	WP_Steam,			/* 水蒸気攻撃 */
	WP_NoMuteki,		/* 無敵時間なし攻撃 */
	WP_NoPlayer,		/* プレイヤー外攻撃 */
} ;

/* 攻撃には関係ないもの */
/* （デモ呼び出しのときなどに使用） */
enum {
	WP_Glk = 128,
	WP_Abk, // アバカン
	WP_M4_Grd_Htc, // GRD付きＭ4
} ;

#define	TARGET_WP_ALL		I64(0x00000000ffffffff)
#define	TARGET_WP_NO_ALL	I64(0x7fffffff00000000)

/* 武器タイプ フラグ */
#define WP_NONE	    	(I64(1) << WP_None)		/* 素手 */
#define WP_M92		(I64(1) << WP_m92)		/* ベレッタＭ９（麻酔銃） */
#define	WP_USP		(I64(1) << WP_Usp)		/* ＵＳＰ */
#define	WP_SOCOM	(I64(1) << WP_Socom)		/* ソーコム */
#define	WP_PSG1		(I64(1) << WP_Psg1)		/* ＰＳＧ－１ （スナイパーライフル）*/
#define	WP_RGB6		(I64(1) << WP_Rgb6)		/* ＲＧＢ＿６（Ｇランチャー） */
#define	WP_NIKITA	(I64(1) << WP_Nikita)	/* ニキータ */
#define	WP_STINGER	(I64(1) << WP_Stinger)	/* スティンガー */
#define	WP_CLAYMORE	(I64(1) << WP_Claymore)	/* クレイモア地雷 */
#define	WP_C4BOMB	(I64(1) << WP_C4Bomb)	/* Ｃ４爆弾 */
#define	WP_CHAFFGRENADE	(I64(1) << WP_ChaffGrenade)	/* チャフグレネード */
#define	WP_STUNGRENADE	(I64(1) << WP_StunGrenade)	/* スタングレネード */
#define WP_MIC		(I64(1) << WP_Mic)		/* 指向性マイク */
#define WP_BLADE	(I64(1) << WP_Blade)		/* 高周波ブレード */
#define	WP_COLDSPRAY	(I64(1) << WP_ColdSpray)	/* 爆弾凍結スプレー */
#define WP_AKS		(I64(1) << WP_Aks)		/* AKS-74U */
#define	WP_MAGAZINE	(I64(1) << WP_Magazine)	/* マガジン */
#define	WP_GRENADE	(I64(1) << WP_Grenade)	/* グレネード */
#define	WP_M4		(I64(1) << WP_m4)		/* Ｍ４ */
#define	WP_PSG1T	(I64(1) << WP_Psg1T)	/* ＰＳＧ１－Ｔ（麻酔） */
#define	WP_BOOK		(I64(1) << WP_Book)

#define	WP_PLAYER	(I64(1) << WP_Player)	/* プレイヤーの攻撃であることを明示 */
#define	WP_BLADEFAINT	(I64(1) << WP_BladeFaint)	/* ブレード嶺撃ち */
#define	WP_BLADESTAB	(I64(1) << WP_BladeStab)	/* ブレード突き刺し */
#define	WP_BLADEGUARD	(I64(1) << WP_BladeGuard)	/* 敵用。ブレードで防御される攻撃 */

#define	WP_BOXREMOVE	(I64(1) << WP_BoxRemove)	/* ダンボールはがし */
#define	WP_KICK1		(I64(1) << WP_Kick1)		/* キック一発目 */

#define WP_PUNCH	(I64(1) << WP_Punch)		/* パンチ */
#define WP_KICK		(I64(1) << WP_Kick)		/* キック */
#define WP_THROW	(I64(1) << WP_Throw)		/* なげ */
#define WP_HANG		(I64(1) << WP_Hang)		/* 首絞め */
#define WP_GUS		(I64(1) << WP_Gus)		/* ガス */
#define WP_TUMBLE	(I64(1) << WP_Tumble)	/* 転ばし */
#define WP_BODY		(I64(1) << WP_Body)		/* 押しのけ */
#define WP_ONCORPSE	(I64(1) << WP_OnCorpse)		/* 落とし穴 */

#define WP_PUNCHR	(I64(1) << WP_PunchR)		/* パンチ */
#define WP_PUNCHL	(I64(1) << WP_PunchL)		/* パンチ */
#define WP_LASERSIGHT	(I64(1) << WP_LaserSight)		/* レーザー */
#define	WP_THROWG	(I64(1) << WP_ThrowG)  	/* 飛び中グレネード */
#define	WP_NOBLOOD	(I64(1) << WP_NoBlood)	/* 血が出ない攻撃 */
#define	WP_PUTC4	(I64(1) << WP_PutC4)	/* Ｃ４体付け */
#define	WP_STUNFAR	(I64(1) << WP_StunFar)	/* スタングレネード離れた場所ダメージ */

#define	WP_STAMP		(I64(1) << WP_Stamp)		/* 踏みつけ */
#define	WP_MECABREAK	(I64(1) << WP_MecaBreak)	/* 機械の故障等 */
//#define	WP_MECABREAK		WP_ROBOT		/* 機械の故障等 */

#define	WP_SHOTGUN_NEAR		(I64(1) << WP_ShotGun_Near)
#define	WP_SHOTGUN_FAR		(I64(1) << WP_ShotGun_Far)
#define	WP_HEADMARK			(I64(1) << WP_HeadMark)
#define	WP_BOXKICK			(I64(1) << WP_BoxKick)
#define	WP_WALLCRASH		(I64(1) << WP_WallCrash)
#define	WP_EXTINGUISHER		(I64(1) << WP_Extinguisher)

#define	WP_NOGUARD			(I64(1) << WP_NoGuard)
#define	WP_NORECOVER		(I64(1) << WP_NoRecover)

#define	WP_STEAM			(I64(1) << WP_Steam)
#define	WP_NOMUTEKI			(I64(1) << WP_NoMuteki)
#define	WP_NOPLAYER			(I64(1) << WP_NoPlayer)

#define	WP_BULLET	(WP_USP|WP_SOCOM|WP_AKS|WP_SHOTGUN_FAR| \
					 WP_PSG1|WP_M4)
#define	WP_BLOW		(WP_C4BOMB|WP_CLAYMORE|WP_NIKITA|WP_PUNCH|WP_RGB6|WP_STINGER| \
					 WP_KICK|WP_GRENADE|WP_TUMBLE)
#define	WP_BLAST	(WP_C4BOMB|WP_CLAYMORE|WP_NIKITA|WP_STINGER|WP_RGB6|WP_GRENADE)

#define	WP_PUNCHALL	(WP_PUNCH|WP_PUNCHL|WP_PUNCHR)
#define	WP_SOFTBLOW	(WP_WALLCRASH)
#define	WP_NO_BLOOD	(WP_NOBLOOD|WP_STUNFAR)
#define WP_WPNONE	(WP_PUNCH|WP_PUNCHL|WP_PUNCHR|WP_KICK|WP_KICK1)
#define WP_BLADALL	(WP_BLADESTAB|WP_BLADEFAINT|WP_BLADE)


/* ThrowGを「武器本体」とする */
#define	WP_WeaponCore		WP_ThrowG
#define	WP_WEAPONCORE		WP_THROWG

/* 没った奴 */
#define	WP_Famas		(63)
#define	WP_Spp1M		(63)
#define	WP_Robot		(63)
#define	IT_Magazine		(63)

#define	WP_FAMAS		(I64(1)<<WP_Famas)
#define	WP_SPP1M		(I64(1)<<WP_Spp1M)
#define	WP_ROBOT		(I64(1)<<WP_Robot)

/*---------------------------------------------------------------*/

/* 装備品 */
enum {
    IT_None = 0,   	/* 素手 */	
    IT_Ration,		/* レーション */
    IT_DummyScope,	/* ダミー双眼鏡 */
    IT_Medicine,	/* 風邪薬 */
    IT_Styptic,		/* 止血剤 */
    IT_Diazepam,	/* シアゼパム */
    IT_Uniform,		/* ゴル兵制服 */
    IT_Jacket,		/* 防弾チョッキ */
    IT_Stealth,		/* ステルス迷彩 */
    IT_MineDetector,	/* 地雷探知器 */
	/* 10 */
    IT_BombSenserA,	/* 爆弾解体センサーＡ */
    IT_BombSenserB,	/* 爆弾解体センサーＢ */
    IT_NightVision,	/* 暗視ゴーグル */
    IT_Thermal,		/* 熱源ゴーグル */
    IT_Scope,		/* 双眼鏡 */
    IT_Camera,		/* デジカメ */
    IT_CBBox,		/* ダンボール */
    IT_Tabacco,		/* たばこ */
    IT_Card,		/* ＩＤカード */
    IT_Shaver,		/* 髭剃り */
	/* 20 */
    IT_PHS,			/* 携帯電話 */
    IT_TnkCamera,	/* タンカー編カメラ */
	IT_CBBoxB,		/* ダンボールＢ */
	IT_CBBoxC,		/* ダンボールＣ */
	IT_CBBoxWet,	/* 濡れダンボール */
	IT_VibSensor,	/* 振動センサー */
	IT_CBBoxD,		/* ダンボールＤ */
	IT_CBBoxE,		/* ダンボールＥ */
	IT_Razor,		/* 無駄毛処理器 */
	IT_SocomSpprsr,	/* ソコムサプレッサ */
	/* 30 */
	IT_AKSpprsr,	/* ＡＫサプレッサ */
	IT_DummyTnkCamera,	/* ダミータンカーカメラ */
	IT_MugenBandana,	/* 無限バンダナ */
	IT_DogTag,			/* ドッグタグ */
	IT_MODisc,			/* ＭＯディスク */
	IT_UspSpprsr,		/* ＵＳＰサプレッサ */
	IT_MugenWig,		/* 無限カツラ */
	IT_WigA,			/* カツラＡ（Ｏ２無限） */
	IT_WigB,			/* カツラＢ（握力無限）*/
	IT_WigC,			/* カツラＣ */
	/* 40 */
	IT_WigD,			/* カツラＤ */
    MAX_ITEMS	
} ;

/*---------------------------------------------------------------*/

/* サイトの表示管理( GM_SightStatus ) */
/* ビットがたっていれば表示不可 */
enum {
	SGT_Camera = 		0x0001,
	SGT_Nikita =		0x0002,
	SGT_Thermal = 		0x0004,
	SGT_NightVision =	0x0008,
	SGT_Stinger = 		0x0010,
	SGT_RayView = 		0x0020,
	SGT_VTR =			0x0040,
	SGT_Scope = 		0x0080,
	SGT_Psg1 = 			0x0100,
	SGT_TankerCamera =	0x0200,

	SGT_Invisible	 =	0x8000,
} ;

/*---------------------------------------------------------------*/

/* 人型モデルオブジェクト番号（２１関節） */
#define HUMAN21_KOSHI            (0)
#define HUMAN21_ONAKA            (1)
#define HUMAN21_MUNE             (2)
#define HUMAN21_MIGI_KATA        (3)
#define HUMAN21_MIGI_UDE1        (4)
#define HUMAN21_MIGI_UDE2        (5)
#define HUMAN21_MIGI_TE          (6)
#define HUMAN21_HIDARI_KATA      (7)
#define HUMAN21_HIDARI_UDE1      (8)
#define HUMAN21_HIDARI_UDE2      (9)
#define HUMAN21_HIDARI_TE        (10)
#define HUMAN21_KUBI             (11)
#define HUMAN21_ATAMA            (12)
#define HUMAN21_MIGI_ASHI1       (13)
#define HUMAN21_MIGI_ASHI2       (14)
#define HUMAN21_MIGI_KAKATO      (15)
#define HUMAN21_MIGI_TSUMASAKI   (16)
#define HUMAN21_HIDARI_ASHI1     (17)
#define HUMAN21_HIDARI_ASHI2     (18)
#define HUMAN21_HIDARI_KAKATO    (19)
#define HUMAN21_HIDARI_TSUMASAKI (20)

/* 人型モデルオブジェクト番号（１６関節） */
#define HUMAN16_KOSHI            (0)
#define HUMAN16_MUNE             (1)
#define HUMAN16_MIGI_UDE1        (2)
#define HUMAN16_MIGI_UDE2        (3)
#define HUMAN16_MIGI_TE          (4)
#define HUMAN16_KUBI             (5)
#define HUMAN16_ATAMA            (6)
#define HUMAN16_HIDARI_UDE1      (7)
#define HUMAN16_HIDARI_UDE2      (8)
#define HUMAN16_HIDARI_TE        (9)
#define HUMAN16_MIGI_ASHI1       (10)
#define HUMAN16_MIGI_ASHI2       (11)
#define HUMAN16_MIGI_KAKATO      (12)
#define HUMAN16_HIDARI_ASHI1     (13)
#define HUMAN16_HIDARI_ASHI2     (14)
#define HUMAN16_HIDARI_KAKATO    (15)

/*---------------------------------------------------------------*/

/* ＳＥ モード */
enum {
	GM_SEMODE_NORMAL,	/* 俯瞰時はカメラターゲット、主観時はカメラ位置を中心に計算 */
	GM_SEMODE_BOMB,		/* 遠くても必ず少しは聞こえる */
	GM_SEMODE_REAL,		/* 主観やビハインドになったときだけ聞こえる */
	GM_SEMODE_CAMPOS,	/* 常にカメラ位置中心に計算 */
	GM_SEMODE_CAMTRG,	/* 常にカメラターゲット中心に計算 */
	GM_SEMODE_MIC		/* 指方向性マイク時のみ聞こえる */
} ;

/* SDステータス */
enum {
	GM_SDSTATUS_IDLE,	//なにもしていない
	GM_SDATATUS_READY,	//準備中
	GM_SDSTATUS_PLAY	//再生中
} ;

/*---------------------------------------------------------------*/

/* ストリーミング用 */

enum {
	GM_STREAM_STATE_INIT_WAIT = -2,
	GM_STREAM_STATE_INIT = -1,
	GM_STREAM_STATE_WAIT = 0,
	GM_STREAM_STATE_PLAY = 1,
	GM_STREAM_STATE_READ_END = 2,
	GM_STREAM_STATE_END = 3
};

#define GM_STREAM_PLAY_WAIT		0x00000001
#define GM_STREAM_EXEC_PROC		0x00000002

#define GM_STREAM_PAUSE_MENU	0x00000004	// MENU表示時音声PAUSE
						//		0x00000008  はstrctrl内部で使用

#define GM_STREAM_CHANNEL_0		0x00000010
#define GM_STREAM_CHANNEL_1		0x00000020
#define GM_STREAM_CHANNEL_MASK	(GM_STREAM_CHANNEL_0 | GM_STREAM_CHANNEL_1)
#define GM_STREAM_NO_PAUSE		0x00000040

//#define GM_STREAM_END_REQUEST	0x00000080	// 内部使用

#define GM_STREAM_FLAG_CHANNEL( _ch )	( GM_STREAM_CHANNEL_0 << ( _ch ) )

#define GM_STREAM_FLAG_PROC( _proc )	( GM_STREAM_EXEC_PROC | ( (_proc) << 8 ) )

#define	GM_STREAM_FLAG_3D		0x00000080	// 音声ストリーム3Dフラッグ

/* -------------------------------------------------------*/
#define GM_MAX_VOL		(0x3F)		/* 最大ボリューム */
#define GM_MAX_PAN		(0x1F)		/* 最大パン */
#define GM_VOL_BOMB		(24)		/* ボム系最小ボリューム */
#define GM_PAN_CENTER	(0x20)		/* パン真ん中 */
#define GM_MAX_TRACK	(0x20)

/* ターゲット中心 */
#define GM_SE_MAX_DIS_CAMTRG	(1000)		/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_CAMTRG	(7300)	/* 聞こえなくなる距離 */
#define GM_SE_VOL_CAMTRG ( (GM_SE_MIN_DIS_CAMTRG - GM_SE_MAX_DIS_CAMTRG) / GM_MAX_VOL )	/* 音量単位 */

#define GM_SE_MAX_DIS_CAMTRG_BOMB	(3000)		/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_CAMTRG_BOMB	(7800)	/* 聞こえなくなる距離 */
#define GM_SE_VOL_CAMTRG_BOMB ( (GM_SE_MIN_DIS_CAMTRG_BOMB - GM_SE_MAX_DIS_CAMTRG_BOMB) / (GM_VOL_BOMB) )	/* 音量単位 */

/* カメラ中心 */
#define GM_SE_MAX_DIS_CAMPOS	(0)			/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_CAMPOS	(9500)		/* 聞こえなくなる距離 */
#define GM_SE_VOL_CAMPOS ( (GM_SE_MIN_DIS_CAMPOS - GM_SE_MAX_DIS_CAMPOS) / GM_MAX_VOL )	/* 音量単位 */

#define GM_SE_MAX_DIS_CAMPOS_BOMB	(2000)			/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_CAMPOS_BOMB	(9200)		/* 聞こえなくなる距離 */
#define GM_SE_VOL_CAMPOS_BOMB ( (GM_SE_MIN_DIS_CAMPOS_BOMB - GM_SE_MAX_DIS_CAMPOS_BOMB) / (GM_VOL_BOMB) )	/* 音量単位 */

/* 指向性マイク */
#define GM_SE_MAX_DIS_MIC_IN	(4000)			/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_MIC_IN	(12000)			/* 聞こえなくなる距離 */
#define GM_SE_VOL_MIC_IN ( (GM_SE_MIN_DIS_MIC_IN - GM_SE_MAX_DIS_MIC_IN) / (GM_MAX_VOL) )	/* 音量単位 */

#define GM_SE_MAX_DIS_MIC_OUT	(0)			/* 最大音量でなる距離 */
#define GM_SE_MIN_DIS_MIC_OUT	(2000)			/* 聞こえなくなる距離 */
#define GM_SE_VOL_MIC_OUT ( (GM_SE_MIN_DIS_MIC_OUT - GM_SE_MAX_DIS_MIC_OUT) / (GM_MAX_VOL) )	/* 音量単位 */



/* ---- 世界の限界 ---------------------------------------------------*/
#define GM_WORLD_LIMIT_UPPER  (  1000000.0f )	/* 床が無かった場合の世界の天井 */
#define GM_WORLD_LIMIT_BOTTOM ( -1000000.0f )	/* 床が無かった場合の世界の底 */


/*----------------------------------------------------------------

  武器モデル名のデファイン

----------------------------------------------------------------*/

/* ＵＳＰ */
#define	MDL_USP				(123600)	/* usp */	
#define	MDL_USP_SUB			(3242287)	/* usp_sub */
#define	MDL_USP_AMO			(3223612)	/* usp_amo */
#define	MDL_USP_SPL			(3242137)	/* usp_spl */
#define	MDL_USP_EMB			(3227695)	/* usp_emb */

#define	MDL_USP_SP			(6917057)	/* usp_sp */
#define	MDL_USP_SP_SUB		(4715454)	/* usp_sp_sub */

#define	TXT_LSIGHT_LINE		(6792560)	/* lsight_msk.bmp */
#define	TXT_LSIGHT_POINT	(6715088)	/* rcm_l_msk.bmp */

/* Ｍ９２ */
#define	MDL_M92				(113490)	/* m92 */
#define	MDL_M92_SUB			(5338807)	/* m92_sub */
#define	MDL_M92_AMO			(5320132)	/* m92_amo */
#define	MDL_M92_SPL			(5338657)	/* m92_spl */
#define	MDL_M92_BUL			(2512988)	/* m92_bul2 */

/* ＦＡＭＡＳ */
#define	MDL_FAMAS			(108051)	/* fms */
#define	MDL_FAMAS_SUB		(6387043)	/* fms_sub */
#define	MDL_FAMAS_AMO		(6368368)	/* fms_amo */
#define	MDL_FAMAS_SPL		(6386893)	/* fms_spl */
#define	MDL_FAMAG_EMB		(6372451)	/* fms_emb */

/* ＣＨＡＦＦ */
#define	MDL_CHAFF			(104786)	/* cgr */
#define	MDL_CHAFF_SUB		(104786)	/* cgr */
#define	MDL_CHAFF_PIN		(5334819)	/* cgr_pin */
#define	MDL_CHAFF_LEVER		(5945943)	/* cgr_lever */

#define	MDL_CHAFF_RAI		(5336606)	/* cgr_rai */
#define	MDL_CHAFF_SUB_RAI	(5336606)	/* cgr_rai */
#define	MDL_CHAFF_PIN_RAI	(6712059)	/* cgr_pin */
#define	MDL_CHAFF_LEVER_RAI	(10944558)	/* cgr_lever */

#define	MDL_CHAFF_FRG		(2623419)	/* cgr_frg1 */

/* ＧＲＥＮＡＤＥ */
#define	MDL_GRENADE			(109230)	/* grn */
#define	MDL_GRENADE_SUB		(109230)	/* grn */
#define	MDL_GRENADE_PIN		(1140793)	/* grn_pin */
#define	MDL_GRENADE_LEVER	(6230359)	/* grn_lever */

#define	MDL_GRENADE_RAI			(1142580)	/* grn_rai */
#define	MDL_GRENADE_SUB_RAI		(1142580)	/* grn_rai */
#define	MDL_GRENADE_PIN_RAI		(12741388)	/* grn_pin_rai */
#define	MDL_GRENADE_LEVER_RAI	(10962334)	/* grn_lever_rai */

/* ＳＴＵＮ ＧＲＥＮＡＤＥ */
#define	MDL_STUN			(121170)	/* sgr */
#define	MDL_STUN_SUB		(121170)	/* sgr */
#define	MDL_STUN_PIN		(5335843)	/* sgr_pin */
#define	MDL_STUN_LEVER		(6994519)	/* sgr_lever */

#define	MDL_STUN_RAI		(5337630)	/* sgr_rai */
#define	MDL_STUN_SUB_RAI	(5337630)	/* sgr_rai */
#define	MDL_STUN_PIN_RAI	(6712123)	/* sgr_pin_rai */
#define	MDL_STUN_LEVER_RAI	(11010094)	/* sgr_lever_rai */

/* ＳＯＣＯＭ */
#define	MDL_SOCOM			(121037)	/* scm */
#define	MDL_SOCOM_SUB		(3084817)	/* scm_sub1 */
#define	MDL_SOCOM_SP		(6818748)	/* scm_sp */
#define	MDL_SOCOM_SP_SUB	(3084818)	/* scm_sub2 */
#define	MDL_SOCOM_AMO		(77724)		/* scm_amo */

/* ＳＰＰ */
#define	MDL_SPP				(121456)	/* spp */
#define	MDL_SPP_SUB			(3242153)	/* spp_sub */
#define	MDL_SPP_BUL			(3224755)	/* spp_bul */

/* ＰＳＧ－１ */
#define	MDL_PSG				(118471)	/* psg */
#define	MDL_PSG_AMO			(10563323)	/* psg_amo */

/* ＳＴＩＮＧＥＲ */
#define	MDL_STINGER			(121575)	/* stg */
#define	MDL_STINGER_SUB		(10582192)	/* stg_sub */
#define	MDL_STINGER_MSL		(10575994)	/* stg_msl */
#define	MDL_STINGER_MSL2	(2887558)	/* stg_msl2 */

/* ＮＩＫＩＴＡ */
#define	MDL_NIKITA			(116180)	/* nkt */
#define	MDL_NIKITA_MSL		(7429929)	/* nkt_msl */

#define	TXT_NIKITA_LIGHT	(6715088)	/* rcm_l_msk.bmp */

/* ＣＬＡＹＭＯＲＥ */
#define	MDL_CLAYMORE		(104941)	/* clm */
#define	MDL_CLAYMORE_SUB	(95393)		/* clm_sub */

/* Ｃ４ */
#define	MDL_C4				(104754)	/* cfr */
#define	MDL_C4_SUB			(104754)	/* cfr */
#define	MDL_C4_TRP			(5339203)	/* cfr_trp */

/* ＲＧＢ－６ */
#define	MDL_RGB				(120130)	/* rgb */
#define	MDL_RGB_SUB			(5339222)	/* rgb_sub */
#define	MDL_RGB_BUL			(5321824)	/* rgb_bul */
#define	MDL_RGB_AMO			(5320547)	/* rgb_amo */

/* ブレード */
#define	MDL_BLADE			(9603818)	/* hfb_mt */
#define	MDL_BLADE_INV		(1679741)	/* hfb_mineuchi_mt */
#define	MDL_BLADE_SUB		(15446734)	/* hfb_sub_mt */
#define	MDL_BLADE_INV_SUB	(1319992)	/* hfb_mineuchi_sub_mt */

/* マカロフ */
#define	MDL_MKR				(115154)	/* mkr */
#define	MDL_MKR_SUB			(5338911)	/* mkr_sub */

/* ＡＫＳ－７４Ｕ */
#define	MDL_AKS				(102867)	/* aks */
#define	MDL_AKS_SP			(15403928)	/* aks_sp */
#define	MDL_AKS_AMO			(6368044)	/* aks_amo */
#define	MDL_AKS_EMB			(6372127)	/* aks_emb */

#define	MDL_AKS_RAI				(6385062)	/* aks_rai */
#define	MDL_AKS_RAI_SP			(13941638)	/* aks_rai_sp */
#define	MDL_AKS_RAI_SUB			(9925084)	/* aks_rai_sub */
#define	MDL_AKS_RAI_SP_SUB		(6118230)	/* aks_sp_rai_sub */
#define	MDL_AKS_RAI_AMO_SUB		(15782261)	/* aks_amo_rai_sub */

/* ｍ４ａ1 */
#define	MDL_M4A				(7473930)	/* m4a_nm */
#define	MDL_M4A_SH			(3056556)	/* m4a_shld */
#define	MDL_M4B				(7506473)	/* m4b_gl */
#define	MDL_M4C				(7539616)	/* m4c_sc */
#define	MDL_M4A_AMO			(4271549)	/* m4a_amo */

#define	MDL_M4A_RAI			(4288567)	/* m4a_rai */
#define	MDL_M4A_RAI_SUB		(10842629)	/* m4a_rai_sub */
#define MDL_M4A_RAI_AMO     (10823954)  /* m4a_rai_amo */

/* スコープ */
#define	MDL_SCOPE			(3240470)	/* scp_rai */

/* デジカメ */
#define	MDL_CAMERA			(93782)		/* dcm_rai */

/*GRD付きＭ４(敵用)*/
#define	MDL_M4B_GRD			(15221147)	/*m4_grn_htc*/
#define	MDL_M4B_GRD_AMO		(8421706)	/*m4a_grn_amo_htc*/
#define	MDL_M4B_GRD_EMB		(12139984)	/*demo_m4_grn_emb*/
#define	MDL_M4B_GRD_BUL		(12137178)	/*demo_m4_grn_bul*/

/*天狗兵 カタナ*/
#define	MDL_TNG_KANATA		(3990220)	/*tng_kanata*/
#define	MDL_TNG_KANATA_G		(3214028)	/*tbl_katana*/
/* 雑誌 */
#define	MDL_BOOK			(12417010)	/* syuukanshi_rhand */
#define	MDL_BOOK_MSL		(1351572)	/* shuukanshi */

/* スパス */	
#define	MDL_SPS				(121459)   	/* sps */
#define	MDL_SPS_AMO			(6369206)	/* sps_amo */
#define	MDL_SPS_EMB			(6373289)	/* sps_emb */

/* シングルアクションアーミー */
#define	MDL_SAA				(120961)	/* saa */

/* Ｐ９０ */
#define	MDL_P90				(116560)	/* p90 */
#define	MDL_P90_AMO			(3223172)	/* p90_amo */

/*GLOCK*/
#define	MDL_GLOCK				(109035)	/* glk */
#define	MDL_GLOCK_AMO			(14757037)	/* glk_amo */

/*ABAKAN*/
#define	MDL_ABAKAN				(102571)	/* abk */
#define	MDL_ABAKAN_AMO			(14756633)	/* abk_amo */


/* Ｓｐｅｃｉａｌ Ｓｃｏｕｔ Ｋｎｉｆｅ */
#define	MDL_SSK				(121547)	/* ssk */
#define	MDL_SSK_BUL			(14759096)	/* ssk_bul */
#define	MDL_SSK_COG			(14759923)	/* ssk_cog */
#define	MDL_SSK_INV			(14766050)	/* ssk_inv */

/* ThermalGoggles */
#define	MDL_IRG				(122188)	/* tgl */

/* NightVision */
#define	MDL_NVG				(116044)	/* ngl */

/* サングラス */
#define	MDL_RAIDEN_GLASSES	(3342560)	/* rai_glass_mh_mt */
#define	MDL_SNAKE_GLASSES	(9208032)	/* sna_glass_mh_mt */
#define	MDL_PLISKIN_GLASSES	(1401054)	/* iro_glass_mh_mt */

#define	MDL_SNAKE_GLASSES_BLACK		(10973328)	/* sna_glass_black_mh_mt */
#define	MDL_PLISKIN_GLASSES_BLACK	(14643186)	/* iro_glass_black_mh_mt */

#define	MDL_RAIDEN_GLASSES_MIRROR	(9634016)		/* rai_glass_sh_mt */
#define	MDL_SNAKE_GLASSES_MIRROR	(15499488)		/* sna_glass_sh_mt */
#define	MDL_SNAKE_GLASSES_BLACK_MIRROR	(487569)	/* sna_glass_black_sh_mt */

/*----------------------------------------------------------------

  ダメージ値のデファイン

----------------------------------------------------------------*/

#define	DMG_BLAST			(40)
#define	FNT_BLAST			(10)

/*----------------------------------------------------------------

  予約トラップ名のデファイン

----------------------------------------------------------------*/

#define	GM_TRP_BOMBLOST		(5961588)	/* bomblost */
#define	GM_TRP_BOMBLOST_G	(14540210)	/* bomblost_g */

/*-----------------------------------------------------------------
	VRモード中の進行管理フラグ
	GM_VRStatus の中身
------------------------------------------------------------------*/
#define	GM_VR_ENEMY_END			(0x0001)	//敵兵全滅
#define	GM_VR_TAB_ENEMY_END		(0x0002)	//殺してはいけない敵を殺した
#define	GM_VR_EFFECT			(0x0004)	//VR風エフェクト
#define	GM_VR_IDLE				(0x0008)	//敵や的を待機状態にする
#define	GM_VR_DESTROY			(0x0010)	//敵や的を消去する
#define	GM_VR_CLEAR				(0x0020)	//ステージクリアした
#define	GM_VR_POSE_CLEAR		(0x0040)	//ポーズ中にクリアした（写真撮影用）
#define	GM_VR_GAME_OVER			(0x0080)	//ゲームオーバー確定 （実際にゲームオーバー処理にいってるとはかぎらない）
#define	GM_VR_NO_CONTINUE		(0x0100)	//コンティニュー選択不可
#define	GM_VR_ENEMY_POLY_VANISH	(0x0200)	//敵兵がポリゴン分解で消える
#define	GM_VR_EXIT_TO_SPECIAL	(0x0400)	//ゲームオーバーEXITの行き先が特別指定
#define	GM_VR_SNAKETALES		(0x0800)	//テイルズ中である（VR以外でも使用）
#define	GM_VR_PHOTOGRAPH		(0x1000)	//写真ミッション中である (ANOTHER MISSION)
#define	GM_VR_WINDOW_OPEN		(0x2000)	//VR指令ウィンドウが開いている
#define	GM_VR_LOW_BLAST_FX		(0x4000)	//爆発エフェクト少なめ
#define	GM_VR_ENEMY_POLY_VANISH_OFF	(0x8000)	//敵兵がポリゴン分解で消えないようにした

//GM_AnotherPlayer の中身
enum {
	GM_ANOTHER_PLAYER_SNAKE = 0,	//スネーク
	GM_ANOTHER_PLAYER_RAIDEN,		//ライデン
	GM_ANOTHER_PLAYER_OLD_SNAKE,	//前作スネーク
	GM_ANOTHER_PLAYER_PLISKIN,		//プリスキン
	GM_ANOTHER_PLAYER_TUXEDO,		//タキシードスネーク
	GM_ANOTHER_PLAYER_BLADE,		//刀ライデン
};


/*-----------------------------------------------------------------
	特殊Ｃ４ステータス
------------------------------------------------------------------*/
enum {
	C4MAN_STATUS_NONE,
	C4MAN_STATUS_BOMB,
	C4MAN_STATUS_NO_KAITAI,
	C4MAN_STATUS_DROP,
} ;

/*-----------------------------------------------------------------
	GM_StageHappening ステージでの出来事
------------------------------------------------------------------*/
#define GM_STAGE_HAPPEN_DETECT			0x00000001	/* 発見された */
#define GM_STAGE_HAPPEN_ALERT			0x00000002	/* 危険モードになった */
#define GM_STAGE_HAPPEN_ENEMY_KILL		0x00000004	/* 敵兵殺した */
#define GM_STAGE_HAPPEN_ENEMY_FAINT		0x00000008	/* 敵兵気絶させた */
#define GM_STAGE_HAPPEN_ENEMY_SLEEP		0x00000010	/* 敵兵眠らせた */
#define GM_STAGE_HAPPEN_SNEAK2CAUTUION	0x00000020	/* 潜入から警戒になった */

#endif
