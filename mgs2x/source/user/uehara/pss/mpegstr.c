//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mpeg.c
		MPEG2 PSS再生ルーチン

	2000/11/06	K.Uehara
	$Id: mpegstr.c,v 1.1.1.3 2002/11/19 11:51:34 Yoshizawa1 Exp $
*/

#if defined(BP_WIN32) || defined(BP_360)
typedef __int64 int64;
#elif defined(BP_PS3) || defined(BP_VITA)
typedef long long int64;
#else
#error todo
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libmpeg.h>
#include <libcdvd.h>
#include <string.h>

#include "libdg.h"
#include "mts.h"
#include "cdbios.h"
#include "gameheader.h"
#include "def_dma.h"
#include "libfs.h"
#include "stream.h"
#include "strctrl.h"

#include "BP_Renderer.h"
#include "BP_MovieSupport.h"
#include "BP_Misc.h"

#define BP_MPEG_TODO_BREAK

#define TB_WIDTH	512
#define TB_W		9
#define TB_H		9

/* ---------------------------------------------------------------------- */
typedef struct {	
   DG_DMAPACK_VIEWMAPPING viewmapping;
   DG_DMAPACK_TEX texture;
   DG_DMAPACK_ALPHA alpha;
   DG_DMAPACK_SPRT sprt;
   DG_DMAPACK_TAG end;
} DRAW_PACKET;

/* ---------------------------------------------------------------------- */
/*
	work
*/

typedef struct _mpegstr_Work {
	GV_ACT actor;
	DRAW_PACKET draw;
   DG_TEX_LIN* movie_tex;
	int status;
	int end_flag;

	/* 読み込み場所など */
	int top_pos;
	int file;
	int stream_id;
	void *stream_h;

	int stream_type;
	int filesize;
	int pos;
	int nowsize;
	int mpegwork_size;
	int file_th_id;
	int end_proc;
	int x, y;
	int width, height;
	int repeat_flag;
	int pad_cancel;
	int framenum;

	int bReqCancel;
	int disp_width;	
	int disp_height;
   BP_Movie_Handle movie_handle;
   int force_finish_stream;
   int priority;

	GM_STREAM_CONTROL *ctrl;

	/* 描画用 */
	DG_DMAPACK dmapack;

   int has_started_movie;
} Work;


static Work *work_ptr;
static Work* pCurrentActor;

typedef struct _REQUEST_INFO {
	int top_pos;
	int file;
	int width;
	int height;
	int x;
	int y;
	int repeat;
	int pad_cancel;
	int	end_proc;
	int priority;
	int disp_width;	
	int disp_height;	
} REQUEST_INFO;

static REQUEST_INFO request_info = { -1, FS_FILE_MOVIE };

static void *NewMpegPssMovieStrProg( REQUEST_INFO* pReqinfo );

/* ---------------------------------------------------------------------- */

static void make_draw_packet( Work *work, DRAW_PACKET *draw )
{
   float x0, y0, x1, y1;

   DG_TEX* dgTex = BP_Movie_Get_Texture(work->movie_handle);

   DG_SetDmapackViewMapping(&draw->viewmapping, 0.0f, 0.0f, BP_REAL_SCREEN_X, BP_REAL_SCREEN_Y);

   DG_SetDmapackTex(&draw->texture, dgTex);
   DG_SetDmapackAlpha( &draw->alpha, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

   BP_Movie_GetMovieCoordinates(work->x, work->y, work->disp_width, work->disp_height, &x0, &y0, &x1, &y1);

   DG_SetDmapackSprt( &draw->sprt, x0, y0, 0.f, 0.f, x1, y1,  1.0f, 1.0f, DG_MakeDmaPackColorFromInt(0x80808080));
   DG_SetDmapackEnd(&draw->end);
}

/* ---------------------------------------------------------------------- */
/*
	ストリームドライバ
*/

static void *NewStreamMovieDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	work_ptr->stream_type = type;
	work_ptr->ctrl = ctrl;

	work_ptr->stream_h = ctrl->stream_h;
	FS_StreamLock( ctrl->stream_h );

	return GM_STREAM_DRIVER_NO_LINK;
}

static GM_STREAM_DRIVER driver = {
	NULL, 0, /*driver: */NewStreamMovieDriver,
};

