//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pause.c
		ポーズして字幕を出してボタンがおされたらPROC実行
	2000/05/29	K.Uehara
	$Id: pause.c,v 1.1.1.3 2002/11/19 11:51:37 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "font.h"

typedef struct {
	GV_ACT actor;
	int name;
	char *message;
	int proc;
	int	exec ;
	int flags;
	int pause;
} Work;

static void Act( Work *work )
{
	MENU_ResetColor();
	MENU_Locate( 256, 200, MENU_MODE_CENTER );
	MENU_Printf( work->message );

	if( GV_PadDataDirect[ 0 ].press & PAD_A ){
		GV_PauseOffActorSystem( work->pause );
		if ( work->proc != 0 ) GCL_ExecProc( work->proc, NULL );
		else if ( work->exec != 0 ) GCL_ExecBlock( ( char * )work->proc, NULL );
		{
			int i;
			for( i = 0; i < 4; i++ ){
				if( work->flags & ( 1 << i ) ){
					DG_Chanls[ i ].flag = 1;
				} else {
					DG_Chanls[ i ].flag = 0;
				}
			}
		}
		DG_Chanls[ 4 ].bg_clear_flag = 0;
		GV_DestroyActor( work );
	}
}

static void GetResources( Work *work )
{
	{
		int i, flags;
		flags = 0;
		for( i = 0; i < 4; i++ ){
			if( DG_Chanls[ 0 ].flag ){
				flags |= ( 1 << i );
			}
			DG_Chanls[ i ].flag = 0;
		}
		work->flags = flags;
	}
	DG_Chanls[ 4 ].bg_clear_flag = 1;
}

void *NewDebugPauseProg( char *mesg, int proc, int pause )
{
	Work *work;

	work = ( Work * )GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 10 );
	GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, NULL );

		GetResources( work );

		work->message = mesg;
		work->proc = proc;
		work->pause = GV_PAUSE_STOP;
	}
	return work;
}


void *NewDebugPause( int name, int map )
{
	Work *work;

	work = ( Work * )GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 10 );
	GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, NULL );

		GetResources( work );

		work->message = GCL_GetNextString();
		work->proc = GCL_GetOptionValue( 'p', 0 );
		work->exec = GCL_GetOptionValue( 'e', 0 );
		work->pause = GV_PAUSE_STOP;
		GV_PauseOnActorSystem( work->pause );
	}
	return work;
}
