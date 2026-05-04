//
//	res_common.gcl
//	常駐データシナリオ共通
//
//	2001/02/14	M.Sonoyama
//	$Id: res_common.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $

command	フォント初期化
//command VRAM初期化  /*未実装 2002.01.31*/

//----------------------------------------------------------------
// リソース関連定義場所
//

// メニュー用説明文リソース設定

//日本語版
#ifdef d:JAPANESE
#include	"menu_exp_j.h"
#else

//英語版
#include	"menu_exp_e.h"

// ヨーロッパ版
#ifdef d:PAL
#include	"menu_exp_g.h"
#include	"menu_exp_f.h"
#include	"menu_exp_i.h"
#include	"menu_exp_s.h"
#endif

#endif


//日本語版
#ifdef d:JAPANESE
#include "savegame_j.rsc"
#else

//英語版
#include "savegame_e.rsc"

// ヨーロッパ版
#ifdef d:PAL
#include "savegame_eu.rsc"
#endif

#endif


//日本語版
#ifdef d:JAPANESE
#include "stagename_j.h"
#else
//英語版
#include "stagename_e.h"
#endif


// リソースの登録
#ifdef d:NTSC

command 常駐リソース設定 \
	-string [武器説明:*] 1 [アイテム説明:*] 2 [無線メッセージ:*] 3 [無線相手:*] 4 \
			[セーブゲームリソース:*] 5 [ステージ名:*] 6 [難易度名:*] 7

#endif


#ifdef d:PAL

if ( $w:言語 == d:LANG_FRENCH ){
	command 常駐リソース設定 \
		-string [武器説明_F:*] 1 [アイテム説明_F:*] 2 [無線メッセージ_F:*] 3 [無線相手_F:*] 4 \
				[セーブゲームリソースフランス語:*] 5 [ステージ名:*] 6 [難易度名:*] 7
} else if ( $w:言語 == d:LANG_GERMANY ){
	command 常駐リソース設定 \
		-string [武器説明_G:*] 1 [アイテム説明_G:*] 2 [無線メッセージ_G:*] 3 [無線相手_G:*] 4 \
				[セーブゲームリソースドイツ語:*] 5 [ステージ名:*] 6 [難易度名:*] 7
} else if ( $w:言語 == d:LANG_ITALY ){
	command 常駐リソース設定 \
		-string [武器説明_I:*] 1 [アイテム説明_I:*] 2 [無線メッセージ_I:*] 3 [無線相手_I:*] 4 \
				[セーブゲームリソースイタリア語:*] 5 [ステージ名:*] 6 [難易度名:*] 7
} else if ( $w:言語 == d:LANG_SPANISH ){
	command 常駐リソース設定 \
		-string [武器説明_S:*] 1 [アイテム説明_S:*] 2 [無線メッセージ_S:*] 3 [無線相手_S:*] 4 \
				[セーブゲームリソーススペイン語:*] 5 [ステージ名:*] 6 [難易度名:*] 7
} else {
	command 常駐リソース設定 \
		-string [武器説明:*] 1 [アイテム説明:*] 2 [無線メッセージ:*] 3 [無線相手:*] 4 \
				[セーブゲームリソース:*] 5 [ステージ名:*] 6 [難易度名:*] 7
}

#endif


// 常駐サウンド読み込み
//chara サウンドマネージャー ＳＤマネ -p 0 /*未実装 2002.01.31*/

//----------------------------------------------------------------

