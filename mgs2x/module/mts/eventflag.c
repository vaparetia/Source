/*
	eventflag.c
		MTSイベントフラグ管理ルーチン
	2000/07/10	K.Uehara
	$Id: eventflag.c,v 1.4 2001/05/14 02:06:22 usr01475 Exp $

*/

#include <stdio.h>
#include <eekernel.h>

#include "mts.h"
#include "debugutil.h"

MTS_EVENTFLAG *MTS_CreateEventFlag( MTS_EVENTFLAG *eventflag )
{
	eventflag->wait_threadid = 0;
	eventflag->mode = 0;

	return eventflag;
}

static void timeout_callback( int id, unsigned short time, void *arg )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	MTS_EVENTFLAG *ev = arg;

	/* タイムアウト */
	ev->value = -1;

	BP_iWakeupThread( ev->wait_threadid );

	ExitHandler();
#endif
}

static inline int ev_check( MTS_EVENTFLAG *ev )
{
	int mode = ev->mode;
	int mask = ev->mask;
	int value = ev->value;

	if( mode & MTS_EW_MODE_AND ){
		if( ( value & mask ) == mask ){
			return 1;
		}
	} else if( mode & MTS_EW_MODE_OR ){
		if( ( value & mask ) != 0 ){
			return 1;
		}
	}
	return 0;
}

int MTS_WaitEventFlag( MTS_EVENTFLAG *ev, int mask, int mode, int timeout )
{
#if 1 //BP_PS2
   BP_TODO_BREAK;
   return 0;
#else
	int value;
	int alarm;

	DI();

	ev->mode = mode;
	ev->mask = mask;
	ev->wait_threadid = BP_GetThreadId();

	if( ev_check( ev ) ){
		// すでに条件が成立していた
		if( mode & MTS_EW_MODE_CLEAR ){
			ev->mode = 0;
			ev->value = 0;
		}
		EI();
		return MTS_EW_NO_WAIT;
	}

	EI();

	alarm = SetAlarm( timeout, timeout_callback, ev );
	if( alarm < 0 ){
		return MTS_EW_KERNEL_ERR;
	}

	BP_SleepThread();
	BP_CancelWakeupThread( ev->wait_threadid );

	value = ev->value;
	if( mode & MTS_EW_MODE_CLEAR ){
		DI();
		ev->wait_threadid = 0;
		ev->mode = 0;
		ev->value = 0;
		EI();
	}

	ReleaseAlarm( alarm );
	if( value < 0 ){
		return MTS_EW_TIMEOUT;
	}

	return value;
#endif
}

int MTS_SetEventFlag( MTS_EVENTFLAG *ev, int mask )
{
	/* 割り込みではない */
   BP_TODO_BREAK;
#if 0 //BP_PS2

	DI();
	if( ( ev->value & mask ) == 0 ){
		ev->value |= mask;

		if( ev_check( ev ) ){
			EI();
			BP_WakeupThread( ev->wait_threadid );
		}
	}
	EI();
#endif
	return MTS_EW_OK;
}

int MTS_iSetEventFlag( MTS_EVENTFLAG *ev, int mask )
{
	/* 割り込み中 */
   BP_TODO_BREAK;
#if 0 //BP_PS2

	if( ( ev->value & mask ) == 0 ){
		ev->value |= mask;
		if( ev->wait_threadid == 0 ) return MTS_EW_OK;

		if( ev_check( ev ) ){
			BP_iWakeupThread( ev->wait_threadid );
		}
	}
#endif
	return MTS_EW_OK;
}

int MTS_iCancelEventFlag( MTS_EVENTFLAG *ev )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	/* 割り込み中 */
	ev->value = -1;

	if( ev->wait_threadid == 0 ) return MTS_EW_OK;
	BP_iWakeupThread( ev->wait_threadid );
#endif
	return MTS_EW_OK;
}
