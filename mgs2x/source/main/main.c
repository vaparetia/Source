//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	スタートアップモジュール

	1999/03/23 K.Uehara
	$Id: main.c,v 1.1.1.3 2002/11/19 11:42:00 Yoshizawa1 Exp $
*/

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libcdvd.h>

#include <stdio.h>
#include <stdlib.h>

#include "libgv.h"
#include "libdg.h"
#include "libhzx.h"
#include "libmt.h"
#include "libgcl.h"
#include "libgv.cnf"
#include "libfs.h"
#include "gameheader.h"

#include "../user/kano/memcard_checker/memcard_access.h"

#include "mts.h"

#include "sd_ee.h"

#ifdef BP_360
#include <xtl.h>
#endif

#include "bp_math.h"//BP_MATH
#include "bp_threading.h"
#include "BP_Debug.h"
#include "BP_Camera.h"
#include "BP_Misc.h"

#if defined(BP_VITA)
#include "../system/libgv/gesture_vta.h"
#endif

extern void ps2_threading_emulation_init(void(*primaryfiberfunction)(void* arg));
void main2( void * arg );

extern void BP_MainLoopBegin();

int main_argc;
char **main_argv;

/* ---------------------------------------------------------------------- */

//BP - Setup PS3 process priority and stack size (using default 64k stack)
#ifdef _PS3
#include <sys/process.h>
SYS_PROCESS_PARAM(1001, 0x10000)
#endif


/* ---------------------------------------------------------------------- */
/*
	boot_thへのリクエスト機構
*/

static int boot_th_id = 0;


/* ---------------------------------------------------------------------- */
/*
	ベイ型HDDモジュール対応のための関数
*/

#ifdef HDDSUPPORT

static int boot_th_request = 0;

enum {
	REQUEST_NORMAL_EXIT = 0,
	REQUEST_POWER_OFF = 1,
};

static void power_off_callback( void *param )
{
	boot_th_request = REQUEST_POWER_OFF;

	BP_iWakeupThread( ( int )param );
}

static void setup_hd_power_off( void )
{
	sceCdPOffCallback( power_off_callback, ( void * )boot_th_id );
}

static void do_power_off( void )
{
	for( ;; ){
		int stat;

		sceDevctl( "pfs:", PDIOC_CLOSEALL, NULL, 0, NULL, 0 );
		sceDevctl( "hdd:", HDIOC_DEV9OFF, NULL, 0, NULL, 0 );
		sceCdPowerOff( &stat );
	}
}

#endif

/* ---------------------------------------------------------------------- */
/*
	メインループ
*/

#define STACK_SIZE ( 16 * 1024 )

static u_long128 main_stack[ STACK_SIZE / sizeof( u_long128 ) ];

extern void BP_EndFrame();

void Main( int id, void *arg )
{
	/*
		Initialize PlayStation2
	*/

#if 0 //BP_PS2
	sceDevVif0Reset();
	sceDevVif1Reset();
	sceDevVu0Reset();
	sceDevVu1Reset();
	sceGsResetPath();

	EnableCache( INST_CACHE | DATA_CACHE );
	sceDmaReset( 1 );

	sceSifInitRpc( 0 );
#endif

{
	int i;
	for( i = 0; i < main_argc; i++ ){
		printf( "ARG%d = %s\n", i, main_argv[ i ] );
	}
}

   // Init BP systems
   BP_SetTimeBased_PAL_NTSC_Globals();

   BP_Camera_Init();
   BP_PS2_rand();

	/*
		Initialize IOP Module
	*/

	printf( "LOAD IOP MODULES ..\n" );

	FS_ResetIOP();
	// この時点ではGVのメモリ管理が始まっていないことが前提
	FS_LoadIopModules( MEM_ADDR );
#ifdef HDDSUPPORT
	setup_hd_power_off();
#endif

	DG_VramClear();	/* ＶＲＡＭを完全にクリアする */
	//sceGsResetGraph( 0, SCE_GS_NOINTERLACE, SCE_GS_NTSC, SCE_GS_FRAME );

	/*
		Initialize Private System
	*/
	GV_StartDaemon();
	FS_StartDaemon();

	DG_StartDaemon();
	HZX_StartDaemon();

	GCL_Initialize();
	MT_Initialize();
	GM_StartDaemon();

	GV_SetSystemResident();	// ここまでにResidentに確保されたメモリをsystemとして固定

	/* メモリーカードライブラリ初期化 */
	MCAccessInit(NULL,1);

	/*
		Initialize Sound System
	*/
	sd_init();

	/*
		main loop
	*/
	for( ;; )
   {
      BP_MainLoopBegin();
		GV_ExecActorSystem();
      BP_EndFrame();
	}

#if defined(BP_VITA)
   GestureShutdown();
#endif
}

/* ---------------------------------------------------------------------- */
/*
	起動関数。pri = 1でくる。
*/

extern void BP_InitMain();

char* gpExeToRelaunch = "Default.XEX";
int gOriginalNumCommandLineArgs = 0;

char** gLaunchData_vArgs;
int gLaunchData_NumArgs = 0;

extern char* gLaunchDataX360;

int main( int argc, char *argv[] )
{
   main_argc = argc;
   main_argv = argv;
#ifdef BP_360
   {
      char const kMGS2_ApplicationRootDirectory[] = "d:\\mgs2";
      char * commandLine = GetCommandLine();

      BP_TransformIntoCommandLineArgs(commandLine, &gOriginalNumCommandLineArgs, &main_argv);
      main_argc = BP_PreParseCommandLineArgs(1, gOriginalNumCommandLineArgs, main_argv, kMGS2_ApplicationRootDirectory);

      {
         DWORD dwLaunchDataSize = 0;    
         DWORD dwStatus = XGetLaunchDataSize( &dwLaunchDataSize );
         if( dwStatus == ERROR_SUCCESS )
         {
            gLaunchDataX360 = malloc(dwLaunchDataSize);
            dwStatus = XGetLaunchData( gLaunchDataX360, dwLaunchDataSize );
            BP_TransformIntoCommandLineArgs(gLaunchDataX360, &gLaunchData_NumArgs, &gLaunchData_vArgs);
            BP_PreParseCommandLineArgs(0, gLaunchData_NumArgs, gLaunchData_vArgs, kMGS2_ApplicationRootDirectory);
            BP_CleanupTransformIntoCommandLineArgs(gLaunchData_NumArgs, gLaunchData_vArgs);
         }
      }
   }
#endif
   BP_InitMath();
   BP_InitMain();

   ps2_threading_emulation_init(main2);

#ifdef BP_360
   BP_CleanupTransformIntoCommandLineArgs(gOriginalNumCommandLineArgs, main_argv);
#endif

#ifdef BP_360
   if( gLaunchDataX360 )
   {
      free(gLaunchDataX360);
      gLaunchDataX360 = NULL;
   }
#endif
}

void main2( void* arg )
{
	boot_th_id = BP_GetThreadId();

	MTS_BootThread( "MGS2MAIN", Main, 18, main_stack, STACK_SIZE, NULL );

	for( ;; ){
#ifdef HDDSUPPORT
		/* BootThreadの最後でSleepしているはず */
		switch( boot_th_request ){
		  case REQUEST_POWER_OFF:
			do_power_off();
			break;
		}
		boot_th_request = 0;
#endif
		printf( "ExitMainLoop\n" );
//		HANGUP();

		BP_SleepThread();
	}

	for( ;; );
}
