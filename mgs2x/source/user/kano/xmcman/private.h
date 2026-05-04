/*
  内部ヘッダ

  2002/04/15 M.Kobayashi
  $Id: private.h,v 1.12 2002/09/25 06:56:00 usr03700 Exp $

 */

// 文字列クラス
typedef struct MCX_STRINGS {
	char*	str;			// 文字列
	int		len;			// 長さ
	short	x;			
	short	y;				// 位置
	int		col;			// 色
	u_char	nofocus_a;		// フォーカスなしのときのアルファ
	MCX_INTERP intrp_alpha;	// アルファ補間
	SPR_OBJ**	ppObj;	// スプライト列へ
	struct MCX_STRINGS* pNext;		// リンク
} MCX_STRINGS;

extern void MCX_StringsInitBuffer( SPR_OBJ** ppObj, int len,
								   int	tex_handle,
								   int tex_strcode );
extern void MCX_StringsCreate(MCX_STRINGS* pStrings, char* str, short x, short y, int col , u_char nofocus_a);
extern void MCX_StringsDelete(MCX_STRINGS* pStrings);
extern void MCX_StringsPrintf(MCX_STRINGS* pStrings, const char* fmt, ... );
extern void MCX_StringsMove( MCX_STRINGS* pStrings, int n, int x );
extern void MCX_StringsUpdate( void );
extern void MCX_StringsShow( MCX_STRINGS* pStrings, int ctr );
extern void MCX_StringsHide( MCX_STRINGS* pStrings, int ctr );
extern void MCX_StringsNofocus( MCX_STRINGS* pStrings, int ctr );

// 文字列クラス(MENU_系)
typedef struct MCX_MENU_STRINGS {
	char*	pRes;	// 文字列リソース
	// 実際の画面上の位置
	short	x1,y1;
	short	w,h;
	// テクスチャバッファ上の位置
	short	u1,v1;
	short	uw,vh;
	u_int	col;	// 色
	MCX_INTERP	intrp_alpha;	// アルファ補間
	struct MCX_MENU_STRINGS* pNext;	// リンク
} MCX_MENU_STRINGS;

extern void MCX_MStringsInit( void* pfm );
extern void MCX_MStringsCreate( MCX_MENU_STRINGS* pstr,
								char* pRes,
								short x1, short y1,
								short w, short h,
								short u1, short v1,
								short uw, short vh,
								u_int	col );
extern void MCX_MStringsDelete( MCX_MENU_STRINGS* pstr );
extern void MCX_MStringsUpdate( void );
extern void MCX_MStringsShow( MCX_MENU_STRINGS* pstr, int ctr );
extern void MCX_MStringsHide( MCX_MENU_STRINGS* pstr, int ctr );
extern void MCX_MStringsPrintf( MCX_MENU_STRINGS* pstr, const char* fmt, ... );
extern void MCX_MStringsClearLine( MCX_MENU_STRINGS* pms );
extern void MCX_MStringsNofocus( MCX_MENU_STRINGS* pstr, int ctr );

// L2D の部品を制御(L2Dシステム内でやるべき。。。)
typedef struct MCX_L2D_MORF {
	int			codeobj;	// L2Dオブジェクトコード	
	int			codestart;	// スタートキーコード
	int			codeend;	// エンドキーコード
	int			l2d_handle;	// L2Dハンドル
	struct	MCX_L2D_MORF* pNext;		// リンクリスト	
	MCX_INTERP	intrp_rate;	// モーフィング割合補間オブジェクト (0-1)
} MCX_L2D_MORF;

extern void MCX_L2DMorfCreate(MCX_L2D_MORF* pmorf, int l2d_handle, int codeobj );
extern void MCX_L2DMorfDelete(MCX_L2D_MORF* pmorf );
extern void MCX_L2DMorfUpdate( void );
extern void MCX_L2DMorfSet( MCX_L2D_MORF* pmorf, int codestart, int codeend, int ctr );
extern void MCX_L2DMorfExit( void );


// ファイルセレクト画面クラス（共通部分）
typedef struct MCX_FILE_SELECT {
	MCX_STATE	state;
	SPR_OBJ		**ppobjbuf;

	int	 		cursel;		// 現在のファイル選択肢	
	int			nselection;	// 選択肢の数
	int			maxpage;
	int			curpage;
	u_int		flag;
	int			yn_cursor;		// Yes/No カーソル位置

	SPR_OBJ		*pSprcursor[ 2 ];	// カーソル表示スプライト
	MCX_INTERP	intrp_cursor_y;	// カーソルｙ座標補間
	MCX_INTERP	intrp_cursor_a;	// カーソルalpha 補間

	MCX_STRINGS		str_yes;
	MCX_STRINGS		str_slash;
	MCX_STRINGS		str_no;
	MCX_STRINGS		str_page;
	MCX_STRINGS		str_pagenum;

	MCX_MENU_STRINGS	mstr_create;
	MCX_MENU_STRINGS	mstr_overwrite;

	MCX_INTERP*		pIntrpJudge;	// 補間終了判断に使う

//	int			ctr;	// 終了ウェイト用カウンタ

	void		(*ShowCurrentPage)( MCX_MAN* pm, void* ps );
	void		(*HideCurrentPage)( MCX_MAN* pm, void* ps );
	void		(*EnterYesNo)( MCX_MAN* pm, void* ps );	// YesNo に入るときのＧＵＩ変化
	void		(*ExitYesNo)( MCX_MAN* pm, void* ps );	// YesNo から出るときのＧＵＩ変化
	void		(*SetFileName)( MCX_MAN* pm, void* ps );// セーブをする直前にファイルネームを決める
	void		(*MoveFocus)( MCX_MAN* pm, void* ps, int prevcursor );	// カーソルが動いた瞬間の処理
	void		(*GuiLocal)( MCX_MAN* pm, void* ps );		// ＧＵＩ非共通部分
	void		(*FileComplete)( MCX_MAN* pm, void* ps );	// ファイル操作成功

	void		(*GuiActive)( MCX_MAN* pm, void* ps );	// ACTIVE 時関数
	BOOL		(*CursorMove)( MCX_MAN* pm, void* ps, u_short press );	// カーソル移動処理
	short		(*Select2Y)( int sel );

} MCX_FILE_SELECT;

