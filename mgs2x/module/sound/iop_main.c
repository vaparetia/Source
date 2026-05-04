#include	<stdio.h>
#include	<kernel.h>
#include	<sys/types.h>
#include	<sif.h>
#include	<sifcmd.h>
#include	<sifrpc.h>
#include	<timerman.h>
#include	<libsd.h>
#include	"sd_debug.h"
#include	"sd_incl.h"
#include	"sd_ext.h"
//#include	"g_sound.h"
//BP_PS2 #include "../jsifman/jsifman.h"

#include <string.h>

#include "BP_SoundEffectOverride.h"
#include "BP_SoundSupport.h"

#define		BASE_PRIORITY	64
#define		PRI_SD_INT		(BASE_PRIORITY-2)
#define		PRI_SD_SET		(BASE_PRIORITY-1)
#define		PRI_SD_EE_LOAD	(BASE_PRIORITY)
#define		PRI_SD_MAIN		(BASE_PRIORITY+1)
#define		DEV				0x100

typedef struct {
	void *ptr;
	int data;
} QUEUE;

#define MAX_QUEUE 64

typedef struct {
	/* QUEUE 情報 */
	QUEUE queue[ MAX_QUEUE ];
	int top;
	int bottom;
	int thread_id;

	/* EEへ送信するステータス情報 */
	int status[ 8 ];	/* 32 Byte */
	void *ee_address;
} COM_QUEUE;
static COM_QUEUE com_queue;
///*DEBUG*/ void *save_ee_addr;

//int			buff[0x1000]	__attribute__((aligned(64)));	//--- EEから転送されるRPCの引数の受け口
int				id_SdInt, id_SdMain, id_SdSet, id_SdEELoad;
volatile int 	id_HSyncTim;
volatile int 	common;

/*---- 外部宣言 ----*/
extern int sd_init (void);
extern int SdMain (void);
extern int SdEELoad (void);
extern int SdInt (void);
extern int sd_mem_alloc(void);
extern void sd_set (int sound_code);

extern unsigned int str2_counter[2];
extern unsigned int lnr8_counter;
extern unsigned int	str2_first_load[2];
extern unsigned int	lnr8_first_load;
extern unsigned int	str2_load_code[2];
extern unsigned int	lnr8_load_code;
extern unsigned int	str2_status[2];
extern unsigned int	lnr8_status;
extern int			str2_iop_load_set[2];
extern unsigned int	str2_play_counter[2];
extern unsigned int	lnr8_play_counter;
extern unsigned int	str2_pause_fg[2];
extern unsigned int	str3_pause_fg;

/*DEBUG*/	extern struct EE_ADDR ee_addr[2];

static int i_counter = 0;

/*---------------------------------------------------------------*/
// EEへ送信するステータス情報
/*---------------------------------------------------------------*/

//void sd_set_status( int status0 )
//{
//	int status1=0;
//
//	com_queue.status[ 0 ] = status0;
//	if (wave_fp) status1 |= 4;
//	if (se_fp) status1 |= 2;
//	if (sng_fp) status1 |= 1;
//	com_queue.status[ 1 ] = status1;
//}
#define SNG_PLAY_01		0x01000001	/*ＢＧＭ再生番号１*/

extern int vag_err_fg;
extern int lnr8_err_fg;

