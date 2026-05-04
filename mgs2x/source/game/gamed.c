//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gamed.c
		ゲーム進行デーモン

	1999/05/26	K.Uehara
	$Id: gamed.c,v 1.16 2002/12/22 10:37:06 takaki Exp $
*/
#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libcdvd.h>

#include "jimaku.h"

#include "BP_BuildDefines.h"
#include "BP_DebugPad.h"
#include "BP_Debug.h"
#include "BP_SoundSupport.h"
#include "BP_Misc.h"
#include "BP_SplashScreen.h"
#include "BP_Renderer.h"

#if 0 //BP_PS2
#include "jsifman.h"
#endif

#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "libgv.h"
#include "libfs.h"
#include "libdg.h"
#include "libgcl.h"
#include "libdg.cnf"
#include "libhzx.h"
#include "libmt.h"
#include "util.h"

#include "game.h"
#include "font.h"

#include	"g_sound.h"

#include	"g_define.h"
#include	"g_macro.h"
#include	"g_struct.h"

#include	"debugmenu.h"
#include	"strctrl.h"
#include	"game.x"

#ifdef PSX2
#include	"sd_ee.h"
#include	"sonycode.h"
#endif

#ifdef KP_XBOX
#include	"jimaku.h"
#endif

#ifdef KP_WINDOWS
#include	"filtergraph.h"
#endif

extern void GM_ResetMenuPrimManager( void );

#define START_UP_DIRECTORY	"init"

/*
	システム制御変数宣言
*/
//int	GM_StagePlayTime = 0 ;	/* -> linkvar.h */
int	GM_CurrentStage = 0 ;
int	GM_StageHappening = 0 ;	/* ステージがスタートしてから発生した出来事 */

int GM_GameStatus = 0 ;
int	GM_GameStatusScn = 0 ;

int	GM_MenuStatus = 0 ;		
int	GM_MenuStatusScn = 0 ;

int GM_LoadRequest = 0;
int GM_LoadComplete;

int GM_PadVibrations1[ GV_PAD_MAX ] = { 0 } ;
int GM_PadVibrations2[ GV_PAD_MAX ] = { 0 } ;

int GM_PadVibrations1Force[ GV_PAD_MAX ] = { 0 } ;
int GM_PadVibrations2Force[ GV_PAD_MAX ] = { 0 } ;

#ifdef KP_WINDOWS
int GM_DispVibration1 = 0 ;
int GM_DispVibration2 = 0 ;

int GM_DispVibration1Force = 0 ;
int GM_DispVibration2Force = 0 ;
#endif

int	GM_GameOverTimer = 0 ;
int	GM_GameOverFlag = 0 ;
#ifdef DEBUG
int	GM_GameOverStartTime = 0 ;
#endif

int GM_SdStatusCount[2] ;	/* [0]now [1]old */


int	GM_PauseRequest = 0 ;
int	GM_DisablePauseCount = 0 ;

int GM_PadResetDisable = 0;
static int pad_reset_count = 0;


#if defined(DEBUG_MODE) || BP_ENABLE_DEBUG_CAMERA()
int	GM_Debug2PMode = 0 ;
int GM_DebugModeEnable = 1;
#endif

/* ボスラッシュ、ローポリ劇場ではオプションフラグを保存しておく必要があるため */
short Stock_GM_Configuration;
int   Stock_GM_Configuration2;

extern int BP_BASE_TICK();
#ifdef PAL
#define	TIME_BASE	(BP_BASE_TICK())
#else
#define	TIME_BASE	(BP_BASE_TICK())
#endif

// 呼びわけ koba4
void *( *SK_GameOverAct )( int );

// Game, Photo, VR, Tales のセーブデータの数
short GM_SaveDataNum[ 4 ];

// アプリケーション側からの処理要求
#ifdef KP_WINDOWS
static	BOOL	GM_AppReqPause_ByActive ;	// Active状態変更によるPAUSE要求
static	BOOL	GM_AppReqPause_ByExit ;		// 終了処理によるPAUSE要求

BOOL	GM_AppExit_ConfReq ;	// 終了処理問い合わせ要求
BOOL	GM_AppExit_Conf ;		// 終了処理問い合わせ中
#endif

/* ------------------------------------------------------------- */
/* 適当なテストをする時はここで */

static int gm_disc_error_count = 0;

//#define TEST

#ifdef TEST
static void Test()
{
}
#endif

/* ------------------------------------------------------------- */

/*
	処理定数
*/

#define KILLING_COUNT	3	// ステージ切り替え時の待ち

enum GAMED_STATE {
	WAIT_LOAD,		/* ロード完了待ち */
	WORKING,		/* ステージ処理中 */
};

typedef struct {
	GV_ACT actor;
	enum GAMED_STATE status;			// 処理状態
	int killing_count;					// ステージ切り替え用
} Work;

/* ------------------------------------------------------------- */

static void ResetSystem( void )
{
	GV_ResetSystem();
	DG_ResetSystem();
	GCL_ResetSystem();
	HZX_ResetSystem() ;	 /* M.Sonoyama */
	GM_InitVolumParam( ) ; /* Y.Kore */

	GM_ResetDebugMenu();
}

static void ResetMemory( void )
{
	HZX_ResetMemory();
	DG_ResetTexture();
	GV_ResetMemory();
	GM_ResetChara();
}

static void ResetStage( void )
{
	// ロードする前に各モジュールをリセットする
	GM_InitRadarSystem();
	GM_ResetVibration() ;
	//乱数初期化
	BP_PS2_srand( 0 ) ;
}

extern void PL_InitVariable( void );
extern void BRK_InitSystem( void ) ; /* T.Morita 2001.09.18 */
extern void	GM_VRFUNC_Init( void ) ;

static void StartStage( void )
{
	// ロード完了後に各管理プロセスをスタートする
	GM_SetupChara();
	GM_ResetVariable() ;
	GM_ResetCallbackProc() ;
	GM_InitHomingSystem() ;	/* Y.Korekado */
	GM_InitEneFindSystem() ;	/* Y.Korekado */
	GM_InitInterruptSystem() ;	/* Y.Korekado */
	GM_InitMapSystem() ;	 /* M.Sonoyama */
	GM_InitWhereSystem() ;   /* M.Sonoyama */
	GM_StartVibrationSystem() ;
	GM_StartTargetSystem() ; /* M.Sonoyama */
	HZX_StartTrapSwitchSystem() ; 	/* M.Sonoyama */
	HZX_InitTrapBehindSystem() ;	/* M.Sonoyama */
	GM_StartCamera() ;	 /* M.Sonoyama */
	GV_ResetPadSystem() ;
	PL_InitVariable() ;  /* プレイヤー、武器関連変数初期化 M.Sonoyama */
	GM_StreamReset();
	GM_VRFUNC_Init( ) ;	/* Y.Korekado */
	GM_StageHappening = 0 ;	/* Y.Korekado */

/*重野追加 ２ｄデーモン*/
	SPR_Init2D_ObjectDriver() ;
	BRK_InitSystem() ;   /* 壊れ物システムの初期化 T.Morita 2001.09.18 */
	
	/* ゲームオーバー関数を初期化 */
	SK_GameOverAct = NULL ;

	{	/* S.Okajima */
		extern void OK_EffectInitialize( void );
		OK_EffectInitialize();
	}

#ifdef KP_XBOX
	GM_CurrentAddr = GM_INVALID_ADDR ;
#endif

#if 1 //BP_XBOX def KP_XBOX
	GM_ResetMenuPrimManager();	// 常駐テクスチャの再イニシャライズに対応するため
#endif
}

/* ---------------------------------------------------------------------- */
/*
	システムデバッグメニュー
*/

#ifdef DEBUG_MODE

static int ErrorDispDisable = 0;

static int system_conf_func( int value )
{
	if( value == -1 ){
		return GM_Configuration;
	} else {
		GM_Configuration = value;
		printf( "GM_Configration = %X\n", value );
	}
	return value;
}

#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu1 = {
	class: "SYSTEM", menu: "VIBRATE", max: 2,
	items: ( char *[] ){ "ON", "OFF" },
	mask: GM_CONFIG_VIBRATION_OFF,
	values: ( int [] ){ 0, GM_CONFIG_VIBRATION_OFF },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
  } ;
#else
static char *debugmenu1_items[] = { "ON", "OFF" } ;
static int   debugmenu1_values[] = { 0, GM_CONFIG_VIBRATION_OFF } ;
static GM_DEBUG_MENU debugmenu1 = {
	NULL, "SYSTEM", "VIBRATE",
	debugmenu1_items,
	debugmenu1_values,
	(int *)system_conf_func,
	GM_CONFIG_VIBRATION_OFF,
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC,
	2,
	0,0
};
#endif	

#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu2 = {
	class: "SYSTEM", menu: "CAPTION", max: 2,
	items: ( char *[] ){ "ON", "OFF" },
	mask: GM_CONFIG_CAPTION_OFF,
	values: ( int [] ){ 0, GM_CONFIG_CAPTION_OFF },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
  } ;
#else
static char *debugmenu2_items [] = { "ON", "OFF" } ;
static int	 debugmenu2_values[] = { 0, GM_CONFIG_CAPTION_OFF } ;
static GM_DEBUG_MENU debugmenu2 = {
	NULL, 
	"SYSTEM",
	"CAPTION",
	debugmenu2_items ,
	debugmenu2_values,
	( int * )system_conf_func, /*target*/
	GM_CONFIG_CAPTION_OFF, /*mask*/
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC, /*type*/
	2,/*max*/
	0,0
};
#endif

