/*
	Multi Thread System for EE
		デバッグ用のラッパモジュール
	2000/05/18	K.Uehara
	$Id: mts.c,v 1.14 2002/09/11 06:18:37 usr01475 Exp $
*/

#include <stdio.h>
#include <string.h>
#include <eekernel.h>

#include "mts.h"
#include "debugutil.h"

#include "bp_threading.h"

//BP
static void DI() {}
static void EI() {}

/* ---------------------------------------------------------------------- */
/*
	タスク管理構造体
*/

typedef struct {
	DEBUG_STACKCHECK_INFO info;
	int id;
	int pri;
	void (*func)( int id, void *arg );
	void *arg;
} MTS_THREAD;

static MTS_THREAD threads[ MTS_THREAD_MAX ];

#ifdef MASTER
#define mts_ASSERT( a, ... )
//static char MTS_SIG[] = "";
#else
#define mts_ASSERT( a, ... )	if( !(a) ) { printf( __VA_ARGS__ ); *( int * )1 = 0; }
static char MTS_SIG[] = "MTS.KCEJ($Date: 2002/09/11 06:18:37 $)\n";
#endif

/* ---------------------------------------------------------------------- */
/*
	例外時ダンプルーチン
*/
#ifndef MASTER

static void ( *exception_dump_func )( void ) = NULL;
static int ( *is_text_addr_func )( void *addr ) = NULL;

static inline int is_stack_in( DEBUG_STACKCHECK_INFO *info, int spval )
{
	char *sp = ( char * )spval;

	if( info->top < sp && info->top + info->size > sp ){
		return 1;
	}
	return 0;
}

#if 0 //BP_PS2
static void mts_exception_dump( unsigned long *regs )
{
	extern int DEBUG_get_stack_size( DEBUG_STACKCHECK_INFO *info );
	MTS_THREAD *p;
	MTS_THREAD *cur;
	int i, n, sp;

	sp = EXCEP_SP( regs );

	p = threads;
	n = 0;
	cur = NULL;
	for( i = 0; i < MTS_THREAD_MAX; i++, p++ ){
		if( p->id >= 0 ){
			excep_printf( "%c%-12s P%3d S%5d/%5d %08X\n"
						  , is_stack_in( &p->info, sp ) ? '>' : ' '
						  , p->info.name, p->pri
						  , DEBUG_get_stack_size( &p->info )
						  , p->info.size
						  , p->info.top );
			if( is_stack_in( &p->info, sp ) ){
				cur = p;
			}
			n++;
		}
	}
	excep_printf( "STACK TRACE\n" );
	excep_printf( "%08X %08X ", regs[ 32 - 1 ], regs[ 31 - 1 ] );
	i = 2;

	if( cur != NULL && is_text_addr_func != NULL ){
		unsigned int *ip;
		void *bottom;
		int n;
		ip = ( unsigned int * )sp;
		bottom = ( void * )( cur->info.top + cur->info.size );

		for( n = 0; ( void * )ip < bottom; ip++ ){
			unsigned int addr = *ip;
			if( ( *is_text_addr_func )( ( void * )addr ) ){
				/* 多分アドレスだと思う */
				excep_printf( "%08X ", addr );
				i++;
				n++;
				if( i >= 6 ){
					excep_printf( "\n" );
					i = 0;
				}
			}
		}
	}
	if( i > 0 ){
		excep_printf( "\n" );
	}

	if( exception_dump_func != NULL ){
		( *exception_dump_func )();
	}
	excep_printf( "\n" );
}
#endif

#endif

void MTS_SetExceptionCallback( void (*func)( void ) )
{
#ifndef MASTER
	exception_dump_func = func;
#endif
}

void MTS_SetIsTextFunc( int (*func)( void *addr ) )
{
#ifndef MASTER
	is_text_addr_func = func;
#endif
}


/* ---------------------------------------------------------------------- */
/*
	スレッド管理ルーチン
*/