/* ---------------------------------------------------------------------- */
/*
	Act, Die
*/
static void OnRenderVideoCallback(unsigned char* pBuffer, int* pOutBytesRead)
{
   char *ptr;
   int size;

   // can't start if there's no stream YET, it might be a frame behind.
   if ( !work_ptr || work_ptr->stream_h == NULL )
   {
      *pOutBytesRead = 0;
      return;
   }

   ptr = FS_StreamGetData( work_ptr->stream_h, work_ptr->stream_type );
   if( ptr == NULL ){
      int status = FS_StreamGetStatus(work_ptr->stream_h);
      if (status == FS_STR_STOP ) {
         printf( "READ END\n" );
         *pOutBytesRead = -1;
      }
      else {
         *pOutBytesRead = 0;
      }

      return;
   }
   else {
      size = FS_STREAM_GET_SIZE(ptr);
      *pOutBytesRead = size;
      memcpy(pBuffer, ptr, size);
   }

   FS_StreamFreeData( work_ptr->stream_h, ptr );
}

enum {
	STATE_INIT,
	STATE_PLAY,
	STATE_END,
};

static void Act( Work *work )
{
	switch( work->status ){
	  case STATE_INIT:
	   work_ptr = work;
		work->stream_h = NULL;
		{
			int flag;

			if( work->end_proc > 0 && work->repeat_flag == 0 ){
				flag = GM_STREAM_FLAG_PROC( work->end_proc );
			} else {
				flag = 0;
			}
			work->stream_id = GM_MovieStream2( work->file, work->top_pos, flag );
		}

		work->status = STATE_PLAY;
		break;

     case STATE_PLAY:
		{
         int BPShutDown = 0;

         if (!work->movie_handle && !work->force_finish_stream)
         {
            GM_StreamStop( work->stream_id );
            work->end_flag = 1;
            printf( "uehara/pss/mpegstr.c: end_flag = 1 because of no movie handle\n" );
            work->bReqCancel = FALSE;
         }
         else
         {
            if (!work->stream_h)
               return;

            if (!work->has_started_movie)
            {
               char* ptr = FS_StreamGetData(work->stream_h, work->stream_type);

               if ( ptr == NULL )
               {
                  // AS(JM) 
                  // The stream was aborted prior to our movie player starting, so shut it down ASAP
                  // Fix for MGSTWO-3392
                  BPShutDown = 1;
               }
               else
               {
                  int fileSize = FS_STREAM_GET_OPTION(ptr);
                  FS_StreamUngetData(work->stream_h, ptr);

                  BP_Movie_Start(work->movie_handle, fileSize);
                  work->has_started_movie = 1;
               }
            }

            DG_UnDrawFrameCount = 0;

            if ( BPShutDown )
            {
               // If we are shutting down, let it happen organically below...
            } 
            else if( work->movie_handle )
            {
               int movie_state = BP_Movie_Get_State(work->movie_handle);

               if (movie_state == MOVIE_STATE_STOPPED)
               {
                  BPShutDown = 1;
               }
            }
            else 
            {
               // If no more movie, stall until stream is done.
               work->end_flag = 1;
            }
         }           

         if( (GV_PadDataDirect[ 0 ].release & work->pad_cancel) || work->bReqCancel)
         {
            BP_Movie_Stop(work->movie_handle);
            GM_StreamStop(work->stream_id);
            work->end_flag = 1;
            printf( "uehara/pss/mpegstr.c: end_flag = 1 because of pad cancel or rc: %d %d\n", GV_PadDataDirect[ 0 ].release & work->pad_cancel, work->bReqCancel );
            work->bReqCancel = FALSE;
         }

         if (work->end_flag || BPShutDown)
         {
            work->status = STATE_END;

            if (work->stream_h)
               FS_StreamUnlock( work->stream_h );
            break;
         }      

         {
            float gameFrame = (float)work->ctrl->tick / 300.0f;
            BP_Movie_Set_Time(work->movie_handle, gameFrame);

            work->framenum++;
         }
		}
		break;

	  case STATE_END:
      {
         if( cdbios_get_status() != 0 ){
            return;
         }

         if( GM_StreamStatus( work->stream_id ) != GM_STREAM_STATE_END ){
            return;
         }

         work->stream_h = NULL;

         if( work->repeat_flag == 0 ){
            printf( "uehara/pss/mpegstr.c: end_flag = 1 because of repeat==0\n" );
            work->end_flag = 1;
         }

         if( work->end_flag ){
            printf( "END\n" );
            GV_DestroyActor( work );
         } else {
            printf( "REPEAT\n" );
            work->status = STATE_INIT;
            work->framenum = 0;
            work->has_started_movie = 0;
         }
      }
		break;
	}
}