void sd_set_status( void )
{
	int i;
	int status1=0;

	com_queue.status[ 0 ] = 0;

/*内蔵サウンドのみ、ポーズ中*/
	if ( int_pause_fg ) status1 |= 0x200000;
/*ソング＋ストリーミング、ポーズ中*/
	if ( sng_pause_fg ) status1 |= 0x100000;
/*VAGストリーミング(ch0)、ポーズ中*/
	if ( str2_pause_fg[0] ) status1 |= 0x80000;
/*VAGストリーミング(ch1)、ポーズ中*/
	if ( str2_pause_fg[1] ) status1 |= 0x40000;
/*LNR8/AC-3ストリーミング、ポーズ中*/
	if ( str3_pause_fg ) status1 |= 0x20000;
/*LNR8形式と思ってリードしてみると、VAG形式だった*/
	if ( lnr8_err_fg ) status1 |= 0x10000;
/*VAG形式と思ってリードしてみると、LNR8形式だった*/
	if ( vag_err_fg ) {
		status1 |= 0x8000;
//		PRINTF(("STATUS=%x\n", status1));
	}
/*内蔵BGM再生中*/
	if ( sng_status == 3 ) status1 |= 0x4000;
/*STR2転送中*/
	if (str2_iop_load_set[1]) status1 |= 0x2000;
/*STR1転送中*/
	if (str2_iop_load_set[0]) status1 |= 0x1000;
/*STR2再生中*/
	if (str2_load_code[1] || lnr8_load_code) status1 |= 0x40;
/*STR1再生中*/
	if (str2_load_code[0]) status1 |= 0x20;
/*STR2最初のデータロード中、再生と同時にOFF*/
	if (str2_first_load[1] || lnr8_first_load) status1 |= 0x10;
/*STR1最初のデータロード中、再生と同時にOFF*/
	if (str2_first_load[0]) status1 |= 0x8;
/*波形ロード中*/
	if (wave_load_code || pak_load_status) status1 |= 4;
/*ＳＥロード中*/
	if (se_load_code || pak_load_status) status1 |= 2;
/*ＢＧＭロード中*/
	if (sng_load_code || pak_load_status) status1 |= 1;
	else {
		for (i=0; i<0x10;i++) {
			if (sd_sng_code_buf[i] >= SNG_PLAY_01) {
				status1 |= 1;
				break;
			}
		}
	}
/*書き込み完了*/
	status1 &= 0x7FFFFFFF;
	com_queue.status[ 1 ] = status1;

//SPU2への転送済みサイズ・カウンタ
	com_queue.status[ 2 ] = str2_counter[0];
#if 0	// Add By K.Uehara counter time check
	if( com_queue.status[ 4 ] != str2_play_counter[0] || str2_play_counter[ 0 ] == 0 ) {
	    static struct SysClock prev;
	    static int prev_counter;

	    if( str2_play_counter[0] <= 0 ){
			GetSystemTime( &prev );
			prev_counter = i_counter;
	    } else if( ( str2_play_counter[ 0 ] % 100 ) == 0 ){
			int sec1, sec2, usec1, usec2;
			int sec, usec;
			struct SysClock now;
			GetSystemTime( &now );
			SysClock2USec( &prev, &sec1, &usec1 );
			SysClock2USec( &now, &sec2, &usec2 );

			sec = sec2 - sec1;
			usec = usec2 - usec1;
			if( usec < 0 ){
				sec --;
				usec += 1000000;
			}
			printf( "SEC %d.%06d PC %d CN %d\n", sec, usec, str2_play_counter[0], i_counter - prev_counter );
	    }
	}
#endif
	if (str2_status[1]) {
		com_queue.status[ 3 ] = str2_counter[1];
		com_queue.status[ 4 ] = str2_play_counter[0];
		com_queue.status[ 5 ] = str2_play_counter[1];
	} else {
		com_queue.status[ 3 ] = lnr8_counter;
		com_queue.status[ 4 ] = str2_play_counter[0];
		com_queue.status[ 5 ] = lnr8_play_counter;
	}

/*DEBUG*/	com_queue.status[ 6 ] = ((ee_addr[0].set_ctr)<<16)+ee_addr[0].read_ctr;
/*DEBUG*/	com_queue.status[ 7 ] = ((ee_addr[1].set_ctr)<<16)+ee_addr[1].read_ctr;
}

void sd_send_status( void )
{
	COM_QUEUE *q;
	q = &com_queue;

/*DEBUG*/
//	if( save_ee_addr != NULL ){
	if( q->ee_address != NULL ){
#if 1 //BP
      memcpy( q->ee_address, q->status, sizeof( q->status ) );
#else
		if ((sif_send_mem( q->ee_address, q->status, sizeof( q->status ) ))==0)
			PRINTF(("ERROR:sif_send_mem<iop_main.c>\n"));
#endif
	} else {
///*DEBUG*/	PRINTF(("ERROR:save_ee_addr = NULL\n"));
	}
//PRINTF(("Reply=%x\n", q->status[1]));
}

