//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	delay.c
		遅延実行モジュール

	1999/11/11 K.Uehara
	$Id: delay.c,v 1.1.1.3 2002/11/19 11:51:36 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#endif

#include "gameheader.h"

int gBP_Hack_DisablePauseDuringPlantLoad = 0;

typedef	struct delay_Work 
{
	GV_ACT	actor;
	int name;
	GCL_ARGS arg;
	void *command;
   int isExecBlock;
	int time;
	int flag;
	int argv[ GCL_MAX_ARGS ];
   int lastTickCount;
} Work ;

static void Act( Work *work )
{
#if 0
	if( work->flag == 0 && GM_GameOverTimer != 0 ){
		GV_DestroyActor( work ) ;
		return;
	}
#endif

   // add a more direct check for "undraw" mode
   // this was the original intent of the lastTickCount code
   // see Marco's CL 8634: "Applies localized fix for "Delay" object counting down too fast during "undraw" mode."
   if (DG_UnDrawFrameCount > 0)
   {
      if( work->lastTickCount == DG_TickCount )
         return;
   }
   work->lastTickCount = DG_TickCount;

	if( GM_GameStatus & STATE_DEMO ){
	    extern int DM_FrameSkip ;
	    work->time -= DM_FrameSkip ;
	}
	if( --work->time <= 0 ){
		int res;
		if( work->isExecBlock ){
			/* BLOCK */
			char *p;
#if 1
         p = (void *) work->command;
#else
#  if 1	// メモリ空間が0x80000000以降だったら、この命令をはずす。
			p = ( char * )( ( int )work->command & ~0x80000000 );
#  endif
#  ifdef KP_XBOX
			p = (void*)( (int)p | ( (int)work & 0x80000000 ) );
#  endif
#endif
			res = GCL_ExecBlock( p, &work->arg );
		} else {
			/* PROC */
			res = GCL_ExecProc( ( int )work->command, &work->arg );
		}
		if( res == 0 ){
			GV_DestroyActor( work ) ;
		} else {
			if ( res > 3 ) res = DIRECT_TICK( res );
			work->time = res;
		}
	}
}

static void Die( Work *work )
{
   gBP_Hack_DisablePauseDuringPlantLoad = 0;
}

static void *UTL_DelayedExecCommand( void *command, GCL_ARGS *arg, int time, int isExecBlock )
{
	Work *work;

//	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActor( GV_ACTOR_PREV, sizeof( Work ) ) ;
	if ( work != NULL ) {
		int i;
		int *s, *d;

		if( arg == NULL ){
			work->arg.argc = 0;
		} else {
			s = arg->argv;
			d = work->arg.argv = work->argv;
			for( i = work->arg.argc = arg->argc; i > 0; i -- ){
				*( d ++ ) = *( s ++ );
			}
		}

		if( time < 0 ){
			work->flag = 1;
			time = -time;
		} else {
			work->flag = 0;
		}
		work->time = time;
		work->command = command;
      work->isExecBlock = isExecBlock;
      work->lastTickCount = -1;

		GV_SetActor( work, Act, Die ) ;
	}
	return work ;
}

void *NewDelay( int name, int map )
{
	Work *work;
	int time;
	void *command;
    int isExecBlock = 0;

	time = 0;
	command = NULL;
	if( GCL_GetOption( 't' ) != NULL ){
		time = GCL_GetNextInt();
		if ( time > 3 ) time = DIRECT_TICK( time );
	}
	if( GCL_GetOption( 'p' ) != NULL ){
		command = ( void * )GCL_GetNextInt();
	}
	if( GCL_GetOption( 'e' ) != NULL ){
		int type, value;
		GCL_GetNextValue( GCL_NextStr(), &type, &value );
		ASSERT( type == GCL_BLOCK );
		command = ( char * )( value );
      isExecBlock = 1;
	}
	if( GCL_GetOption( 'g' ) != NULL ){
		time = -time;
	}
	ASSERT( time > 0 && command != NULL );

	if( ( work = UTL_DelayedExecCommand( command, NULL, time, isExecBlock ) ) != NULL ){
		work->name = name;

		if( GCL_GetOption( 'a' ) != NULL ){
			int i;
			work->arg.argv = work->argv;
			for( i = 0; i < GCL_MAX_ARGS; i++ ){
				if( GCL_NextStr() == NULL ) break;
				work->arg.argv[ i ] = GCL_GetNextInt();
			}
			work->arg.argc = i;
		}
		GV_SetActorMessageKill( work, name );
	}
   // BP Hack to disable pause when loading up save at end of tanker
   // If pause is enabled and the user pauses while this delay object is active,
   // the user will see a black screen with no indication a pause is stopping progress.
   // Note: This only happens when the script is executing from a load at the end of tanker,
   // a natural playthru already has script logic to disable pause during this delay
   if( strcmp( GM_GetArea(), "d13t" ) == 0 )
   {
      gBP_Hack_DisablePauseDuringPlantLoad = 1;
   }
	return work;
}

