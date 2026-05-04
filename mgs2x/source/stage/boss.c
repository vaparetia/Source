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
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewPadControl;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA GM_COM_PauseOff;
extern NEWCHARA NewArraySet;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA NewSK_BossTelop2;
/* 10 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command 変数初期化 */
		{ 0x00053DA0, NewGclVarClear },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command パッド操作 */
		{ 0x006A725A, NewPadControl },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ポーズリセット */
		{ 0x00BD28C4, GM_COM_PauseOff },
	/* command 配列セット */
		{ 0x00C74F97, NewArraySet },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* chara ボスラッシュ２テロップ表示 */
		{ 0x00F466B6, NewSK_BossTelop2 },
/* gcl chara end */
	{ 0, NULL }
};