/*---------------------------------------------------------------*/
// EEからのコマンド受信処理
/*---------------------------------------------------------------*/
#if 0 //BP_PS2
static void sif_callback_func( SIF_CALLBACK_PARAM *hd, void *data )
{
	COM_QUEUE *q;

	q = ( COM_QUEUE * )data;

	if( hd->fno == 0 ){
		/* 初期化パケット */
		q->ee_address = ( void * )hd->data[ 0 ];
///*DEBUG*/	if (save_ee_addr == NULL) {
//				save_ee_addr = q->ee_address;
//				PRINTF(("*** OK:sif_callback_func:ee_address=%x ***\n", save_ee_addr));
//			}
///*DEBUG*/	else {
//				PRINTF(("\n*** ERROR:sif_callback_func:ee_address=%x(%x) ***\n", q->ee_address, save_ee_addr));
//			}
		return;
	}

	q->queue[ q->bottom ].data = hd->fno;
	q->bottom = ( q->bottom + 1 ) % MAX_QUEUE;
//	BP_iWakeupThread( id_SdSet );
	BP_iWakeupThread( q->thread_id );

}
/*---------------------------------------------------------------*/
// SdSetスレッド・メイン
/*---------------------------------------------------------------*/
int SdSet (void)
{
	unsigned int	sd_code;
	volatile COM_QUEUE *q = &com_queue;

	while (1) {
		BP_SleepThread();

		if( q->top != q->bottom ){
			sd_code = q->queue[ q->top ].data;
			q->top = ( q->top + 1 ) % MAX_QUEUE;
			sd_set(sd_code);
		}
//		sd_set_status( q->top );
//		sd_send_status();
	}
	BP_ExitThread();
}
/*---------------------------------------------------------------*/
// EEとの通信スレッド・メイン
/*---------------------------------------------------------------*/
static void RecieveInit(u_long64 thid)
{
	volatile COM_QUEUE *q = &com_queue;

	q->top = q->bottom = 0;
	q->thread_id = thid;
	sif_set_callback_func( IOP_SIF_SD_SET, sif_callback_func, &com_queue );
	q->ee_address = NULL;
#if 0
//	printf( "THREAD LOOP START\n" );
	for( ;; ){
		BP_SleepThread();
		while( q->top != q->bottom ){
			printf( "QUEUE %d\n", q->queue[ q->top ].data );
			q->top = ( q->top + 1 ) % MAX_QUEUE;
		}
	}
#endif
}
/*---------------------------------------------------------------*/
// H-SYNC 割り込みハンドラ
/*---------------------------------------------------------------*/
static int next_clock;

