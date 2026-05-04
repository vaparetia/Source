//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ＩＰＵストリーミングドライバ (Streaming driver)

	2001/01/10 K.Takabe
	$Id: ipustream.c,v 1.1.1.3 2002/11/19 11:41:52 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<eekernel.h>
#include	<eeregs.h>
#include	<libgraph.h>
#include	<libipu.h>

#include	"mgs_type.h"
#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libdg.h"
#include	"stream.h"
#include	"g_define.h"
#include	"def_dma.h"

#include	"strctrl.h"

#include "BP_MovieSupport.h"
#include	"ipu.h"


/* ストリームドライバ登録情報 */
void *NewStreamIpuDriver( GM_STREAM_CONTROL *ctrl, int type );
static GM_STREAM_DRIVER driver = {
   /*next:*/ NULL,
   /*type:*/ 0,
	/*driver:*/ NewStreamIpuDriver,
};
extern double BP_GetGlobalGameTime();

/* ---------------------------------------------------------------- */
typedef struct _ipustream_Work {
	GV_ACT_EX	actor;
	int			mode ;			/* 動作モード（0:stream 1:mem） */
	/* 通常ストリーム用ワーク */
	int			type;			/* ストリームタイプ */
	GM_STREAM_CONTROL *ctrl;	/* ストリームコントロール */
	/* オンメモリストリーム用ワーク */
	
	int			state ;			/* 動作ステータス */
	int			flag ;			/* 動作フラグ */
	int			frame ;			/* 現在のフレーム数 */
   
   int         BP_frameStartTickCount;

	
   BP_Movie_Handle movie_handle;
   int movie_width;
   int movie_height;

   // Mem streams
   char* mem_ipu__memlocation;
   int mem_ipu__totalSize;
   int mem_ipu__currentOffset;
   double mem_ipu__startTime;

   int DEBUG_TotalDataRippedOutOfStreamBuffer;
} Work;

static int	DriverInit = 0 ;
static Work	*work_ptr = NULL;
static int MovieWidth = 0;
static int MovieHeight = 0;
static BP_Movie_Handle MovieHandle = 0;

/* ---------------------------------------------------------------- */
enum {
	STATE_READY		= 0x0000,	/* 待機中 */
	STATE_DECODE	= 0x0001,	/* ＩＰＵストリーム展開中 */
	STATE_CONVERT	= 0x0002,	/* ＩＰＵ変換 */
	STATE_SLEEP		= 0x0003,	/* 待機中 */
	STATE_END		= 0x0004,	/* 再生終了 */
} ;

/* ---------------------------------------------------------------- */
static int StreamIsEnd( Work *work )
{
	switch ( work->mode ){
	  case 0:
		return ( GM_IS_STREAM_END( work->ctrl ) );
	  default:
		return ( 0 ) ;
	}
}

static void OnRenderVideoCallback_MemIPU(unsigned char* pBuffer, int* pOutBytesRead)
{
   char *ptr;
   int size = 0;
   Work* pLocalWorkPtr = work_ptr;

   // can't start if there's no object
   if ( (pLocalWorkPtr == NULL) )
   {
      *pOutBytesRead = 0;
      return;
   }

   {
      int size = 65536;
      if( pLocalWorkPtr->mem_ipu__currentOffset >= pLocalWorkPtr->mem_ipu__totalSize )
      {
         *pOutBytesRead = 0;
         return;
      }
      else if( pLocalWorkPtr->mem_ipu__currentOffset + size > pLocalWorkPtr->mem_ipu__totalSize )
      {
         size = pLocalWorkPtr->mem_ipu__totalSize - pLocalWorkPtr->mem_ipu__currentOffset;
      }

      *pOutBytesRead = size;
      ptr = pLocalWorkPtr->mem_ipu__memlocation + pLocalWorkPtr->mem_ipu__currentOffset;
      pLocalWorkPtr->mem_ipu__currentOffset += size;
      pLocalWorkPtr->mem_ipu__currentOffset %= pLocalWorkPtr->mem_ipu__totalSize;
      memcpy(pBuffer, ptr, size);
   }
}


