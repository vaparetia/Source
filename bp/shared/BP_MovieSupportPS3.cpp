//----------------------------------------------------------------------------
// BP_MovieSupportPS3.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Backend/PS3/PS3CTexture.h"

#include "BP_MovieSupport.h"
#include "BP_MovieSupportPS3.h"

#include "BP_RendererDebug.h"
#include "BP_Memory.h"

#include <sys/sys_time.h>
#include <sys/time_util.h>
#include <cell/atomic.h>

#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

//----------------------------------------------------------------------------

#define VALIDATE_RETURN(RET, STRING) \
   if(RET != CELL_OK) \
   printf("Error: %s (Ret: %d)\n", #STRING, RET);\
   BPE_VERIFY( CELL_OK == ret, false, #STRING )

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------

static const int           skStreamBufSize   = 1024 * 1024; // Size of movie internal streaming buffer
static const unsigned int  skPicBufSize      = 256 * 1024;  // Store for AU buffers for decoder

static const unsigned int  skSizeStreamData  = 256 * 1024;  // Stream grab granularity size

static int const           kMaxFrameWidth    = 1280;        // Maximum movie width
static int const           kMaxFrameHeight   = 720;         // Maximum movie height

// 0 -> Reserved (NULL HANDLE)
// 1 -> Slot for movie player
static int const     kMaxMoviePlayers = 2;

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

unsigned char*                   gStreamBuffer = NULL;
unsigned char*                   gPictureBuffer = NULL;
unsigned char*                   gDecodeBuffer = NULL;

static CellVdecTypeEx            gDecType;
static CellVdecAttr              gDecAttr;

static CMoviePlayer* gMoviePlayers[kMaxMoviePlayers];

CellSpurs* g_SpursForMovie;

//----------------------------------------------------------------------------

extern int gBP_Movie_ShowStats;

//----------------------------------------------------------------------------

static BP_Movie_Handle const Find_Free_MoviePlayer()
{
   // movie player index 0 is reserved for a NULL handle.
   for( int i = 1; i < kMaxMoviePlayers; ++i )
   {
      if( gMoviePlayers[i] == NULL )
         return (BP_Movie_Handle)i;
   }
   
   return (BP_Movie_Handle)0;
}

//----------------------------------------------------------------------------

#define USE_AVC_DEC() 0

// Find the next AU in the input stream.
static unsigned int get_access_unit(unsigned char *in_addr, unsigned int es_size, unsigned char **au_addr, unsigned int *au_length)
{
   bool detect = false;
   unsigned int start_point = 0, read_length = 0;

   *au_addr = NULL;

   while(true)
   {
      unsigned char *parse_head = in_addr + read_length;

      if( es_size < (read_length + 4) ) 
         return 0;

      // Find beginning of mpeg packet (always starts with 2x 0 byte followed by a 1 byte.
      if( parse_head[0] || parse_head[1] || (1 != parse_head[2]) )
      {
         read_length++; 
         continue;
      }

      if ( 
#if USE_AVC_DEC()
         (0x00 < (parse_head[3] & 0x1f)) &&
         (0x0a > (parse_head[3] & 0x1f))
#else
         (0xb8 == parse_head[3]) || //	group start code.
         (0xb7 == parse_head[3]) || //	sequence end code.
         (0x00 == parse_head[3]) || //	picture start code.
         (0xb3 == parse_head[3])    //	sequence start code.
#endif
         )
      {
         if( detect )
         {
#if USE_AVC_DEC()
            if( 0x06 > (parse_head[3] & 0x1f) )
               detect = false;
#else
            if( 0xb7 == parse_head[3] )
               read_length += 4; //	sequence end code.

            *au_length = read_length - start_point;
            return read_length;
#endif
         }
         else
         {
#if USE_AVC_DEC()
            /*E search SPS, PPS, SEI, AUD. */
            if( 0x05 < (parse_head[3]&0x1f) )
            {
               detect = true;/*E goto search VCL. */

               if( NULL == *au_addr )
               {
                  start_point = read_length;
                  *au_addr = in_addr + start_point;
               }
               else
               {
                  *au_length = read_length - start_point;
                  return read_length;
               }
            }
            //E first mb in picture
            else if( parse_head[4]&0x80 )
            {
               /*E this is not BD-ROM or PAMF pattern. */
               if( NULL == *au_addr )
               {
                  start_point = read_length;
                  *au_addr = in_addr + start_point;
               }
               else
               {
                  *au_length = read_length - start_point;
                  return read_length;
               }
            }
#else
            if( 0x00 == parse_head[3] ) 
               detect = true;	//	picture start code.

            if(!*au_addr)
            {
               start_point = read_length;
               *au_addr = in_addr + start_point;
            }
#endif
         }
      }
      read_length += 4;
   }
   return 0;
}

//----------------------------------------------------------------------------

CMoviePlayer::CMoviePlayer(BP_Movie_Render_Callback readCallback, char* pMemoryStream)
: mFullscreen(0)
, mMPEGState( kMPEGState_Waiting )
, mpTexture( NULL )
, mpQueuedTexture( NULL )
, mReadCallback(readCallback)
, mpMemoryStream(pMemoryStream)
, mVdecHandle(NULL)
, mPicCnt(0)
, mAuDone(0)
, mIsEOF(false)
, mBytesRead(0)
, mCurrStreamPos(0)
, mFrameStreamPos(0)
, mMostRecentBytesRead(0)
, mPicCurr(0)
, mPicFree(0)
, mNumFrame(0)
, mTotal(0)
, mCurrFrame(0)
, mPrevFrame(0)
, mVideoTime(0.0f)
, mFrameRate(0.0f)
, mbDone(false)
, mNumDrawnFrame(0)
, mError( 0 )
, mbStart( false )
{
   memset( mFrames, 0, sizeof( mFrames ) );
}

//----------------------------------------------------------------------------

CMoviePlayer::~CMoviePlayer()
{
   CloseMPEG();

   if( mpTexture )
      CTexture::DeleteTexture(mpTexture);

   if(mpQueuedTexture)
      CTexture::DeleteTexture(mpQueuedTexture);
}

//----------------------------------------------------------------------------

bool CMoviePlayer::ContinueReading()
{
   bool didReadData = false;

   mIsEOF = false;

   //Consumed more than "skSizeStreamData" worth of streamed data?
   if (mFrameStreamPos > skSizeStreamData)
   {
      // Yes, so copy across as much data as we've got buffered at the moment.
      memcpy(gStreamBuffer, gStreamBuffer + skSizeStreamData, mCurrStreamPos - skSizeStreamData);
      //update write positions of streaming.
      mCurrStreamPos -= skSizeStreamData;
      mFrameStreamPos -= skSizeStreamData;
   }

   if (mCurrStreamPos <= (skStreamBufSize - skSizeStreamData) )
   {
      int const kMaxAssumedBlockSize = 64 * 1024;
      
      int totalRead = 0;
      while( (totalRead + kMaxAssumedBlockSize) <= skSizeStreamData )
      {
         int bytesRead = 0;
         mReadCallback(gStreamBuffer + mCurrStreamPos, &bytesRead);
         
         BPE_VERIFY(bytesRead <= kMaxAssumedBlockSize, false, "Read callback returned too much data!");

         // No more data for now!
         if( bytesRead == 0 )
            break;

         // EOF
         if( bytesRead == -1 )
         {
            if( gBP_Movie_ShowStats )
               printf("MPEG[%d]: EOF (Buffer now contains %d bytes of data)\n", gpRenderBackend->GetFrameCount(), mCurrStreamPos - mFrameStreamPos);

            mIsEOF = true;
            break;
         }

         didReadData = true;

         mCurrStreamPos += bytesRead;
         mBytesRead += bytesRead;
         totalRead += bytesRead;
      }
   }

   return didReadData;
}

//----------------------------------------------------------------------------

void CMoviePlayer::ContinueDecoding()
{
   unsigned char *au_start;
   unsigned int au_length, length;
   while ((length = get_access_unit(gStreamBuffer + mFrameStreamPos, mCurrStreamPos - mFrameStreamPos, &au_start, &au_length)))
   {
      BPE_VERIFY(au_length <= skPicBufSize, false, "Picture too big for picture buffer");

      // This is the oldest frame that is currently being decoded.
      int const oldestDecodingFrame = cellAtomicNop32((uint32_t*)&mAuDone);
      int const areFramesBeingDecoded = oldestDecodingFrame < mNumFrame;

      // Does pic overrun current buffer AND does wrapping means we're not reading into still used space.
      if( mPicCurr + au_length > skPicBufSize )
      {
         // Check if there are actually any frames currently decoding?
         // If the oldest decoding frame counter matches our CURRENT frame, then there are NONE in progress.
         if( areFramesBeingDecoded )
         {
            int const oldestFrameStartOffset = mFrames[oldestDecodingFrame % kNumFrames].mStart - gPictureBuffer;

            if(gBP_Movie_ShowStats)
               printf("MPEG[%d]: XXX %d + %d > %d (earliest start ptr: %d at index %d)\n", gpRenderBackend->GetFrameCount(), mPicCurr, au_length, skPicBufSize, oldestFrameStartOffset, oldestDecodingFrame);
            
            // Does wrapping around cause the new frame to fit in the real free space (excluding region of buffer used by frames currently in flight)
            if( (0 + au_length) < oldestFrameStartOffset )
            {
               mPicCurr = 0;
            }
            else
            {
               // Not ready to decode this frame because there is no space in the buffer.
               break;
            }
         }
         else
         {
            // No frames in flight
            if(gBP_Movie_ShowStats)
               printf("MPEG[%d]: XXX %d + %d > %d\n", gpRenderBackend->GetFrameCount(), mPicCurr, au_length, skPicBufSize);
            mPicCurr = 0;
         }
      }

      // Space for another decoded pic?
      if( !areFramesBeingDecoded || (mPicCurr >= mPicFree && mPicCurr < skPicBufSize) || (mPicCurr < mPicFree && mPicCurr + au_length < mPicFree) )
      {
         if(gBP_Movie_ShowStats)
            printf("MPEG[%d]: YYY (%d >= %d && %d < %d) || (%d < %d && %d + %d < %d)\n", gpRenderBackend->GetFrameCount(), mPicCurr, mPicFree, mPicCurr, skPicBufSize, mPicCurr, mPicFree, mPicCurr, au_length, mPicFree);

         //Copy across the access unit
         memcpy(gPictureBuffer + mPicCurr, au_start, au_length);

         //record the position and size of au in the pic buffer
         mFrames[mNumFrame%kNumFrames].mStart = gPictureBuffer + mPicCurr;
         mFrames[mNumFrame%kNumFrames].mLength = au_length;
         mFrames[mNumFrame%kNumFrames].mEnd = mPicCurr + au_length;

         CellVdecAuInfo au_info;
         CellVdecDecodeMode mode;
         au_info.size      = au_length;
         au_info.startAddr = gPictureBuffer + mPicCurr;

         au_info.userData          = 0;
         au_info.codecSpecificData = 0;
         au_info.pts.lower         = CELL_VDEC_PTS_INVALID;
         au_info.pts.upper         = CELL_VDEC_PTS_INVALID;
         au_info.dts.lower         = CELL_VDEC_DTS_INVALID;
         au_info.dts.upper         = CELL_VDEC_DTS_INVALID;
         mode = CELL_VDEC_DEC_MODE_NORMAL;

         int ret = cellVdecDecodeAu( mVdecHandle, mode, &au_info );
         if( CELL_OK != ret )
         {
            if( CELL_VDEC_ERROR_BUSY == ret )
            {
               //printf("cellVdecDecodeAu busy\n");
               break;
            }
            else
            {
               break;
            }
         }
         else
         {
            if( gBP_Movie_ShowStats )
            {
               //printf("MPEG[%d]: Decoded %d bytes (Buffer now contains %d bytes of data)\n", gpRenderBackend->GetFrameCount(), length, mCurrStreamPos - mFrameStreamPos);
               printf("MPEG[%d]: Queued Frame[%d] for Decode: offset: %d size: %d end: %d\n", gpRenderBackend->GetFrameCount(), mNumFrame, mPicCurr, au_info.size, mFrames[mNumFrame%kNumFrames].mEnd);
            }

            mPicCurr += au_length;
            mNumFrame++;
            mFrameStreamPos += length;
            mTotal += length;
         }
      }
      else
      {
         break;
      }

      //only one decode per frame allowed to avoid hitches.
      break;

   }//while (AU's to decode)

   if( gBP_Movie_ShowStats )
   {
      int bytesInBuffer = mCurrStreamPos - mFrameStreamPos;

      BP_DebugText_Print("MPEG: Buffer contains %d bytes (%d%%)", bytesInBuffer, (mCurrStreamPos - mFrameStreamPos) * 100 / skStreamBufSize);
      
      if( bytesInBuffer > 0 )
         printf("MPEG[%d]: Buffer contains %d bytes (%d%%)\n", gpRenderBackend->GetFrameCount(), bytesInBuffer, (mCurrStreamPos - mFrameStreamPos) * 100 / skStreamBufSize);
   }
}

//----------------------------------------------------------------------------

unsigned int CMoviePlayer::vdec_callback_static(CellVdecHandle handle, CellVdecMsgType msg_type, int err_code, void * pParam)
{
   CMoviePlayer * const pPlayer = (CMoviePlayer*)( pParam );
   unsigned int ret = pPlayer->VdecCallback( handle, msg_type, err_code );
   return ret;
}

//----------------------------------------------------------------------------

unsigned int CMoviePlayer::VdecCallback(CellVdecHandle handle, CellVdecMsgType msg_type, int err_code)
{
   switch( msg_type )
   {
   case CELL_VDEC_MSG_TYPE_AUDONE:
      {
         mPicFree = mFrames[mAuDone%kNumFrames].mEnd;
         mAuDone++;
      }
      break;

   case CELL_VDEC_MSG_TYPE_PICOUT:
      {
         mPicCnt++;
      }
      break;

   case CELL_VDEC_MSG_TYPE_SEQDONE:
      {
         mbDone = true;
      }
      break;

   case CELL_VDEC_MSG_TYPE_ERROR:
      {
         mError = err_code;
         break;
      }
      break;

   default:
      {
         mError = 42;
      }
      break;
   }
   return 0;
}

//----------------------------------------------------------------------------

void CMoviePlayer::StartMPEG()
{
   switch (mMPEGState) 
   {
   case kMPEGState_Waiting:
      {
         mPicFree = skPicBufSize;
         mMPEGState = kMPEGState_Opening;
      }

   case kMPEGState_Opening:
   case kMPEGState_Open:
   case kMPEGState_Buffering:
   case kMPEGState_Buffered:
   case kMPEGState_Ready:
   case kMPEGState_Playing:
   case kMPEGState_Stopped:
      {
         mbStart = true;
      }
      break;

   case kMPEGState_Closed:
   default:
      printf("MPEG: Can't START movie in state: %d\n", mMPEGState);
      BPE_VERIFYA(false, "Can't start mpeg in current state");
      break;
   }
}

//----------------------------------------------------------------------------

void CMoviePlayer::CloseMPEG()
{
   int ret;

   if( mVdecHandle )
   {
      ret = cellVdecEndSeq( mVdecHandle );
      VALIDATE_RETURN(ret, "cellVdecEndSeq failed");

      //Stop video
      ret = cellVdecClose(mVdecHandle);
      VALIDATE_RETURN(ret, "cellVdecClose failed");
   }

   //Put it in a permanent done state.  Must reconstruct the class to play another movie.
   mMPEGState = kMPEGState_Closed;
}

//----------------------------------------------------------------------------

void CMoviePlayer::Update(int const shouldDraw)
{
	switch (mMPEGState)
   {
   case kMPEGState_Waiting:
      break;

	case kMPEGState_Opening:
		{
         mMPEGState = kMPEGState_Open;
		}
	case kMPEGState_Open:
		{
			mMPEGState = kMPEGState_Buffered;
		}
	case kMPEGState_Buffering:
      {
         mMPEGState = kMPEGState_Buffered;
      }
	case kMPEGState_Buffered:
		{
         // BP - Set up decoder
         // File is open, now initialize decoder:

         const int32_t vdec_spu_prio = 100;
         const int32_t vdec_ppu_prio = 1000;
         const size_t lib_stack_size = 8192;
         const uint32_t vdec_spu_num = 2;        // Number of SPUs to use for Decoding.

         CellVdecResourceEx resource;
         CellVdecResourceSpurs resSpurs;
         resource.memAddr = gDecodeBuffer;
         resource.numOfSpus = vdec_spu_num;
         resource.memSize = gDecAttr.memSize;
         resource.ppuThreadPriority = vdec_ppu_prio;
         resource.spuThreadPriority = vdec_spu_prio;
         resource.ppuThreadStackSize = lib_stack_size;
         resource.spursResource = &resSpurs;
         resSpurs.spursAddr = g_SpursForMovie;
         
         for (int i = 0; i < 8; i++)
            resSpurs.tasksetPriority[i] = 1;

         resSpurs.tasksetMaxContention = vdec_spu_num;

         CellVdecCb reg_cb;
         reg_cb.cbFunc = vdec_callback_static;
         reg_cb.cbArg = this;

         int ret = cellVdecOpenEx (&gDecType, &resource, &reg_cb, &mVdecHandle);
         VALIDATE_RETURN(ret, "cellVdecOpenEx failed");

         ret = cellVdecStartSeq( mVdecHandle );
         VALIDATE_RETURN(ret, "cellVdecStartSeq failed");

         mMPEGState = kMPEGState_Ready;
		}

	case kMPEGState_Ready:
		{
         ContinueReading();
         ContinueDecoding();

		   if (mbStart)
		   {
            //Game has triggered fmv start.  Advance state to kMPEGPlaying.
			   mMPEGState = kMPEGState_Playing;
		   }
         else
         {
            break;
         }
      }

	case kMPEGState_Playing:
		{
         ContinueReading();

         // Begin decoding all AUs that we've loaded from file:
         ContinueDecoding();

         // NOTE: While theoretically it would be the right thing not to actually decode frames that can't be seen (due to rendering being disabled) 
         // this causes a movie not to properly finish playing if is still has data to flush (i.e. one single frame of data) and the rendering gets disabled due 
         // to the end of the "stream" being detected by the demo stream playback.
         // In this case the stream waits for it to finish, but the movie decoding object never detects it's own "end of stream" state.
         // This was the case in w11a when accessing the first node terminal (MGS2)
         // The cinematic ended exactly one frame too early before the movie causing it to stall waiting for the movie to finish.
#if 0    
         // Only read and decode data, don't actually create textures while we shouldn't be rendering!
         if( !shouldDraw )
            break;
#endif

         if(mPicCnt == 0)
         {
            // BP - nothing more to do on this cycle.
            break;
         }

         if(mIsEOF && mNumDrawnFrame == mPicCnt)
         {
            mPicCnt = 0;
            mNumDrawnFrame = 0;
            mbStart = false;
            mMPEGState = kMPEGState_Stopped;
            break;
         }

         // Figure out which frame(s) we want to display.
         mPrevFrame = mCurrFrame;

         int wantFrame = (int) (mVideoTime * mFrameRate);
         mCurrFrame = wantFrame;

         // Render all full frames that have been decoded and we want to display:

         while( mNumDrawnFrame < mPicCnt && mpQueuedTexture == NULL )
         {
            const CellVdecPicItem *pi;
            // Get the oldest decoded frame.
            int ret = cellVdecGetPicItem( mVdecHandle, &pi );
            if( CELL_OK != ret )
            {
               // XXX This appears to happen every other playback - very strange ...
               printf("cellVdecGetPicItem failed ... 0x%x\n", ret );
            }
            else
            {
               if (pi->status)
               {
                  printf("MPEG Decode error 0x%08X on frame %d\n", pi->status, mNumDrawnFrame);
#if MARCO
                  BPE_VERIFY(false, false, "MPEG Decode error");
#endif
               }

#if USE_AVC_DEC()
               CellVdecAvcInfo* picInfo = (CellVdecAvcInfo*)pi->picInfo;
               int const width = picInfo->horizontalSize;
               int const height = picInfo->verticalSize;
               int const frameRateCode = picInfo->frameRateCode;

//                CELL_VDEC_AVC_FRC_24000DIV1001            = 0x00,
//                CELL_VDEC_AVC_FRC_24                      = 0x01,
//                CELL_VDEC_AVC_FRC_25                      = 0x02,
//                CELL_VDEC_AVC_FRC_30000DIV1001            = 0x03,
//                CELL_VDEC_AVC_FRC_30                      = 0x04,
//                CELL_VDEC_AVC_FRC_50                      = 0x05,
//                CELL_VDEC_AVC_FRC_60000DIV1001            = 0x06,
//                CELL_VDEC_AVC_FRC_60                      = 0x07,


#else
               CellVdecMpeg2Info *picInfo = (CellVdecMpeg2Info *) pi->picInfo;
               int const width = picInfo->horizontal_size;
               int const height = picInfo->vertical_size;
               int const frameRateCode = picInfo->frame_rate_code - 1;

//                CELL_VDEC_MPEG2_FRC_FORBIDDEN               = 0x00,
//                CELL_VDEC_MPEG2_FRC_24000DIV1001            = 0x01,
//                CELL_VDEC_MPEG2_FRC_24                      = 0x02,
//                CELL_VDEC_MPEG2_FRC_25                      = 0x03,
//                CELL_VDEC_MPEG2_FRC_30000DIV1001            = 0x04,
//                CELL_VDEC_MPEG2_FRC_30                      = 0x05,
//                CELL_VDEC_MPEG2_FRC_50                      = 0x06,
//                CELL_VDEC_MPEG2_FRC_60000DIV1001            = 0x07,
//                CELL_VDEC_MPEG2_FRC_60                      = 0x08,
#endif
               BPE_VERIFY( width <= kMaxFrameWidth && height <= kMaxFrameHeight, false, "Unexpected video file");

               static float frameRate[] =
               {
                  24.0F / 1.001F,
                  24.0F,         
                  25.0F,         
                  30.0F / 1.001F,
                  30.0F,         
                  50.0F,         
                  60.0F / 1.001F,
                  60.0F,         
               };
               
               float currentFrameRate = frameRate[frameRateCode];
               if( mFrameRate != currentFrameRate )
               {
                  mFrameRate = currentFrameRate;
                  printf("MPEG: Frame Rate: %f\n", mFrameRate);
               }

               // Copy out the decoded picture.
               CellVdecPicFormat pic_format;
               pic_format.alpha = 255;
               pic_format.formatType = CELL_VDEC_PICFMT_RGBA32_ILV;
               pic_format.colorMatrixType = CELL_VDEC_COLOR_MATRIX_TYPE_BT709;

               mpQueuedTexture = (CTexture*)CBaseTexture::CreateTexture(width, height, 1, CBaseTexture::kFormat_A8R8G8B8 );

               mpQueuedTexture->mTexture.remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 | CELL_GCM_TEXTURE_REMAP_REMAP << 12 | CELL_GCM_TEXTURE_REMAP_REMAP << 10 | CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
                                                 CELL_GCM_TEXTURE_REMAP_FROM_G << 6 | CELL_GCM_TEXTURE_REMAP_FROM_R << 4 | CELL_GCM_TEXTURE_REMAP_FROM_A << 2 | CELL_GCM_TEXTURE_REMAP_FROM_B;

               void* pTexureBuffer;
               int pTextureBufferPitch = 0;
               mpQueuedTexture->Lock(&pTexureBuffer, &pTextureBufferPitch);
               
               ret = cellVdecGetPicture( mVdecHandle, &pic_format, pTexureBuffer);

               mpQueuedTexture->Unlock();

               if( CELL_OK == ret )
               {
               }
               else
               {
                  printf("cellVdecGetPicture failed ... 0x%x\n", ret );
                  break;
               }
            }

            //One per video frame.
            break;

         }//if(can decode a frame)

         if( gBP_Movie_ShowStats )
            printf("MPEG[%d]: PicCnt: %d NumDrawn: %d CurrFrame: %d\n", gpRenderBackend->GetFrameCount(), mPicCnt, mNumDrawnFrame, mCurrFrame );

         if(mNumDrawnFrame <= mCurrFrame && mpQueuedTexture != NULL )
         {
            //Have a decoded texture and we want to immediately display it.
            if( mpTexture )
               CTexture::DeleteTexture(mpTexture);

            mpTexture = mpQueuedTexture;
            mpQueuedTexture = NULL;
            
            ++mNumDrawnFrame;

            mDGTex.BP_TextureHandle = (unsigned int)mpTexture;
         }
         else
         {
            //Not yet time or don't yet have one.
            //Hold it for the next frame.
         }

         // playback done
         if( mbDone )
         {
            CloseMPEG();
            break;
         }
		}
      break;

   case kMPEGState_Stopped:
      {
         ContinueReading();
         ContinueDecoding();

         if( mbStart )
         {
            mMPEGState = kMPEGState_Playing;
         }
      }
      break;

   case kMPEGState_Closed:
      break;

	default:
      BPE_VERIFY( false, false, ("TODO") );
		break;
	}
}

//----------------------------------------------------------------------------

void BP_Movie_Init()
{
   int ret;

   memset(gMoviePlayers, 0, sizeof(gMoviePlayers));

#if USE_AVC_DEC()
   static CellVdecAvcSpecificInfo sInfo;
   sInfo.thisSize = sizeof(CellVdecAvcSpecificInfo);
   sInfo.maxDecodedFrameWidth = kMaxFrameWidth;
   sInfo.maxDecodedFrameHeight = kMaxFrameHeight;
   sInfo.disableDeblockingFilter = false;
   sInfo.numberOfDecodedFrameBuffer = 3; //The value of max_dec_frame_buffering never exceeds 3 for PAMF streams


   //All video files are mpeg2
   gDecType.codecType = CELL_VDEC_CODEC_TYPE_AVC;
   gDecType.profileLevel = CELL_VDEC_AVC_LEVEL_3P1;
#else
   static CellVdecMpeg2SpecificInfo sInfo;
   sInfo.thisSize = sizeof(CellVdecMpeg2SpecificInfo);
   sInfo.maxDecodedFrameWidth = kMaxFrameWidth;
   sInfo.maxDecodedFrameHeight = kMaxFrameHeight;

   //All video files are mpeg2
   gDecType.codecType = CELL_VDEC_CODEC_TYPE_MPEG2;
   gDecType.profileLevel = CELL_VDEC_MPEG2_MP_HL;
#endif

   gDecType.codecSpecificInfo = &sInfo;

   ret = cellVdecQueryAttrEx (&gDecType, &gDecAttr);
   VALIDATE_RETURN(ret, "Failed to get codec info");

   const int decodeBufSize = gDecAttr.memSize;

   //Allocate memory for video streaming
   gStreamBuffer = (unsigned char*)BP_Memory_Calloc( skStreamBufSize, 64, kMT_Permanent, kMC_Movie );
   gDecodeBuffer = (unsigned char*)BP_Memory_Calloc( decodeBufSize, 64, kMT_Permanent, kMC_Movie );
   gPictureBuffer = (unsigned char*)BP_Memory_Calloc( skPicBufSize, 64, kMT_Permanent, kMC_Movie );
}

//----------------------------------------------------------------------------

void BP_Movie_Shutdown()
{
   // Free all movie memory
   BP_Memory_Free( gStreamBuffer );   
   BP_Memory_Free( gDecodeBuffer );   
   BP_Memory_Free( gPictureBuffer );   

   // Clear pointers
   gStreamBuffer = NULL;   
   gDecodeBuffer = NULL;   
   gPictureBuffer = NULL;   
}

//----------------------------------------------------------------------------

BP_Movie_Handle BP_Movie_Create(int x, int y, int width, int height, DG_TEX_TRANS* pTexTrans, BP_Movie_Render_Callback renderCallback, int isFullscreenMovie, char* pMemoryStream, int memorySize)
{
   // Override passed in flag since it's wrong for MGS3 special/action movies etc?!
   // This simple method correctly detects full screen movies - see google doc "CP4 Movie Stats" for movie sizes.
   isFullscreenMovie = (width >= 512) && (height >= 320);

#if BP_TGS_DEMO()

   if(isFullscreenMovie)
      return 0;
#endif

   BP_Movie_Handle handle = Find_Free_MoviePlayer();

   if( handle != 0 )
   {
      CMoviePlayer* pPlayer = new CMoviePlayer(renderCallback, pMemoryStream);
      gMoviePlayers[handle] = pPlayer;

      pPlayer->mFullscreen = isFullscreenMovie;

      memset(&pPlayer->mDGTex, 0, sizeof(DG_TEX));

      pPlayer->mDGTex.u_offset = 0;
      pPlayer->mDGTex.v_offset = 0;
      pPlayer->mDGTex.u_scale = 1;
      pPlayer->mDGTex.v_scale = 1;
      pPlayer->mDGTex.tex_id = 0;
      pPlayer->mDGTex.tri_id = 0;
#if MGS_VERSION == 2
      pPlayer->mDGTex.BP_flag = DG_TEXFLAG_UV_CLAMP;
#else
      pPlayer->mDGTex.BP_flag = 0;
#endif

#if MGS_VERSION == 3
      if(pTexTrans)
      {
         memcpy(&pPlayer->mDGTex.tex_trans, pTexTrans, sizeof(*pTexTrans));
         int fullTexWidth, fullTexHeight, offsetX, offsetY;
         DG_GetTexelInfo(&fullTexWidth, &fullTexHeight, &offsetX, &offsetY, &pPlayer->mDGTex);
         pPlayer->mDGTex.u_scale = (real32)width / fullTexWidth;
         pPlayer->mDGTex.v_scale = (real32)height / fullTexHeight;
      }
      else
      {
         memset(&pPlayer->mDGTex, 0, sizeof(pPlayer->mDGTex));
         pPlayer->mDGTex.u_scale = 1.0f;
         pPlayer->mDGTex.v_scale = 1.0f;
      }
#endif

      pPlayer->mDGTex.BP_TextureHandle = (unsigned int)&gpRenderBackend->GetBlackMap();
   }

   return handle;
}

//----------------------------------------------------------------------------

void BP_Movie_ResetTextureToBlack(BP_Movie_Handle hMovie)
{
   // Ignore time that it wants us to be at.
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      DG_TEX* pTex = &pPlayer->mDGTex;
      pTex->BP_TextureHandle = (unsigned int)&gpRenderBackend->GetBlackMap();
   }
}

//----------------------------------------------------------------------------

void BP_Movie_Destroy(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   // Destroy movie player
   if( pPlayer )
   {
      delete pPlayer;
   }

   gMoviePlayers[hMovie] = NULL;
}

//----------------------------------------------------------------------------

void BP_Movie_RenderActiveMovies(int const shouldDraw)
{
   // While rendering is disabled prebuffering COULD occur.
   int moviePlayerCount = 0;

   // movie player index 0 is reserved for a NULL handle.
   for( int i = 1; i < kMaxMoviePlayers; ++i )
   {
      CMoviePlayer* pPlayer = gMoviePlayers[i];

      if( pPlayer )
      {
         pPlayer->Update(shouldDraw);
         ++moviePlayerCount;
      }
   }

   if( gBP_Movie_ShowStats )
      BP_DebugText_Print("Active Movie Players: %d", moviePlayerCount);
}

//----------------------------------------------------------------------------

void BP_Movie_Set_Time(BP_Movie_Handle hMovie, float time)
{
   // Ignore time that it wants us to be at.
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      pPlayer->mVideoTime = bpe::max_val(time, 0.0f); //Ensure caller does not pass in a negative time
   }
}

