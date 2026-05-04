/*
	eventflag.c
		MTSイベントフラグ管理ルーチン
	2000/07/10	K.Uehara
	$Id: delay.c,v 1.3 2001/05/14 02:06:22 usr01475 Exp $

*/

#include <stdio.h>
#include <eekernel.h>

#include "mts.h"
#include "debugutil.h"

static void alarmfunc( int id, unsigned short time, void *arg )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	BP_iWakeupThread( *( int * )arg );

	ExitHandler();
#endif
}

void MTS_DelayThread( int wait_time )
{
   BP_TODO_BREAK;
#if 0 //BP_PS2
	int alarm;
	int id;
	int t;

	id = BP_GetThreadId();

	while( wait_time > 0 ){
		if( wait_time > 0xFFFF ){
			t = 0xFFFF;
		} else {
			t = wait_time;
		}
		alarm = SetAlarm( t, alarmfunc, &id );
		if( alarm < 0 ){
			scePrintf( "Alarm set err\n" );
			return;
		}
		BP_SleepThread();
		wait_time -= t;
	}
#endif
}