static void OnRenderVideoCallback(unsigned char* pBuffer, int* pOutBytesRead)
{
   char *ptr;
   int size = 0;
   Work* pLocalWorkPtr = work_ptr;

   // can't start if there's no stream YET, it might be a frame behind.
   if( (pLocalWorkPtr == NULL) || (pLocalWorkPtr->ctrl == NULL) || (pLocalWorkPtr->ctrl->stream_h == NULL) )
   {
      *pOutBytesRead = 0;
      return;
   }

   ptr = FS_StreamGetData(pLocalWorkPtr->ctrl->stream_h, pLocalWorkPtr->type);

   if(ptr == NULL)
   {
      int status = FS_StreamGetStatus(pLocalWorkPtr->ctrl->stream_h);
      if (status == FS_STR_STOP ) {
         printf( "READ END\n" );
         *pOutBytesRead = -1;
      }
      else {
         *pOutBytesRead = 0;
      }
   }
   else 
   {
      // Because if(StreamIsEnd(work)) {GV_DestroyActor( work );return;} we must wait until we are in the
      // another state before we totally consume all movie data, if we allow all movie data to be consumed before
      // we have transitioned out of the STATE_READY the actor will be killed upon detecting an empty stream
      // this will leave a orphaned movie player created via GM_StreamIpuDriverInit in a playing state, this could cause
      // very bad things to happen
      if( pLocalWorkPtr->state == STATE_READY )
      {
         // try to get more data
         char* moreDataPtr = FS_StreamGetData(pLocalWorkPtr->ctrl->stream_h, pLocalWorkPtr->type);
         if( moreDataPtr != NULL )
         {
            // If we could get more data put everything is ok
            // put the more data back            
            FS_StreamUngetData(pLocalWorkPtr->ctrl->stream_h, moreDataPtr);
            // let the original data we got go through like normal
            size = FS_STREAM_GET_SIZE(ptr);
            *pOutBytesRead = size;
            memcpy(pBuffer, ptr, size);
            FS_StreamFreeData(pLocalWorkPtr->ctrl->stream_h, ptr);
         }
         else
         {
            // If we could not get any more data put the last data we got back into the buffer   
            // and pretend we have no data
            *pOutBytesRead = 0;
            FS_StreamUngetData(pLocalWorkPtr->ctrl->stream_h, ptr);
         }
      }
      else
      {
         int size = FS_STREAM_GET_SIZE(ptr);
         *pOutBytesRead = size;
         memcpy(pBuffer, ptr, size);
         FS_StreamFreeData(pLocalWorkPtr->ctrl->stream_h, ptr);
      }
   }

   pLocalWorkPtr->DEBUG_TotalDataRippedOutOfStreamBuffer += size;
   FS_StreamSync(pLocalWorkPtr->ctrl->stream_h);
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
   switch (work->state)
   {
   case STATE_READY:
      {
         char* stream_data_ptr = NULL;

         if( work->mem_ipu__memlocation == NULL )
         {
            stream_data_ptr = FS_StreamGetData(work->ctrl->stream_h, work->type);
            if (stream_data_ptr)
            {
               STREAM_TAG* p = (STREAM_TAG*)stream_data_ptr;
               int fileSize = 0;

               fileSize = FS_STREAM_GET_OPTION(p);
               p -= 1; // Rewind to go to info structure
               FS_StreamUngetData(work->ctrl->stream_h, stream_data_ptr);

               work->BP_frameStartTickCount = FS_STREAM_TAG_TIME( p );
               work->state = STATE_DECODE;
               work->frame = 0;

               BP_Movie_Start(work->movie_handle, fileSize);
            }
            else
            {
               if(StreamIsEnd(work)) 
               {
                  GV_DestroyActor( work );
                  return;
               }
            }
         }
         else
         {
            work->mem_ipu__startTime = BP_GetGlobalGameTime();
            work->state = STATE_DECODE;
            work->frame = 0;
            BP_Movie_Start(work->movie_handle, work->mem_ipu__totalSize);
         }
      }
      break;

   case STATE_DECODE:
      {
         int movie_state = BP_Movie_Get_State(work->movie_handle);

         if (movie_state == MOVIE_STATE_PLAYING)
         { 
            float timeAsSecond;
            if( work->mem_ipu__memlocation == NULL )
            {
               int BP_tickCountDelta = work->ctrl->tick - work->BP_frameStartTickCount;
               timeAsSecond = BP_tickCountDelta / 300.0f;
            }
            else
            {
               timeAsSecond = BP_GetGlobalGameTime() - work->mem_ipu__startTime;
            }
            BP_Movie_Set_Time(work->movie_handle, timeAsSecond);
            work->frame++;

            work->state = STATE_CONVERT;
         }
         else if (movie_state == MOVIE_STATE_STOPPED)
         {
             GV_DestroyActor( work );
         }
      }
      break;

   case STATE_CONVERT: 
      work->state = STATE_SLEEP;
      break;

   case STATE_SLEEP:
      break;

   case STATE_END:
      if( work->ctrl == NULL || work->ctrl->state >= GM_STREAM_STATE_READ_END )
          GV_DestroyActor( work );
      break ;
   }
}

static void Die( Work *work )
{
   BP_Movie_Reset(work->movie_handle);
   if( work->mem_ipu__memlocation )
   {
      BP_Movie_Destroy(MovieHandle);
   }
   work->movie_handle = 0;

   if( work == work_ptr )
   {
      work_ptr = NULL;
   }
}