void MTS_ExitThread( void )
{
	int id;

#if 0 //BP ndef MASTER
	DEBUG_STACKCHECK_INFO *p;
	p = DEBUG_ReleaseStack();

	printf( "thread %s: die\n", p->name );
#endif
	id = BP_GetThreadId();
	DI();
	{
		int i;
		for( i = 0; i < MTS_THREAD_MAX; i++ ){
			if( threads[ i ].id == id ){
				threads[ i ].id = -1;
				break;
			}
		}
	}
	EI();
	BP_ExitDeleteThread();
}

void MTS_KillThread( int id )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	int i;
	DI();
	for( i = 0; i < MTS_THREAD_MAX; i++ ){
		if( threads[ i ].id == id ){
			threads[ i ].id = -1;
			break;
		}
	}
	EI();
	if( i == MTS_THREAD_MAX ){
		return;
	}
	BP_TerminateThread( id );
	BP_DeleteThread( id );
#endif
}

static MTS_THREAD *new_thread( void )
{
	int i;

	DI();
	for( i = 0; i < MTS_THREAD_MAX; i++ ){
		if( threads[ i ].id < 0 ){
			threads[ i ].id = 0;
			EI();
			return threads + i;
		}
	}
	EI();
	mts_ASSERT( FALSE, "new_thread" );
	return NULL;
}

static void NewThread( void *arg )
{
	MTS_THREAD *th = arg;

	(*th->func)( th->id, th->arg );

	MTS_ExitThread();
}

int  MTS_NewThread( char *name, void (*func)( int id, void *arg )
					, int pri, void *stack_top, int stack_size, void *arg )
{
	struct ThreadParam param;
	MTS_THREAD *new;

	new = new_thread();
#if 0 //BP ndef MASTER
	/* スタック領域を初期化する */
	DEBUG_SetStackCheck( &new->info, name, stack_top, stack_size );
#endif

	/* 呼出し情報の作成 */
	param.entry = NewThread;
	param.stack = stack_top;
	param.stackSize = stack_size;
	param.initPriority = pri;

#if 0 //BP_PS2
	param.gpReg = &_gp;			// とりあえず
	param.option = 0;
#endif

	new->id = BP_CreateThread( &param );
	mts_ASSERT( new->id >= 0, "BP_CreateThread error %d\n", new->id );

	new->pri = pri;
	new->arg = arg;
	new->func = func;

	BP_StartThread( new->id, new );

	return new->id;
}

static void init_threads( void )
{
	int i;
	for( i = 0; i < MTS_THREAD_MAX; i++ ){
		threads[ i ].id = -1;
	}
}

void MTS_BootThread( char *name, void (*func)( int id, void *arg )
					, int pri, void *stack_top, int stack_size, void *arg )
{

#if 0 //BP_PS2

#ifndef MASTER
	int sp;

	/* 現在のスタックを取得 */
	asm( "move %0, $sp" : "=r"(sp) );
	/* 例外マネージャを初期化 */
	DEBUG_InitException( ( void * )sp );
	DEBUG_SetExceptionCallback( mts_exception_dump );
#endif
	InitThread();
#ifndef MASTER
	printf( MTS_SIG );
#endif
	cprintf( "MTS\n" ); // DUMMY

#endif //BP

	/* threads を初期化 */
	init_threads();

	MTS_NewThread( name, func, pri, stack_top, stack_size, arg );
	BP_SleepThread();	// 自分はとりあえず寝る。
					// 体験版の仕様によっては起きることになるかも。
}

void MTS_Exit( void )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	/* 自分以外のスレッドを削除 */

	int id, i;
	id = BP_GetThreadId();
	for( i = 0; i < MTS_THREAD_MAX; i++ ){
		int tid = threads[ i ].id;
		if( tid >= 0 && tid != id ){
			MTS_KillThread( tid );
		}
	}
#endif
}

void MTS_Panic( void )
{
   BP_BREAK;
#ifndef MASTER
	*( int * )1 = 0;
#endif
}
