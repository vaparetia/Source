/*
  Xbox 用メモリカードマネージャ

  2002/04/10 M.Kobayashi
  $Id: xmcman.h,v 1.8 2002/08/12 10:28:05 usr03700 Exp $
  
 */
#ifndef __XMCMAN_H__
#define __XMCMAN_H__

#include <xtl.h>

#include "../mcman/mcman.h"

enum {
	// ユーザ側設定
	MCX_MAN_FLAG_SAVE		= 0x00000001,	// セーブ時に立つ
	MCX_MAN_FLAG_FIRSTCHECK	= 0x00000002,	// 初期チェック時に立つ
	MCX_MAN_FLAG_DOGTAG		= 0x00000004,	// ドッグタグロード時に立つ
	MCX_MAN_FLAG_PHOTO		= 0x00000010,	// 写真セーブロード
	MCX_MAN_FLAG_GAME		= 0x00000020,	// ゲームセーブロード
	MCX_MAN_FLAG_VR			= 0x00000040,	// ＶＲセーブロード
	MCX_MAN_FLAG_SNAKE_T	= 0x00000080,	// スネークテイルズセーブロード
	MCX_MAN_FLAG_CODECMEM	= 0x00000100,	// codec memory 使用
	// システム側が設定
	MCX_MAN_FLAG_NOSPACE	= 0x00000200,	// ハードディスクに空き容量なし
	MCX_MAN_FLAG_MANYDIRS	= 0x00000400,	// 4096 のデータが既にある
};


// ゲームデータ構造体

#define MCX_PHOTO_NAME_MAX	21
#define MCX_VR_NAME_MAX		16
//#define	MCX_FILE_INFO_SIZE	64		// ファイル情報サイズ

#define MCX_MAX_PATH		31		// 内部で使う最大パス名長さ
#define MCX_MAX_GAMENAME	31		// 内部で使う最大ダッシュボード名長さ

typedef struct MCX_PHOTO_INFO {
	char	name[ MCX_PHOTO_NAME_MAX ];
	u_char	col[3];	//r,g,b
} MCX_PHOTO_INFO;

typedef union MCX_FILE_INFO {
	MCMAN_INFODATA	gameinfo;
	MCX_PHOTO_INFO	photoinfo;
	MCMAN_VR_INFODATA	vrinfo;
//	char			dummy[ MCX_FILE_INFO_SIZE ];
} MCX_FILE_INFO;

typedef struct MCX_FILEPROP {
	WIN32_FIND_DATA		finddata;
	char				szDir[ MCX_MAX_PATH + 1];				// セーブゲームディレクトリ
	WCHAR				szDashbordName[ MCX_MAX_GAMENAME + 1];	// ダッシュボード上で表示される名前
	MCX_FILE_INFO		info;							// 内部使用ファイル情報
	short				id;								// file id;
	char				type;							// ファイルタイプ
	u_char				flag;							// 状態フラグ
} MCX_FILEPROP;

#define MCX_FILEPROP_FLAG_DAMAGED	0x01				// 壊れファイル

// ファイルサイズ
#define		MCX_GAME_DATA_SIZE			( sizeof( MCMAN_GAMEDATA ) + sizeof( MCX_FILE_INFO ) + sizeof( MCX_SIGNATURE ) )
#define		MCX_PHOTO_DATA_SIZE			(1024 * 24 + sizeof( MCX_FILE_INFO ) + sizeof( MCX_SIGNATURE ) )
#define		MCX_VR_DATA_SIZE			( sizeof( MCMAN_GAMEDATA ) + sizeof( MCX_FILE_INFO ) + sizeof( MCX_SIGNATURE ) )
#define		MCX_SNAKE_T_DATA_SIZE		( sizeof( MCMAN_GAMEDATA ) + sizeof( MCX_FILE_INFO ) + sizeof( MCX_SIGNATURE ) )


typedef struct MCX_L2D_CODE_TABLE {	// Ｌ２Ｄカスタマイズ用
	// アクション
	int		a_check_show;
	int		a_file_show;
	int		a_hide;
} MCX_L2D_CODE_TABLE;

typedef struct MCX_MAN MCX_MAN;
typedef struct MCX_STATE MCX_STATE;

typedef struct MCX_STATE {	// 各状態クラス
	void	(*GUI)( MCX_MAN* pm, void* ps );					// GUI ルーチン
	void	(*Handler)( MCX_MAN* pm, void* ps, int event ); 	// イベントハンドラ
	void	(*Enter)( MCX_MAN* pm, void* ps );					// 起動時関数
	void	(*ClearStart)( MCX_MAN* pm, void* ps );				// 画面フェードアウト開始
	void	(*Exit)( MCX_MAN* pm, void* ps );					// 終了時関数
	void	(*L2dHandler)( MCX_MAN*pm, void* ps, int sign, int value );	// Ｌ２Ｄシグナルハンドラ
	int		gui_state;		// ＧＵＩ状態
} MCX_STATE;