#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu3 = {
	class: "SYSTEM", menu: "RADAR", max: 2,
	items: ( char *[] ){ "ON", "OFF" },
	mask: GM_CONFIG_RADAR_OFF,
	values: ( int [] ){ 0, GM_CONFIG_RADAR_OFF },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
};
#else
static char * debugmenu3_items[]  = { "ON", "OFF" } ;
static int    debugmenu3_values[] = { 0, GM_CONFIG_RADAR_OFF } ;
static GM_DEBUG_MENU debugmenu3 = {
	NULL,
	"SYSTEM",
	"RADAR",
	debugmenu3_items,
	debugmenu3_values,
	( int * )system_conf_func,/*target */
	GM_CONFIG_RADAR_OFF, /*mask */
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC, /*types */
	2,
	0,0
};
#endif
#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu4 = {
	class: "SYSTEM", menu: "BLOOD", max: 2,
	items: ( char *[] ){ "ON", "OFF" },
	mask: GM_CONFIG_BLOOD_OFF,
	values: ( int [] ){ 0, GM_CONFIG_BLOOD_OFF },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
};
#else
static char * debugmenu4_items[]  = { "ON", "OFF" } ;
static int    debugmenu4_values[] = { 0, GM_CONFIG_BLOOD_OFF } ;
static GM_DEBUG_MENU debugmenu4 = {
	NULL, "SYSTEM", "BLOOD",
	debugmenu4_items,
	debugmenu4_values,
	(int *)system_conf_func,
	GM_CONFIG_BLOOD_OFF,
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC,
	2,
	0,0
};
#endif	
#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu5 = {
	class: "SYSTEM", menu: "SHUKAN", max: 2,
	items: ( char *[] ){ "NORM", "REV" },
	mask: GM_CONFIG_SHUKAN_REVERSE,
	values: ( int [] ){ 0, GM_CONFIG_SHUKAN_REVERSE },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
};
#else
static char * debugmenu5_items[]  = { "NORM", "REV" } ;
static int    debugmenu5_values[] = { 0, GM_CONFIG_SHUKAN_REVERSE } ;
static GM_DEBUG_MENU debugmenu5 = {
	NULL, "SYSTEM", "SHUKAN",
	debugmenu5_items,
	debugmenu5_values,
	(int *)system_conf_func,
	GM_CONFIG_SHUKAN_REVERSE,
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC,
	2,
	0,0
};
#endif	
#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu6 = {
	class: "SYSTEM", menu: "GV_ERROR", max: 2,
	items: ( char *[] ){ "ON", "OFF" },
	values: ( int [] ){ 0, 1 },
	target: ( int * )&ErrorDispDisable,
};
#else 
static char *debugmenu6_items[]  = { "ON", "OFF" } ;
static int   debugmenu6_values[] = { 0, 1 } ;
static GM_DEBUG_MENU debugmenu6 = {
	NULL, "SYSTEM", "GV_ERROR",
	debugmenu6_items,
	debugmenu6_values,
	(int *)&ErrorDispDisable,
	0,
	NULL, /*func*/
	0   , /*strid*/
	0,
	2,
	0,0
};
#endif	
#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu7 = {
	class: "SYSTEM", menu: "QCHANGE_TYPE", max: 2,
	items: ( char *[] ){ "OLD", "NEW" },
	mask: GM_CONFIG_MENU_QCHANGE_EX,
	values: ( int [] ){ 0, GM_CONFIG_MENU_QCHANGE_EX },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
};
#else
static char *debugmenu7_items[]  = { "OLD", "NEW" } ;
static int   debugmenu7_values[] = { 0, GM_CONFIG_MENU_QCHANGE_EX } ;
static GM_DEBUG_MENU debugmenu7 = {
	NULL, "SYSTEM", "QCHANGE_TYPE",
	debugmenu7_items,
	debugmenu7_values,
	(int *)system_conf_func,
	GM_CONFIG_MENU_QCHANGE_EX,
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC,
	2,
	0,0
};
#endif	
#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu8 = {
	class: "SYSTEM", menu: "MENU_TYPE", max: 2,
	items: ( char *[] ){ "NEW", "OLD" },
	mask: GM_CONFIG_OLD_TYPE_MENU,
	values: ( int [] ){ 0, GM_CONFIG_OLD_TYPE_MENU },
	target: ( int * )system_conf_func,
	type: GM_DEBUG_MENU_FUNC,
};
#else
static char * debugmenu8_items[]  ={ "NEW", "OLD" } ;
static int    debugmenu8_values[] ={ 0, GM_CONFIG_OLD_TYPE_MENU };
static GM_DEBUG_MENU debugmenu8 = {
	NULL, "SYSTEM", "MENU_TYPE",
	debugmenu8_items,
	debugmenu8_values,
	(int *)system_conf_func,
	GM_CONFIG_OLD_TYPE_MENU,
	NULL, /*func*/
	0   , /*strid*/
	GM_DEBUG_MENU_FUNC,
	2,
	0,0
};
#endif	

int GM_DEMO_SkipFlag;

#ifdef __GNUC__	
static GM_DEBUG_MENU DEMO_debugmenu = {
	class: "DEMO", menu: "SKIP", max: 3,
	items: ( char *[] ){ "NONE", "NODATA", "ALL" },
	target: &GM_DEMO_SkipFlag,
};
#else
static char * DEMO_debugmenu_items[]  ={ "NONE", "NODATA", "ALL" } ;
static int    DEMO_debugmenu_values[] ={ 0, 1, 2 };
static GM_DEBUG_MENU DEMO_debugmenu = {
	NULL, "DEMO", "SKIP",
	DEMO_debugmenu_items,
	DEMO_debugmenu_values,
	&GM_DEMO_SkipFlag,
	0x2,
	NULL, /*func*/
	0   , /*strid*/
	0,
	3,
	0,0
};
#endif	

static int GM_PressDebug;

#ifdef __GNUC__	
static GM_DEBUG_MENU debugmenu9 = {
	class: "PAD", menu : "PRESS CHECK", max: 3,
	items: ( char *[] ){ "NO", "EVERY", "ODD", "EVEN" },
	target: ( int * )&GM_PressDebug,
};
#else
static char * debugmenu9_items[]  = { "NO", "EVERY", "ODD", "EVEN" } ;
static int    debugmenu9_values[] ={ 0, 1, 2, 3 };
static GM_DEBUG_MENU debugmenu9 = {
	NULL, "PAD", "PRESS CHECK",
	debugmenu9_items,
	debugmenu9_values,
	&GM_PressDebug,
	0x03,
	NULL, /*func*/
	0   , /*strid*/
	0,
	4,
	0,0
};
#endif	

#ifdef KP_XBOX
/* system/libdg/xchanl.cを参照 */
#define DG_XBOXDEBUG_SAFETYAREA			(0x0001)
extern int		DG_XBoxDebugFlag ;
static char * debugmenu10_items[]  ={ "OFF", "ON" } ;
static int    debugmenu10_values[] ={ 0, DG_XBOXDEBUG_SAFETYAREA };
static GM_DEBUG_MENU debugmenu10 = {
	NULL, "XBOX", "SAFETY-AREA",
	debugmenu10_items,
	debugmenu10_values,
	&DG_XBoxDebugFlag,
	DG_XBOXDEBUG_SAFETYAREA,
	NULL, /*func*/
	0   , /*strid*/
	0,
	2,
	0,0
};
#endif

static void AddSystemDebugMenu( void )
{
	GM_AddDebugMenu( &debugmenu1 );
	GM_AddDebugMenu( &debugmenu2 );
	GM_AddDebugMenu( &debugmenu3 );
	GM_AddDebugMenu( &debugmenu4 );
	GM_AddDebugMenu( &debugmenu5 );
	GM_AddDebugMenu( &debugmenu6 );
	GM_AddDebugMenu( &debugmenu7 );
	GM_AddDebugMenu( &debugmenu8 );
	GM_AddDebugMenu( &debugmenu9 );

	GM_AddDebugMenu( &DEMO_debugmenu );
#ifdef KP_XBOX
	GM_AddDebugMenu( &debugmenu10 );
#endif
}
#endif

/* ------------------------------------------------------------- */
/*
	ポーズ処理
*/

#ifdef KP_WINDOWS
static	BOOL	GM_PausingNow = FALSE ;
#endif

static void PauseOff( void )
{
	/* 強制ポーズ OFF */
	GV_PauseOffActorSystem( GV_PAUSE_PAUSE );
//	GM_SdSet( SNG_FIN_S ) ;
	GM_SdSet( SNG_PAUSEOFF ) ;
#ifdef KP_WINDOWS
	GM_FilterGraphPauseOff();
	GM_PausingNow = FALSE ;
#endif
}

static void PauseOn( void )
{
	/* ポーズ ON */
	if ( !GM_CheckGameStatus( STATE_PAUSE_DISABLE ) ) {
//		GM_SdSet( SNG_FOUTP_S ) ;
        GM_SdSet( SNG_PAUSEON ) ;
		GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;
#ifdef KP_WINDOWS
		GM_FilterGraphPauseOn();
		GM_PausingNow = TRUE ;
#endif
	}
}

static void Pause( void )
{
	if( ( GV_PauseLevel & ~GV_PAUSE_PAUSE ) == 0 ){
		if( !( GV_PauseLevel & GV_PAUSE_PAUSE ) ){
			PauseOn();
		} else {
			PauseOff();
		}
	}
}

/* ------------------------------------------------------------- */

/*
   振動処理
*/
void	GM_SetVibration1( int which, int value )
{
	GM_PadVibrations1[ which ] += value ;
}

void	GM_SetVibration2( int which, int value )
{
	GM_PadVibrations2[ which ] += value ;
}

void	GM_SetForceVibration1( int which, int value )
{
	GM_PadVibrations1Force[ which ] += value ;
}

