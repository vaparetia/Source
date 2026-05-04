//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	soundtest.c
		サウンドテスト

	2001/07/19 K.Uehara
	$Id: soundtest.c,v 1.1.1.3 2002/11/19 11:51:38 Yoshizawa1 Exp $
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

#include "BP_FileSupport.h"

#include "gameheader.h"

typedef struct {
	GV_ACT actor;
	char *argtop;
	char *mes;
	int pos;
	int current;
	int count;
	int prev_dir;
	int status;
	int stream;
   const char** ppVoxFileNames;
   unsigned int* pVoxFileOffsets;
   int numVoxFiles;
} Work;

enum {
	STATUS_SELECT = 0,
	STATUS_PLAY,
};

#define FIRST_REPEAT	(10*2)
#define SECOND_REPEAT	(1)


static void updown( Work *work, int dir )
{
	int i;
	char *mes = NULL;
	int pos = 0;

	work->current = work->current + dir;
	if( work->current < 0 )
		work->current = 0;
   else if (work->current >= work->numVoxFiles)
      work->current = work->numVoxFiles - 1;
	
   work->mes = work->ppVoxFileNames[work->current];
	work->pos = work->pVoxFileOffsets[work->current];
}

static void Act( Work *work )
{
	int button;
	GV_PAD *pad;

	pad = &( GV_PadData[ 0 ] );

	MENU_SetColor( 200, 200, 200 );

	switch( work->status ){
	  case STATUS_SELECT:
		button = pad->status;
		if( button & ( PAD_U | PAD_D ) ){
			int dir;
			int rep = 1;
			int i;

			if( button & PAD_R1 ){
				rep = 32;
			}

			dir = ( button & PAD_U ) ? -1 : 1;
			if( work->prev_dir == dir ){
				work->count --;
				if( work->count < 0 ){
					for( i = 0; i < rep; i++ ){
						updown( work, dir );
					}
					work->count = SECOND_REPEAT;
				}
			} else {
				for( i = 0; i < rep; i++ ){
					updown( work, dir );
				}
				work->count = FIRST_REPEAT;
				work->prev_dir = dir;
			}
		} else {
			work->prev_dir = 0;
		}
		if( pad->release & PAD_A ){
         if (work->pos != -1)
         {
			   work->stream = GM_VoxStream( work->pos, GM_STREAM_CHANNEL_0 );
			   work->status = STATUS_PLAY;
         }
         else
         {
            printf("ERROR PLAYING STREAM: POS WAS -1\n");
         }
		}
		break;
	  case STATUS_PLAY:
		{
			int status;

			status = GM_StreamStatus( work->stream );

			if( status == GM_STREAM_STATE_END ){
				work->status = STATUS_SELECT;
			}

			if( pad->press & PAD_B ){
				GM_StreamStop( work->stream );
			}

			MENU_Locate( 256, 160, MENU_MODE_CENTER );
			MENU_Printf( "PLAYING %2d\n", status );
		}
		break;
	}
	MENU_Locate( 256, 120, MENU_MODE_CENTER );
	MENU_Printf( work->mes );
}

static void Die(Work* work)
{
   free(work->ppVoxFileNames);
   free(work->pVoxFileOffsets);
}

static int GetResources( Work *work )
{
   int i;

	if( GCL_GetOption( 's' ) == NULL ){
		printf( "NO MENU\n" );
		return -1;
	}

   // Get all the vox streams so the vox test can play them back.
   work->numVoxFiles = BP_GetNumFileEntriesFromRemapTable(4); // 4 == vox
   work->pVoxFileOffsets = (unsigned int*)malloc(work->numVoxFiles * sizeof(unsigned int));
   work->ppVoxFileNames = (char **)malloc(work->numVoxFiles * sizeof(char*));
   BP_GetRemapTableEntries(4, work->pVoxFileOffsets, work->ppVoxFileNames);

   // Convert to sectors
   for (i = 0; i < work->numVoxFiles; i++)
   {
      work->pVoxFileOffsets[i] /= 2048;
   }

	work->current = 0;
	work->prev_dir = 0;
	updown( work, 0 );

	work->status = STATUS_SELECT;

	return 0;
}

void *NewSoundTest( void )
{
	Work *work;

	work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) );

	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );

		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}


