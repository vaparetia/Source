/*
	game.h
		ゲームコントロールライブラリヘッダ

	1999/05/26 K.Uehara
	$Id: game.h,v 1.7 2002/11/29 15:09:39 takaki Exp $
*/

#ifndef __GAME_H__
#define __GAME_H__

#ifdef __cplusplus
#define new new_ptr
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
	defines
*/

#define GM_DAEMON_PRIO	16		// ゲームデーモンのプライオリティ


/* ロード用 */

#define GM_REQ_SCREEN_ON	0x0080
#define GM_REQ_RESET		0x0040
#define GM_REQ_EXEC_PROC	0x0020
#define GM_REQ_SAVE_VAR		0x0010
#define	GM_REQ_RESTORE_VAR	0x0100
#define	GM_REQ_CONTINUE		0x0200

#define GM_LOAD_TYPE_MASK	0x000f

#if 1 //BP_DEBUG def DEBUG_MODE
#define	GM_REQ_DEBUG_RESET	0x0002
#endif

/*
	型宣言
*/

typedef void *NEWCHARA( int name, int map );

typedef struct {
	unsigned int class_id;
	NEWCHARA *new;
} CHARA;

/*
	ゲーム制御用変数
*/

/* シナリオとのリンク変数リスト */

#include "linkvar.h"

/* キャラクタエントリ */

extern CHARA MainCharacterEntries[];
extern CHARA *StageCharacterEntries;

/* ゲームのステータス情報 */

//extern int	GM_StagePlayTime ;	/* -> linkvar.h */
extern int	GM_CurrentStage ;
extern int	GM_StageHappening ;
extern int 	GM_GameStatus ;
extern int 	GM_GameStatusScn ;
extern int	GM_MenuStatus ;
extern int	GM_MenuStatusScn ;

/* サウンドステータス */
extern int GM_SdStatusCount[2] ;

/* ロード処理用 */

extern int 	GM_LoadRequest;
extern int 	GM_LoadComplete;
extern int	GM_ChangeResident ;

/* 振動処理用 */

extern int GM_PadVibrations1[] ;
extern int GM_PadVibrations2[] ;

#define	GM_PadVibration1	GM_PadVibrations1[ 0 ]
#define	GM_PadVibration2	GM_PadVibrations2[ 0 ]

#ifdef KP_WINDOWS
extern int GM_DispVibration1 ;
extern int GM_DispVibration2 ;
#endif

/* タンカー編写真セーブ領域 */

#define GM_TankerPicture	(( void * )MISC_BUFFER)
#define GM_TANKER_PICTURE_SIZE	(16*1024)

/* システムコールバック */
enum {
	GM_CALLBACK_LOADING = 0,
	GM_CALLBACK_RESTART,
	GM_CALLBACK_GAMEOVER,
	GM_CALLBACK_CONTINUE,
	GM_CALLBACK_ITEM,
	GM_CALLBACK_WEAPON,
	GM_CALLBACK_EXIT,
	GM_CALLBACK_PLAYER_DEAD,
	GM_CALLBACK_ACTIVE_MAP_CHANGE,
	GM_CALLBACK_ALERT_MODE_ENTER,
	GM_CALLBACK_ALERT_MODE_QUIT,
	GM_CALLBACK_MAX
} ;

/* ゲームオーバーフラグ */
enum {
	GM_OVERFLAG_NONE			= 0x0000,
	GM_OVERFLAG_OVER 			= 0x0001,
	GM_OVERFLAG_START_BY_ACTOR	= 0x0002,	
	GM_OVERFLAG_END_BY_ACTOR	= 0x0004,
	GM_OVERFLAG_EXEC_CALLBACK	= 0x0010,
	GM_OVERFLAG_LOGO_START		= 0x0020,
	GM_OVERFLAG_CLEARED			= 0x0040,
	GM_OVERFLAG_LOADOK			= 0x0080,
} ;

#define	GM_OVERFLAG_START	(GM_OVERFLAG_OVER|GM_OVERFLAG_START_BY_ACTOR)

/* プレイヤーがステージ間で保持すべき状態(GM_PlayerStateFlag) */
enum {
	PL_START_STATE_BLOOD =		0x0001,			/* 出血中 */
	PL_START_STATE_BLADE_OUT =	0x0002,			/* ブレード抜いている状態でスタート */
	PL_START_STATE_BLADE_INV =	0x0004,			/* ブレードさかさでスタート */
	PL_BLADE_MODE_MINEUCHI =	0x0008,			/* ブレード峰討ちモード */
	PL_SOCOM_SPPRSR_ATTACHED =  0x0010,			/* ソコムにサプレッサがついている */
	PL_AK_SPPRSR_ATTACHED =  	0x0020,			/* AKにサプレッサがついている */
	PL_GBSCAP_EXIST =			0x0040,			/* ゴル兵装にキャップが付いている */
	PL_USP_SPPRSR_ATTACHED =	0x0080,			/* ＵＳＰにサプレッサが付いている */
	PL_FACE_WOUNDED =			0x0100,			/* 顔にキズがある */
	PL_SHOWERED =				0x0200,			/* おしっこかけられた */
} ;