static void Die( Work *work )
{
   BP_ConsoleScreenSaverResume();        //BP JG - screen saver active again.


   DG_DequeueDmapack( &work->dmapack );

   if (work->movie_handle)
      BP_Movie_Destroy(work->movie_handle);

   GM_StreamRemoveDriver( &driver );

   if( work->repeat_flag != 0 && work->end_proc != 0 && !GV_IsStageDestroy( work ) ){
      GCL_ExecProc( work->end_proc, NULL );
   }

   GV_SetActorFreeFunc( work, GV_DelayedFree );
   printf( "mpegstr dead\n" );

   ASSERT( pCurrentActor == work );
   pCurrentActor = NULL;

   work_ptr = NULL;

   if( !GV_IsStageDestroy( work ) ) {
      if( request_info.top_pos != -1 ) {
         NewMpegPssMovieStrProg( &request_info );
         request_info.top_pos = -1;
      }
   }
}

/* ---------------------------------------------------------------------- */
/*
	ACTOR起動
*/

static void GetGclOpions( REQUEST_INFO* pinfo )
{
   float newDisplayWidth;

	if( GCL_GetOption( 's' ) == NULL ){
		ASSERT( FALSE );
	}
	pinfo->disp_width = pinfo->width = GCL_GetNextInt();
	pinfo->disp_height = pinfo->height = GCL_GetNextInt();

	if( GCL_GetOption( 'f' ) != NULL ){
		pinfo->top_pos = GCL_GetNextInt();
//		ASSERT( pinfo->top_pos >= 0 );
	} else {
		ASSERT( FALSE );
	}
	if( GCL_GetOption( 'p' ) != NULL ){
		pinfo->end_proc = GCL_GetNextInt();
	} else {
		pinfo->end_proc = 0;
	}

	if( GCL_GetOption( 't' ) != NULL ){
		pinfo->x = GCL_GetNextInt();
		pinfo->y = GCL_GetNextInt();
	}
	if( GCL_GetOption( 'r' ) != NULL ){
		pinfo->repeat = TRUE;
#if 1
      // BP - Changed to PAD_CANCEL to match the xbux version (see mpegstrx.c)
      // Fixes some situations (such as Special/Basic Actions) where the movie player
      // ending determines a cancel condition.  PAD_DEMO_CANCEL uses the PAD_OK button
      // which means that effectively "Proceed" and "Cancel" are the same.
      //
      pinfo->pad_cancel = PAD_CANCEL;
#else
#  error This code path is dead.
      pinfo->pad_cancel = PAD_DEMO_CANCEL; 
#endif
	} else {
		pinfo->repeat = FALSE;
#ifndef PAL
		pinfo->pad_cancel = ( PAD_DEMO_CANCEL | PAD_A );
#else
		pinfo->pad_cancel = PAD_DEMO_CANCEL;
#endif
	}

   //BP JG Basic Actions movie : move and scale this movie for widescreen, as the dimensions are set in script.
   if ( pinfo->disp_width==224 && pinfo->x==246 && pinfo->y==57 )
   {
      newDisplayWidth = (float)pinfo->disp_width*TARGET_ASPECT_X;
      pinfo->x += (int)(((float)pinfo->disp_width - newDisplayWidth)*0.5f);
      pinfo->disp_width = (int)((float)pinfo->disp_width*TARGET_ASPECT_X);
   }

	pinfo->priority = GCL_GetOptionValue( 'R', 249 );
	pinfo->file = FS_FILE_MOVIE;
}

