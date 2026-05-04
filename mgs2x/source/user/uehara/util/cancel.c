//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cancel.c
		デモキャンセル用

	2000/10/11 K.Uehara
	$Id: cancel.c,v 1.1.1.3 2002/11/19 11:51:36 Yoshizawa1 Exp $
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

#include "BP_TrophyLogicMGS2.h"
#include "BP_Misc.h"


typedef struct {
	GV_ACT actor;
	int timer;
	int name;
	int proc;
} Work;

static void Act( Work *work )
{
	if( work->timer > 0 )
   {
		work->timer --;
		return;
	}

	if( GV_PadDataDirect[ 0 ].release & PAD_DEMO_CANCEL )
   {
		GV_DestroyActor( work );
		GCL_ExecProc( work->proc, NULL );
      bp_trophy_skipped_cinematic();

      BP_ConsoleScreenSaverResume();        //BP JG - screen saver active again.
	}
}

void *NewPadCancel( int name, int map )
{
	Work *work;

	if( ( work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) ) ) != NULL )
   {
		if( GCL_GetOption( 'p' ) == NULL )
      {
			HANGUP();
		}
		work->proc = GCL_GetNextInt();
		work->name = name;
		work->timer = 10;

		GV_SetActor( work, Act, NULL );
		GV_SetActorMessageKill( work, name );
	}

   BP_ConsoleScreenSaverSuspend();        //BP JG - prevent the screen saver from activating during the next game sequence
   
   return work;
}
