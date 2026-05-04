/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewCinemaScreen;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewDelay;
extern NEWCHARA NewStreamSet;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA PL_COM_GolCapOn;
extern NEWCHARA PL_COM_GolCapOff;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewStreamStop;
extern NEWCHARA NewFadeInOut_Scn;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA GM_SdSetGcl;
extern NEWCHARA NewWebSite;
/* 17 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara シネマスクリーン */
		{ 0x00690610, NewCinemaScreen },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command Stream */
		{ 0x007AA13A, NewStreamSet },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* command ゴルキャップあり */
		{ 0x00A60221, PL_COM_GolCapOn },
	/* command ゴルキャップなし */
		{ 0x00A6A1EE, PL_COM_GolCapOff },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command StreamStop */
		{ 0x00D30863, NewStreamStop },
	/* chara フェードインアウト */
		{ 0x00E22388, NewFadeInOut_Scn },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command セットサウンドコード */
		{ 0x00F2EEDC, GM_SdSetGcl },
	/* chara WEBサイト */
		{ 0x00FE5F3D, NewWebSite },
/* gcl chara end */
	{ 0, NULL }
};
