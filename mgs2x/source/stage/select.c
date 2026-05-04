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
extern NEWCHARA ComNPlantDogtags;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA ENEMEM_GclRenewMemory;
extern NEWCHARA NewForeach;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewGclAssert;
extern NEWCHARA GM_COM_SnatchEquip;
extern NEWCHARA SetVR_DISCOVERY_CNT;
extern NEWCHARA NewDelay;
extern NEWCHARA NewGclLangUpdate;
extern NEWCHARA ComNTankerDogtags;
extern NEWCHARA VR_GetVrTime;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA NewSelect;
extern NEWCHARA ENEMEM_GclAllReset;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA SIG_DogTagZeroClear;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA Msn_SetDefaultData;
extern NEWCHARA SetVR_ENEKILL_CNT;
extern NEWCHARA NewArraySet;
extern NEWCHARA GM_LoadPack;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewSoundTest;
extern NEWCHARA NewMpegPssMovieStr;
/* 27 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command プラント編取得済みドッグタグ数 */
		{ 0x001F567B, ComNPlantDogtags },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command 敵兵メモリー更新 */
		{ 0x0046FCD2, ENEMEM_GclRenewMemory },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command assert */
		{ 0x006B237D, NewGclAssert },
	/* command 武器アイテム取られ */
		{ 0x006FA5F5, GM_COM_SnatchEquip },
	/* command セットＶＲ発見回数 */
		{ 0x00718756, SetVR_DISCOVERY_CNT },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command 字幕言語設定 */
		{ 0x0074E86B, NewGclLangUpdate },
	/* command タンカー編取得済みドッグタグ数 */
		{ 0x007C7E31, ComNTankerDogtags },
	/* command ＶＲゲットタイム */
		{ 0x0080FB82, VR_GetVrTime },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* chara select */
		{ 0x0087A1C0, NewSelect },
	/* command 敵兵メモリーオールリセット */
		{ 0x0088F9B0, ENEMEM_GclAllReset },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* command ドッグタグゼロクリア */
		{ 0x00B74FD7, SIG_DogTagZeroClear },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command ミッションズデータ初期化 */
		{ 0x00B7F8E7, Msn_SetDefaultData },
	/* command セットＶＲキルカウント */
		{ 0x00BCEE1A, SetVR_ENEKILL_CNT },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* command ロードサウンドパック */
		{ 0x00DC83C5, GM_LoadPack },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara sound_test */
		{ 0x00FE6730, NewSoundTest },
	/* chara MOVIE再生 */
		{ 0x00FFBECE, NewMpegPssMovieStr },
/* gcl chara end */
	{ 0, NULL }
};