enum {
	SAVEGAME_ANS_SUCCESS=0,
	SAVEGAME_ANS_FAILED_CANCEL,
	SAVEGAME_ANS_FAILED_NO_FREEAREA,
	SAVEGAME_ANS_FAILED_NO_CARD,
	SAVEGAME_ANS_FAILED_ERROR_CARD,

	SAVEGAME_ANS_FAILED_UNFORMATTED,
};

enum {
	DEV_HDD		= 0,

	DEV_MU_TOP	= 0x10,
	DEV_MU1A	= DEV_MU_TOP,
	DEV_MU2A,
	DEV_MU3A,
	DEV_MU4A,
	DEV_MU1B,
	DEV_MU2B,
	DEV_MU3B,
	DEV_MU4B,
};
#define GetDeviceMask( dev ) ( 1 << ((( dev & 4 ) << 2) | ( dev & 3 )) )

#define MCX_StringsIsShow( ps ) ( ((ps)->intrp_alpha.ctr == 0) && ((ps)->intrp_alpha.fCurrent != 0.f ) )
#define MCX_StringsIsHide( ps ) ( ((ps)->intrp_alpha.ctr == 0) && ((ps)->intrp_alpha.fCurrent == 0.f ) )

#define MCX_MStringsIsShow( ps ) ( ((ps)->intrp_alpha.ctr == 0) && ((ps)->intrp_alpha.fCurrent != 0.f ) )
#define MCX_MStringsIsHide( ps ) ( ((ps)->intrp_alpha.ctr == 0) && ((ps)->intrp_alpha.fCurrent == 0.f ) )

enum {	// イベント定義
	MCX_EVENT_MC_STATUS,	// メモリカード状態変化
	MCX_EVENT_MC_NULL,		// 対象メモリカード無効化
	MCX_EVENT_FIND_SUCCESS,	// セーブゲーム列挙完了
	MCX_EVENT_SL_ERROR,		// セーブロードエラー
	MCX_EVENT_SL_SUCCESS,	// セーブロード完了
};

enum {	// リクエスト定義
	MCX_REQUEST_FIND_GAME,	// ゲームデータを探す
	MCX_REQUEST_SAVE_GAME,	// ゲームデータを作成する
	MCX_REQUEST_DESTROY,	// スレッド終了
};

enum {	// ＧＵＩ状態
	MCX_GUI_STATE_IDLE,		// 初期状態
	MCX_GUI_STATE_FADEIN,
	MCX_GUI_STATE_ACTIVE,
	MCX_GUI_STATE_SUSPEND,	// なにもしない
	MCX_GUI_STATE_FADEOUT,
	MCX_GUI_STATE_SYSTEMMAX,
};


#define	NUM_PORTS	4
#define NUM_SLOTS	2

#define BLOCK_SIZE	16384

extern MCX_FILEPROP*	MCX_GetFileInfo( int n );

extern void MCX_SaveLoadThread( int id, MCX_MAN* pm );

extern MCX_STATE*	MCX_GetDefaultState_DevSelect( void );
extern MCX_STATE*	MCX_GetDefaultState_FileCheck( void );
//extern MCX_STATE*	MCX_GetDefaultState_FileSelect( void );
extern MCX_STATE*	MCX_GetDefaultState_SaveLoad( void );

extern void	MCX_SetFileSelect( MCX_FILE_SELECT* ps );
extern MCX_STATE*	MCX_GetFileSelectPhoto( void );
extern MCX_STATE*	MCX_GetFileSelectGame( void );
extern MCX_STATE*	MCX_GetFileSelectPhotoLoad( void );
extern MCX_STATE*	MCX_GetPhotoEditState( void );


#define T_SHORT		DIRECT_TICK(10)
#define T_MID		DIRECT_TICK(20)
#define T_LONG		DIRECT_TICK(30)

#define	AUTOREPEAT_INTERVAL_1ST		DIRECT_TICK(15)		// 最初のリピート間隔（共通）


#define	LFILE_NAME_GAME		L"Game Data"
#define	FILE_NAME_GAME		"Game Data"
#define	LFILE_NAME_PHOTO	L"Photo Data"
#define	FILE_NAME_PHOTO		"Photo Data"
#define	LFILE_NAME_VR		L"Missions Data"
#define	FILE_NAME_VR		"Missions Data"
#define	LFILE_NAME_SNAKE_T	L"Snake Tales Data"
#define	FILE_NAME_SNAKE_T	"Snake Tales Data"

#ifdef EU
#define FILE_NAME_ENCODE_RADIX	63
#else
#define FILE_NAME_ENCODE_RADIX	62
#endif


//#define MOUNT_MU	// メモリカードを使う

