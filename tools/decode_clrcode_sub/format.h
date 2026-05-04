/*
  データフォーマット
  2002/08/07 M.Kobayashi

  $Id: format.h,v 1.4 2002/10/02 06:47:24 usr03700 Exp $

*/

// 共通データ
// 各データの後に付く

#define COMMON_CODE_FORMAT { \
	2, 2 }

enum {
	COMMON_PLATFORM,	// 	プラットフォーム
	COMMON_REGION,		// 仕向地
	COMMON_DATA_MAX,
};


// 本編

// 下位からビット数
#define GM_CODE_FORMAT	{ \
	2, 15, 10, 5, \
	8,  8,  8, 6, 1, 1, \
	6,  5,  7, 8, 6, \
	2,  3,  2, 7 }

enum {
	GM_RADAR_TYPE,		// レーダータイプ
	GM_PLAY_TIME,  		// プレイ時間	
	GM_SHOOT, 			// 発砲数
	GM_DAMAGE_RATE,		// ダメージ率

	GM_ALERT, 			// 危険モード回数
	GM_KILL, 			// 殺した数
	GM_CLEARING, 		// クリアリング回避回数
	GM_TANKER_CLEAR, 	// タンカークリア回数
	GM_FUNAMUSHI, 		// ふなむしをくっつけてクリア
	GM_SPECIAL_ITEM,	// 特殊アイテム使用

	GM_CONTINUE, 		// コンティニュー回数
	GM_RATION,			// レーション使用回数
	GM_SAVE,			// セーブ回数
	GM_CAM_BREAK,		// カメラ系破壊回数
	GM_PLANT_CLEAR,		// プラントクリア回数

	GM_VERSION,			// バージョン
	GM_LEVEL,			// ゲームレベル
	GM_T_OR_P,			// タンカーorプラント
	GM_DOGTAG,			// ドッグタグ回収率

	GM_DATA_MAX,

	GM_PS2_DOGTAG_2002 = GM_VERSION,	// ドッグタグ2002モードフラグ

};

#define	GM_CODE_LEN		28
	
	
// ボスさば

#define BS_CODE_FORMAT	{ \
	17, 17, 17, 17,	17, 17, \
	1, 1, 1, 2, 2, 1 }

enum {
	BS_OLG_TIME,	// オルガクリア時間
	BS_FAT_TIME,	// ファットマンクリア時間
	BS_HAR_TIME,	// ハリアークリア時間
	BS_VMP_TIME,	// ヴァンプクリア時間
	BS_RAY_TIME,	// レイクリア時間
	BS_SOL_TIME,	// ソリダスクリア時間

	BS_FAT_DIE,		// ファットマン死亡フラグ
	BS_VMP_DIE,		// ヴァンプ死亡フラグ
	BS_SOL_DIE,		// ソリダス死亡フラグ

	BS_VERSION,		// バージョン
	BS_DIFFICULTY,	// 難易度
	BS_PLAYER,		// プレイヤー種類

	BS_DATA_MAX,
};

#define	BS_CODE_LEN		27

// ボスさば拡張版

#define BE_CODE_FORMAT	{ \
	17, 17, 17, 17,	17, 17, \
	1, 1, 1, 2, 2, 1, \
	17, 1, 17, 1  }

enum {
	BE_OLG_TIME,	// オルガクリア時間
	BE_FAT_TIME,	// ファットマンクリア時間
	BE_HAR_TIME,	// ハリアークリア時間
	BE_VMP_TIME,	// ヴァンプクリア時間
	BE_RAY_TIME,	// レイクリア時間
	BE_SOL_TIME,	// ソリダスクリア時間

	BE_FAT_DIE,		// ファットマン死亡フラグ
	BE_VMP_DIE,		// ヴァンプ死亡フラグ
	BE_SOL_DIE,		// ソリダス死亡フラグ

	BE_VERSION,		// バージョン
	BE_DIFFICULTY,	// 難易度
	BE_PLAYER,		// プレイヤー種類

	BE_W03_TIME,	// w03敵クリア時間
	BE_W03_DIE,		// w03敵死亡フラグ
	BE_TNG_TIME,	// 天狗クリア時間
	BE_TNG_DIE,		// 天狗死亡フラグ

	BE_DATA_MAX,
};

#define	BE_CODE_LEN		35

// VR

#define VR_CODE_FORMAT	{ \
	 8, 10, 20, 17, 17, 2, 1 }

enum {
	VR_NAME,		// プレイヤー名のハッシュ値
//	VR_VER,			// バージョン
	VR_STAGE,		// ステージID
	VR_SCORE,		// スコア
	VR_TIME,		// タイム
	VR_BULLETS,		// 残弾数
	VR_SNEAKING,	// 隠密スコア
	VR_NO_KILL,		// 不殺フラグ
	VR_DATA_MAX,
};

#define	VR_CODE_LEN		20