//----------------------------------------------------------------------------

float BP_Movie_Get_Frame_Time(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      if( pPlayer->mFrameRate > 0.0f )
         return 1.0f / pPlayer->mFrameRate;
   }

   return 1.0f / 30.0f;
}

//----------------------------------------------------------------------------

void BP_Movie_Start(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      pPlayer->StartMPEG();
   }
}

//----------------------------------------------------------------------------

void BP_Movie_Stop(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      switch(pPlayer->mMPEGState)
      {
      case CMoviePlayer::kMPEGState_Waiting:
      case CMoviePlayer::kMPEGState_Opening:
      case CMoviePlayer::kMPEGState_Open:
      case CMoviePlayer::kMPEGState_Buffering:
      case CMoviePlayer::kMPEGState_Buffered:
      case CMoviePlayer::kMPEGState_Ready:
         // Stop us from automatically transitioning into kMPEGState_Playing state
         pPlayer->mbStart = false;
         break;

      case CMoviePlayer::kMPEGState_Playing:
         // Prevent us from transitioning into kMPEGState_Playing state
         // Pull player out of playing state and put in kMPEGState_Ready state
         pPlayer->mbStart = false;
         pPlayer->mMPEGState = CMoviePlayer::kMPEGState_Ready;
         break;

      case CMoviePlayer::kMPEGState_Stopped:
         // Just make sure we don't start playing
         pPlayer->mbStart = false;
         break;

      case CMoviePlayer::kMPEGState_Closed:
         // This case should never technically happen, I feel it is better to silently handle this error condition
         // Prevent us from transitioning into kMPEGState_Playing state if we are ever created
         pPlayer->mbStart = false;
         break;

      default:
         BPE_VERIFYA(false, "Unknown state for stop movie call");
         break;
      }
   }
}

