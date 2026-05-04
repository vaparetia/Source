//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
/*
	mpeg.c
		MPEG2 エンディング再生ルーチン

	2000/11/06	K.Uehara
	$Id: ending.c,v 1.1.1.3 2002/11/19 11:51:34 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#include "gameheader.h"
//#include "common.h"

#include "libfs.h"

#include "mpegstr.h"
#include "strctrl.h"

#include "BP_BuildDefines.h"
#include "BP_MovieSupport.h"

extern u_int64 BP_GetElapsedMicroSeconds();

/* ---------------------------------------------------------------------- */
/*
	work
*/

typedef struct ending_ALL_PACKET
{
   DG_DMAPACK_VIEWMAPPING		viewmapping;

   DG_DMAPACK_TEX			      texture;

   DG_DMAPACK_ALPHA			   alpha;
   DG_DMAPACK_SPRT				sprt;

   DG_DMAPACK_TAG             end;

} ALL_PACKET;

typedef struct ending_Work
{
	GV_ACT actor;
   int end_proc;

   DG_DMAPACK dmapack;
   ALL_PACKET  packet;

   int isPlayingPreMovie;

   BP_Movie_Handle   movieHandle;
   u_int64           movieStartTime;

   int telop_end_flag;
   int cancel_enable;
   
   char bp_endingMovieName[256];

} Work;

/* ---------------------------------------------------------------------- */
/*
	Act, Die
*/

static void Act( Work *work )
{
   if( work->isPlayingPreMovie )
   {
      int const movieStreamTime = GetMovieStreamTime();

#if defined(BP_VITA)
      if( movieStreamTime < 117 * 300 )
#else
      if( movieStreamTime < 120 * 300 )
#endif
      {
         return;
      }

      MovieStream_KillCreditsMoviePlayer();

      work->isPlayingPreMovie = 0;
   }

   // Initialize movie handle first time around.
   if( !work->movieHandle )
   {
      work->movieHandle = BP_Movie_CreateStandalonePlayer(work->bp_endingMovieName);

      // Update dma pack with movie player texture.
      {
         ALL_PACKET* packet = &work->packet;
         DG_SetDmapackTex(&packet->texture, work->movieHandle ? BP_Movie_Get_Texture(work->movieHandle) : NULL);

         work->dmapack.flag &= ~DG_DMAPACK_INVISIBLEMENU;
      }

      DG_UnDrawFrameCount = DG_UNDRAW_MAX;
   }

   if( work->cancel_enable && ( GV_PadData[0].release & PAD_CANCEL ) )
   {
      work->telop_end_flag = 1;
      work->dmapack.flag |= DG_DMAPACK_INVISIBLEMENU;
   }

   // Update movie
   if( work->movieHandle )
   {
      int movie_state = BP_Movie_Get_State(work->movieHandle);

      if (movie_state == MOVIE_STATE_READY)
      {
         DG_UnDrawFrameCount = 0;
         BP_Movie_StandalonePlayer_Play(work->movieHandle);
         work->movieStartTime = BP_GetElapsedMicroSeconds();
      }
      else if (movie_state == MOVIE_STATE_STOPPED)
      {
         work->telop_end_flag = 1;
      }

      if( movie_state == MOVIE_STATE_PLAYING )
      {
         // Update movie time
         u_int64 currentTime = BP_GetElapsedMicroSeconds();
         u_int64 elapsedTimeMicroS = currentTime - work->movieStartTime;
         float elapsedTime = elapsedTimeMicroS / 1000000.0f;
         BP_Movie_Set_Time(work->movieHandle, elapsedTime);
      }
   }
   else
   {
      DG_UnDrawFrameCount = 0;
      work->telop_end_flag = 1;
   }

   if( work->telop_end_flag )
   {
      GV_DestroyActor(work);
   }

}

static void Die( Work *work )
{
	if( !GV_IsStageDestroy( work ) )
   {
		if( work->end_proc != 0 ){
			GCL_ExecProc( work->end_proc, NULL );
		}
	}

   if( work->movieHandle )
      BP_Movie_DestroyStandalonePlayer(work->movieHandle);

   DG_DequeueDmapack( &work->dmapack );

	GV_SetActorFreeFunc( work, GV_DelayedFree );
}

