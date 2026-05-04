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
extern NEWCHARA NewLoadGameScr;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewSetGameOverVox;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA GM_GameOverStartScn;
extern NEWCHARA NewShowPictureForVRBookScn;
extern NEWCHARA GM_GameOverEndScn;
extern NEWCHARA NewGclAssert;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewDelay;
extern NEWCHARA VR_NoContinue;
extern NEWCHARA GM_SeSetVolPanGcl;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewTnkPhotoViewGcl;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA Msn_SetDefaultData;
extern NEWCHARA GM_GameOverChoice;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewVRBook;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewSoundManager;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA ComSetReferenceData;
extern NEWCHARA New2DSprite;
extern NEWCHARA SetSnakeTalesStatus;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewLoadRestart;
extern NEWCHARA GM_SdSetGcl;
/* 36 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* chara ロードゲーム画面マネージャー */
		{ 0x0024D360, NewLoadGameScr },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command ゲームオーバー音声設定 */
		{ 0x005A4652, NewSetGameOverVox },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command ゲームオーバー処理開始 */
		{ 0x00635875, GM_GameOverStartScn },
	/* chara 一枚絵単独表示 */
		{ 0x0064FB35, NewShowPictureForVRBookScn },
	/* command ゲームオーバー処理終了 */
		{ 0x006856C1, GM_GameOverEndScn },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* command セットＶＲ発見回数 */
		{ 0x00718756, SetVR_DISCOVERY_CNT },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command ＶＲコンティニュー禁止 */
		{ 0x00843C09, VR_NoContinue },
	/* command ＳＥセットＶＰ */
		{ 0x0088CBF1, GM_SeSetVolPanGcl },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara タンカーカメラ写真表示 */
		{ 0x009E4D4A, NewTnkPhotoViewGcl },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ミッションズデータ初期化 */
		{ 0x00B7F8E7, Msn_SetDefaultData },
	/* command ゲームオーバー呼びわけ */
		{ 0x00BCB4A2, GM_GameOverChoice },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* chara 紙芝居 */
		{ 0x00C13513, NewVRBook },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara サウンドマネージャー */
		{ 0x00C88AFA, NewSoundManager },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
	/* command セーブ用参照変数登録 */
		{ 0x00DCB281, ComSetReferenceData },
	/* chara ２Ｄスプライト表示 */
		{ 0x00E0DBBA, New2DSprite },
	/* command テイルズステージセット */
		{ 0x00E3C577, SetSnakeTalesStatus },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command load_restart */
		{ 0x00F2DFCD, NewLoadRestart },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
/* gcl chara end */
	{ 0, NULL }
};
