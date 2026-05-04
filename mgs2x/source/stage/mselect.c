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
extern NEWCHARA NewLoadGameScr;
extern NEWCHARA NewSaveGameScr;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewVRSelect_Scn;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewGclAssert;
extern NEWCHARA SetVrStageId;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewDelay;
extern NEWCHARA ComStoreLoadedVariable;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewConfirmSaveScr;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewNodeDaemon;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA Msn_SetDefaultData;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA NewNode;
extern NEWCHARA GM_SdSetGcl;
/* 25 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* chara セーブファイル個数チェック */
		{ 0x0019BD75, NewFileExistCheckScn },
	/* chara ロードゲーム画面マネージャー */
		{ 0x0024D360, NewLoadGameScr },
	/* chara セーブゲーム画面マネージャー */
		{ 0x00274B60, NewSaveGameScr },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* chara ＶＲセレクト */
		{ 0x004CFE6E, NewVRSelect_Scn },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* command セットステージＩＤ */
		{ 0x006C274D, SetVrStageId },
	/* command セットＶＲ発見回数 */
		{ 0x00718756, SetVR_DISCOVERY_CNT },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command store_loadedvar */
		{ 0x0082A05E, ComStoreLoadedVariable },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara セーブ確認画面 */
		{ 0x009E8AEB, NewConfirmSaveScr },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* chara ノードシステム */
		{ 0x00AB381C, NewNodeDaemon },
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
	/* command ノード画面起動 */
		{ 0x00E3549B, NewNode },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
/* gcl chara end */
	{ 0, NULL }
};
