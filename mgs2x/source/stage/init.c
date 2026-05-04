/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA NewVramInitialize;
extern NEWCHARA NewTdgCameraStatus;
extern NEWCHARA NewForeach;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA ComGetLanguage;
extern NEWCHARA NewGameInit;
extern NEWCHARA NewGclLangUpdate;
extern NEWCHARA NewTdgCameraStatusOff;
extern NEWCHARA ComSetProductCode;
extern NEWCHARA NewGetConfig;
extern NEWCHARA NewArraySet;
/* 11 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command VRAM初期化 */
		{ 0x000223D1, NewVramInitialize },
	/* command タンカーカメラステータス */
		{ 0x00264D3F, NewTdgCameraStatus },
	/* command foreach */
		{ 0x00542B2D, NewForeach },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* command ＰＳ２から言語設定を取り出す */
		{ 0x005EB488, ComGetLanguage },
	/* chara MGSシステム初期化 */
		{ 0x006F7D4D, NewGameInit },
	/* command 字幕言語設定 */
		{ 0x0074E86B, NewGclLangUpdate },
	/* command タンカーカメラステータス初期化 */
		{ 0x0081584F, NewTdgCameraStatusOff },
	/* command プロダクトコード設定 */
		{ 0x0092A625, ComSetProductCode },
	/* command getconfig */
		{ 0x0092EB54, NewGetConfig },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
/* gcl chara end */
	{ 0, NULL }
};