void	GM_SetForceVibration2( int which, int value )
{
	GM_PadVibrations2Force[ which ] += value ;
}

#ifdef KP_WINDOWS
void	GM_SetDispVibration1( int value )
{
	GM_DispVibration1 += value ;
}

void	GM_SetDispVibration2( int value )
{
	GM_DispVibration2 += value ;
}

void	GM_SetForceDispVibration1( int value )
{
	GM_DispVibration1Force += value ;
}

void	GM_SetForceDispVibration2( int value )
{
	GM_DispVibration2Force += value ;
}
#endif

void	GM_ResetVibration( void )
{
	int			i ;

	for ( i = 0; i < 2; i ++ ) {
		GM_PadVibrations1[ i ] = 0 ;
		GM_PadVibrations2[ i ] = 0 ;
		GM_PadVibrations1Force[ i ] = 0 ;
		GM_PadVibrations2Force[ i ] = 0 ;
	}
#ifdef KP_WINDOWS
	GM_DispVibration1 = 0 ;
	GM_DispVibration2 = 0 ;
	GM_DispVibration1Force = 0 ;
	GM_DispVibration2Force = 0 ;
#endif
}

/* ------------------------------------------------------------- */

/*
	ゲームオーバー処理 
*/
#ifdef DEBUG
static void	*GameOverStartAct ;
#endif

/* ゲームオーバースタート */
void	GM_GameOver( void *actor )
{
	int			i ;

	if ( GM_GameOverFlag == 0 && GM_GameOverTimer == 0 ) {
		GM_GameOverTimer = 4 ;
#ifdef DEBUG
		GM_GameOverStartTime = GV_Time ;
#endif
		GM_GameOverFlag |= GM_OVERFLAG_OVER ;
		GM_SetGameStatus( STATE_GAMEOVER | STATE_PAUSE_DISABLE ) ;
		GM_SetMenuStatus( MENU_MENU_DISABLE | MENU_RADIO_DISABLE | MENU_GAGE_OFF ) ;
		for ( i = 0; i < GV_PAD_MAX; i ++ ) {
			GV_PadReleaseOnSystem( i ) ;
		}
//		GM_CallCallbackProc( GM_CALLBACK_GAMEOVER ) ;
#ifdef DEBUG
		GameOverStartAct = actor ;
		printf( "game over proc is started by %x\n", GameOverStartAct ) ;
#endif		
	} else {
#ifdef DEBUG
		printf( "warning : game over proc is already started by %x.\n", 
			    GameOverStartAct ) ;
#endif
	}
}

/* ゲームオーバー中かチェック */
int		GM_IsGameOver( void )
{
	if ( !GM_CheckGameStatus( STATE_GAMEOVER ) && 
		 GM_GameOverFlag == 0 && GM_GameOverTimer == 0 ) return 0 ;
	return 1 ;
}

/* 個別ゲームオーバー処理に入る */
void	GM_GameOverProcStart( void *actor ) 
{
	/* ロードがかかっていたらゲームオーバーにならない */
//	if ( GM_LoadRequest == 0 && ( GM_GameOverFlag & GM_OVERFLAG_START ) == 0 ) {
	if ( GM_LoadRequest == 0 && ( GM_GameOverFlag & GM_OVERFLAG_START ) == 0 &&
		!(GM_VRStatus & GM_VR_CLEAR) ) {
		GM_GameOver( actor ) ;
		GM_GameOverTimer = -1 ;
		GM_GameOverFlag |= GM_OVERFLAG_START_BY_ACTOR ;
	} else {
//		printf( "warning : gameoverproc start cancel\n" ) 
	}
}

/* 個別ゲームオーバー処理終了 */
void	GM_GameOverProcEnd( void *actor )
{
#if 0
	if ( ( GM_GameOverFlag & GM_OVERFLAG_START ) == GM_OVERFLAG_START &&
		 ( GM_GameOverFlag & GM_OVERFLAG_END_BY_ACTOR ) == 0 &&
		( void * )actor == GameOverStartAct ) 
#else
	if ( ( GM_GameOverFlag & GM_OVERFLAG_START ) == GM_OVERFLAG_START &&
		 ( GM_GameOverFlag & GM_OVERFLAG_END_BY_ACTOR ) == 0 ) 
#endif
	{
		printf( "gameoverproc end!\n" ) ;
		GM_GameOverFlag |= GM_OVERFLAG_END_BY_ACTOR ;
		GM_GameOverTimer = 2 ;
	} else {
//		printf( "warning : gameoverproc end cancel\n" ) ;
	}
}

/* ゲームオーバークリア */
void	GM_GameOverClear( void )
{
	int			i ;

	GM_GameOverTimer = 0 ;
	GM_GameOverFlag |= GM_OVERFLAG_CLEARED ;
	for ( i = 0; i < GV_PAD_MAX; i ++ ) {
		GV_PadReleaseOffSystem( i ) ;
		GV_PadMaskCancelScn( i, 0 ) ;
		GV_PadReleaseOff( i ) ;
	}
}

void GM_LoadRestart( int force_load_resident )
{
	GM_LoadRequest = 1 ;
	GM_SetArea( GM_SaveArea, ( char * )GCL_GetSavedLinkVar( &GM_SaveAreaDir[ 0 ] ) ) ;	
	{
		char	*cur_res, *saved_res ;

		cur_res = ( char * )GM_SaveResidentDir ;
		saved_res = ( char * )GCL_GetSavedLinkVar( &GM_SaveResidentDir[ 0 ] ) ;
		if ( strcmp( cur_res, saved_res ) ) {
			/* 常駐データの入れ替えも必要な場合 */
			GM_ChangeResidentArea( saved_res ) ;
		}
		if( force_load_resident ){
			GM_ChangeResident = 1;
		}
	}
}

/* ゲームオーバーからのロード */
void	GM_GameOverRestart( int mode )
{
	if ( mode == 0 ) {	/* コンティニュー */
		extern	int		GM_DiazepamCount ;
		/* ポーズ解除 */
		GV_PauseOffActorSystem( GV_PAUSE_PAUSE | GV_PAUSE_MENU | GV_PAUSE_READERROR ) ;
		GM_SdSet( SNG_PAUSEOFF ) ;
		
		GM_ContinueCount ++ ;
		GM_StartAlertMode = ALERT_MODE_SNEAK ;	/* コンティニュー後は潜入モード */
		GM_DiazepamCount = 0 ;
		/* いくつかのリンク変数は強制セーブ */
		GCL_SaveLinkVar( &GM_StartAlertMode, sizeof( GM_StartAlertMode ) ) ;
		GCL_SaveLinkVar( &GM_PlayTime, sizeof( GM_PlayTime ) ) ;
		GCL_SaveLinkVar( &GM_SaveCount, sizeof( GM_SaveCount ) ) ;
		GCL_SaveLinkVar( &GM_ContinueCount, sizeof( GM_ContinueCount ) ) ;
		GCL_SaveLinkVar( &GM_ShootCount, sizeof( GM_ShootCount ) ) ;		
		GCL_SaveLinkVar( &GM_AlertCount, sizeof( GM_AlertCount ) ) ;
		GCL_SaveLinkVar( &GM_KillCount, sizeof( GM_KillCount ) ) ;
		GCL_SaveLinkVar( &GM_DamageCount, sizeof( GM_DamageCount ) ) ;
		GCL_SaveLinkVar( &GM_RationUseCount, sizeof( GM_RationUseCount ) ) ;
		GCL_SaveLinkVar( &GM_ClearingCount, sizeof( GM_ClearingCount ) ) ;
		GCL_SaveLinkVar( &GM_RedFindCount, sizeof( GM_RedFindCount ) ) ;
		GCL_SaveLinkVar( &GM_ClearCodeFlag, sizeof( GM_ClearCodeFlag ) ) ;
		GCL_SaveLinkVar( &GM_MecaKillCount, sizeof( GM_MecaKillCount ) ) ;
//		GM_CallCallbackProc( GM_CALLBACK_CONTINUE ) ;
		if ( GM_SaveArea != *( int * )GCL_GetSavedLinkVar( &GM_SaveArea ) ) {
			/* セーブしたエリアが今と異なる場合 */
			GM_LoadRestart( 0 );
		} else {
			/* 同じ */
			GM_LoadRequest = GM_REQ_RESET ;
		}
		GM_LoadRequest |= GM_REQ_RESTORE_VAR | GM_REQ_CONTINUE ;
	} else {			/* EXIT */
		/* ポーズ解除 */
		GV_PauseOffActorSystem( GV_PAUSE_PAUSE | GV_PAUSE_MENU | GV_PAUSE_READERROR ) ;
		GM_SdSet( SNG_PAUSEOFF ) ;

		GM_PrevArea = GM_SaveArea ;
#ifdef MGS2_VRTRIAL
		GM_SetArea( GM_SaveArea, "trmenu1" ) ;
		GM_ChangeResidentArea( "r_title" ) ;
#else
		if( GM_GameStatus & STATE_VR_ANOTHER ){
			if(GM_VRStatus & GM_VR_SNAKETALES) {
				/* スネークテイルズ */
				GM_SetArea( GM_SaveArea, "n_title" ) ;
				GM_ChangeResidentArea( "r_title" ) ;
			} else {
				/* VR */
				GM_SetArea( GM_SaveArea, "mselect" );
				// レジデントは使いまわし
			}
		} else {
			/* ゲーム本編 */
			GM_SetArea( GM_SaveArea, "n_title" ) ;
			GM_ChangeResidentArea( "r_title" ) ;
		}
#endif
		GM_LoadRequest = 1 ;
		GCL_ChangeSenerioCode( 1574688 ) ;	/* scenerio.gcx */
	}
//	GM_GameOverFlag = 0 ;
}