/* アクター優先順定義 */

/* AFTER2 */
enum {
	GM_TARGET_ACTOR_PRIO = 254,
	GM_RADAR_ACTOR_PRIO = 255,
} ;

/*
	関数宣言
*/

// gamed.c
extern	void	GM_ResetVibration( void ) ;
extern void	GM_InitRTCTime( void ) ;
extern void	GM_GetRTCTime( void ) ;

// in chara.c
void GM_InitChara( void );
void *GM_GetCharaID( int nID );
void GM_ResetChara( void );
void GM_SetupChara( void );
void *GM_GetCharaID( int nID );

// in loader.c
void *GM_StartLoader( char *dir );

// in area.c
void 	GM_InitArea( void );
void 	GM_SetArea( int id, char *dirname );
char 	*GM_GetArea( void );
void 	GM_ChangeResidentArea( char *dirname ) ;

void  BP_CheckResidentAreaHook();

// menupri.c
void *GM_StartMenuPrimManager( void );

// viewer.c
void *GM_StartDebugViewer( void );

// syscllbk.c
void	GM_ResetCallbackProc( void ) ;
void	GM_CallCallbackProc( int ) ;
void	GM_CallCallbackProc2( int which, void *args ) ;

/*
	インライン関数
*/
#if 1 //BP
//#ifdef KP_XBOX
#define GM_SetCurrentMap(_id )  (GM_CurrentMap=(_id))
#else
static	inline	void	GM_SetCurrentMap( int map_bit )
{
    extern int GM_CurrentMap ;

    GM_CurrentMap = map_bit ;
}
#endif

/* 
   デバッグモード 
*/
#if 0 // BP

#ifdef DESIGN_PREVIEW

#ifdef DEBUG
extern	int	GM_Debug2PMode ;
enum {
    GM_DEBUG_MODE_OFF = 0,
    GM_DEBUG_MODE_VIEWER,
    GM_DEBUG_MODE_PREVIEW,
    GM_DEBUG_MODE_CAMERA_SET,
    GM_DEBUG_MODE_PLAYER_STATE,
    GM_DEBUG_MODE_MAX,
    GM_DEBUG_MODE_CAMERA_VIEW,
    GM_DEBUG_MODE_ENEMY_THINK,
    GM_DEBUG_MODE_BGM_COMPOSE,
    GM_DEBUG_MODE_RADAR,
	GM_DEBUG_MODE_PADREC,
	GM_DEBUG_MODE_GAME_STATUS,
	GM_DEBUG_MODE_PATROLS,
} ;
#endif

#else

#ifdef DEBUG
extern	int	GM_Debug2PMode ;
enum {
    GM_DEBUG_MODE_OFF = 0,
    GM_DEBUG_MODE_CAMERA_SET,
    GM_DEBUG_MODE_CAMERA_VIEW,
    GM_DEBUG_MODE_PLAYER_STATE,
    GM_DEBUG_MODE_ENEMY_THINK,
	GM_DEBUG_MODE_PATROLS,
    GM_DEBUG_MODE_VIEWER,
    GM_DEBUG_MODE_PREVIEW,
    GM_DEBUG_MODE_BGM_COMPOSE,
    GM_DEBUG_MODE_RADAR,
	GM_DEBUG_MODE_PADREC,
	GM_DEBUG_MODE_GAME_STATUS,
    GM_DEBUG_MODE_MAX,
} ;
#endif

#endif

#endif // 0 - BP

extern int GM_Debug2PMode;

enum {
   GM_DEBUG_MODE_OFF = 0,
   GM_DEBUG_MODE_CAMERA_SET,
   GM_DEBUG_MODE_CAMERA_VIEW,
   GM_DEBUG_MODE_MAX, // BP - WE ONLY ALLOW CAMERA DEBUG
   GM_DEBUG_MODE_PLAYER_STATE,
   GM_DEBUG_MODE_ENEMY_THINK,
   GM_DEBUG_MODE_PATROLS,
   GM_DEBUG_MODE_VIEWER,
   GM_DEBUG_MODE_PREVIEW,
   GM_DEBUG_MODE_BGM_COMPOSE,
   GM_DEBUG_MODE_RADAR,
   GM_DEBUG_MODE_PADREC,
   GM_DEBUG_MODE_GAME_STATUS,
} ;


extern int GM_DebugModeEnable;

/*
	画面表示printf

	2D primの表示ルーチンは menuprim.h をincludeすること
*/

// locate のフラグに指定
#define MENU_MODE_NORMAL		0
#define MENU_MODE_LEFT			0
#define MENU_MODE_RIGHT			1
#define MENU_MODE_CENTER		2

// SetAlphaModeの引数
#define MENU_ALPHA_ADD			0,2,2,1,0x80
#define MENU_ALPHA_SUB			1,0,0,2,0x80
#define MENU_ALPHA_HALF			0,1,0,1,0x80