//----------------------------------------------------------------------------

void BP_Movie_Reset(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];
   if( pPlayer )
   {
      {
         int ret;
         if( pPlayer->mVdecHandle )
         {
            //Stop video
            ret = cellVdecClose(pPlayer->mVdecHandle);
            VALIDATE_RETURN(ret, "cellVdecClose failed");

            pPlayer->mVdecHandle = NULL;
         }
      }

      pPlayer->mPicCnt = 0;
      pPlayer->mAuDone = 0;
      pPlayer->mIsEOF = false;
      pPlayer->mBytesRead = 0;
      pPlayer->mCurrStreamPos = 0;
      pPlayer->mFrameStreamPos = 0;
      pPlayer->mMostRecentBytesRead = 0;
      pPlayer->mPicCurr = 0;
      pPlayer->mPicFree = 0;
      pPlayer->mNumFrame = 0;
      pPlayer->mTotal = 0;
      pPlayer->mCurrFrame = 0;
      pPlayer->mPrevFrame = 0;
      pPlayer->mStartTime64 = 0;
      pPlayer->mVideoTime = 0;
      pPlayer->mFrameRate = 0;
      pPlayer->mbDone = false;
      pPlayer->mNumDrawnFrame = 0;
      pPlayer->mError = 0;
      pPlayer->mbStart = false;

      pPlayer->mMPEGState = CMoviePlayer::kMPEGState_Waiting;
   }
}