/* ------------------------------------------------------------- */
/*
   プロック処理
*/
int		GM_ForceExecBlock( char *top, GCL_ARGS *args )
{	
	return GCL_ExecBlock( top, args ) ;
}

int		GM_ExecBlock( char *top, GCL_ARGS *args )
{
//	if ( GM_LoadRequest != 0 || GM_CheckGameStatus( STATE_GAMEOVER ) ) {
	if ( GM_LoadRequest != 0 ) {
		return 0 ;
	}	
	return GCL_ExecBlock( top, args ) ;
}

int		GM_ForceExecProc( int proc_id, GCL_ARGS *args )
{
	return GCL_ExecProc( proc_id, args ) ;
}

int		GM_ExecProc( int proc_id, GCL_ARGS *args )
{
//	if ( GM_LoadRequest != 0 || GM_CheckGameStatus( STATE_GAMEOVER ) ) {
	if ( GM_LoadRequest != 0 ) {
		return 0 ;
	}
	return GCL_ExecProc( proc_id, args ) ;
}

/* ------------------------------------------------------------- */

void GM_Reboot( char *boot )
{
#if 0 //BP_PS2 def PSX2	// リセット処理
	extern int main_argc;
	extern char **main_argv;
	char *file;
	static char *args[ 2 ];
	char *string_param = ( char * )MISC_BUFFER;
	int argc;
	char **argv;

	// 起動ファイル名をパラメータでもらう。

	file = string_param;
	if( boot != NULL ){
		strcpy( file, boot );
		string_param += strlen( file ) + 1;

		// 引数作成
		args[ 0 ] = string_param;
		strcpy( args[ 0 ], main_argv[ 0 ] + 8 );
		string_param += strlen( args[ 0 ] ) + 1;

		args[ 1 ] = string_param;
		sprintf( args[ 1 ], "%08X %04X %04X %04X"
				 , GM_Configuration, GM_Language, GM_ScrAdjX, GM_ScrAdjY );
		string_param += strlen( args[ 1 ] ) + 1;

		argc = 2;
		argv = args;
	} else {
		strcpy( file, main_argv[ 0 ] );
		argc = 0;
		argv = NULL;
	}
	// 終了処理

	*GS_PMODE = 0;
	sd_set_cli( SNG_PAUSEON ) ;
	sd_set_cli( 0xFF00000A );
	sd_set_cli( 0xFF00000E );

	cdbios_stop();

	WAIT_HSYNC( 500 );
	WAIT_HSYNC( 500 );

	sd_set_cli( 0xFFFFFFFF );
	// 2 layer DVD
	{
		extern int FS_current_layer;
		if( FS_current_layer != 0 ){
			cdbios_preseek( 1000 );		// layer0 の適当な場所
		}
	}

	WAIT_HSYNC( 500 );
	WAIT_HSYNC( 500 );

	while ( DG_DmaCheckEnd() == 0 );
	sceGsSyncPath( 0, 0 );			/* 念のため */

	DG_VramClear();

	sceGsSyncPath( 0, 0 );			/* 念のため */

	DisableIntc( INTC_VBLANK_S );
	DisableIntc( INTC_VBLANK_E );
	DisableIntc( INTC_GS );
	DisableDmac( DMAC_GIF );
	DisableDmac( DMAC_VIF1 );

	DG_EndDaemon();

	while( cdbios_get_status() != 0 );
#if 0
	sceCdBreak();
	sceCdSync( 0 );
	sceCdStop();
	sceCdSync( 0 );
#endif
	MTS_Exit();

	sceMcEnd();
	scePadEnd();
	sceCdInit( SCECdEXIT );
	sceSifExitCmd();

	for( ;; ){
		LoadExecPS2( file, argc, argv );
	}
#endif	// PSX2
#if 0 //BP_TODO
#ifdef KP_XBOX
#ifdef KP_WINDOWS

#else	// X-Box
#if 0
	XLaunchNewImage( NULL, NULL );	// とりあえずランチャが起動するようにしておく
#else
#ifdef MGS_MASTER
	XLaunchNewImage( "D:\\default.xbe", NULL );
#else
	XLaunchNewImage( "D:\\mgs2.xbe", NULL );
#endif
#endif	
#endif			
#endif

#endif
}

/* ------------------------------------------------------------- */

static inline int ActLoading( Work *work )
{
	// ロード待ち
	if ( GM_LoadComplete == 0 )
   {
		return TRUE;
	}

   //BP Don't allow "load finish" processing to occur while the splash screens are fading.
   // This fixes issues during the initial splash screen display (while background stage loading occurs) with the script executing stalling a 
   // large amount of time (0.7 seconds) due to loading of sound packs stalling.
   // So while the splash screens are fading we simply pretend we're not done loading yet.
   if( !BP_Splash_IsStatic() )
   {
      return TRUE;
   }

	// ロードが完了した
	GM_LoadRequest = 0;
	work->killing_count = 0;

	GM_Configuration = ( GM_Configuration & ~GM_CONFIG_PLAYTIME_STOP );

#ifdef DEBUG_MODE
	GV_SetLoadedMemory();

	GV_ErrorFlag = 0;
#endif

	DG_UnDrawFrameCount = 1;
	// モジュールの初期化
	StartStage();
	// シナリオフォントデータの登録
	font_set_top_addr( FONT_TYPE_GCL, GCL_GetFontDataTop() );

    /* 常駐入れ替えの完了 */
	/* シナリオ起動時に入れ替え命令がくると困るので
	   シナリオ起動前に行う */
	if ( GM_ChangeResident != 0 ) {
		GM_LoadRequest = 1 | GM_REQ_SAVE_VAR ;
		GM_ChangeResident = 0 ;
	}
	/* 新しいステージ名の登録 */
	/* シナリオ起動前に行う */
	GM_CurrentStage = GM_SaveArea ;

	
   BP_SetSpinnerLoadFlag(kLoadFlag_LoadScript, 1);
   // シナリオの起動
	GCL_ExecScript();
printf( "SCENARIO SETUP END : %d\n", DG_TickCount );
	// 管理プロセス起動
   BP_SetSpinnerLoadFlag(kLoadFlag_LoadScript, 0);

//GV_DumpActorSystem( 0 );
//GV_DumpMemory( GV_NORMAL_MEMORY );
	return FALSE;
}

extern int BP_COsContext_ShouldPauseApplication();
extern int gBP_Hack_DisablePauseDuringPlantLoad;

