/*
	STAGE キャラクタテーブル用ひな型ファイル
	シナリオのディレクトリに置いて下さい。
	$Id: _base.c,v 1.2 1999/11/05 07:12:46 usr01475 Exp $
*/

#include <stdio.h>
#include "libgv.h"
#include "../game/game.h"

/* gcl chara */
extern NEWCHARA ComNPlantDogtags;
extern NEWCHARA NewSK_Result;
extern NEWCHARA NewSaveGameScr;
extern NEWCHARA NewGetNewItemScr;
extern NEWCHARA NewAllocDummyMemory;
extern NEWCHARA NewSaveVariable;
extern NEWCHARA NewDelay;
extern NEWCHARA ComNTankerDogtags;
extern NEWCHARA NewClearCodeScr;
extern NEWCHARA NewSetMenuStatus;
extern NEWCHARA NewConfirmSaveScr;
extern NEWCHARA NewSetExchangeSeCode;
extern NEWCHARA ComLocalResourceSet;
extern NEWCHARA NewGetShipwormStatus;
extern NEWCHARA NewLayout2D_Player;
extern NEWCHARA ComMaxTankerDogtags;
extern NEWCHARA NewGclVariableMove;
extern NEWCHARA NewCodecDaemon;
extern NEWCHARA ComMaxPlantDogtags;
/* 19 functions */
/* gcl chara end */

CHARA _StageCharacterEntries[] = {
/* gcl chara */
	/* command プラント編取得済みドッグタグ数 */
		{ 0x001F567B, ComNPlantDogtags },
	/* chara 結果画面 */
		{ 0x0022AE7D, NewSK_Result },
	/* chara セーブゲーム画面マネージャー */
		{ 0x00274B60, NewSaveGameScr },
	/* chara 取得アイテム表示 */
		{ 0x00494239, NewGetNewItemScr },
	/* command メモリオフセット初期化 */
		{ 0x004CEFC1, NewAllocDummyMemory },
	/* command varsave */
		{ 0x005B316E, NewSaveVariable },
	/* chara delay */
		{ 0x00743C9F, NewDelay },
	/* command タンカー編取得済みドッグタグ数 */
		{ 0x007C7E31, ComNTankerDogtags },
	/* chara 新クリアコード表示 */
		{ 0x008155F1, NewClearCodeScr },
	/* command メニュー設定 */
		{ 0x00944E11, NewSetMenuStatus },
	/* chara セーブ確認画面 */
		{ 0x009E8AEB, NewConfirmSaveScr },
	/* command 壁床効果音設定 */
		{ 0x00B44BBB, NewSetExchangeSeCode },
	/* command ローカルリソース設定 */
		{ 0x00B7B5A7, ComLocalResourceSet },
	/* command メニューフナムシ取り付き状態取得 */
		{ 0x00CB6EC4, NewGetShipwormStatus },
	/* chara 2Dレイアウトドライバ */
		{ 0x00CC87A2, NewLayout2D_Player },
	/* command タンカー編総ドッグタグ数 */
		{ 0x00D8CFF4, ComMaxTankerDogtags },
	/* command 解析封じ変数移動 */
		{ 0x00E76D74, NewGclVariableMove },
	/* chara 無線システム */
		{ 0x00E96D82, NewCodecDaemon },
	/* command プラント編総ドッグタグ数 */
		{ 0x00FF5B55, ComMaxPlantDogtags },
/* gcl chara end */
	{ 0, NULL }
};