void *GM_StartMenuPrimManager( void );

void __MENU_Color( int which, int r, int g, int b, int a );
void __MENU_Locate( int which, int x, int y, int flag );
void __MENU_Print( int which, char *buffer );
void __MENU_Printf( int which, char *fmt, ... );
void __MENU_ResetColor( int which );
void __MENU_SetAlphaMode( long64 alpha );

/* アイコン表示用関数 */
void MENU_PutIcon( int type, int x, int y, int ptn, DG_DMAPACK_COLOR col, int flag );
/* 自由形式パケット表示用関数 */
void* MENU_PutPacket( int size );

/* ユーザー独自使用型MENU_Printf *//* 実際のバッファサイズはbuffer_size * 2 bytes になる */
void *NewMenuPrintManager( int buffer_size, int dmapack_flag, int dmapack_phase, int dmapack_prio );
void ___MENU_Color( void *work, int which, int r, int g, int b, int a );
void ___MENU_Locate( void *work, int which, int x, int y, int flag );
void ___MENU_Print( void *work, int which, char *buffer );
void ___MENU_Printf( void *work, int which, char *fmt, ... );
void ___MENU_ResetColor( void *work, int which );
void ___MENU_SetAlphaMode( void *work, long64 alpha );
void ___MENU_PrintMini( void *work_ptr, int which, char *buffer );

#if 0 //BP_GCC def __GNUC__
#define MENU_Printf( ... )	__MENU_Printf( 0, __VA_ARGS__ )
#else
extern void MENU_Printf( char *fmt, ... );
#endif
#define MENU_Locate( _x, _y, _flag )	__MENU_Locate( 0, _x, _y, _flag )
#define MENU_Color( _r, _g, _b, _a )	__MENU_Color( 0, _r, _g, _b, _a )
#define MENU_SetColor( _r, _g, _b )		__MENU_Color( 0, _r, _g, _b, 0 )
#define MENU_ResetColor()				__MENU_ResetColor( 0 );
#define MENU_SetAlphaMode( _a, _b, _c, _d, _fix )\
	__MENU_SetAlphaMode( SCE_GS_SET_ALPHA( _a, _b, _c, _d, _fix ) )

/* メニューウィンドウ用ミニフォント専用 */
#if 0 //BP_GCC def __GNUC__
#define MENU_S_Printf( ... )	__MENU_Printf( 1, __VA_ARGS__ )
#else
extern void MENU_S_Printf( char *fmt, ... );
#endif
#define MENU_S_Locate( _x, _y, _flag )	__MENU_Locate( 1, _x, _y, _flag )
#define MENU_S_Color( _r, _g, _b, _a )	__MENU_Color( 1, _r, _g, _b, _a )
#define MENU_S_SetColor( _r, _g, _b )		__MENU_Color( 1, _r, _g, _b, 0 )
#define MENU_S_ResetColor()				__MENU_ResetColor( 1 );

#if 0 //BP_GCC def __GNUC__
#define _MENU_Printf( ... )	__MENU_Printf( 0, __VA_ARGS__ )
#else
#define _MENU_Printf MENU_Printf
#endif
#define _MENU_Locate( _x, _y, _flag )	__MENU_Locate( 0, _x, _y, _flag )
#define _MENU_Color( _r, _g, _b, _a )	__MENU_Color( 0, _r, _g, _b, _a )

/*
	デバッグ用
*/
#if 0 //BP_GCC def __GNUC__
#define DEBUG_Printf( ... )		__MENU_Printf( 2, __VA_ARGS__ )
#else
extern void DEBUG_Printf( char *fmt, ... );
#endif
#define DEBUG_Locate( _x, _y, _flag )		__MENU_Locate( 2, _x, _y, _flag )
#define DEBUG_Color( _r, _g, _b, _a )		__MENU_Color( 2, _r, _g, _b, _a )
#define DEBUG_SetColor( _r, _g, _b )		__MENU_Color( 2, _r, _g, _b, 0 )
#define DEBUG_ResetColor()					__MENU_ResetColor( 2 )
extern int GM_DebugPrint_Off;	// これ以降DEBUGPRINTをOFFにする。

#ifdef KP_WINDOWS
#include <xtl.h>
/*
	Window Message 処理用
*/
extern	void	GM_WndApp(void) ;
extern	int		GM_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) ;
#endif	// KP_WINDOWS

/* 
   ポーズリクエスト
*/
enum {
	GM_PAUSE_REQ_ON = 1,
	GM_PAUSE_REQ_OFF,
} ;

#ifdef KP_XBOX	
// オーバレイなし用
typedef struct _TABLE_ALL_CHARA{
	CHARA*	pCharaStageEntries;
	char*	pStageName;
} TABLE_ALL_CHARA;

extern TABLE_ALL_CHARA GM_TableAllChara[];
#endif // KP_XBOX
	
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#undef new
#endif

#endif