void *NewStreamIpuDriver( GM_STREAM_CONTROL *ctrl, int type )
{
	Work *work = NULL ;

	if( ( work = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		work_ptr = work;
		work->type = type;
		work->ctrl = ctrl;
		work->state = STATE_READY ;
		work->movie_width = MovieWidth;
      work->movie_height = MovieHeight;
      work->movie_handle = MovieHandle;
	}

   return work;
}

	/*
		オンメモリストリームデータを再生
	*/
void *NewMemStreamIpu( void *ipu_data, int mode, BP_Movie_Handle* pBP_Movie_Handle )
{
	Work *work = NULL ;
   char* const pUniqueID = "BP_MEMMPEG";
   int uniqueIDLength = strlen(pUniqueID);
   char* pIPUStream = (char*)ipu_data;

   if (pIPUStream[uniqueIDLength] != NULL || strcmp(pIPUStream, pUniqueID) != 0)
   {
      printf("NewMemStreamIpu reading corrupt BP_MEMMPEG!\n");
      return NULL;
   }
  
	if( ( work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 0xf2 ) ) != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		work_ptr = work ;
		work->mode = 1 ;

		work->mem_ipu__memlocation = ipu_data ;
      {
         work->mem_ipu__memlocation += uniqueIDLength+1; // advance past pUniqueID and NULL character
         // Extract size from the next 4 bytes
         work->mem_ipu__totalSize = *(int*)work->mem_ipu__memlocation;
         BP_LE_SwapSInt_Inp(&work->mem_ipu__totalSize);
         work->mem_ipu__memlocation += sizeof(int);

         //
         MovieWidth = 256;
         MovieHeight = 256;
         MovieHandle = BP_Movie_Create(0, 0, MovieWidth, MovieHeight, NULL, OnRenderVideoCallback_MemIPU, 0 /*isFullscreenMovie*/, work->mem_ipu__memlocation, work->mem_ipu__totalSize);
         *pBP_Movie_Handle = MovieHandle;

         work->movie_width = MovieWidth;
         work->movie_height = MovieHeight;
         work->movie_handle = MovieHandle;
      }

		work->state = STATE_READY ;
	}

	return work;
}
/* ---------------------------------------------------------------------- */
/*
	ドライバ登録 
*/

BP_Movie_Handle GM_StreamIpuDriverInit(int width, int height)
{
   if (DriverInit++) 
      return MovieHandle;

   GM_StreamAddDriver(&driver, CHUNK_TYPE_MOVIE);
   MovieWidth = width;
   MovieHeight = height;
   MovieHandle = BP_Movie_Create(0, 0, MovieWidth, MovieHeight, NULL, OnRenderVideoCallback, 0 /*isFullscreenMovie*/, NULL /*pMemoryStream*/, 0 /*memorySize*/);

   return MovieHandle;
}

/* ＩＰＵストリームドライバ停止 */
void GM_StreamIpuDriverEnd( void )
{
   if (--DriverInit != 0) 
      return ;

   BP_Movie_Destroy(MovieHandle);
   GM_StreamRemoveDriver(&driver);
}

/* ---------------------------------------------------------------------- */
/*
	外部呼び出しインターフェイス
*/
/* ストリームの終了判定 */
int GM_StreamIsEndIpu( void )
{
   if (work_ptr == NULL) 
      return 1;
   else
      return StreamIsEnd(work_ptr);
}

/* ストリームの映像幅・高さ・最大フレーム数を取得 */
int GM_StreamGetIpuInfo( int *width, int *height, int *frame )
{
   if (!work_ptr)
      return 0;

   if (width)
      *width = work_ptr->movie_width;
   if (height)
      *height = work_ptr->movie_height;

   if (*frame)
      *frame = -1;

   return 1;
}
/* ストリームの再生フレーム数取得 */
int GM_StreamGetIpuFrame( void )
{
	/* 終了なら真を返す */
	if (work_ptr == NULL) return 
      (-1);

	if (work_ptr->state == STATE_READY) 
      return -1;

	return work_ptr->frame;
}

/* 純粋な展開後ＩＰＵマクロブロックデータへのポインタ取得 */
void* GM_StreamGetIpu( void )
{
   if (work_ptr == NULL) 
      return NULL;
   else if (work_ptr->state != STATE_SLEEP) 
      return NULL;
   else
      return (void*)&work_ptr->movie_handle;
}

/* 現在のデータを開放して次のデータを取得する */
void GM_StreamFreeIpu( void )
{
   if (work_ptr == NULL) 
      return;
   else if (work_ptr->state != STATE_SLEEP)
      return;

   work_ptr->state = STATE_DECODE;
}

/* 展開後マクロブロックをラスタイメージに変換してコピーする */
void GM_StreamCopyIpuImage( void *dst, int dst_width, int dst_height, int dst_x, int dst_y, int flag )
{	
}

void BP_ipustream_BSS_Init()
{
   DriverInit = 0 ;
   work_ptr = NULL;
   MovieWidth = 0;
   MovieHeight = 0;
   MovieHandle = 0;
}

