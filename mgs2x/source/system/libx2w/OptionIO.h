//=============================================================================
// 2002/11/14
//		OptionIO.h : オプション 入出力用 ヘッダ
//												copyright(C) 2002 inuniku
//=============================================================================
#pragma once
//-----------------------------------------------------------------------------
// 型定義( Global.h読んでなければ実装するのに必要なので定義する )
#ifndef SGLOBAL
 #define for			if( 0 ); else for
 #ifndef ON
  #define ON			1
 #endif // ON
 #ifndef OFF
  #define OFF			0
 #endif // OFF

 typedef unsigned long	uLong;
 typedef   signed long	sLong;
 typedef unsigned int	uInt;
 typedef   signed int	sInt;
 typedef unsigned short	uShort;
 typedef   signed short	sShort;
 typedef unsigned char	uChar;
 typedef   signed char	sChar;
 typedef unsigned long	DWord;
 typedef unsigned short	Word;
 typedef unsigned char	Byte;
 typedef unsigned char	Bool;
#endif // SGLOBAL
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 設定情報が入ってるファイル名
#define OPTIONFILENAME			"MGS2SSET.ini"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 設定情報を大きく区切るセクション名
#define SEC_STARTING		"STARTING"			// 「Starting Setup」（起動設定）
#define SEC_DRAWING			"DRAWING"			// 「Drawing Quality」（描画クオリティ設定）
#define SEC_PRODUCT			"PRODUCT"			// 「Production Quality」（演出クオリティ設定）
#define SEC_SOUND			"SOUND"				// 「Sound」（サウンド設定）
#define SEC_KEYCONF			"KEYCONF"			// 「KEY Config」（ボタン配置設定）
#define SEC_RANKING			"RANKING"			// 「Ranking」（パスワードをインターネットに送信）
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 起動設定キー情報
#define KEY_SSECPU			"ssu"				// SSU CPU( 0:OFF 1:ON )
#define KEY_WINMODE			"win"				// Window Mode( 0:OFF 1:ON )
#define KEY_32ZBUFF			"zbuf"				// 32Bit Z Buffer( 0:OFF 1:ON )
#define KEY_BPP				"bpp"				// ビット数( 0:16bit 1:32bit )
#define KEY_WIDTH			"width"				// スクリーンサイズ - 幅
#define KEY_HEIGHT			"height"			// スクリーンサイズ - 高さ
#define KEY_DRIVERNAME		"driver"			// ドライバー名（文字列）
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 描画設定＆演出設定共通キー情報
#define KEY_ISCUSTOM		"mode"				// カスタムか自動設定かのモード指定( 0:LEVEL 1:CUSTOM )
#define KEY_LEVEL			"level"				// レベル設定( レベル値 0～15 )
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 描画設定キー情報
#define KEY_RENDERSIZE		"rendersize"		// レンダリングサイズ( Low 0～4 High )
#define KEY_DETAIL			"detail"			// 影の精密度( 0:OFF Low 1～5 High )
#define KEY_QUALITY			"quality"			// モデル描画クオリティ( Low 0～3 High )
#define KEY_CLEANESS		"clearness"			// レンダリング鮮明度( Blur 0 ～ 15 Clear )
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 演出設定キー情報
#define KEY_FOCUS			"focus"				// フォーカス効果( 0:OFF 1:ON )
#define KEY_MOTIONBLUR		"motionblur"		// モーションブラー系効果( 0:OFF 1:ON )
#define KEY_VRMODE			"vrmode"			// VRモード破壊エフェクト( 0:OFF 1:ON )
#define KEY_STEALTH			"stealth"			// 光学迷彩効果( 0:Easy 1:Normal )
#define KEY_RADIOFOCUS		"rfocus"			// 無線画面フォーカス効果( 0:OFF 1:ON )
#define KEY_CROSSFADE		"crossfade"			// クロスフェードレベル( 0:OFF Low 1～7 High )
#define KEY_EFFQUANTITY		"effectquantity"	// エフェクト量レベル( Low 0～15 High )
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// サウンド設定キー情報
//#define KEY_DRIVERNAME		"driver"		// サウンドドライバー名（文字列）
//#define KEY_QUALITY			"quality"		// 再生クオリティ( 0:11khz 1:22khz 2:44khz )
#define KEY_SE				"se"				// SEの発音数( Low 0～9 High )
#define KEY_3DQUALITY		"tdSound"			// ３Dサウンド品質( Low:0 Middle:1 High:2 )
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ボタン設定キー情報
#define KEY_KEYBOARD		"key%02d_%d"		// キーボード
#define KEY_JOYPAD			"pad%02d"			// ジョイパッド
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ランキング設定キー情報
#define KEY_RANKNUM			"count"				// ランキングデータ個数
#define KEY_RANKDATA		"data%d"			// ランキングデータ
#define KEY_USERID			"id"				// ユーザー登録ID
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 詳細設定
// -- 起動設定 --
#define DEF_SSECPU			ON
#define DEF_WINMODE			OFF
#define DEF_32ZBUFF			ON
#define DEF_BPP				1
#define DEF_WIDTH			640
#define DEF_HEIGHT			480
#define SSCREEN_MIN_CX		( 640 )				// スクリーンサイズの最小幅
#define SSCREEN_MIN_CY		( 480 )				// スクリーンサイズの最小高さ
#define SSCREEN_MAX_CX		( 1600 )			// スクリーンサイズの最大幅
#define SSCREEN_MAX_CY		( 1200 )			// スクリーンサイズの最大高さ
// -- 描画設定 --
#define DEF_DCUSTOM			OFF
#define DEF_DLEVEL			12
#define DEF_RENDERSIZE		3
#define DEF_DETAIL			3
#define DEF_QUALITY			3
#define DEF_CLEANESS		15
// -- 演出設定 --
#define DEF_PCUSTOM			OFF
#define DEF_PLEVEL			12
#define DEF_FOCUS			ON
#define DEF_STEALTH			ON
#define DEF_MOTIONBLUR		ON
#define DEF_RADIOFOCUS		ON
#define DEF_VRMODE			ON
#define DEF_CROSSFADE		5
#define DEF_EFFQUANTITY		12
// -- サウンド設定 --
#define DEF_SNDQUALITY		1
#define DEF_SE				5
#define DEF_3DSOUND			1
// -- ボタン設定 --
#define MAXBTN				27					// キー設定のボタン数
#define DEF_BTN				0
// -- ランキング --
#define DEF_RANKNUM			0
#define fmtRANKINGDATA		"%d-%d-%d %d:%d,%d,%s"
#define ID_STRLEN			13
#define PASS_STRLEN_MAX		16
#define PASS_STRLEN_MIN		 6
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ショートマクロ
#define GPI							GetPrivateProfileInt
#define GPS							GetPrivateProfileString
#define WPS							WritePrivateProfileString
#define WPSM( sec, key, n, s, f )	WPS( sec, key, _itoa( n, s, 10 ), f )
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 構造体定義
typedef struct _SOptionIO
{
	// 起動設定
	struct 
	{
		Bool	m_bSSECPU;				// SSE CPU使用の有無
		Bool	m_bWinMode;				// Window Mode使用の有無
		Bool	m_b32bitZBuff;			// 32bitZBuffer使用の有無
		Byte	m_ucBpp;				// ビット数
		Word	m_wWidth;				// スクリーン幅
		Word	m_wHeight;				// スクリーン高さ
		char	m_szDriverName[64];		// ドライバ名
	} m_setting;
	// 描画設定
	struct
	{
		Byte	m_ucRenderSize;			// カスタム - レンダリングサイズ
		Byte	m_ucDetail;				// カスタム - 影の精密度
		Byte	m_ucQuality;			// カスタム - モデル描画クオリティ
		Byte	m_ucClearness;			// カスタム - レンダリング鮮明度
	} m_drawing;
	// 演出設定
	struct
	{
		Bool	m_bFocus;				// フォーカス効果の有無
		Bool	m_bStealth;				// 光学迷彩効果の有無
		Bool	m_bMotionBlur;			// モーションブラーの有無
		Bool	m_bRadioFocus;			// 無線画面フォーカス効果の有無
		Bool	m_bVRMode;				// VRモード破壊エフェクトの有無
		Byte	m_ucCrossFade;			// クロスフェードレベル
		Byte	m_ucEfQuantity;			// エフェクト量レベル
		Byte	m_ucResv;				// 予約
	} m_product;
	// サウンド設定
	struct
	{
		Byte	m_ucQuality;			// 再生クオリティ（レート）
		Byte	m_ucSE;					// SE発音数
		Byte	m_uc3DQuality;			// ３Dサウンドクオリティ
		char	m_szDriverName[64];		// ドライバー名
	} m_sound;
} SOptionIO;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// プロトタイプ宣言
//-----------------------------------------------------------------------------
//	S_MGS2SGetModuleDirectory
//	[内容]
//		メタルギアソリッドのインストール先フォルダを調べる
//
//	[引数]
//		pszBuf		インストール先の場所名を格納するためのバッファ
//		size		バッファサイズ
//	[戻り値]
//		TRUE  : 成功
//		FALSE : 失敗
//				失敗すると、pszBufには今実行しているファイルのパスが格納されます
//
//-----------------------------------------------------------------------------
BOOL S_MGS2SGetModuleDirectory( char* pszBuf, size_t size );
//-----------------------------------------------------------------------------
//	S_MGS2SSaveRankingPassword
//	[内容]
//		メタルギアソリッド ランキングをオプション用初期化ファイルに
//		保存します
//
//	[引数]
//		iMode			モード
//						0 : SONS OF LIBERTY
//						1 : BOSS SURVIVAL
//						2 : TOP RECORD CODE
//		pszPassword		終端にNULLを含んだ パスワードの文字列
//						SONS OF LIBERTY : 28文字
//						BOSS SURVIVAL	: 35文字
//						TOP RECORD CODE	: 20文字
//	[戻り値]
//		なし
//
//-----------------------------------------------------------------------------
void S_MGS2SSaveRankingPassword( const int, const char* );
//-----------------------------------------------------------------------------
//	S_MGS2SSettingLoad
//	[内容]
//		メタルギアソリッド 設定ファイルのロードします
//		各項目に対してロードできなかった
//		もしくは、ファイルそのものがなかった場合は
//		デフォルト値が入るようになっています
//
//	[引数]
//		pO			SOptionIO構造体（結果を格納します）
//		szFilePath	設定ファイル名を含んだ絶対パスを指定
//	[戻り値]
//		なし
//
//-----------------------------------------------------------------------------
void S_MGS2SSettingLoad( SOptionIO* pO, const char* pszFilePath );
//-----------------------------------------------------------------------------
//=============================================================================
// end of OptionIO.h
//=============================================================================