//----------------------------------------------------------------------------

int BP_Movie_Get_State(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      switch(pPlayer->mMPEGState)
      {
      case CMoviePlayer::kMPEGState_Waiting:
      case CMoviePlayer::kMPEGState_Opening:
      case CMoviePlayer::kMPEGState_Open:
      case CMoviePlayer::kMPEGState_Buffering:
      case CMoviePlayer::kMPEGState_Buffered:
      case CMoviePlayer::kMPEGState_Ready:
         return MOVIE_STATE_READY;

      case CMoviePlayer::kMPEGState_Playing:
         return MOVIE_STATE_PLAYING;

      case CMoviePlayer::kMPEGState_Stopped:
      case CMoviePlayer::kMPEGState_Closed:
         return MOVIE_STATE_STOPPED;
      }
   }
   else
   {
      return MOVIE_STATE_STOPPED;
   }
}

//----------------------------------------------------------------------------

DG_TEX* BP_Movie_Get_Texture(BP_Movie_Handle hMovie)
{
   CMoviePlayer* pPlayer = gMoviePlayers[hMovie];

   if( pPlayer )
   {
      return &pPlayer->mDGTex;
   }

   return NULL;
}

//----------------------------------------------------------------------------

int BP_Movie_IsPlayingFullscreen()
{
   // movie player index 0 is reserved for a NULL handle.
   for( int i = 1; i < kMaxMoviePlayers; ++i )
   {
      CMoviePlayer* pPlayer = gMoviePlayers[i];
      if( ( pPlayer ) && ( pPlayer->mFullscreen ) )
      {
         return 1;
      }
   }

   return 0;
}

//----------------------------------------------------------------------------
