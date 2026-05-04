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
extern NEWCHARA NewResidentResourceSet;
extern NEWCHARA NewFontInit;
/* 3 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command VRAM初期化 */
		{ 0x000223D1, NewVramInitialize },
	/* command 常駐リソース設定 */
		{ 0x00AB5A2A, NewResidentResourceSet },
	/* command フォント初期化 */
		{ 0x00DE07C0, NewFontInit },
/* gcl chara end */
	{ 0, NULL }
};
