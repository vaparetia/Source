//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	libgv

	初期化関連
	システムデーモン

	1999/03/23 K.Uehara
	$Id: gvd.c,v 1.10 2002/12/24 07:17:56 takaki Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"libgv.h"
#include	"libgv.cnf"

extern void GV_ExitPad(void);

// #include	"mts.h"

#include "BP_Misc.h"
#include "BP_BaseRenderer.h"
#include "BP_Memory.h"

int GV_Time;

char*		GV_MemTop = NULL;

#define SCRATCH_SIZE (16*1024)

#if BP_VITA

#  include "scetypes.h"
#  include "sce_atomic.h"

#endif

#if BP_VITA
__thread
#endif
unsigned char *BP_ScratchPadGV = NULL;
#define skMaxScratchPads (6)
unsigned char *BP_ScratchPadAll[skMaxScratchPads] = { 0 };
volatile int gScratchPadsInUse = 0;

void GV_AS_EnableScratchpadFromThisThread()
{
   if ( BP_ScratchPadGV == NULL )
   {
#if BP_VITA
      int index = sceAtomicIncrement32( &gScratchPadsInUse );
#else
      int index = gScratchPadsInUse++;
#endif

      assert( index < skMaxScratchPads );

      printf( "AS_SCRATCHPAD: Scratchpad %dth create\n", index );

      BP_ScratchPadGV = BP_ScratchPadAll[ index ];
   }
}

void *GV_NoiseBlock = NULL;

#ifndef KP_WINDOWS
#define GV_NOISE_SIZE	(286720*2)
#else
#define GV_NOISE_SIZE	(640*480*2)
#endif

#ifdef KP_XBOX
byte*		GV_MemTop = NULL;
#error THIS IS UNUSED
#if __SCRATCHPAD_USE_VIRTUAL_ALLOC__
static void*	ScratchAddr = NULL;
#else
#error THIS IS UNUSED
DWORD	ScratchAddr[SCRATCH_SIZE>>2];
#endif

#endif

#if 0
unsigned int GV_ProfileClock = 0;
unsigned short GV_ProfileLog[ 2 ][ GV_PROFILE_MAX ];
unsigned short GV_ChanlProfileNum[ 2 ];
unsigned short GV_DrawProfileNum[ 2 ];
#endif
#ifdef DEBUG_MODE
unsigned int GV_ErrorFlag;
#endif

#ifndef MGS_MASTER
static void Excep_callback( void )
{
	if( GV_DebugMes != NULL ){
		// デバッグ情報表示 :GMを呼び出している
		extern char *GM_GetArea( void );
#ifdef KP_XBOX
		excep_printf( "MARK is %s STAGE is %s\n", GV_DebugMes, GM_GetArea() );
#else
		extern int _compiled_time[];
		printf( "MARK is %s STAGE is %s [%08X]\n", GV_DebugMes, GM_GetArea()
					  , _compiled_time[ 0 ] );
#endif
	}
	GV_CheckActorList();
	GV_CheckMemory();
}

#ifdef PSX2
static int is_call_instruction( void *addr )
{
	unsigned int inst;

	inst = *( unsigned int * )( (u_char*)addr - 8 );
	if( ( ( inst & 0xFC000000 ) ==    0x0C000000 )	// jal
		|| ( ( inst & 0xFC00003F ) == 0x00000009 ) // jalr
		|| ( ( inst & 0xFC180000 ) == 0x04100000 ) // bltzal, ...
		){
		return 1;
	}
	return 0;
}

static int Excep_Is_Text( void *addr )
{
	// そのアドレスがテキストエリアかどうかを返す。

	extern int _text_top[];
	extern int _text_end[];
	extern int _mgs2_keep_end[];
	// 常駐部かどうか
	if( addr > ( void * )_text_top && addr < ( void * )_text_end ){
		return is_call_instruction( addr );
	}
	// 非常駐部かどうか
	if( addr > ( void * )_mgs2_keep_end[ 4 ] && addr < ( void * )_mgs2_keep_end[ 5 ] ){
		return is_call_instruction( addr );
	}
	return 0;
}
#endif
#endif