enum {  // 状態クラス番号
	MCX_STATE_DEV_SELECT,	// デバイスセレクト
	MCX_STATE_FILE_CHECK,	// ファイルチェック
	MCX_STATE_FILE_SELECT,	// ファイルセレクト
	MCX_STATE_SL_VERIFY,	// セーブロード確認
	MCX_STATE_SL,			// セーブロード中
	MCX_STATE_MAX,
	// 以下マネージャ内部状態
	MCX_STATE_INIT,			// 初期状態
	MCX_STATE_END,			// 終了
};

#define	MCX_EVQ_MAX		16	// 	イベントキュー長さ
#define	MCX_REQ_MAX		16	// 	リクエストキュー長さ

typedef struct MCX_INTERP {	// GUI用線形補間サービス
	float	fCurrent;	// カレント値
	short	target;		// ターゲット値
	short	ctr;		// 補間カウンタ
	struct MCX_INTERP* pNext;	// ポインタ
} MCX_INTERP;

typedef struct MCX_SIGNATURE {
	DWORD	dwFileLength;
//BP    XCALCSIG_SIGNATURE Signature;
} MCX_SIGNATURE;

typedef struct MCX_MAN {	// マネージャクラス
	GV_ACT_EX	actor;	   
	int			curstate;	// カレントステート
	u_int		flag;
	int			result;		// 上位プロセスに渡す結果
	MCX_STATE*	pState[ MCX_STATE_MAX ];

	u_short		press;				// パッド状態
	u_short		pad_status;			// パッド状態
	int			repeat_interval;	// オートリピート間隔
	int			repeat_ctr;			// オートリピートカウンタ
	
	DWORD		devstate;	// デバイスステート
	DWORD		devchange;	// デバイス変更
	int			curdev;		// カレントデバイス
	int			l2d_handle;// 使用L2D
	int			l2d_handle2;// 使用L2D
	int			font_tri_handle;	// 使用文字テクスチャハンドル
	MCX_L2D_CODE_TABLE	l2d_tab;	

	int			hThread;		// スレッドハンドル
	int			hSemaChange;	// 自己状態書き換えセマフォ
	int			hSemaEvent;		// イベント用セマフォ

	MCX_INTERP*	pitop;		// 線形補間サービスリスト
	
	int			evq[ MCX_EVQ_MAX ];	// イベントキュー
	int			nev;				// イベント数
	int			req[ MCX_REQ_MAX ]; // リクエストキュー
	int			nre;				// リクエスト数

	MCX_FILEPROP*	pFiles;			// ファイル列挙
	int				nData;			// データ数
	int				latestfile;		// 最新ファイル
	int				maxdata;		// 最大ファイル数
	MCX_FILEPROP*	pCurfile;		// 操作対象ファイル

//	char		ext;				// データ拡張子（１文字）
	WCHAR*		pDSFileFmt;			// ダッシュボードファイル名	
	HANDLE		hFile;				// ファイルハンドル	
	HANDLE		hSignature;			// シグネチャ計算ハンドル
	MCX_SIGNATURE	sign;			// シグネチャ
	BOOL		bSave;				// save?load?

	int			require_block;		// 必要ブロック数
	int			free_block;			// 空きブロック数

	void*		pDataBuffer;		// （必要な場合）データバッファの位置
	void*		pLoadBuffer;		// （必要な場合）ロードデータの位置

	void*		pFontManager;		// MENU_系フォントマネージャ

	BOOL		(*SaveLoad)( void* pw );	// セーブロード時の関数
	BOOL		(*CreateTex)( void* ps );	// セーブイメージ作成
	void* 		pSlParam;					// セーブロード時パラメータ
	void		(*ExitFunc)( void* pw );	// 終了時の関数
	void* 		pExParam;					// 終了時パラメータ
} MCX_MAN;

extern void	MCX_InitManager( MCX_MAN* pm,
							 int l2d_handle, void* pFontManager, u_int flag );
extern void	MCX_QueueEvent( int ev );
extern void	MCX_QueueRequest( int re );
extern void	MCX_ExitState( MCX_STATE* ps );
extern void	MCX_ClearStart( int state );
extern void MCX_ChangeState( MCX_STATE* ps, int state );
extern void MCX_SetSaveLoadFunc( BOOL (*SaveLoad)( void* pw ),
								 void* pSlParam );
extern void MCX_SetExitFunc( void (*Exit)( void* pw ),
							 void* pExParam );

extern void MCX_SetAutoRepeat( int repeat_interval );

extern void MCX_SetFileName( MCX_FILEPROP* pFile, int id );
extern HANDLE	MCX_Open( DWORD dwDesiredAccess );
extern int	MCX_Write( HANDLE hFile, void* pBuf, int size );
extern int	MCX_Read( HANDLE hFile, void* pBuf, int size );
extern int MCX_Close( HANDLE hFile );

extern void* MCX_Malloc( int size );
extern void MCX_Free( void* ptr );

extern void MCX_AddInterpService( MCX_INTERP* pi );
extern void MCX_DeleteInterp( MCX_INTERP* pi );


extern void *CODEC_NewSnakeTalesSave( void );
extern void *CODEC_XNewMCSave(int l2d_strcode);

extern void MCX_ResetLoaddataID( void );
extern void MCX_SetLoaddataID( u_int type, int id );
extern short MCX_GetLoaddataID( u_int type );

#endif // __XMCMAN_H__