/* ---------------------------------------------------------------------- */

static int GetResources( Work *work )
{
   int nameId = GCL_GetOptionValue( 'n', 0 );
   int telopId = GCL_GetOptionValue( 't', 0 );

   if( GCL_GetOption( 'f' ) != NULL )
   {
      // Force stream to be a specific format since we're replacing ALL of its data.
      int top_pos = GCL_GetNextInt();
      top_pos = (top_pos & 0x0FFFFFFF) | TAGFLAG_SOUND_8BIT;

      RequestMovieStream(top_pos, 512, 320, 0, 0, 0, 0, 512, 320);

      work->isPlayingPreMovie = 1;
   }
   else
   {
      work->isPlayingPreMovie = 0;
   }

   if( GCL_GetOption( 'p' ) != NULL )
   {
		work->end_proc = GCL_GetNextInt();
	} 
   else 
   {
		work->end_proc = 0;
	}

   work->cancel_enable = 0;

   {
      ALL_PACKET* packet = &work->packet;
      work->dmapack.autopacket = packet;
      DG_SetDmapackViewMapping( &packet->viewmapping, 0.0f, 0.0f, (float)DRAW_WIDTH, (float)DRAW_HEIGHT );
      DG_SetDmapackAlpha( &packet->alpha, SCE_GS_SET_ALPHA( 0, 0, 0, 0, 0x80 ) );

      DG_SetDmapackTex(&packet->texture, NULL);

      DG_SetDmapackSprt(&packet->sprt, 0, 0, 0.0f, 0.0f, 512, 448, 1.0f, 1.0f, DG_MakeDmaPackColorFromInt(0x80808080));

      DG_SetDmapackEnd(&packet->end);
   }

   {
      DG_DMAPACK *pack;

      pack = &( work->dmapack );

      pack->flag = DG_DMAPACK_MENU|DG_DMAPACK_INVISIBLEMENU;
      pack->phase = DG_DMAPACK_PHASE_AFTER;
      pack->priority = 250;
      DG_QueueDmapack(pack);
   }

   if( telopId == 0x0052c23b /*ending_st_e*/)
   {
      switch(nameId)
      {
      case 0x00c2d82a: // titlea_st_e
         strcpy(work->bp_endingMovieName, "misc/mgs2_snaketale_a");
         break;
      case 0x00c2d82c: // titleb_st_e
         strcpy(work->bp_endingMovieName, "misc/mgs2_snaketale_b");
         break;
      case 0x00c2d82e: // titlec_st_e
         strcpy(work->bp_endingMovieName, "misc/mgs2_snaketale_c");
         break;
      case 0x00c2d830: // titled_st_e
         strcpy(work->bp_endingMovieName, "misc/mgs2_snaketale_d");
         break;
      case 0x00c2d832: // titlee_st_e
         strcpy(work->bp_endingMovieName, "misc/mgs2_snaketale_e");
         break;
      }
#if defined(BP_PS3)
      strcat(work->bp_endingMovieName, ".m2v");
#elif defined(BP_360)
      strcat(work->bp_endingMovieName, ".wmv");
#elif defined(BP_VITA)
      strcat(work->bp_endingMovieName, ".mp4");
#else
#error Invalid platform.
#endif

   }
   else
   {
      // Assume default ending credits movie
#if defined(BP_PS3)
      if(BP_Area_JP())
         strcpy(work->bp_endingMovieName, "misc/staffroll_ps3_jpn.mp2");
      else
         strcpy(work->bp_endingMovieName, "misc/staffroll_ps3_eng.mp2");
#elif defined(BP_360)
      if(BP_Area_JP())
         strcpy(work->bp_endingMovieName, "misc/staffroll_360_jpn.wmv");
      else
         strcpy(work->bp_endingMovieName, "misc/staffroll_360_eng.wmv");
#elif defined(BP_VITA)
      if(BP_Area_JP())
         strcpy(work->bp_endingMovieName, "misc/staffroll_vta_jpn.mp4");
      else
         strcpy(work->bp_endingMovieName, "misc/staffroll_vta_eng.mp4");
#endif

   }


	GM_SetGameStatus(STATE_DEMO);

	return 0;
}

void *NewEnding( int name, int map )
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