unsigned int HIntHandler(void *ptr)
{
	i_counter++;

	if ( (BP_iWakeupThread(id_SdInt)) != KE_OK )
		PRINTF(("ERR:IntHandler:Can'tWakeup-Sdint\n"));
#if 0
	return NEXT_ENABLE;
#else
	return next_clock;
#endif
}
/*---------------------------------------------------------------*/
// H-SYNC 割り込みタイマ設定
/*---------------------------------------------------------------*/
void setTimer( volatile int* common )
{
	int intNo;
#if 0
	id_HSyncTim = AllocHardTimer(TC_HLINE, 16, 1); //HSYNCタイマー取得
	intNo = GetHardTimerIntrCode(id_HSyncTim); //割込番号の取得
	RegisterIntrHandler( //割り込みハンドラ登録
		intNo,
		HTYPE_C,
		HIntHandler,
		(void *)common
	);
	SetTimerCompare(id_HSyncTim, 0x50);  /* 5.048mSec. */
//	SetTimerCompare(id_HSyncTim, 66);  /* 1/240 tick */
	SetTimerMode( id_HSyncTim , tEXTC_1 | tGATF_0 | tZRET_1 | tREPT_1 | tCMP_1);
	EnableIntr(GetHardTimerIntrCode(id_HSyncTim));
#else
#if 0	// lib 2.0.3 以前
	struct SysClock clock;
	USec2SysClock( 5000, &clock ); // 5.00 msec
	id_HSyncTim = AllocHardTimer(TC_SYSCLOCK, 32, 1); //HSYNCタイマー取得
	intNo = GetHardTimerIntrCode(id_HSyncTim); //割込番号の取得
	RegisterIntrHandler( //割り込みハンドラ登録
		intNo,
		HTYPE_C,
		HIntHandler,
		(void *)common
	);
	SetTimerCompare( id_HSyncTim, clock.low );
	SetTimerMode( id_HSyncTim , tEXTC_1 | tGATF_0 | tZRET_1 | tREPT_1 | tCMP_1);
	EnableIntr( intNo );
#else	// lib 2.0.5以降
	struct SysClock clock;
	USec2SysClock( 2500, &clock ); // 2.50 msec for AC-3
//	USec2SysClock( 5000, &clock ); // 5.00 msec
//printf( "CLOCK %d %d\n", clock.hi, clock.low );
	next_clock = clock.low;
	id_HSyncTim = AllocHardTimer(TC_SYSCLOCK, 32, 1); //HSYNCタイマー取得
	SetTimerHandler( id_HSyncTim, clock.low, HIntHandler, ( void * )common );
	SetupHardTimer( id_HSyncTim, TC_SYSCLOCK, TM_NO_GATE, 1 );
	StartHardTimer( id_HSyncTim );
#endif
#endif
}
/*---------------------------------------------------------------*/
//サウンドスレッド生成
/*---------------------------------------------------------------*/
int createThread( void )
{
	struct ThreadParam tp;

/* サウンド初期化 */
//	sd_init();

/*DEBUG*/PRINTF(("Complete IOP SOUND SYSTEM Initialize.\n"));

/* ワークエリア初期化 */
	sd_mem_alloc();
/* SdMainスレッド起動(メッセージ駆動) */
	tp.attr         = TH_C;
	tp.entry        = SdMain;
	tp.initPriority = PRI_SD_MAIN;
	tp.stackSize    = 0x4000;
	tp.option       = 0;
	id_SdMain = BP_CreateThread( &tp );
	if (id_SdMain <= 0) PRINTF(("ERR:BP_CreateThread:SdMain\n"));
	BP_StartThread( id_SdMain, 0 );
/* SdEELoadスレッド起動(メッセージ駆動) */
	tp.attr         = TH_C;
	tp.entry        = SdEELoad;
	tp.initPriority = PRI_SD_EE_LOAD;
	tp.stackSize    = 0x4000;
	tp.option       = 0;
	id_SdEELoad = BP_CreateThread( &tp );
	if (id_SdEELoad <= 0) PRINTF(("ERR:BP_CreateThread:SdEELoad\n"));
	BP_StartThread( id_SdEELoad, 0 );
/* SdSetスレッド起動(メッセージ駆動) */
	tp.attr         = TH_C;
	tp.entry        = SdSet;
	tp.initPriority = PRI_SD_SET;
	tp.stackSize    = 0x2000;
	tp.option       = 0;
	id_SdSet = BP_CreateThread( &tp );
	if (id_SdSet <= 0) PRINTF(("ERR:BP_CreateThread:SdSet\n"));
	BP_StartThread( id_SdSet, 0 );

/* EEとの通信スレッド起動 */
//	tp.attr         = TH_C;
//	tp.entry        = RecieveLoop;
//	tp.initPriority = BASE_PRIORITY - 4;
//	tp.stackSize    = 0x800;
//	tp.option       = 0;
//	th = BP_CreateThread( &tp );
//	BP_StartThread( th, th );

	RecieveInit( id_SdSet );

/* SdIntスレッド起動(H-SYNC割り込み駆動) */
	tp.attr         = TH_C;
	tp.entry        = SdInt;
	tp.initPriority = PRI_SD_INT;
	tp.stackSize    = 0x4000;
	tp.option       = 0;
	id_SdInt = BP_CreateThread( &tp );
	if (id_SdInt <= 0) PRINTF(("ERR:BP_CreateThread:SdInt\n"));
	BP_StartThread( id_SdInt, 0 );
	setTimer(&common);	//SdInt割り込みハンドラ起動
/* IOPシステム情報表示 */
	PRINTF(("===>QueryMemSize=%x\n", (u_int)QueryMemSize()));
	PRINTF(("===>QueryTotalFreeMemSize=%x\n", (u_int)QueryTotalFreeMemSize()));
	PRINTF(("===>QueryMaxFreeMemSize=%x\n", (u_int)QueryMaxFreeMemSize()));

/* スリープ処理 */
//for (;;); //ver0.5.5 ここで無限ループさせないとコマンドプロンプトに戻る
	BP_SleepThread(); //この後、dsiconsで画面出力を参照

	return( 0 );
}
/*---------------------------------------------------------------*/
// IOPメイン関数
/*---------------------------------------------------------------*/
int start( void )
{
//	BootParam *bp;
	struct ThreadParam tp;
	int th;
#if 0
	bp = QueryBootParam(KEY_IOPbootparam);
	if( bp != NULL && (bp->eword[0] & (0x10|IBP_no_iop_service))!=0 ) {
	 	printf(" No sound driver(%lX) \n",bp->eword[0]);
//		return 1;
	}
#endif

	FlushDcache();

	sceSdInit(0);
	CpuEnableIntr();
	EnableIntr( INUM_DMA_4 );
	EnableIntr( INUM_DMA_7 );
	EnableIntr( INUM_SPU );

	tp.attr         = TH_C;	//記述言語
	tp.entry        = createThread;	//スレッドエントリアドレス
	tp.initPriority = BASE_PRIORITY;	//優先度(01:Highest - 127:Lowest)
	tp.stackSize    = 0x400;	//Stack Size
	tp.option       = 0;
	th = BP_CreateThread( &tp );
	if ( th <= 0 ) {
		PRINTF(("\nIOPERR:Can't Create Thread.\n"));
		return( 1 );
	}
	BP_StartThread( th, 0 );
	return( 0 );
}

