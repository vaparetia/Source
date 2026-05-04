/*
	debugmenu.h
		デバッグメニューヘッダ
	2001/02/19	K.Uehara
	$Id: debugmenu.h,v 1.1.1.3 2002/11/19 11:41:47 Yoshizawa1 Exp $
*/

/*
	pause中に2Pのボタンを押すと出現
	フラグ関係の管理を行なう。
	基本的にステージをまたぐと消えるので、各キャラの起動時に登録する。

	クラス(max8char)とMENU(max16char)によって識別
		
		"PLAYER"  "INVISIBLE MODE"
		"ENEMY"   "BLIND MODE"

	種類は以下のようになる。

	n値文字列
		"OFF", "ON", "TYPE 1", ...
	n値数字
		0, 1, 2, 3, 4, ...

	登録
		見やすいので,GCC拡張(C99標準)の形式を推奨。

	static GM_DEBUG_MENU debug_menu = {
		class:	"PLAYER",
		menu:	"INVISIBLE",
		max:    2,
		items:  ( char *[] ){ "ON", "OFF" },
		values: ( int [] ){ 1, 0 },
		target: &_DEBUG_Player_INVISIBLE,	// intの変数へのポインタ。代入される。
		mask:   0x00000001,
		type:   GM_DEBUG_MENU_FLAG,
	};

	items がNULLの場合は 0, 1, 2 .. (max-1)を表示。
	values が NULLの場合は、0 .. (max-1)が設定される。
	maskが指定されると,それ以外のビットは影響を受けない。無指定か0なら全ビット変更。

	typeは無指定(0)の場合はGM_DEBUG_MENU_FLAG(単に代入)。
	GM_DEBUG_MENU_FUNCを指定するとtargetをint (*func)(int)と読み変えて,
	変更時と表示時にその関数を呼び出す。
	typeに、GM_DEBUG_MENU_DECをORすると、メニューの表示が10進表記になる。

	重複登録はチェックされる。
	コピーされるわけではないので,実体はstaticで持っておく必要がある。
	関数はDEBUG_MODEがなければ消えるがGM_DEBUG_MENUの実体は消えないので,
	#ifdef DEBUG_MODEで囲っておく。

	起動時にcdrom.img/debug.cnfがあったら、それを読み込む。
	debug.cnfは以下のようなスクリプト。

	---
	"PLAYER" "INVISIBLE MODE" 1
	"ENEMY"  "BLIND_MODE"     0
	--

	数字はvaluesではなく、カレントの番号。
	スクリプト読み込みは,gameinit時に行なう。
	MENUをStrIDで検索する。GM_AddDebugMenu時に初期値として設定。
	したがってステージが切り替わる時にリセットされる。
*/

#ifndef _DEBUG_MENU_
#define _DEBUG_MENU_ 1

typedef struct _gm_debug_menu {
	struct _gm_debug_menu *next;	// システム使用
	char *class;
	char *menu;
	char **items;
	int *values;
	int *target;
	int mask;
	int ( *func )( int value, struct _gm_debug_menu *menu );
	int strid;						// 自動設定
	unsigned char type;
	unsigned char max;
	unsigned char current;			// システム使用
	unsigned char padding;
} GM_DEBUG_MENU;

#define GM_DEBUG_MENU_TYPE_MASK	0x0F
enum {
	GM_DEBUG_MENU_FLAG = 0,
	GM_DEBUG_MENU_FUNC,
};
#define GM_DEBUG_MENU_DEC		0x10	// 10進表記

void GM_AddDebugMenu( GM_DEBUG_MENU *menu );

/* 内部使用 */

typedef struct {
	unsigned int value;
	unsigned int strid;
} GM_DEBUG_MENU_TABLES;

void GM_InitDebugMenu( void );
void GM_ResetDebugMenu( void );
void GM_ActDebugMenu( GV_PAD *pad );
void GM_LoadDebugMenuConf( void );
void GM_SetDebugMenuTables( GM_DEBUG_MENU_TABLES *table, int table_num );

/* DEBUG_MODE がなければ,消す */

#ifndef _DEBUG_MENU_BODY_

#ifndef DEBUG_MODE

#define GM_InitDebugMenu()
#define GM_ResetDebugMenu()
#define GM_ActDebugMenu()
#define GM_LoadDebugMenuConf()

#define GM_AddDebugMenu( a )

#endif	// ndef DEBUG_MODE

#endif  // ndef _DEBUG_MENU_BODY_

#endif	// _DEBUG_MENU_