static inline int ActGame( Work *work )
{
#ifdef DEBUG_MODE
	static int pause_req = 0;
	static int pad_dir = -1 ;

	if( pause_req ){
		Pause();
		pause_req = 0;
	}

	if( GM_PressDebug != 0 ){
		if( GM_PressDebug == 1
			|| ( GM_PressDebug == 2 && DG_Clock % 2 == 0 )
			|| ( GM_PressDebug == 3 && DG_Clock % 2 == 1 )
			){
			GV_PadData[ 0 ].press = GV_PadData[ 0 ].status;
			GV_PadDataDirect[ 0 ].press = GV_PadDataDirect[ 0 ].status;
		}
	}
#endif

#ifdef TEST
	Test();
#endif

	// サウンドステータスカウント更新
	GM_SdStatusCount[1] = GM_SdStatusCount[0] ;
#if 0 //BP_SOUND def PSX2 /* T.Morita まだ駄目 sd_statusは落ちる2002.04.30 */
	GM_SdStatusCount[0] = *((int *)sd_status() + 2) ;
#else
	GM_SdStatusCount[0] = 0 ;
#endif

#ifdef KP_XBOX
	GM_SdUpdate();	// ３Ｄサウンドパラメータ更新
#endif	
	
	
	//ゲーム実行中の処理
	if ( !( GM_Configuration & GM_CONFIG_VIBRATION_OFF ) &&
	     !( GM_CheckMenuStatus( MENU_VIBRATE_DISABLE ) ) ) {
		//パッド振動処理
		int			i ;
		int			v1, v2 ;

#ifdef KP_WINDOWS
		if ( !GM_CheckGameStatus( STATE_VIB_PAUSE0 ) ) {
			v1 = GM_DispVibration1 ;
			v2 = GM_DispVibration2 ;
		}
		if ( !GM_CheckGameStatus( STATE_VIB_PAUSE1 ) ) {
			v1 += GM_DispVibration1Force ;
			v2 += GM_DispVibration2Force ;
		}

		if ( v1 > 0 ) {
			DG_SetDispVibration1( v1 ) ;
		}
		if ( v2 > 0 ) {
			DG_SetDispVibration2( v2 ) ;
		}

		GM_DispVibration1 = 0 ;
		GM_DispVibration2 = 0 ;
		GM_DispVibration1Force = 0 ;
		GM_DispVibration2Force = 0 ;
#endif
		for ( i = 0; i < 2; i ++ ) {
			v1 = v2 = 0 ;
			if ( !GM_CheckGameStatus( STATE_VIB_PAUSE0 ) ) {
				v1 = GM_PadVibrations1[ i ] ;
				v2 = GM_PadVibrations2[ i ] ;
			}
			if ( !GM_CheckGameStatus( STATE_VIB_PAUSE1 ) ) {
				v1 += GM_PadVibrations1Force[ i ] ;
				v2 += GM_PadVibrations2Force[ i ] ;
			}
			if ( v1 > 0 ) GV_SetPadVibration1( i, 1 ) ;
			if ( v2 > 0 ) GV_SetPadVibration2( i, v2 ) ;
//if ( v1 > 0 || v2 > 0 ) printf( "<%d> %d %d %d %d\n", i, 
//							     GM_PadVibrations1[ i ], GM_PadVibrations2[ i ],
//							     GM_PadVibrations1Force[ i ], GM_PadVibrations2Force[ i ] ) ;

			GM_PadVibrations1[ i ] = GM_PadVibrations2[ i ] = 0 ;
			GM_PadVibrations1Force[ i ] = GM_PadVibrations2Force[ i ] = 0 ;
		}
	}
	
	//ノイズパワー
	if( GM_NoisePowerOn != NOISE_ZERO ) {
		GM_NoisePower = GM_NoisePowerOn ;
		GM_NoisePowerOn = NOISE_ZERO ;
	} else {
		GM_NoisePower = GM_NoisePowerOn ;
	}

	/* ゲームオーバー処理 */
	if ( GM_GameOverTimer != 0 ) {
		/* すべてのポーズを解除 */
		if ( GM_GameOverTimer > 0 ){
			GV_PauseOffActorSystem( GV_LEVEL_NORMAL ) ;
			GM_SdSet( SNG_PAUSEOFF ) ;
		}
		/* ロード要求をリセット */
		GM_LoadRequest = 0 ;
		if ( GM_GameOverTimer > 0 && -- GM_GameOverTimer == 0 ) {
			/* ノーマルポーズ */
			//GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;
			GV_PauseOnActorSystem( GV_PAUSE_MENU ) ;
			/* ここでコールバック */
			if ( !( GM_GameOverFlag & GM_OVERFLAG_EXEC_CALLBACK ) ) {
				GM_CallCallbackProc( GM_CALLBACK_GAMEOVER ) ;
				GM_GameOverFlag |= GM_OVERFLAG_EXEC_CALLBACK ;
			}
			/* 携帯ＳＥＯＦＦ */
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_PHONE000 ) ;
			ASSERT( SK_GameOverAct != NULL ) ;
			/* ゲームオーバー画面スタート */
			if ( !( GM_GameOverFlag & GM_OVERFLAG_LOGO_START ) ) {
				if ( SK_GameOverAct( 0 ) != NULL ) {
					printf( "game over logo start!\n" ) ;
					GM_GameOverTimer = -1 ;				
					GM_GameOverFlag |= GM_OVERFLAG_LOGO_START ;
					GM_SetGameStatus( STATE_DISP_GAMEOVER ) ;
					GM_JimakuHide() ;	
				} else {
					/* メモリが足りなくてロゴが表示できない */
					printf( "game over no memory! : kill actor and try again\n" ) ;
					GV_DestroyActorSystem( GV_KILL_USER_M ) ;
					//GV_PauseOffActorSystem( GV_PAUSE_PAUSE ) ;/* ポーズは解除しないとマズイ */
					GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;/* ポーズは解除しないとマズイ */
					//GM_SdSet( SNG_PAUSEOFF ) ;	/* しなくて良いと思われる */
					GM_GameOverTimer = 1 ;
				}	
			}
		} else if ( GM_GameOverTimer < 0 && 
				   !( GM_GameOverFlag & GM_OVERFLAG_LOGO_START ) ) {
			/* 保険機構。２０秒まってもＧａｍｅＯｖｅｒにならなかったら
			   無理矢理ゲームオーバーにする */
#ifdef DEBUG			
			if ( GV_Time - GM_GameOverStartTime > ( 300 / TIME_BASE ) * 20 ) {
				printf( "warning : gameover wait time over\n" ) ;
				GM_GameOverProcEnd( GameOverStartAct ) ;
			}
#endif
		}
		return TRUE ;
	} 

	//ポーズリクエスト
	if ( GM_PauseRequest != 0 ) {
		if ( ( GV_PauseLevel & ~GV_PAUSE_PAUSE ) == 0 ) {
			if ( GM_PauseRequest == GM_PAUSE_REQ_ON ) {
				/* サウンドコードはPAUSEONでなく、FOUTS_Sを使う */
				//PauseOn() ;
				GM_SdSet( SNG_FOUTS_S ) ;
				GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;
			}
			else if ( GM_PauseRequest == GM_PAUSE_REQ_OFF ) PauseOff() ;
		}
	}

	// ロード要求のチェック
	if( GM_LoadRequest != 0 ) {
		if ( ( ( GV_PauseLevel & GV_PAUSE_PAUSE ) == 0 ) &&
			 ( !GM_IsGameOver() || 
			   ( GM_IsGameOver() && ( GM_GameOverFlag & GM_OVERFLAG_CLEARED ) ) ) ) {
			/* ロード要求 */
			GM_PauseRequest = 0 ;
			GM_DisablePauseCount = 0 ;
			return FALSE;
		}
	}
#if BP_ENABLE_DEBUG_HELPER() //def DEBUG_MODE
	/* デバッグリスタート */
	if ( GM_LoadRequest & GM_REQ_DEBUG_RESET ) {
	    GM_LoadRequest &= ~GM_REQ_DEBUG_RESET ;
		GM_PauseRequest = 0 ;
		GM_DisablePauseCount = 0 ;
	    return FALSE ;
	}
#endif

//#ifndef DESIGN_PREVIEW
	if( ( GV_PauseLevel & ~GV_PAUSE_PAUSE ) == 0
#if 0
		&& ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON
							   | MENU_STREAM_CH_0 | MENU_STREAM_CH_1 ) == 0 )
#else
		&& ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON ) == 0 )
#endif
		&& ( GM_GameStatus & ( STATE_DEMO | STATE_PAUSE_DISABLE
							   | STATE_PAD_DEMO | STATE_PRG_DEMO
							   | STATE_GAMEOVER ) ) == 0
		&& DG_UnDrawFrameCount == 0 ){
#ifdef KP_XBOX 		// Xbox TCR 対策
#ifndef KP_WINDOWS	// Windowsでは必要無い
		extern int	GV_PadMask[ 4 * 2 ] ; 	/* マスク指定用 */
		if ( GM_PauseRequest == 0 &&
			 (GV_PadData[ 0 ].flag & GV_PAD_DISCONNECT )
			 && !( GV_PauseLevel & GV_PAUSE_PAUSE )
			 && GM_DisablePauseCount <= 0
			 && !( GV_PadData[ 0 ].flag & GV_PAD_RELEASE )	// パッドリリース中はポーズできない
			 && !( GV_PadMask[ 0 ] & GV_PadMask[ 1 ] & PAD_STA ) // スタートがマスクされてる間はポーズできない
			 && !( GM_GameStatus & ( STATE_SCN_DEMO | STATE_SCN_DEMO | STATE_PRG_DEMO | STATE_PAD_DEMO ))
			 ) {
			Pause();
		}
#endif
#endif
#ifdef KP_WINDOWS		// Windows用処理

#if FALSE			// Windows98/95/ME系で誤作動を起こすので一時はずし
		/*-- Windowアプリがアクティブではない場合のPAUSE --*/
		if( GM_PauseRequest == 0 && 
		    !(GV_PadData[ 0 ].flag & GV_PAD_RELEASE) &&	// PAD入力ＯＫの時のみ
			GM_AppReqPause_ByActive ) {			 // (注)外部スレッドからアクセスされる
		    if ( GM_DisablePauseCount <= 0 ) {
				/* GM_DisablePuaseCount > 0 のときはPauseOnできない */
				if ( !( GV_PauseLevel & GV_PAUSE_PAUSE ) ) {
					Pause();
				}
				GM_AppReqPause_ByActive = FALSE;
			}
		} 

		/*-- 終了要求処理中のPAUSE --*/
		if( GM_PauseRequest == 0 && 
		    !(GV_PadData[ 0 ].flag & GV_PAD_RELEASE) &&	// PAD入力ＯＫの時のみ
			GM_AppReqPause_ByExit) {			 // (注)外部スレッドからアクセスされる
		    if ( GM_DisablePauseCount <= 0 ) {
				/* GM_DisablePuaseCount > 0 のときはPauseOnできない */
				if ( !( GV_PauseLevel & GV_PAUSE_PAUSE ) ) {
					Pause();
				}
				GM_AppReqPause_ByExit = FALSE;
			}
		} 
#endif
#endif

		if( GM_PauseRequest == 0 &&   //no scripted pause pending?
        (  ( GV_PadData[ 0 ].press & PAD_STA )  //press start
        //BP - added logic to pause application when XMB/etc. is open
        //only if game would be allowed to manually pause here.
#if 1 //BP
        || (   ( GV_PauseLevel==0 ) //game not paused at all
           &&  BP_COsContext_ShouldPauseApplication() //system wants to pause
           && !( GV_PadData[ 0 ].flag & (GV_PAD_RELEASE|GV_PAD_MASK) ) //controller not disabled through scripting
           )
        )
#endif
        )
      {	
          if ( gBP_Hack_DisablePauseDuringPlantLoad == 0 )
          {
             if ( GM_DisablePauseCount <= 0 || ( GV_PauseLevel & GV_PAUSE_PAUSE ) )
             {
                /* GM_DisablePuaseCount > 0 のときはPauseOnできない */
                Pause();
             }
          }
		} 

		/* ポーズ表示 */
		if( GV_PauseLevel & GV_PAUSE_PAUSE ){
#ifdef KP_XBOX
			// 仮・ちゃんとしたフォントで多言語対応したほうがよさそう。。。
			if( GV_PadData[ 0 ].flag & GV_PAD_DISCONNECT ) {
#if 0	// discon.c に移動		
				if( GV_Time % 90 < 50 ){
					MENU_ResetColor();
					MENU_Locate( DRAW_WIDTH / 2, DRAW_HEIGHT / 8 * 5, MENU_MODE_CENTER );
					MENU_Printf( "Please reconnect the controller" );
					MENU_Locate( DRAW_WIDTH / 2, DRAW_HEIGHT / 8 * 5 + 16, MENU_MODE_CENTER );
					MENU_Printf( "and press START to continue" );
				}
#endif				
			}else
#endif
			{
#ifdef DEBUG_MODE
			if( GV_Time % 90 < 50 ){
				MENU_ResetColor();
				MENU_Locate( DRAW_WIDTH / 2, DRAW_HEIGHT / 8 * 5, MENU_MODE_CENTER );
				MENU_Printf( "PAUSE" );
			}
			if( GM_DebugModeEnable ){
				/* ステージ名表示 */
				if ( GM_GetArea() != NULL && GV_Time % 90 < 50 ) {
					MENU_Locate( DRAW_WIDTH / 2, DRAW_HEIGHT / 8 * 5 + 16, MENU_MODE_CENTER );
					MENU_Printf( "%s\n", GM_GetArea() ) ;
				}

				if( GV_PadData[ 0 ].press & PAD_AR ){
					PauseOff();
					pause_req = 1;
				} else {
					FVECTOR		vec ;
					int 		dir, diff ;
			    
					vec.vx = -( float )( GV_PadData[ 0 ].right_dx - 128 ) ;
					vec.vz = -( float )( GV_PadData[ 0 ].right_dy - 128 ) ;
					if ( vec.vx < -ANALOG_MARGIN_F || vec.vx > ANALOG_MARGIN_F ||
						 vec.vz < -ANALOG_MARGIN_F || vec.vz > ANALOG_MARGIN_F ) {
						dir = GV_VecDir2( &vec ) ;
						diff = GV_DiffDirAbs( dir ,pad_dir ) ;
						if ( diff > 512 ) {
							PauseOff();
							pause_req = 1;
							pad_dir = dir ;
						}
					}
				}
			}
#endif
			}
		}
	}
