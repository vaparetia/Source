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
extern NEWCHARA NewFileExistCheckScn;
extern NEWCHARA NewEnding;
extern NEWCHARA NewSK_Result;
extern NEWCHARA NewLoadGameScr;
extern NEWCHARA NewSTSelect_Scn;
extern NEWCHARA NewSaveGameScr;
extern NEWCHARA NewReadSavedVariable;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewGclAssert;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewDelay;
extern NEWCHARA NewStreamSet;
extern NEWCHARA ComStoreLoadedVariable;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewConfirmSaveScr;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA Msn_SetDefaultData;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA ComSetReferenceData;
extern NEWCHARA NewLoadRestart;
extern NEWCHARA GM_SdSetGcl;
/* 27 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* chara セーブファイル個数チェック */
		{ 0x0019BD75, NewFileExistCheckScn },
	/* chara エンディング */
		{ 0x001F119A, NewEnding },
	/* chara 結果画面 */
		{ 0x0022AE7D, NewSK_Result },
	/* chara ロードゲーム画面マネージャー */
		{ 0x0024D360, NewLoadGameScr },
	/* chara スネークテイルズセレクト */
		{ 0x00262B46, NewSTSelect_Scn },
	/* chara セーブゲーム画面マネージャー */
		{ 0x00274B60, NewSaveGameScr },
	/* command save_var_read */
		{ 0x0034CC13, NewReadSavedVariable },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* command セットＶＲ発見回数 */
		{ 0x00718756, SetVR_DISCOVERY_CNT },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command store_loadedvar */
		{ 0x0082A05E, ComStoreLoadedVariable },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara セーブ確認画面 */
		{ 0x009E8AEB, NewConfirmSaveScr },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ミッションズデータ初期化 */
		{ 0x00B7F8E7, Msn_SetDefaultData },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
	/* command セーブ用参照変数登録 */
		{ 0x00DCB281, ComSetReferenceData },
	/* command load_restart */
		{ 0x00F2DFCD, NewLoadRestart },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
/* gcl chara end */
	{ 0, NULL }
};