void print_time( void )
{
	printf( "TICK %d\n", i_counter * 5 / 10 );
}

/* ---------------------------------------------------------------------- */
/*
	PlayTimeなどの管理用。
	1/200のtickに変換する。
*/

void reset_play_counter( STR_TIME *st, int offset )
{
	struct SysClock now;
	GetSystemTime( &now );

	st->start = ( ( unsigned long long )now.hi << 32 ) | now.low;
	st->offset = offset;
}

int get_play_counter( STR_TIME *st )
{
	struct SysClock now;
	unsigned long long t;
	int s;

	GetSystemTime( &now );

	t = ( ( unsigned long long )now.hi << 32 ) | now.low;

	s = ( t - st->start ) / ( 36864000 / 200 ) + st->offset;

	return s;
}

void sd_exit( void )
{
	StopHardTimer( id_HSyncTim );
	FreeHardTimer( id_HSyncTim );

	printf( "iop_main:intr stop\n" );
}
#else

extern unsigned long long BP_GetElapsedMicroSeconds();

void reset_play_counter( STR_TIME *st, int offset )
{
   unsigned long long t = BP_GetElapsedMicroSeconds();

   st->start = t;
   st->offset = offset;
}

int get_play_counter( STR_TIME *st )
{
   int s;

   unsigned long long t = BP_GetElapsedMicroSeconds();

   s = (unsigned long)(( t - st->start ) / ( 1000000ULL / 200 )) + st->offset;

   return s;
}

#endif //BP_PS2

// AS MCampbell - Hack flag to make sure the sound effect memory globals have been allocated and set up BEFORE the 
// audio system is ticked below!
// Fixes: MGSTWO-3396
static volatile int gBP_HasSDMemAlloc = 0;

void bp_iop_main( int * status )
{
   //replaces start() / createThread() from PS2
   sd_mem_alloc();
   gBP_HasSDMemAlloc = 1;

   //SdMain thread handled by BP_SdMainLoopIteration()

   //SdEELoad thread handled by BP_SdEELoadLoopIteration()

   //SdSet thread not necessary as we bypass the command queue

   //RecieveInit function
   {
      volatile COM_QUEUE *q = &com_queue;
      q->top = q->bottom = 0;
      q->ee_address = status;
   }

   //SdInt thread handled by BP_SdIntLoopIteration()
}

extern void BP_SdMainLoopIteration();
extern void BP_SdEELoadLoopIteration();
extern void BP_SdIntLoopIteration();

void bp_sddrv_loop_iteration()
{
   if (!gBP_HasSDMemAlloc)
      return;

   BP_SdMainLoopIteration();
   BP_SdEELoadLoopIteration();
   BP_SdIntLoopIteration();
   BP_SoundSupport_Update();
}