void GV_ResetPacketMemory( void )
{
	// パケットメモリの初期化
#if 1
#ifdef PSX2
	GV_InitMemoryBlock( GV_PACKET_MEMORY0, GV_MEMORY_DYNAMIC, PACK_ADDR0, PACK_SIZE );
	GV_InitMemoryBlock( GV_PACKET_MEMORY1, GV_MEMORY_DYNAMIC, PACK_ADDR1, PACK_SIZE );
#else
	/* XBOXではlibdg側でメモリ確保＆初期化を行うため */
	//GV_InitMemoryBlock( GV_PACKET_MEMORY0, GV_MEMORY_DYNAMIC, PACK_ADDR0, PACK_SIZE );
	//GV_InitMemoryBlock( GV_PACKET_MEMORY1, GV_MEMORY_DYNAMIC, PACK_ADDR1, PACK_SIZE );
#endif
#else
	void *top0, *top1;
	top0 = malloc( PACK_SIZE );
	GV_InitMemoryBlock( GV_PACKET_MEMORY0, GV_MEMORY_DYNAMIC, top0, PACK_SIZE );
	top1 = malloc( PACK_SIZE );
	GV_InitMemoryBlock( GV_PACKET_MEMORY1, GV_MEMORY_DYNAMIC, top1, PACK_SIZE );
#endif
}

static void ResetMemorySystem( void )
{
   BP_DestroyResources((unsigned int)MEM_ADDR, (unsigned int)MEM_BOTTOM);
   BP_FlushDestroyedResources();

#ifdef PSX2
	GV_InitMemorySystemAll();
#endif

	GV_ResetPacketMemory();
	GV_InitMemoryBlock( GV_NORMAL_MEMORY, GV_MEMORY_STATIC, MEM_ADDR, MEM_SIZE );
}

void GV_ResetSystem( void )
{
	GV_InitMessageSystem();
}

void GV_ResetMemory( void )
{
	// キャッシュデータのクリア
	GV_FreeCacheSystem();

	// メモリのクリア
	ResetMemorySystem();

	// 常駐データの復元
	GV_ReinitResidentData();
}

/* -------------------------------------------- */

typedef struct {
	GV_ACT actor;
} Work;

static void Act( Work *work )
{
	/* GV 管理デーモン */
	GV_Time ++;

	GV_UpdatePadSystem();
	GV_CleanMemorySystem();

	if( GV_PauseLevel == 0 ){
		/* なんらかの停止状態ではメッセージを更新しない */
		GV_ClearMessageSystem();
	}
}

void BP_InitMemory()
{
   static int const skScratchpadSize = 16 * 1024
#if MGS_VERSION==2
      * 2;
      // *2 is for DG local work
#endif

   int i;

   if(GV_MemTop == NULL)
   {
      int mem_total_size = MEM_TOTAL_SIZE;
      // Align our memory heap to 128-byte blocks, since TRI allocations in stage data want 128-byte granularity
      GV_MemTop = (char*)BP_Memory_Calloc( mem_total_size, 128, kMT_Permanent, kMC_GVHeap );
   }

   for ( i = 0; i < skMaxScratchPads; ++i )
   {
      if ( BP_ScratchPadAll[i] == NULL )
      {
         BP_ScratchPadAll[i] = (char*) BP_Memory_Calloc( skScratchpadSize, 64, kMT_Permanent, kMC_ScratchPad );
      }
   }

   GV_AS_EnableScratchpadFromThisThread();
}