//#endif
	GM_PauseRequest = 0 ;
	if ( GM_DisablePauseCount > 0 ) -- GM_DisablePauseCount ;

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable && ( ErrorDispDisable == 0 ) ){
		if( GV_ErrorFlag != 0 ){
			static char errormsg[] = GV_ERROR_CHAR;
			int i;

			for( i = 0; i < sizeof( errormsg ); i++ ){
				int c;
				DEBUG_Locate( 512 - 32 - 8 * i, 340, 0 );
				if( GV_ErrorFlag & ( 1 << i ) ){
					c = errormsg[ i ];
				} else {
					c = '-';
				}
				DEBUG_Printf( "%c", c );
			}
		}
	}
#endif
#ifdef PSX2   
	if( ( GV_PadDataDirect[ 0 ].status == ( PAD_L1 | PAD_L2 | PAD_R1
										 | PAD_R2 | PAD_SEL | PAD_STA ) )
		&& ! GM_PadResetDisable
		&& FS_MediaType() == FS_MEDIA_DISC
		){

		if( pad_reset_count ++ > 20 ){
			// リブート処理
			GM_Reboot( NULL );
		}
	} else {
		pad_reset_count = 0;
	}
#else
	if( ( GV_PadDataDirect[ 0 ].status == ( PAD_SEL | PAD_STA ) )
		&& ! GM_PadResetDisable	){
		if( pad_reset_count ++ > DIRECT_TICK( 180 ) ){
			// リブート処理
			GM_Reboot( NULL );
		}
	} else {
		pad_reset_count = 0;
	}
#endif	

	// 時間カウントアップ
	if ( !( GV_PauseLevel & GV_PAUSE_PAUSE )
		 && !( GM_Configuration & GM_CONFIG_PLAYTIME_STOP ) ) {
//		GM_PlayTime ++;
		GM_StagePlayTime ++ ;
	}

#if BP_TGS_DEMO()
   BP_TGS_DEMO_Update();
#endif
   BP_CheckForSoftReset();

	return TRUE;
}

static inline int LeaveStage( Work *work )
{
	int			load = 0 ;
	char		loaddir[ 8 ] ;

	// 終了処理と終了チェック
	// 終了できる時にはTRUEを返す
	if( 0 ) return FALSE;

	if( GM_StreamIsPlay() > 0 ){
		return FALSE;
	}

	ResetSystem();
	ResetStage();
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		GV_CheckMemoryLeak();
	}
#endif
	if( !( GM_LoadRequest & GM_REQ_RESET ) ){
		GM_CallCallbackProc( GM_CALLBACK_LOADING ) ;
		load = 1 ;
		strcpy( loaddir, GM_GetArea() ) ;
	} else {
		GM_CallCallbackProc( GM_CALLBACK_RESTART ) ;
	}
	if ( GM_LoadRequest & GM_REQ_SAVE_VAR ) {
		printf( "save variable\n" ) ;
		GCL_SaveVar() ;
	} else if ( GM_LoadRequest & GM_REQ_RESTORE_VAR ) {	
		printf( "restore variable\n" ) ;
		GCL_RestoreVar() ;
	}
	if ( GM_LoadRequest & GM_REQ_CONTINUE ) {
		/* コンティニュー時流血フラグをリセット */
		GM_PlayerStateFlag &= ~PL_START_STATE_BLOOD ;	
		/* コンティニューコールバック */
		/* 初期体力値などを変えられる */
		GM_CallCallbackProc( GM_CALLBACK_CONTINUE ) ;
	}
	if ( load ) {
		if ( GM_ChangeResident == 1 ) {
			GV_ResetResidentMemory() ;
			GM_InitResource() ;
		}
	    ResetMemory();	/* ResetResidentMemoryの後じゃないと
						   常駐の再登録をしちゃう */
		GM_StartLoader( loaddir ) ;
	}
	return TRUE;
}

/* ------------------------------------------------------------- */

#ifdef PSX2
#define DISC_ERROR_MARK_XTOP		472
#define DISC_ERROR_MARK_YTOP		48
#define DISC_ERROR_SHOW_COUNT		(5*60)
#endif
#ifdef KP_XBOX
#define DISC_ERROR_MARK_XTOP		(472*640/512-4)
#define DISC_ERROR_MARK_YTOP		(48)
#define DISC_ERROR_SHOW_COUNT		(10*60)
static int ReadErrorShowFlag = 0;
#endif


void GM_ReadError( int flag )
{
#ifdef PSX2
	MENU_PutIcon( 0, DISC_ERROR_MARK_XTOP, DISC_ERROR_MARK_YTOP
				 , flag, DG_MakeDmaPackColorFromInt( 0x80808080 ), 0 );
#endif
#ifdef KP_XBOX
/*
	MENU_ResetColor();
	MENU_Locate( DRAW_WIDTH / 2, DRAW_HEIGHT / 8 * 5, MENU_MODE_CENTER );
	MENU_Printf( "There's a problem with the disc you're using." );
	MENU_Printf( "It may be dirty or damaged. " );
*/
	{
		char *mes;

		mes = GM_GetResource( 5, 15 );
		if( mes != NULL ){
			GM_JimakuSetPosY( 240 );
			GM_JimakuShow( 0, mes );
			ReadErrorShowFlag = 1;
		}
	}
#endif
}

void GM_ReadErrorHide( void )
{
#ifdef KP_XBOX
	if( ReadErrorShowFlag ){
		GM_JimakuSetPosY( -1 );
		GM_JimakuHide();
		ReadErrorShowFlag = 0;
	}
#endif
}

int BP_IsDebugPaused()
{
   return((GV_PauseLevel & GV_PAUSE_DEBUG) != 0);
}

void BP_SetDebugPause( int pause )
{
   if( pause )
   {
      // Set debug pause on
      GV_PauseLevel |= GV_PAUSE_DEBUG;

      // Pause sound
      GM_SdSet( SNG_PAUSEON );
#if defined(BP_360) || defined(BP_PS3)
      BP_SetAllDirectOutputStreamPaused(1);
#endif
   }
   else
   {
      // Set debug pause off
      GV_PauseLevel &= ~GV_PAUSE_DEBUG;

      // Resume sound
      GM_SdSet( SNG_PAUSEOFF );
#if defined(BP_360) || defined(BP_PS3)
      BP_SetAllDirectOutputStreamPaused(0);
#endif
   }
}

