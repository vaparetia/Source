/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewGclVarClear;
extern NEWCHARA NewSelScrMan;
extern NEWCHARA NewTheaterScr;
extern NEWCHARA NewJimakuControl;
extern NEWCHARA NewFileExistCheckScn;
extern NEWCHARA NewClearedGameMenuScr;
extern NEWCHARA ComNPlantDogtags;
extern NEWCHARA NewLoadGameScr;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewReadSavedVariable;
extern NEWCHARA NewPreviousStory;
extern NEWCHARA NewAlbumMng;
extern NEWCHARA NewMCCheckScr;
extern NEWCHARA NewForeach;
extern NEWCHARA NewTitleBackground;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewSpecialMenu_Eng;
extern NEWCHARA ComLoadOptions;
extern NEWCHARA NewGclAssert;
extern NEWCHARA NewSpecialMenu_Page;
extern NEWCHARA NewDogtagMng;
extern NEWCHARA NewSpecialScr;
extern NEWCHARA NewDelay;
extern NEWCHARA NewGclLangUpdate;
extern NEWCHARA NewStreamSet;
extern NEWCHARA ComNTankerDogtags;
extern NEWCHARA NewOptionScr;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewTitleKonami;
extern NEWCHARA ComStoreLoadedVariable;
extern NEWCHARA NewGclReboot;
extern NEWCHARA NewNodeFrame;
extern NEWCHARA ComGetLoaddataTime;
extern NEWCHARA NewBossrushScr;
extern NEWCHARA SIG_DogTagZeroClear;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA Msn_SetDefaultData;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA ComGetNowTime;
extern NEWCHARA ComStoreOptions;
extern NEWCHARA NewLoadRestart;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewTitleScrMan;
extern NEWCHARA NewMpegPssMovieStr;
/* 46 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* chara ゲームセレクト画面マネージャー */
		{ 0x000A7CFB, NewSelScrMan },
	/* chara ローポリ劇場選択画面 */
		{ 0x0010CF2F, NewTheaterScr },
	/* command 字幕制御 */
		{ 0x0018D40B, NewJimakuControl },
	/* chara セーブファイル個数チェック */
		{ 0x0019BD75, NewFileExistCheckScn },
	/* chara クリアデータロード後のゲーム開始画面 */
		{ 0x001B970C, NewClearedGameMenuScr },
	/* command プラント編取得済みドッグタグ数 */
		{ 0x001F567B, ComNPlantDogtags },
	/* chara ロードゲーム画面マネージャー */
		{ 0x0024D360, NewLoadGameScr },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command save_var_read */
		{ 0x0034CC13, NewReadSavedVariable },
	/* chara シャドーモセス */
		{ 0x00397FD4, NewPreviousStory },
	/* chara アルバムモード */
		{ 0x003D5D04, NewAlbumMng },
	/* chara メモリーカード警告画面 */
		{ 0x003F9531, NewMCCheckScr },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* chara ノード画面バックグラウンド */
		{ 0x0055080C, NewTitleBackground },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara 英語版スペシャルメニュー */
		{ 0x005B6E21, NewSpecialMenu_Eng },
	/* command 保存していたオプションフラグを戻す */
		{ 0x005C526A, ComLoadOptions },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* chara スペシャルメニューページ付き */
		{ 0x006DDD6A, NewSpecialMenu_Page },
	/* chara ドッグタグモード */
		{ 0x006E6572, NewDogtagMng },
	/* chara スペシャル画面マネージャー */
		{ 0x006EEB2F, NewSpecialScr },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command 字幕言語設定 */
		{ 0x0074E86B, NewGclLangUpdate },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command タンカー編取得済みドッグタグ数 */
		{ 0x007C7E31, ComNTankerDogtags },
	/* chara オプション画面マネージャー */
		{ 0x007E3320, NewOptionScr },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara コナミロゴ表示 */
		{ 0x0081767C, NewTitleKonami },
	/* command store_loadedvar */
		{ 0x0082A05E, ComStoreLoadedVariable },
	/* command reboot */
		{ 0x0082CB3E, NewGclReboot },
	/* chara ノード画面フレームアニメーション */
		{ 0x009530AE, NewNodeFrame },
	/* command ロードデータの秒単位タイムスタンプ */
		{ 0x0098137C, ComGetLoaddataTime },
	/* chara ボスラッシュ選択画面 */
		{ 0x00A3D28A, NewBossrushScr },
	/* command ドッグタグゼロクリア */
		{ 0x00B74FD7, SIG_DogTagZeroClear },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ミッションズデータ初期化 */
		{ 0x00B7F8E7, Msn_SetDefaultData },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command 現在の秒単位時間 */
		{ 0x00CD2045, ComGetNowTime },
	/* command オプションフラグの内容を保存 */
		{ 0x00D44DC7, ComStoreOptions },
	/* command load_restart */
		{ 0x00F2DFCD, NewLoadRestart },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara タイトル画面マネージャー */
		{ 0x00FC14A5, NewTitleScrMan },
	/* chara MOVIE再生 */
		{ 0x00FFBECE, NewMpegPssMovieStr },
/* gcl chara end */
	{ 0, NULL }
};
