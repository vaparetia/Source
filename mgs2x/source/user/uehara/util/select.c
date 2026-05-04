//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	select.c
		デバッグ用セレクタ

	1999/09/06 K.Uehara
	$Id: select.c,v 1.1.1.3 2002/11/19 11:51:37 Yoshizawa1 Exp $
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

#include "debugmenu.h"

//BP
const char * gBP_InitialStage = NULL;
#define BP_RESIDENT_RAIDEN 0x00d3115e
#define BP_RESIDENT_SSNAKE 0x009cf967
#define BP_RESIDENT_SNKTLS 0x000f4e36

static int sBP_InitialStageResidentLoadProc = 0;
static int sBP_InitialStageProc = 0;

typedef struct _SBP_StageSelectProc
{
   int            mResidentLoadProc;
   int            mStageLoadProc;
   const char *   mName;
} SBP_StageSelectProc;

//N.B. to get the proc name of a stage, set a breakpoint at the line
//    GCL_ExecProc( work->proc, NULL );
//in Act() and check the value of work->proc at the moment you select the stage from the 'select' stage.
static const SBP_StageSelectProc skStageSelectProcs[] =
{
   { BP_RESIDENT_SSNAKE, 0x0027d9f5, "w00a" },
   { BP_RESIDENT_SSNAKE, 0x002f6624, "w00b" },
   { BP_RESIDENT_SSNAKE, 0x002f6a24, "w00c" },
   { BP_RESIDENT_SSNAKE, 0x002ec224, "w01a" },
   { BP_RESIDENT_SSNAKE, 0x00fac51d, "w01f" },
   { BP_RESIDENT_SSNAKE, 0x002fee25, "w01b" },
   { BP_RESIDENT_SSNAKE, 0x002fd224, "w01c" },
   { BP_RESIDENT_SSNAKE, 0x002fde24, "w01d" },
   { BP_RESIDENT_SSNAKE, 0x002fea24, "w01e" },
   { BP_RESIDENT_SSNAKE, 0x000c8e96, "w02a" },
   { BP_RESIDENT_SSNAKE, 0x00efed23, "w03a" },
   { BP_RESIDENT_SSNAKE, 0x0032c624, "w03b" },
   { BP_RESIDENT_SSNAKE, 0x0029d9f4, "w04a" },
   { BP_RESIDENT_SSNAKE, 0x0029ddf4, "w04b" },
   { BP_RESIDENT_SSNAKE, 0x0029e1f4, "w04c" },

   { BP_RESIDENT_SSNAKE, 0x002825ce, "d00t" },
   { BP_RESIDENT_SSNAKE, 0x0028a5ce, "d01t" },
   { BP_RESIDENT_SSNAKE, 0x002a25ce, "d04t" },
   { BP_RESIDENT_SSNAKE, 0x002aa5ce, "d05t" },
   { BP_RESIDENT_SSNAKE, 0x002aa5d0, "d06a1d" },
   { BP_RESIDENT_SSNAKE, 0x002aa5cf, "d06a2d" },
   { BP_RESIDENT_SSNAKE, 0x0029d9f5, "d10t" },
   { BP_RESIDENT_SSNAKE, 0x0038a5cf, "d11t" },
   { BP_RESIDENT_SSNAKE, 0x003925ce, "d12t" },
   { BP_RESIDENT_SSNAKE, 0x003925cf, "d12t2"},
   { BP_RESIDENT_SSNAKE, 0x003925d0, "d12t3"},
   { BP_RESIDENT_SSNAKE, 0x002413d7, "d12t4"},
   { BP_RESIDENT_SSNAKE, 0x0039a5ce, "d13t" },

   { BP_RESIDENT_SSNAKE, 0x003925ce, "t12a1d" },
   { BP_RESIDENT_SSNAKE, 0x003925d0, "t12a3d" },

   { BP_RESIDENT_RAIDEN, 0x00dc795f, "d001p01" },
   { BP_RESIDENT_RAIDEN, 0x008bf4a2, "d001p02" },
   { BP_RESIDENT_RAIDEN, 0x005c7960, "d005p01" },
   { BP_RESIDENT_RAIDEN, 0x005c8220, "d005p03" },
   { BP_RESIDENT_RAIDEN, 0x00af0067, "d055p01" }, // after vamp
   { BP_RESIDENT_RAIDEN, 0x00847ab8, "d082p01" },

   { BP_RESIDENT_RAIDEN, 0x00b35f86, "d021p01_ninja" }, //Ninja intro
   { BP_RESIDENT_RAIDEN, 0x00af3eff, "d021p01" }, //Fatman after
   { BP_RESIDENT_RAIDEN, 0x000817ee, "d045p01" }, //Harrier before
   { BP_RESIDENT_RAIDEN, 0x007fd1eb, "d070p01" }, //AG Startup

   { BP_RESIDENT_RAIDEN, 0x000a8897, "w11a" },
   { BP_RESIDENT_RAIDEN, 0x0060c624, "w11b" },
   //{ BP_RESIDENT_RAIDEN, 0x008d8ff7, "w11c" },
   { BP_RESIDENT_RAIDEN, 0x00763831, "w11c" }, //VS Fortune
   
   { BP_RESIDENT_RAIDEN, 0x00df6527, "w12a" },
   { BP_RESIDENT_RAIDEN, 0x00280e9c, "w12b" },
   { BP_RESIDENT_RAIDEN, 0x00005528, "w12c" },
   { BP_RESIDENT_RAIDEN, 0x0000d528, "w13a" },
   { BP_RESIDENT_RAIDEN, 0x00395df4, "w13b" },
   { BP_RESIDENT_RAIDEN, 0x00634224, "w14a" },
   { BP_RESIDENT_RAIDEN, 0x0058c1d2, "w14a_after_vamp" },
   { BP_RESIDENT_RAIDEN, 0x0064c224, "w15a" },
   { BP_RESIDENT_RAIDEN, 0x0064c224, "w15a-1" },
   { BP_RESIDENT_RAIDEN, 0x0066c224, "w15a-2" },
   { BP_RESIDENT_RAIDEN, 0x003a5df4, "w15b" },
   { BP_RESIDENT_RAIDEN, 0x00664224, "w16a" },
   { BP_RESIDENT_RAIDEN, 0x00035c1e, "w16a_stillman" },
   { BP_RESIDENT_RAIDEN, 0x00455AC2, "bomb_event_start_w16b" },
   { BP_RESIDENT_RAIDEN, 0x0067c224, "w17a" },
   { BP_RESIDENT_RAIDEN, 0x00694224, "w18a" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w19a" },
   { BP_RESIDENT_RAIDEN, 0x00e78e9e, "w20a" },
   { BP_RESIDENT_RAIDEN, 0x00fdea9c, "sniping_event_end" },
   { BP_RESIDENT_RAIDEN, 0x00816cbd, "w20c" },
   { BP_RESIDENT_RAIDEN, 0x00ced52b, "w21a" },
   { BP_RESIDENT_RAIDEN, 0x00ced52b, "w21a-1" },
   { BP_RESIDENT_RAIDEN, 0x00d2ee1b, "w21a-2" },
   { BP_RESIDENT_RAIDEN, 0x0092c224, "w21a-3" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w21b" },
   { BP_RESIDENT_RAIDEN, 0x00f1fe58, "w22a" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w23a" },
   { BP_RESIDENT_RAIDEN, 0x00495df4, "w23b" },
   { BP_RESIDENT_RAIDEN, 0x00914224, "w24a" },
   { BP_RESIDENT_RAIDEN, 0x00944624, "w24b" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w24c" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w24d" },
   { BP_RESIDENT_RAIDEN, 0x00E72ADE, "psg1_event" },
   { BP_RESIDENT_RAIDEN, 0x00fdea9c, "w25a_alt" }, // was w20a?
   { BP_RESIDENT_RAIDEN, 0x00750782, "w25a" },
   { BP_RESIDENT_RAIDEN, 0x0078c225, "w25a_harrier" },
   { BP_RESIDENT_RAIDEN, 0x0095c624, "w25b" },
   { BP_RESIDENT_RAIDEN, 0x0095d224, "w25c" },
   { BP_RESIDENT_RAIDEN, 0x00aa3152, "w25d" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w28a" },
   { BP_RESIDENT_RAIDEN, 0x00a3d224, "w31a" },
   { BP_RESIDENT_RAIDEN, 0x001005ca, "shell2_in" },
   { BP_RESIDENT_RAIDEN, 0x00bfc624, "w31b" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w31c" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w31d" },
   { BP_RESIDENT_RAIDEN, 0x00ace74e, "w31f" },
   { BP_RESIDENT_RAIDEN, 0x00a74224, "w32a" },
   { BP_RESIDENT_RAIDEN, 0x00c14624, "w32b" },
   { BP_RESIDENT_RAIDEN, 0x00b2da24, "w41a-1" },
   { BP_RESIDENT_RAIDEN, 0x001ed9d7, "w41a-after_goumon" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w42a" },
   { BP_RESIDENT_RAIDEN, 0x00f1c224, "w43a" },
   { BP_RESIDENT_RAIDEN, 0x00875648, "w44a" },
   { BP_RESIDENT_RAIDEN, 0x00711efe, "w45a" },
   { BP_RESIDENT_RAIDEN, 0x00000000, "w46a" },
   { BP_RESIDENT_RAIDEN, 0x0004c225, "w51a" },
   { BP_RESIDENT_RAIDEN, 0x0010785f, "w61a" },

   { BP_RESIDENT_SSNAKE, 0x002973ad, "vsolga" },
   { BP_RESIDENT_SSNAKE, 0x002973ad, "vsorga" },

   { BP_RESIDENT_RAIDEN, 0x00763831, "vsfortune" },
   { BP_RESIDENT_RAIDEN, 0x0084933d, "vsfatman" },
   { BP_RESIDENT_RAIDEN, 0x00750782, "vsharrier" },
   { BP_RESIDENT_RAIDEN, 0x00b3e044, "vsvamp" },
   { BP_RESIDENT_RAIDEN, 0x00a58f0a, "vsray" },
   { BP_RESIDENT_RAIDEN, 0x0010785f, "vssolidus" },
   
   { BP_RESIDENT_RAIDEN, 0x00ace74e, "emma_find_demo_end" },
   { BP_RESIDENT_RAIDEN, 0x0078d8a4, "emma_3" },
   { BP_RESIDENT_RAIDEN, 0x00847ab8, "ending" },
   { BP_RESIDENT_RAIDEN, 0x0077d713, "boss_survival" },  //(playing as Raiden)
   { BP_RESIDENT_RAIDEN, 0x00687253, "nikita_event_start" },
   { BP_RESIDENT_RAIDEN, 0x00e7a15b, "nikita_event_end" },
   { BP_RESIDENT_RAIDEN, 0x0058729c, "vsharrier_end" },

   { BP_RESIDENT_SNKTLS, 0x0066c43f, "ta02a" },
   { BP_RESIDENT_SNKTLS, 0x00975e02, "ta03b" },
   { BP_RESIDENT_SNKTLS, 0x00ebc0f4, "tc_endb" },
   { BP_RESIDENT_SNKTLS, 0x006804b3, "tb_enda" },
   { BP_RESIDENT_SNKTLS, 0x0058cb4e, "ta20c" },
   { BP_RESIDENT_SNKTLS, 0x00eac074, "ta24c" },
   { BP_RESIDENT_SNKTLS, 0x0095677a, "tsp03a" },
   { BP_RESIDENT_SNKTLS, 0x00976202, "a31a_d5" },
   { BP_RESIDENT_SNKTLS, 0x007ba5d2, "tales_a" }
};

void BP_SetInitialStage( const char * stagename )
{
   int i;
   //Ensure that everything is initialized properly by only executing the initial stage jump
   //per the procs inside select.gcl, otherwise stuff isn't loaded and variables aren't
   //initialized properly.

   //special case for starting in select stage.
   if( !strcmp( stagename, "select" ) )
   {
      gBP_InitialStage = "select";
      return;
   }

   //pick which resident stage to load and which stage to jump to in the script.
   for( i=0; i < sizeof( skStageSelectProcs ) / sizeof( *skStageSelectProcs ); ++i )
   {
      if( !strcmp( stagename, skStageSelectProcs[i].mName ) )
      {
         //always jump straight to select.
         gBP_InitialStage = "select";

         //Set the script procs to run.
         sBP_InitialStageResidentLoadProc = skStageSelectProcs[i].mResidentLoadProc;
         sBP_InitialStageProc = skStageSelectProcs[i].mStageLoadProc;

         if( !sBP_InitialStageProc )
            BP_BREAK;   //TODO

         printf("BP - jumping to stage %s.\n", stagename );
         return;
      }
   }

   printf("BP - unknown stage name %s.  Stage jump failed.\n", stagename );
}

/* ---------------------------------------------------------------------- */
/*
	select
*/

typedef struct _select_Work {
	GV_ACT actor;
	char *argtop;
	char *mes;
	int proc;
	int current;
	int count;
	int prev_dir;
} Work;

#define FIRST_REPEAT	(10*2)
#define SECOND_REPEAT	(2*2)

static void updown( Work *work, int dir )
{
	int i;
	char *mes = NULL;
	int proc = 0;

	work->current = work->current + dir;
	if( work->current < 0 ){
		work->current = 0;
	}
	GCL_SetArgTop( work->argtop );
	for( i = 0; i <= work->current; i++ ){
		if( GCL_NextStr() == NULL ){
			work->current = i;
			break;
		}
		mes = GCL_GetString( GCL_NextStr() );
		proc = GCL_GetInt( GCL_NextStr() );
	}
	work->mes = mes;
	work->proc = proc;
#if 1 //BP def DEBUG_MODE
	printf( "Select [%s]\n", mes ) ;
#endif
}

static void Act( Work *work )
{
	int button;
	GV_PAD *pad;

#ifndef GOLD_VERSION
   //BP - perform jump just once.
   if( sBP_InitialStageProc )
   {
      GCL_ExecProc( sBP_InitialStageResidentLoadProc, NULL );
      GCL_ExecProc( sBP_InitialStageProc, NULL );
      sBP_InitialStageResidentLoadProc = 0;
      sBP_InitialStageProc = 0;
      GV_DestroyActor( work );
   }
#endif

	pad = &( GV_PadDataDirect[ 0 ] );

	button = pad->status;

	if( button & ( PAD_U | PAD_D ) ){
		int dir;

		dir = ( button & PAD_U ) ? -1 : 1;
		if( work->prev_dir == dir ){
			work->count --;
			if( work->count < 0 ){
				updown( work, dir );
				work->count = SECOND_REPEAT;
			}
		} else {
			updown( work, dir );
			work->count = FIRST_REPEAT;
			work->prev_dir = dir;
		}
	} else {
		work->prev_dir = 0;
	}

	if( pad->press & PAD_CANCEL ){
		int cur;
		do {
			cur = work->current;
			updown( work, 1 );
		} while( cur != work->current );
	}
	if( pad->release & PAD_OK ){
      printf(">>>Entering stage %s: %08x\n", work->mes, work->proc);
		GCL_ExecProc( work->proc, NULL );
		GV_DestroyActor( work );
	}
#if 1
	MENU_SetColor( 200, 200, 200 );
	MENU_Locate( 256, 120, MENU_MODE_CENTER );
	MENU_Printf( work->mes );
#endif
}

static int GetResources( Work *work )
{
	if( GCL_GetOption( 's' ) == NULL ){
		printf( "NO MENU\n" );
		return -1;
	}
	work->argtop = GCL_NextStr();
	work->current = 0;
	work->prev_dir = 0;
	updown( work, 0 );

	return 0;
}

void *NewSelect( void )
{
	Work *work;

	work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) );

	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, NULL );

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}