static void Act( Work *work )
{
   /*
		DISC ERROR CHECK
	*/
	int disc_status;

	disc_status = FS_GetDiscStatus();

	if( disc_status & FS_DISC_ERROR ){
		gm_disc_error_count ++;

		if( gm_disc_error_count > DISC_ERROR_SHOW_COUNT ){
			/* ５秒以上読み込めなかった場合 */
			if( ( GV_PauseLevel & GV_PAUSE_READERROR ) == 0 ){
				GM_ReadError( ( gm_disc_error_count % 60 < 30 ) ? 0 : 1 );
			}
			GV_PauseOnActorSystem( GV_PAUSE_DISCERROR );
		}
	} else if( disc_status == 0 ){
		if( gm_disc_error_count > 0 && ( GV_PauseLevel & GV_PAUSE_DISCERROR ) ){
			// リードエラー状態だった
			if( ( GV_PauseLevel & GV_PAUSE_READERROR ) == 0 ){
				GM_ReadError( 0 );
#ifdef KP_XBOX
				GM_ReadErrorHide();
#endif
			}
			GV_PauseOffActorSystem( GV_PAUSE_DISCERROR );
		}
		gm_disc_error_count = 0;
	}


	/*
		ゲームデーモン本体
	*/
	switch( work->status ){
	  case WAIT_LOAD:
		// ロード中, 終了待ち状態
		if( !ActLoading( work ) ){

         work->status = WORKING;
#ifdef DEBUG_MODE
			AddSystemDebugMenu();
#endif
		}
		break;
	  case WORKING:
		if( work->killing_count <= 0 ){
			if( !ActGame( work ) ){
			    // ステージ切り替え要求があった
			    /* ポーズ解除 */
			    GV_PauseOffActorSystem( GV_PAUSE_PAUSE | GV_PAUSE_MENU | GV_PAUSE_READERROR ) ;
				GM_SdSet( SNG_PAUSEOFF ) ;

            // clear up the scene
            {
               DG_WaitForThreadedRenderComplete();
               BP_Render_HelpDrainUltWork();
            }

            GV_DestroyActorSystem( GV_KILL_STAGE_M ) ;
			    work->killing_count = KILLING_COUNT ;
				DG_UnDrawFrameCount = DG_UNDRAW_MAX;

				if( GM_StreamIsPlay() > 0 ){
					/* ストリーミング再生中 */
					GM_StreamStopAll();
				}
			}
		} else {
		    // 各Actorの終了待ち
			if( -- work->killing_count <= 0 ){
			    if( !LeaveStage( work ) ){
					//まだ終了できない
					work->killing_count = 1;
			    } else {
					// ロード状態に移行
					work->status = WAIT_LOAD;
					/* デモステートをクリア */
					GM_ResetGameStatus( STATE_DEMO ) ;
					//GV_DumpActorSystem( 0 );
					//GV_DumpMemory( GV_NORMAL_MEMORY ) ;
			    }
			}
		}

#if defined(DEBUG_MODE) || BP_ENABLE_DEBUG_CAMERA()


		if( GM_DebugModeEnable ){

			if ( GV_PadDataDirect[ 1 ].press & PAD_STA ) {
				if ( ++ GM_Debug2PMode == GM_DEBUG_MODE_MAX ) {
					GM_Debug2PMode = GM_DEBUG_MODE_OFF ;
				}
			}

         //BP_PAUSE - begin debug pause/frame step logic

#if defined(STEVEB) && defined(BP_WIN32)
#define BP_DEBUG_PAUSE_CONTROLLER   0
#else
#define BP_DEBUG_PAUSE_CONTROLLER   1
#endif

         // Toggle debug pause?
         if ( BP_DebugPad_Press( BP_DEBUG_PAUSE_CONTROLLER, BP_PAD_SELECT ) )
         {
            BP_SetDebugPause( !BP_IsDebugPaused() );
         }

         // Press Square to frame step when paused?
         if( ( BP_IsDebugPaused() ) && ( BP_DebugPad_Repeat( BP_DEBUG_PAUSE_CONTROLLER, PS2_PAD_SQUARE ) ) )
         {
            // Set frame step mode
            gBP_PauseFrameStep = 1;
         }

         // Perform frame step?
         if ( gBP_PauseFrameStep )
         {
            // Resume?
            if( BP_IsDebugPaused() )
            {
               BP_SetDebugPause( 0 );
            }
            else 
            {
               // Pause
               BP_SetDebugPause( 1 );

               // Clear frame step mode
               gBP_PauseFrameStep = 0;
            }
         }

         //BP_PAUSE - end debug pause/frame step logic


#ifdef DEBUG
			if( ( GV_PadDataDirect[ 1 ].press & PAD_SEL )
				&& GM_Debug2PMode != GM_DEBUG_MODE_CAMERA_SET ){
				GV_DumpActorSystem( ( GV_PadData[ 1 ].status != PAD_SEL ) ? 1 : 0 );
			}
#endif
#if 0 //BP_DEBUG
			/* デバッグリスタート */
			if ( GV_PauseLevel == GV_PAUSE_PAUSE ) {

				GM_ActDebugMenu( &GV_PadDataDirect[ 0 ] );

				if ( GV_PadDataDirect[ 0 ].status & PAD_L1 ) {
					if( GV_PadDataDirect[ 0 ].press & PAD_A ){
						GM_LoadRequest = GM_REQ_DEBUG_RESET | GM_REQ_RESTORE_VAR | 0x1 ;
						PauseOff() ;
					} 
					if( GV_PadDataDirect[ 0 ].press & PAD_B ){
						GM_LoadRequest = GM_REQ_DEBUG_RESET | GM_REQ_RESET | GM_REQ_RESTORE_VAR ;
						PauseOff() ;
					} else if ( GV_PadDataDirect[ 0 ].press & PAD_X ) {
						GM_PrevArea = GM_SaveArea ;
#ifdef MGS2_VRTRIAL
						GM_SetArea( GM_SaveArea, "mkselct" ) ;
#else
						GM_SetArea( GM_SaveArea, "select" ) ;
#endif
						GCL_ChangeSenerioCode( 1574688 ) ;	/* scenerio.gcx */
						GM_Result = 9999 ;
						GM_LoadRequest = GM_REQ_DEBUG_RESET | 0x1 ;
						PauseOff() ;
					}
				}
			}
#elif BP_ENABLE_DEBUG_HELPER()
         {
            extern int gBP_DebugStageAction;

            switch(gBP_DebugStageAction)
            {
               // Reload?
            case 0:
               {
                  GM_LoadRequest = GM_REQ_DEBUG_RESET | GM_REQ_RESTORE_VAR | 0x1 ;
                  gBP_DebugStageAction = -1;
               }
               break;

               // Reset?
            case 1:
               {
                  GM_LoadRequest = GM_REQ_DEBUG_RESET | GM_REQ_RESET | GM_REQ_RESTORE_VAR ;
                  gBP_DebugStageAction = -1;
               }
               break;

               // Jump to select stage
            case 2:
               {
                  GM_PrevArea = GM_SaveArea ;
#ifdef MGS2_VRTRIAL
                  GM_SetArea( GM_SaveArea, "mkselct" ) ;
#else
                  GM_SetArea( GM_SaveArea, "select" ) ;
#endif
                  GCL_ChangeSenerioCode( 1574688 ) ;	/* scenerio.gcx */
                  GM_Result = 9999 ;
                  GM_LoadRequest = GM_REQ_DEBUG_RESET | 0x1 ;

                  gBP_DebugStageAction = -1;
               }
               break;

            default:
               break;
            }
         }
#endif
		}
#endif
		break;
	}

	// 時間カウントアップ
	if ( !( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_READERROR | GV_PAUSE_DISCERROR ) )
		 && !( GM_Configuration & GM_CONFIG_PLAYTIME_STOP ) ) {
#ifdef PSX2	///	
#if 0
		GM_PlayTime ++;
#else
      if (!DG_Arm_SkipThisFrame())
      {
         GM_PlayTime += DG_PassageTick ;
      }
#endif
#else
		GM_PlayTime ++;
#endif		
	}
}

static int GM_LoadInitBin( void *prog, int id )
{
#if 0 //BP_PS2
	// ステージごとのプログラムバイナリの変更
#ifdef PSX2 /// とりあえず	
	extern int _mgs2_keep_end[];

	extern int fs_loaded_file_size;

	memcpy( _mgs2_keep_end, prog, fs_loaded_file_size ) ;

printf( "LoadInitBin %X %X %d\n", _mgs2_keep_end, prog, fs_loaded_file_size );
printf( "Program Bottom = %X\n", ( char * )_mgs2_keep_end + fs_loaded_file_size );
#endif ///  

#endif

	return 1;
}

void GM_StartDaemon()
{
	static Work Work;
	
	/*
		ライブラリの初期化
		ゲーム開始時に一度だけ実行される
	*/
	GM_InitChara();
	GM_InitScript();
	GM_InitResource();

#ifdef PSX2	/// とりあえず
	GV_SetLoader( 'b', ( GV_LOADFUNC )GM_LoadInitBin );
#endif	///

#ifdef PSX2	/// 使っていない
	{ /* ParticleData Loader */
		int PARLoader(void *buf,int id);
		GV_SetLoader('p',(GV_LOADFUNC)PARLoader);
	}
#endif ///	

	// 変数初期化

	
	GM_PlayTime = 0;
	GM_LoadRequest = 0;
	GM_LoadComplete = 0;
	GM_ChangeResident = 0 ; /* area.c */
	GM_SdStatusCount[0] = GM_SdStatusCount[1] = 0 ;

	GV_ZeroMemory( GM_SaveDataNum, sizeof( GM_SaveDataNum ) );

	/// KP_XBOX:イニシャライズだけで動きません
	GM_InitRTCTime() ;

	// デーモン起動

	GV_InsertActorPriority( GV_ACTOR_DAEMON, &Work, 0xF0 );
	GV_SetActor( &Work, Act, NULL );

	ResetSystem();

	ResetMemory();

	Work.status = WAIT_LOAD;
	Work.killing_count = 0;

	GM_SetArea( 0, START_UP_DIRECTORY );
	GM_StartLoader( START_UP_DIRECTORY );

#ifdef DEBUG_MODE
#ifdef PSX2	/// いらない
	GM_LoadDebugMenuConf();
#endif ///	
#endif

   //
#ifdef BP_360
   {
      int defaultYAxisFlipVariable = BP_GetDefaultYAxisFlipVaraible();
      if( defaultYAxisFlipVariable )
      {
         GM_Configuration |= GM_CONFIG_SHUKAN_REVERSE;
      }
      else
      {
         GM_Configuration &= ~GM_CONFIG_SHUKAN_REVERSE;
      }
   }
#endif
}

/*----------------------------------------------------------------*/

/* シナリオコマンド */

/* 変数セーブ */
int		NewSaveVariable( void )
{
	char *p;
	printf( "save variable\n" ) ;

	if( ( p = GCL_NextStr() ) == NULL ){
		GCL_SaveVar() ;
	} else {
		while( *p != GCL_END ){
			ASSERT( ( *p & 0xF0 ) == GCL_VAR || ( ( *p & 0xF0 ) == GCL_ARRAY ) );
			p = GCL_VarSaveBuffer( p );
		}
	}

	return 0 ;
}

