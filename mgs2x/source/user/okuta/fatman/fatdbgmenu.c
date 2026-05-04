/*
    fatdbgmenu.c
    ファットマン戦デバックメニュー
    2001/04/18 Masafumi Okuta
    $Id: fatdbgmenu.c,v 1.1.1.3 2002/11/19 11:47:58 Yoshizawa1 Exp $
*/
enum{ // データ用スイッチ
FATDBG_NONE,
FATDBG_PUTLV,  			// レベルベース表示
FATDBG_PUTTYPE, 		// タイプベース表示
FATDBG_CONT, 			// コンテナゾーン表示
FATDBG_BEHIND_POS,		// ビハインドデータ:位置
FATDBG_BEHIND_AREA,		// ビハインドデータ:位置
};
enum{ // ステータス用スイッチ
FATSTDBG_NONE,
FATSTDBG_MUTEKI, 		// 無敵表示
FATSTDBG_TRG,			// ターゲット
FATSTDBG_EYEPOS, 		// 捕捉データ表示
FATSTDBG_ROUTE, 		// ルート表示
FATSTDBG_SOUND, 		// サウンド
};
enum{ // 思考用スイッチ
FATTHKDBG_OFF,
FATTHKDBG_ON, 		// デバック思考
};

// メニュー用変数
static int nDbgFlag = 0;
static int nViewDbg = 0;
static int nFatStatusDbg = 0;
static int nFatThinkDbg = 0;
static int nFatMotDbg = 0;
static int nFatBombDbg = 0;

// yano
#ifdef PSX2

static GM_DEBUG_MENU debug_menu = {
    class:	"FATMAN",
    menu:	"DUMP-MODE",
    max:	2,
    items:( char *[] ){ "ON", "OFF" },
    values: ( int [] ){ 1, 0 },
    target: 	&nDbgFlag,
    mask:	0x00000001,	// 必須
};

static GM_DEBUG_MENU dbgview_menu = {
    class:	"FATMAN",
    menu:	"VIEW-MODE",
    max:	6,
    items:( char *[] ){ "OFF", "LEVEL", "TYPE", "CONTENA", "BEHIND-POS", "BEHIND-AREA" },
    values: ( int [] ){ 0, FATDBG_PUTLV, FATDBG_PUTTYPE, FATDBG_CONT, FATDBG_BEHIND_POS, FATDBG_BEHIND_AREA },
    target: 	&nViewDbg,
    mask:	0x0000000f,	// 必須
};

static GM_DEBUG_MENU dbgstatus_menu = { // ステータスデバック
    class:	"FATMAN",
    menu:	"STATUS-MODE",
    max:	6,
    items:( char *[] ){ "OFF", "MUTEKI_V", "TARGET", "EYEPOS", "NEXT_ROUTE", "SOUND" },
    values: ( int [] ){ FATSTDBG_NONE, FATSTDBG_MUTEKI, FATSTDBG_TRG, 
			FATSTDBG_EYEPOS, FATSTDBG_ROUTE, FATSTDBG_SOUND },
    target: 	&nFatStatusDbg,
    mask:	0x0000000f,	// 必須
};

static GM_DEBUG_MENU dbgthink_menu = { // 思考デバック
    class:	"FATMAN",
    menu:	"THINK-MODE",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){ FATTHKDBG_OFF, FATTHKDBG_ON },
    target: 	&nFatThinkDbg,
    mask:	0x0000000f,	// 必須
};

static GM_DEBUG_MENU dbgmot_menu = { // 思考デバック
    class:	"FATMAN",
    menu:	"MOTION-CHECK",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){ 0, 1 },
    target: 	&nFatMotDbg,
    mask:	0x0000000f,	// 必須
};

static GM_DEBUG_MENU dbgbomb_menu = { // 爆弾設置デバック
    class:	"FATMAN",
    menu:	"BOMB-CHECK",
    max:	2,
    items:( char *[] ){ "OFF", "ON" },
    values: ( int [] ){ 0, 1 },
    target: 	&nFatBombDbg,
    mask:	0x00000001,	// 必須
};

#else

static char *debug_menu_items[] = {"ON","OFF"};
static int debug_menu_values[] = { 1, 0 };
static GM_DEBUG_MENU debug_menu = { 
	NULL,
	"FATMAN",
	"DUMP-MODE",
	debug_menu_items,
	debug_menu_values,
	&nDbgFlag,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *dbgview_menu_items[] = { "OFF", "LEVEL", "TYPE", "CONTENA", "BEHIND-POS", "BEHIND-AREA" };
static int dbgview_menu_values[] = { 0, FATDBG_PUTLV, FATDBG_PUTTYPE, FATDBG_CONT, FATDBG_BEHIND_POS, FATDBG_BEHIND_AREA };
static GM_DEBUG_MENU dbgview_menu = { 
	NULL,
	"FATMAN",
	"VIEW-MODE",
	dbgview_menu_items,
	dbgview_menu_values,
	&nViewDbg,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	6,
	0,
	0
};

static char *dbgstatus_menu_items[] = { "OFF", "MUTEKI_V", "TARGET", "EYEPOS", "NEXT_ROUTE", "SOUND" };
static int dbgstatus_menu_values[] = { FATSTDBG_NONE, FATSTDBG_MUTEKI, FATSTDBG_TRG, 
										 FATSTDBG_EYEPOS, FATSTDBG_ROUTE, FATSTDBG_SOUND };
static GM_DEBUG_MENU dbgstatus_menu = { 
	NULL,
	"FATMAN",
	"STATUS-MODE",
	dbgstatus_menu_items,
	dbgstatus_menu_values,
	&nFatStatusDbg,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	6,
	0,
	0
};

static char *dbgthink_menu_items[] = { "OFF", "ON" };
static int dbgthink_menu_values[] = { FATTHKDBG_OFF, FATTHKDBG_ON }; 
static GM_DEBUG_MENU dbgthink_menu = { 
	NULL,
	"FATMAN",
	"THINK-MODE",
	dbgthink_menu_items,
	dbgthink_menu_values,
	&nFatStatusDbg,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

static char *dbgmot_menu_items[] = { "OFF", "ON" };
static int dbgmot_menu_values[] = { 0, 1 };
static GM_DEBUG_MENU dbgmot_menu = { 
	NULL,
	"FATMAN",
	"MOTION-CHECK",
	dbgmot_menu_items,
	dbgmot_menu_values,
	&nFatMotDbg,
	0x0000000f,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};


static char *dbgbomb_menu_items[] = { "OFF", "ON" };
static int dbgbomb_menu_values[] = { 0, 1 };
static GM_DEBUG_MENU dbgbomb_menu = { 
	NULL,
	"FATMAN",
	"BOMB-CHECK",
	dbgbomb_menu_items,
	dbgbomb_menu_values,
	&nFatBombDbg,
	0x00000001,	 /* 必須 */
	NULL,
	0,
	0,
	2,
	0,
	0
};

#endif
