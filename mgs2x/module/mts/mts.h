/*
	Multi Thread System for EE
		デバッグ用のラッパモジュール
	2000/05/18	K.Uehara
	$Id: mts.h,v 1.7 2002/09/11 06:18:37 usr01475 Exp $
*/

#ifndef __MTS_H__
#define __MTS_H__

#ifndef __DEBUGUTIL_H__
#include "debugutil.h"
#endif

#define MTS_THREAD_MAX	8	// スレッドの最大数

#ifdef __cplusplus
extern "C" {
#endif

/* 基本スレッド管理 */

void MTS_BootThread( char *name, void (*func)( int id, void *arg )
					, int pri, void *stack_top, int stack_size, void *arg );

int  MTS_NewThread( char *name, void (*func)( int id, void *arg )
					, int pri, void *stack_top, int stack_size, void *arg );

void MTS_ExitThread( void );
void MTS_KillThread( int id );

int  MTS_GetThreadId( char *name );

void MTS_SetExceptionCallback( void (*func)( void ) );
// 例外発生時にそのアドレスがテキストエリアかどうかを返す。
void MTS_SetIsTextFunc( int (*func)( void *addr ) );


void MTS_SetPrintFunc( void (*func)( char *mes ) );

void MTS_Panic( void );

#define MTS_PANIC( ... )	MTS_Panic()

/* イベントフラグ */

typedef struct {
	int value;
	int mask;
	int mode;
	int wait_threadid;
} MTS_EVENTFLAG;

MTS_EVENTFLAG *MTS_CreateEventFlag( MTS_EVENTFLAG *eventflag );

int MTS_WaitEventFlag( MTS_EVENTFLAG *ev, int mask, int mode, int timeout );

#define MTS_EW_MODE_AND		0x0001
#define MTS_EW_MODE_OR		0x0002
#define MTS_EW_MODE_CLEAR	0x0004

enum {
	MTS_EW_OK = 0,			// 正常終了
	MTS_EW_NO_WAIT  = -1,	// すでに条件が成立していた
	MTS_EW_TIMEOUT = -2,	// timeout(hsync)経過しても条件が成立しなかった
	MTS_EW_KERNEL_ERR = -3,
};

int MTS_SetEventFlag( MTS_EVENTFLAG *ev, int mask );
int MTS_iSetEventFlag( MTS_EVENTFLAG *ev, int mask );

int MTS_CancelEventFlag( MTS_EVENTFLAG *ev );

extern inline void MTS_ClearEventFlag( MTS_EVENTFLAG *ev )
{
	ev->value = 0;
}

/* delaythread */

#define ONE_V_HSYNC	262

void MTS_DelayThread( int wait_time );
/* wait_timeはHsync ( < 約250 Vsync ) */

#ifdef __cplusplus
}
#endif

#endif /* __MTS_H__ */