int NewReadSavedVariable( void )
{
	return GCL_ReadSavedVar( GCL_NextStr() );
}

/* メニューステータス操作 */
/* シナリオ専用 */
int		NewSetMenuStatus( void )
{
	int			v ;

	v = GCL_GetOptionValue( 'm', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_WEAPON_OFF | MENU_ITEM_OFF ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_WEAPON_OFF | MENU_ITEM_OFF ) ;		
	}
	v = GCL_GetOptionValue( 'g', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_GAGE_OFF ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_GAGE_OFF ) ;
	}
	v = GCL_GetOptionValue( 'r', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_RADAR_OFF ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_RADAR_OFF ) ;
	} 
	v = GCL_GetOptionValue( 's', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_SUBWIN_OFF ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_SUBWIN_OFF ) ;
	}
	v = GCL_GetOptionValue( 'v', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_VIBRATE_DISABLE ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_VIBRATE_DISABLE ) ;
	}
	v = GCL_GetOptionValue( 'A', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_RADIO_DISABLE ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_RADIO_DISABLE ) ;
	}	
	v = GCL_GetOptionValue( 'p', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetGameStatusScn( STATE_PAUSE_DISABLE ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetGameStatusScn( STATE_PAUSE_DISABLE ) ;
	}
	v = GCL_GetOptionValue( 'j', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_SetGameStatusScn( STATE_RADAR_JAMMING ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_ResetGameStatusScn( STATE_RADAR_JAMMING ) ;
	}
	v = GCL_GetOptionValue( 'd', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_ResetMenuStatusScn( MENU_WEAPON_DISABLE | MENU_ITEM_DISABLE ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_SetMenuStatusScn( MENU_WEAPON_DISABLE | MENU_ITEM_DISABLE ) ;
	}		
	v = GCL_GetOptionValue( 'n', 0 ) ;
	if ( v == GM_STRCODE_ON ) {
		GM_SetMenuStatusScn( MENU_NODE_ACCESSED ) ;
	} else if ( v == GM_STRCODE_OFF ) {
		GM_ResetMenuStatusScn( MENU_NODE_ACCESSED ) ;
	}
	return 0 ;
}

/* シナリオデモ */
int		NewStartScenarioDemo( void )
{
	GM_SetGameStatus( STATE_SCN_DEMO ) ;
	return 0 ;
}

int		NewEndScenarioDemo( void )
{
	GM_ResetGameStatus( STATE_SCN_DEMO | STATE_DEMO ) ;
	return 0 ;
}

/* シナリオ、GM_StageHappening 取得 */
int	GM_GetStageHappeningForScn( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ) ;
	GCL_SetVarRef( &ref, 0, GM_StageHappening ) ;

	return 1 ;
}

/* シナリオ、GM_GameStatus 取得 */
int	GM_GetGameStatusForScn( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ) ;
	GCL_SetVarRef( &ref, 0, GM_GameStatus ) ;

	return 1 ;
}

/* シナリオ、GM_CautionLevel 取得 */
int	GM_GetCautionLevelForScn( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ) ;
	GCL_SetVarRef( &ref, 0, GM_CautionLevel ) ;

	return 1 ;
}

/* シナリオゲームオーバー処理開始 */
int	GM_GameOverStartScn( void )
{
	int			proc ;

	GM_GameOverProcStart( NULL ) ;
	if ( GCL_GetOption( 'l' ) != NULL ) {
		GM_GameOverFlag |= GM_OVERFLAG_LOADOK ;
	}
	proc = GCL_GetOptionValue( 'e', -1 ) ;
	if ( proc != -1 ) {
		GM_ForceExecBlock( ( char * )proc, NULL ) ;
		return 1 ;
	} 
	proc = GCL_GetOptionValue( 'p', -1 ) ;
	if ( proc != -1 ) {
		GM_ForceExecProc( proc, NULL ) ;
	}	
	return 1 ;
}

/* シナリオゲームオーバー処理終了 */
int	GM_GameOverEndScn( void )
{
	GM_GameOverProcEnd( NULL ) ;
	return 1 ;
}

/* ゲームオーバー処理中かどうかをチェックする */
int	GM_COM_IsGameOver( void )
{
	return GM_IsGameOver() ;
}

/* シナリオからポーズ */
/* ボスラッシュの開始／終了でしか使わないように */
int	GM_COM_PauseOn( void )
{
	if ( GM_IsGameOver() || GM_LoadRequest != 0 ) return 0 ;
	//GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;
	GM_PauseRequest = GM_PAUSE_REQ_ON ;
	return 0 ;
}

int	GM_COM_PauseOff( void )
{
	if ( GM_IsGameOver() || GM_LoadRequest != 0 ) return 0 ;
	//GV_PauseOffActorSystem( GV_PAUSE_PAUSE ) ;
	GM_PauseRequest = GM_PAUSE_REQ_OFF ;
	return 0 ;
}





/*

  本体から時間を取得する



  */

sceCdCLOCK G_rtc;

/* 本体から時間を取得する */
void			GM_InitRTCTime( void )
{
	G_rtc.stat=0;
	G_rtc.second=0;
	G_rtc.minute=0;
	G_rtc.hour=0;
	G_rtc.day=0x01;
	G_rtc.month=0x01;
	G_rtc.year=0x00;
}

void			GM_GetRTCTime( void )
{
   int years, months, days, hours, minutes, seconds;
   BP_GetLocalTime(&years, &months, &days, &hours, &minutes, &seconds);

	G_rtc.stat = 0;
	G_rtc.second = (u_char) (( seconds % 10 ) | (( seconds / 10 ) << 4 ));
	G_rtc.minute = (u_char) (( minutes % 10 ) | (( minutes / 10 ) << 4 ));
	G_rtc.hour   = (u_char) (( hours   % 10 ) | (( hours   / 10 ) << 4 ));
	G_rtc.day    = (u_char) (( days    % 10 ) | (( days    / 10 ) << 4 ));
	G_rtc.month  = (u_char) (( months  % 10 ) | (( months  / 10 ) << 4 ));
	G_rtc.year   = (u_char) (( years   % 10 ) | (( years   / 10 % 10) << 4 ));
}

int		ComGetSaveFileNum( void )
{
	int type = GCL_GetOptionValue('m', 0);
	return GM_SaveDataNum[ type ];
}

#ifdef KP_WINDOWS

/*
	Window Message による終了処理
*/
void	GM_WndProc_ExitGame(HWND hWnd)
{
	int	sel ;

	/*-- VSyncエミュレーション停止  --*/
	DG_StopVSyncEmulation() ;

	/*-- Sound停止 --*/
	if( !GM_PausingNow )
	{
		GM_SdSet( SNG_PAUSEON ) ;	// Sound Pause On
		GM_FilterGraphPauseOn() ;
	}

	GM_AppReqPause_ByExit = TRUE ;
	GM_AppExit_Conf = TRUE ;	// 確認中フラグON

	/*-- 終了意思確認 --*/
	sel = X2W_ErrorPrintfID(X2W_MB_OKCANCEL | MB_DEFBUTTON2,
						X2WERR_ID_ASK_ERROR_EXIT) ;
	switch( sel )
	{
	  case IDOK :	// OK選択
		ShowWindow(hWnd, SW_HIDE) ;				// Window非表示
		PostMessage(hWnd, WM_DESTROY, 0, 0) ;	// 終了メッセージ投函
		break ;

	  default :
		GM_AppExit_Conf = FALSE ;
		if( !GM_PausingNow )
		{
			GM_SdSet( SNG_PAUSEOFF ) ;	// Sound Pause Off
			GM_FilterGraphPauseOff() ;
		}
		break ;
	}

	/*-- VSyncエミュレーション再開  --*/
	DG_RestartVSyncEmulation() ;

	GM_AppReqPause_ByExit = FALSE;
}

/*
	Window Message 処理用
*/
int	GM_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int	ret ;

	ret = -1 ;
	switch( msg )
	{
	  /* アクティブ状態変更 */
	  case WM_ACTIVATE:
		switch( wParam )
		{ 
		  /* 非アクティブ化 */
		  case 0 :
			GM_AppReqPause_ByActive = TRUE ;	// PAUSE要求
			break ;

		  /* マウスクリックによるアクティブ化 */
		  case 1 :
		  case 2 :
			GM_AppReqPause_ByActive = FALSE ;	// 要求取り下げ
			break ;
		}
		break ;

	  /* 終了 */
	  case WM_CLOSE :
		if( !GM_AppExit_Conf ){ GM_AppExit_ConfReq = TRUE ; }
		ret = 0 ;
		break ;

	  /* ESC入力による終了 */
	  case WM_KEYDOWN :		// キーボードからのキー取得
		switch(wParam)
		{
		  case VK_ESCAPE :	// 'ESC'
			if( !GM_AppExit_Conf ){ GM_AppExit_ConfReq = TRUE ; }
			ret = 0 ;
			break ;
		}
		break ;
	}

	return( ret ) ;
}


/*
	Windowアプリケーション処理(MessageLoopから呼ばれる)
*/
void	GM_WndApp_MessLoop(HWND hWnd)
{
#if FALSE
	/* 終了メッセージ発行 */
	if( GM_AppExit_Req )
	{
		PostQuitMessage(0);			// 終了
		GM_AppExit_Req = FALSE ;
	}
#endif
}

/*
	Windowアプリケーション処理
*/
void	GM_WndApp(void)
{
	/* 終了確認 */
	if( GM_AppExit_ConfReq )
	{
		GM_WndProc_ExitGame(DG_hWnd) ;
		GM_AppExit_ConfReq = FALSE ;
	}
}
#endif	// KP_WINDOWS