void GV_StartDaemon( void )
{
	/*	GVライブラリの初期化 */
	static Work work;

#ifndef MGS_MASTER
//	MTS_SetExceptionCallback( Excep_callback );
#ifdef PSX2
//	MTS_SetIsTextFunc( Excep_Is_Text );
#endif
#endif

#ifdef KP_XBOX	
#if	 __SCRATCHPAD_USE_VIRTUAL_ALLOC__
	// メモリの確保
	if( ScratchAddr == NULL ){
#ifndef KP_WINDOWS
		ScratchAddr = VirtualAllocEx( GetCurrentProcess(), SCRATCH_ADDRESS, SCRATCH_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
#else
		ScratchAddr = VirtualAlloc( SCRATCH_ADDRESS, SCRATCH_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
#endif
		ASSERT( ScratchAddr == SCRATCH_ADDRESS ) ;
#ifdef DEBUG_MODE
		if( !ScratchAddr ){
			LPVOID lpMsgBuf;

			// メッセージ作成
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
						| FORMAT_MESSAGE_FROM_SYSTEM
						| FORMAT_MESSAGE_IGNORE_INSERTS,
					    NULL,
					    GetLastError(),
					    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
						(LPTSTR)&lpMsgBuf,
						0,
						NULL);

			printf("[ ERROR ] %s\n", lpMsgBuf) ;
			LocalFree(lpMsgBuf);

			// 終了
			ASSERT(0) ;
		}
#endif
		ASSERT( ScratchAddr == SCRATCH_ADDRESS );
	}
#endif
			
	if(GV_MemTop == NULL){
      //		GV_MemTop = (byte*)GlobalAlloc(GPTR, MEM_TOTAL_SIZE);

#if 0 //BP_GENERAL
      GV_MemTop = (byte*)XPhysicalAlloc(MEM_TOTAL_SIZE, MAXULONG_PTR, 0, PAGE_READWRITE);
#else
      GV_MemTop = (byte*)malloc(MEM_TOTAL_SIZE);
      memset( GV_MemTop, 0, MEM_TOTAL_SIZE ); //BP
#endif

printf( "GV_MemTop = %X\n", GV_MemTop );
		ASSERT( GV_MemTop != NULL );

		// ＶＲ用セーブ領域を特別に初期化
		memset(MISC_BUFFER + (16 * 1024), 0, (9 * 1024));
	}

#endif	// KP_XBOX

   if( GV_NoiseBlock == NULL )
   {
      // ノイズテクスチャ用ランダム領域。
      GV_NoiseBlock = malloc(GV_NOISE_SIZE);
      {
         // noiseをrandに初期化
         unsigned char *p;
         int i;

         p = GV_NoiseBlock;

         for( i = 0; i < GV_NOISE_SIZE; i++ ){
            *( p++ ) = ( BP_PS2_rand() >> 5 ) & 0xFF;
         }
      }
   }

	BP_InitMemory();

	GV_InitResidentMemory();
	GV_ResetMemory();
	GV_ResetLoader();

	/*
		プロセス管理ルーチンの初期化
	*/
	GV_InitActorSystem();
	GV_InitCacheSystem();

	/*
		パッドルーチンの初期化
	*/
	GV_InitPadSystem();

	/*
		GV 管理デーモンの起動
	*/
	GV_InitActor( GV_ACTOR_DAEMON, &work, NULL );
	GV_SetActorKillLevel( &work, GV_KILL_LEVEL_SYSTEM );
	GV_SetActorClass( &work, GV_CLASS_SYSTEM );
	GV_SetActor( &work, Act, NULL );

	GV_Time = 0;

	INIT_HSYNC_TIMER();
	RESET_HSYNC_TIMER();
}

void GV_KillDaemon( void )
{
	/*	GVライブラリの解放 */

	/*
		パッドルーチンの解放
	*/
	GV_ExitPad();

	/*
		プロセス管理ルーチンの終了処理
	*/
#ifndef KP_WINDOWS	// Windows版では、mainw.cで直接やってます
	GV_KillActorSystem() ;
#endif

#ifdef KP_WINDOWS
	/*
		領域の解放
	*/
	if( GV_NoiseBlock )
	{
		free(GV_NoiseBlock) ;
		GV_NoiseBlock = NULL ;
	}
	if( GV_MemTop )
	{
		free(GV_MemTop) ;
		GV_MemTop = NULL ;
	}
#endif

#ifndef KP_WINDOWS
#if	 __SCRATCHPAD_USE_VIRTUAL_ALLOC__
	// スクラッチパッド領域解放(多分必要無い)
	if( ScratchAddr ){
		VirtualFree(ScratchAddr, SCRATCH_SIZE, MEM_DECOMMIT) ;
		VirtualFree(ScratchAddr, SCRATCH_SIZE, MEM_RELEASE) ;
		ScratchAddr = NULL ;
	}
#endif
#endif
}

//BP_GENERAL - Emulation for HSync Timer

inline int get_hsync_freq()
{
   // PAL and NTSC have different refresh rates (50hz vs 60hz) 
   // so return the appropriate magic number

   return BP_IsPAL() ? 15625 : 15734;
}

#ifdef BP_PS3
#include <sys/sys_time.h>

static uint64_t sTimerFreq = 0;
static uint64_t sLastTick = 0;

static inline uint64_t get_current_ticks()
{
   uint64_t ticks;
   do
   {
#ifdef __SNC__
      ticks = __builtin_mftb();
#else
      asm volatile ("mftb %0" : "=r"(ticks));
#endif
   }
   while ((ticks & 0xFFFFFFFFUL) == 0);     // Bug fix for timer wrapping (https://ps3.scedev.net/forums/nodejump/35093)

   return ticks;
}

void INIT_HSYNC_TIMER(void)
{
   sTimerFreq = sys_time_get_timebase_frequency();
}

void RESET_HSYNC_TIMER(void)
{
   sLastTick = get_current_ticks();
}

u_int GET_HSYNC_TIMER(void)
{
   int HSYNC_FREQ = get_hsync_freq();
   return ( ( get_current_ticks() - sLastTick ) * ( (uint64_t) HSYNC_FREQ ) / sTimerFreq ) & 0xFFFFFFFF;
}

#elif BP_VITA

#include <kernel/processmgr.h>

static SceUInt64 sBaseTick = 0;
static SceUInt32 sTimerFreq = 1000000;

void INIT_HSYNC_TIMER(void)
{
}

void RESET_HSYNC_TIMER()
{
   sBaseTick = sceKernelGetProcessTimeWide();
}

u_int GET_HSYNC_TIMER()
{
   SceUInt64 currentTick;
   SceUInt64 gameTicks;

   int HSYNC_FREQ = get_hsync_freq();

   currentTick = sceKernelGetProcessTimeWide();
   gameTicks = ( currentTick - sBaseTick ) * ((unsigned long long) HSYNC_FREQ) / sTimerFreq;

   return (u_int)( gameTicks & 0xFFFFFFFF );
}

#elif BP_WIN32

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#if 1

static LONGLONG llFreq;
static LARGE_INTEGER	liCurtime;
static float	fHSyncParam;

void INIT_HSYNC_TIMER(void)
{
   int HSYNC_FREQ = get_hsync_freq();
   int bUsingQPF;
   LARGE_INTEGER qwTicksPerSec;
   bUsingQPF = QueryPerformanceFrequency(&qwTicksPerSec);
   if (!bUsingQPF) { // QueryPerformanceFrequencyが使えない環境
      ASSERT(0);
   }
   llFreq = qwTicksPerSec.QuadPart;	// <- 3579545 = 0x369e99(Alpha2 Win32)
   printf( "llFreq %d\n", llFreq );
   // QueryPerformanceCounter から HSYNC への変換用

   fHSyncParam = HSYNC_FREQ / (float)llFreq;
}

void RESET_HSYNC_TIMER(void)
{
   QueryPerformanceCounter(&liCurtime);
}

u_int GET_HSYNC_TIMER(void)
{
   LARGE_INTEGER	liTime;

   QueryPerformanceCounter(&liTime);

   return (UINT32)( ( (liTime.QuadPart - liCurtime.QuadPart) ) * fHSyncParam ) & 0xFFFF;
}

#else
// アセンブラを使って書き換え。

#if 0
typedef union _LARGE_INTEGER 
{
   struct {
      unsigned long LowPart;
      long HighPart;
   };
   __int64 QuadPart;
} LARGE_INTEGER;
#endif

static LARGE_INTEGER lastTime;
static unsigned int hsyncparam;

void INIT_HSYNC_TIMER(void)
{
   int HSYNC_FREQ = get_hsync_freq();
   hsyncparam = 733333333 / HSYNC_FREQ;
}

void RESET_HSYNC_TIMER(void)
{
   __asm {
      rdtsc;
      mov lastTime.LowPart,eax;
      mov lastTime.HighPart,edx;
   }
}

u_int GET_HSYNC_TIMER(void)
{
   LARGE_INTEGER clock;
   __asm {
      rdtsc;
      mov clock.LowPart,eax;
      mov clock.HighPart,edx;
   }
   return ( unsigned int ) ( ( clock.QuadPart - lastTime.QuadPart ) / hsyncparam ) & 0xFFFF;
}

#endif

#elif BP_360

#include <Xtl.h>

static LONGLONG llFreq;
static LARGE_INTEGER	liCurtime;
static float fHSyncParam;

void INIT_HSYNC_TIMER(void)
{
   int HSYNC_FREQ = get_hsync_freq();
   int bUsingQPF;
   LARGE_INTEGER qwTicksPerSec;
   bUsingQPF = QueryPerformanceFrequency(&qwTicksPerSec);
   if (!bUsingQPF) { // QueryPerformanceFrequencyが使えない環境
      ASSERT(0);
   }
   llFreq = qwTicksPerSec.QuadPart;	// <- 3579545 = 0x369e99(Alpha2 Win32)
   printf( "llFreq %d\n", llFreq );

   // QueryPerformanceCounter から HSYNC への変換用
   fHSyncParam = HSYNC_FREQ / (float)llFreq;
}

void RESET_HSYNC_TIMER(void)
{
   QueryPerformanceCounter(&liCurtime);
}

u_int GET_HSYNC_TIMER(void)
{
   LARGE_INTEGER	liTime;

   QueryPerformanceCounter(&liTime);

   return (UINT32)( ( (liTime.QuadPart - liCurtime.QuadPart) ) * fHSyncParam ) & 0xFFFF;
}

#else
#  error Unknown platform
#endif