static int GetResources( Work *work, REQUEST_INFO* pinfo )
{
	work->width = pinfo->width;
	work->height = pinfo->height;

	work->disp_width = pinfo->disp_width;
	work->disp_height = pinfo->disp_height;

	work->top_pos = pinfo->top_pos;
printf( "PSS %d\n", work->top_pos );

	work->end_proc = pinfo->end_proc;

	if( pinfo->top_pos == -1 || FS_GetStreamTop( pinfo->file, pinfo->top_pos ) == -1 ) {
		// ムービーがない
		return -1;
	}

   work->x = pinfo->x;
	work->y = pinfo->y;

#if 1
	if( pinfo->repeat ) {
		work->repeat_flag = 1;
	} else {
		work->repeat_flag = 0;
	}
#endif
	/*
		表示関係
	*/
	{
      DG_DMAPACK *pack;

      //BP: Note this isFullscreenMovie assumption is not correct (think basic action movies), but from a TGS Demo standpoint (which is what this flag is used for it holds true).
      work->movie_handle = BP_Movie_Create(work->x, work->y, work->width, work->height, NULL, OnRenderVideoCallback, 1/*isFullscreenMovie*/, NULL /*pMemoryStream*/, 0 /*memorySize*/);
		pack = &( work->dmapack );
		pack->flag = DG_DMAPACK_MENU;
		pack->phase = DG_DMAPACK_PHASE_AFTER;
		pack->priority = pinfo->priority;
      pack->autopacket = &work->draw;

		DG_QueueDmapack( pack );
   	make_draw_packet( work, &work->draw );
	}

   work->force_finish_stream = 0;
   work->has_started_movie = 0;

	work->framenum = 0;
	work->end_flag = 0;
	work->file_th_id = -1;
	work->file = pinfo->file;
   work->priority = pinfo->priority;

	if( work->repeat_flag == 0 ){
	
		DG_UnDrawFrameCount = DG_UNDRAW_MAX;
		GM_SetGameStatus(STATE_DEMO);
	} 

	work->pad_cancel = pinfo->pad_cancel;	
	GM_StreamAddDriver( &driver, CHUNK_TYPE_MOVIE );

	ASSERT( pCurrentActor == NULL );
	pCurrentActor = work;

	work->bReqCancel = FALSE;

   BP_ConsoleScreenSaverSuspend();        //BP JG - prevent the screen saver from activating during the next game sequence


	return 0;
}

void *NewMpegPssMovieStr( int name, int map )
{
	Work *work;

	work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) );

	if( work != NULL ){
		REQUEST_INFO info;

		GV_SetActor( &( work->actor ), Act, Die );
		GetGclOpions( &info );

		if( GetResources( work, &info ) < 0 ){
			GV_DestroyActor( work );
			pCurrentActor = work;
			return work;
		}
	}
	return work;
}


// プログラム呼出用

static void *NewMpegPssMovieStrProg( REQUEST_INFO* pReqinfo  )
{
	Work *work;

	work = ( Work * )GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) );

	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );

		if( GetResources( work, pReqinfo ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}

// Movie をリクエストする 最後にリクエストしたものが有効
// 		top_pos        : ストリームデータの位置
//		width,height   : ムービーの幅、高さ
//		x,y	           : ムービーの表示位置	
//		repeat         : 真なら繰返し再生する			  
//		pad_cancel     : パッドでキャンセル可能ならそのボタンを入れる		
void RequestMovieStream( int top_pos, int width, int height,
						 int x, int y, 
						 int repeat, int pad_cancel,
						 int disp_width, int disp_height)
{
	REQUEST_INFO info;

	info.file = FS_FILE_MOVIEVR;

	if( FS_GetStreamTop( info.file, top_pos ) == -1 ) {
		// ムービーがない
		return;
	}
	info.top_pos = top_pos;
	info.width = width;
	info.height = height;
	info.x = x;
	info.y = y;
	info.repeat = repeat;
	info.pad_cancel = pad_cancel;
	info.end_proc = 0;
	info.priority = 249;

	info.disp_width = disp_width;
	info.disp_height = disp_height;

	if( pCurrentActor == NULL ) {
		NewMpegPssMovieStrProg( &info );
	} else {
		request_info = info;
	}
}

// Request したものを含めてムービーをキャンセルする
void CancelMovieStream( void )
{
	if( pCurrentActor != NULL ) {
		pCurrentActor->bReqCancel = TRUE;
	}
	request_info.top_pos = -1;
}

int GetMovieStreamTime()
{
   if( pCurrentActor && pCurrentActor->ctrl )
   {
      return pCurrentActor->ctrl->tick;
   }
   
   return -1;
}

void MovieStream_KillCreditsMoviePlayer()
{
   if( pCurrentActor && pCurrentActor->movie_handle )
   {
      BP_Movie_Destroy(pCurrentActor->movie_handle);
      pCurrentActor->movie_handle = 0;

      pCurrentActor->force_finish_stream = 1;
   }
}

void BP_mpegstr_BSS_Init()
{
   work_ptr = NULL;
}
