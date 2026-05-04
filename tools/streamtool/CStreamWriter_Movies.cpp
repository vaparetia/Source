#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CStreamWriter_Movies.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

const unsigned int kMaxPacketSizeX360 = 65536;
const unsigned int kMaxPacketSizeVita = 65536;
const unsigned int kPreBufferSizeX360 = 65536*6;// Start this out at + half 360 total buffer size

const unsigned int kMaxPacketSizePS3 = 64 * 1024;

const int kMemoryBufferSize = 3*1024*1024; // kMemoryBufferSize >= kMaxPacketSizeX360 && kMemoryBufferSize >= kMaxPacketSizePS3

//----------------------------------------------------------------------------

namespace
{
#define USE_AVC_DEC 0

   static void FindReplacementMovieFiles( std::vector<std::string>& filePaths, const EPlatform curPlatform )
   {
      char acPath[FILENAME_MAX];
      sprintf(acPath, "%s/%s", gOutputFolder, gCurrStreamName);

      for( int i = 0; i < 2; ++i )
      {
         if( i == 1 )
         {
            static const char *kDummyString = "/dummy";

            //Try this path but in the US folder.  We don't keep multiregion copies of converted movies around
            //if they are the same source data as US.
            strcat(acPath, kDummyString);
            BP_ReplaceStreamSourceDataRegion( acPath, acPath, "us" );
            acPath[strlen(acPath) - strlen(kDummyString)] = 0;
         }

         System::String^ srcPath = gcnew System::String(acPath);
         array<System::String^>^ files = nullptr;

         if (curPlatform == kPlatform_X360)
         {
            files = System::IO::Directory::GetFiles(srcPath, "*.wmv", System::IO::SearchOption::AllDirectories);
         }
         else if (curPlatform == kPlatform_PS3 )
         {
#if USE_AVC_DEC
            files = System::IO::Directory::GetFiles(srcPath, "*.bsf", System::IO::SearchOption::AllDirectories);
#else
            files = System::IO::Directory::GetFiles(srcPath, "*.mp2", System::IO::SearchOption::AllDirectories);
#endif
         }
         else if (curPlatform == kPlatform_Vita)
         {
            files = System::IO::Directory::GetFiles(srcPath, "*.mp4", System::IO::SearchOption::AllDirectories);
         }

         if( files && files->Length )
         {
            for(int i = 0; i < files->Length; ++i)
               filePaths.push_back(BP_ConvertManagedString(files[i]));

            // If we found files
            break;
         }
      }
   }

   // Find the next AU in the input stream.
   // Returns how much data was consumed.

   static unsigned int MPEG_Get_Access_Unit(unsigned char *in_addr, unsigned int es_size, unsigned char **au_addr, unsigned int *au_length, float* frameRate)
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
#if USE_AVC_DEC
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

#if 0 //MPEG2 Only?
            // Video Sequence Start Code
            if( parse_head[3] == 0xb3 )
            {
               int const frameRateIndex = parse_head[7] & 0xf;

               static float skFrameRates[] =
               {
                  0.0F,              // CELL_VDEC_MPEG2_FRC_FORBIDDEN            = 0x00,
                  24.0F / 1.001F,     // CELL_VDEC_MPEG2_FRC_24000DIV1001         = 0x01,
                  24.0F,              // CELL_VDEC_MPEG2_FRC_24                   = 0x02,
                  25.0F,              // CELL_VDEC_MPEG2_FRC_25                   = 0x03,
                  30.0F / 1.001F,     // CELL_VDEC_MPEG2_FRC_30000DIV1001         = 0x04,
                  30.0F,              // CELL_VDEC_MPEG2_FRC_30                   = 0x05,
                  50.0F,              // CELL_VDEC_MPEG2_FRC_50                   = 0x06,
                  60.0F / 1.001F,     // CELL_VDEC_MPEG2_FRC_60000DIV1001         = 0x07,
                  60.0F,              // CELL_VDEC_MPEG2_FRC_60                   = 0x08,
               };

               *frameRate = skFrameRates[frameRateIndex];
            }
#endif

            if( detect )
            {
#if USE_AVC_DEC
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
#if USE_AVC_DEC
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

   static AVPacket packet;
   static int currentDecodedBytes;
   static int bytesRemaining=0;
   inline void StartNewMovie()
   {
      // set packet.data to NULL to indicate it
      // doesn't have to be freed
      packet.data=NULL;
      //
      currentDecodedBytes = kPreBufferSizeX360;
      bytesRemaining = 0;
   }

   bool GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, 
      int videoStream, AVFrame *pFrame, std::vector<int>& aBytesPerFrame)
   {
      static uint8_t  *rawData;
      int             bytesDecoded;
      int             frameFinished;

      // Decode packets until we have decoded a complete frame
      while(true)
      {
         // Work on the current packet until we have decoded all of it
         while(bytesRemaining > 0)
         {
            // Decode the next chunk of data
            AVPacket pkt;
            
            av_init_packet(&pkt);
            pkt.data = rawData;
            pkt.size = bytesRemaining;

            bytesDecoded=avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &pkt);

            av_free_packet(&pkt);

            // Was there an error?
            if(bytesDecoded < 0)
            {
               fprintf(stderr, "Error while decoding frame\n");
               return false;
            }
            currentDecodedBytes += bytesDecoded;

            bytesRemaining-=bytesDecoded;
            rawData+=bytesDecoded;

            // Did we finish the current frame? Then we can return
            if(frameFinished)
            {
               aBytesPerFrame.push_back(currentDecodedBytes);
               return true;
            }
         }

         // Read the next packet, skipping all packets that aren't for this
         // stream
         do
         {
            // Free old packet
            if(packet.data!=NULL)
               av_free_packet(&packet);

            // Read new packet
            if(av_read_frame(pFormatCtx, &packet)<0)
               goto loop_exit;
         } while(packet.stream_index!=videoStream);

         bytesRemaining=packet.size;
         rawData=packet.data;
      }

loop_exit:

      // Decode the rest of the last frame
      AVPacket pkt;

      av_init_packet(&pkt);
      pkt.data = rawData;
      pkt.size = bytesRemaining;

      bytesDecoded=avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
         &pkt);

      av_free_packet(&pkt);

      // Free last packet
      if(packet.data!=NULL)
         av_free_packet(&packet);

      currentDecodedBytes += bytesDecoded;

      aBytesPerFrame.push_back(currentDecodedBytes);
      return frameFinished!=0;
   }

   float GetVideoFPS(char const * pFilename)
   {
      float fps;

      AVCodec         *pCodec;

      // Register all formats and codecs
      av_register_all();

      // Open video file
      AVFormatContext *pFormatCtx;
      if(av_open_input_file(&pFormatCtx, pFilename, NULL, 0, NULL) !=0 )
         return 0.0f; // Couldn't open file

      // Retrieve stream information
      if(av_find_stream_info(pFormatCtx)<0)
         return 0.0f; // Couldn't find stream information

      // Find the first video stream
      int videoStream = -1;
      for(int i=0; i<pFormatCtx->nb_streams; i++)
      {
         if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
         {
            videoStream=i;
            break;
         }
      }

      if(videoStream == -1)
         return 0.0f; // Didn't find a video stream

      // Get a pointer to the codec context for the video stream
      AVStream* st = pFormatCtx->streams[videoStream];

      fps = (float)st->r_frame_rate.num / (float)st->r_frame_rate.den;

      // Close the video file
      av_close_input_file(pFormatCtx);

      return fps;
   }

   int GetVideoInformation(char const * pFilename, std::vector<int>& aBytesPerFrame, float& fps, float& durationSec)
   {
      AVFormatContext *pFormatCtx;
      int             i, videoStream;
      AVCodecContext  *pCodecCtx;
      AVCodec         *pCodec;
      AVFrame         *pFrame; 
      AVFrame         *pFrameRGB;
      int             numBytes;
      uint8_t         *buffer;

      // Register all formats and codecs
      av_register_all();

      // Open video file
      if(av_open_input_file(&pFormatCtx, pFilename, NULL, 0, NULL)!=0)
         return -1; // Couldn't open file

      // Retrieve stream information
      if(av_find_stream_info(pFormatCtx)<0)
         return -1; // Couldn't find stream information

       durationSec = (double)pFormatCtx->duration / AV_TIME_BASE;

      // Dump information about file onto standard error
      /*dump_format(pFormatCtx, 0, pFilename, false);*/

      // Find the first video stream
      videoStream=-1;
      for(i=0; i<pFormatCtx->nb_streams; i++)
      {
         if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
         {
            videoStream=i;
            break;
         }
      }
      if(videoStream==-1)
         return -1; // Didn't find a video stream

      // Get a pointer to the codec context for the video stream
      AVStream* st = pFormatCtx->streams[videoStream];
      fps = (float)st->r_frame_rate.num / (float)st->r_frame_rate.den;
      pCodecCtx=st->codec;

      // Find the decoder for the video stream
      pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
      if(pCodec==NULL)
         return -1; // Codec not found

      // Inform the codec that we can handle truncated bitstreams -- i.e.,
      // bitstreams where frame boundaries can fall in the middle of packets
      if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
         pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

      // Open codec
      if(avcodec_open(pCodecCtx, pCodec)<0)
         return -1; // Could not open codec

      // Hack to correct wrong frame rates that seem to be generated by some 
      // codecs
      /*
      if(pCodecCtx->frame_rate>1000 && pCodecCtx->frame_rate_base==1)
         pCodecCtx->frame_rate_base=1000;
         */

      // Allocate video frame
      pFrame=avcodec_alloc_frame();

      // Allocate an AVFrame structure
      pFrameRGB=avcodec_alloc_frame();
      if(pFrameRGB==NULL)
         return -1;

      // Determine required buffer size and allocate buffer
      numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
         pCodecCtx->height);
      buffer=new uint8_t[numBytes];

      // Assign appropriate parts of buffer to image planes in pFrameRGB
      avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
         pCodecCtx->width, pCodecCtx->height);

      // Reset variables
      StartNewMovie();
      aBytesPerFrame.clear();
      // Read frames
      i=0;
      while(GetNextFrame(pFormatCtx, pCodecCtx, videoStream, pFrame, aBytesPerFrame))
      {
         ++i;
      }

      // Free the RGB image
      delete [] buffer;
      av_free(pFrameRGB);

      // Free the YUV frame
      av_free(pFrame);

      // Close the codec
      avcodec_close(pCodecCtx);

      // Close the video file
      av_close_input_file(pFormatCtx);

      return 0;
   }

   int GetVideoInformation_WidthHeight(char* const pFilename, int& OutWidth, int& OutHeight)
   {
      AVFormatContext *pFormatCtx;
      int             i, videoStream;
      AVCodecContext  *pCodecCtx;
      AVCodec         *pCodec;

      // Register all formats and codecs
      av_register_all();

      // Open video file
      if(av_open_input_file(&pFormatCtx, pFilename, NULL, 0, NULL)!=0)
         return -1; // Couldn't open file

      // Retrieve stream information
      if(av_find_stream_info(pFormatCtx)<0)
         return -1; // Couldn't find stream information

      // Dump information about file onto standard error
      /*dump_format(pFormatCtx, 0, pFilename, false);*/

      // Find the first video stream
      videoStream=-1;
      for(i=0; i<pFormatCtx->nb_streams; i++)
      {
         if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
         {
            videoStream=i;
            break;
         }
      }
      if(videoStream==-1)
         return -1; // Didn't find a video stream

      // Get a pointer to the codec context for the video stream
      AVStream* st = pFormatCtx->streams[videoStream];
      //fps = (float)st->r_frame_rate.num / (float)st->r_frame_rate.den;
      OutWidth = st->codec->coded_width;
      OutHeight = st->codec->coded_height;
      pCodecCtx=st->codec;

      // Find the decoder for the video stream
      pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
      if(pCodec==NULL)
         return -1; // Codec not found

      // Inform the codec that we can handle truncated bitstreams -- i.e.,
      // bitstreams where frame boundaries can fall in the middle of packets
      if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
         pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

      // Close the video file
      av_close_input_file(pFormatCtx);

      return 0;
   }
}

//----------------------------------------------------------------------------
CStreamWriter_Movies::~CStreamWriter_Movies()
{
   if( IPU_info__fp != NULL )
   {
      fclose(IPU_info__fp);
      IPU_info__fp = NULL;
   }
}


//----------------------------------------------------------------------------
bool CStreamWriter_Movies::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   mpCurrentReplacementFileName = NULL;
   maReplacementFileNameIndex = 0;
   maReplacementFileName.clear();
   FindReplacementMovieFiles( maReplacementFileName, curPlatform );

   bool fileExists = false;
   for( int ii=0;ii<maReplacementFileName.size();++ii )
   {
      printf("Found replacement file at %s\n", maReplacementFileName[ii].c_str());
      fileExists = true;
   }

   return fileExists;
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::WriteOutInfo( STREAM_TAG & streamTag)
{
   if( streamTag._type == STREAM_TYPE_IPU && (gInfoModeMask & kInfoMode_IPUMask) )
   {
      if( IPU_info__hasInfo == false )
      {
         IPU_info__movieDataStartTick = streamTag._time;
         IPU_info__prevTick = streamTag._time;
         IPU_info__hasInfo = true;
      }
      else
      {
         int deltaTick = streamTag._time - IPU_info__prevTick;
         std::map<int,IPU_INFO_STRUCT>::iterator it = IPU_info__mapDeltaTicks.find(deltaTick);
         if( it == IPU_info__mapDeltaTicks.end() )
         {
            IPU_info__mapDeltaTicks[deltaTick] = IPU_INFO_STRUCT(1, streamTag._size);
         }
         else
         {
            IPU_INFO_STRUCT& infoStruct = it->second;
            ++infoStruct.count;
            infoStruct.totalSize += streamTag._size;
         }
         //
         IPU_info__prevTick = streamTag._time;
      }
   }
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::FinishInfo()
{
   if( (gInfoModeMask & kInfoMode_IPUMask) )
   {
      if( IPU_info__hasInfo )
      {
         char buffer[FILENAME_MAX];
         // ensure file is open
         if( IPU_info__fp == NULL )
         {
            sprintf(buffer, "%s/IPU_INFO.txt", gOutputFolder);
            fopen_s(&IPU_info__fp, buffer, "wb");
            if( IPU_info__fp == NULL )
            {
               printf( "Failed to open up IPU info file!\n" );
               return;
            }
         }
         if( IPU_info__conversionlist_fp == NULL )
         {
            sprintf(buffer, "%s/IPU_conversionlist.txt", gOutputFolder);
            fopen_s(&IPU_info__conversionlist_fp, buffer, "wb");
            if( IPU_info__conversionlist_fp == NULL )
            {
               printf( "Failed to open up IPU conversion list file!\n" );
               return;
            }
         }
         // write out all our info
         {
            fprintf( IPU_info__fp, "---------------------------------------------------------------------------------------------------" );
            fprintf( IPU_info__fp, "Fullpath: %s/%s/%s\n", gOutputFolder, gCurrStreamName, gCurrStreamName );
            fprintf( IPU_info__fp, "Filename: %s:\n", gCurrStreamName );
            fprintf( IPU_info__fp, "Movie start tick: %d:\n", IPU_info__movieDataStartTick );
            for( std::map<int,IPU_INFO_STRUCT>::iterator it = IPU_info__mapDeltaTicks.begin(); it != IPU_info__mapDeltaTicks.end(); ++it )
            {
               IPU_INFO_STRUCT const & infoStruct = it->second;
               if( infoStruct.count > 5 )
               {
                  fprintf( IPU_info__fp, "DeltaTick: %d count: %d\n", it->first, infoStruct.count );
               }
               else
               {
                  fprintf( IPU_info__fp, "DeltaTick: %d count: %d size: %d\n", it->first, infoStruct.count, infoStruct.totalSize );
               }
            } 
         }
         // write out conversion list
         {
            fprintf( IPU_info__conversionlist_fp, "%s/%s/%s.avi\n", gOutputFolder, gCurrStreamName, gCurrStreamName ); 
         }
         // clear data for next stream
         IPU_info__hasInfo = false;
         IPU_info__mapDeltaTicks.clear();
      }
   }
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::ModifyCurrentSourcePacket(STREAM_TAG &streamTag, unsigned char * const packetBuf, EPlatform /*curPlatform*/ )
{
   (void)packetBuf; // unused

   // Uses this function to detect when the first movie packet arrives.
   if (streamTag._type == STREAM_TYPE_IPU || streamTag._type == STREAM_TYPE_M2V)
   {
      if (!m_startWritingPackets)
      {
         float currTime = (float)streamTag._time / 300.0f;
         if( currTime - m_prevTime > mLastMovieLength )
         {
            m_startWritingPackets = true;
            m_prevTime = currTime;
            mMovieDataStartTick = streamTag._time;
         }
      }
   }
}

//----------------------------------------------------------------------------
unsigned int CStreamWriter_Movies::GetRebuildPlatformMask() const
{
   return kPlatformMask_PS3 | kPlatformMask_X360 | kPlatformMask_Vita;
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::LoadUpNextReplacementFile(EPlatform curPlatform)
{
   if( maReplacementFileNameIndex >= maReplacementFileName.size() )
   {
      return;
   }

   mpCurrentReplacementFileName = &maReplacementFileName[maReplacementFileNameIndex++];

   fopen_s(&m_pMovieSource__fp, mpCurrentReplacementFileName->c_str(), "rb");
   if (!m_pMovieSource__fp)
      return;

   fseek(m_pMovieSource__fp, 0, SEEK_END);
   m_totalBytes = ftell(m_pMovieSource__fp);
   fseek(m_pMovieSource__fp, 0, SEEK_SET);

   if( m_memoryBuffer == NULL )
      m_memoryBuffer = new unsigned char[kMemoryBufferSize]; 

   m_bufferPos = 0;

   m_startWritingPackets = false;
   mMovieDataStartTick = 0;

   if( curPlatform == kPlatform_PS3 )
   {
      mFrameRate = GetVideoFPS(mpCurrentReplacementFileName->c_str());

      mFrameInfos.clear();
      int frameCounter = 0;

      unsigned int currentFileOffset = 0;

      unsigned int bufferStartFileOffset = 0;
      int bufferValidSize = 0;

      while( currentFileOffset < m_totalBytes )
      {
         unsigned char* auAddr;
         unsigned int auSize;

         unsigned int const bufferEndFileOffset = bufferStartFileOffset + bufferValidSize;

         int const bufferRelativeOffset = currentFileOffset - bufferStartFileOffset;
         int const remainingValidBufferSize = bufferValidSize - bufferRelativeOffset;

         unsigned int const bytesRead = MPEG_Get_Access_Unit(m_memoryBuffer + bufferRelativeOffset, remainingValidBufferSize, &auAddr, &auSize, &mFrameRate);

         currentFileOffset += bytesRead;

         // Buffer does not contain enough data to decode a full frame.
         if( bytesRead == 0 )
         {
            // Shift all valid data in the buffer to the beginning.
            if( remainingValidBufferSize > 0 )
               memmove(m_memoryBuffer, m_memoryBuffer + bufferRelativeOffset, remainingValidBufferSize);

            // After the memmove our total amount of valid data reduces.
            bufferValidSize = remainingValidBufferSize;

            // Adjust beginning file offset of buffer
            bufferStartFileOffset += bufferRelativeOffset;

            // Amount of space that is currently unused in buffer at the end.
            unsigned int const bufferFreeSpace = kMemoryBufferSize - bufferValidSize;
            // Remaining amount of unconsumed data in the file.
            unsigned int const remainingFileSize = m_totalBytes - bufferEndFileOffset;

            // Read the lesser of free space in buffer or remaining size in file.
            unsigned int readSize = std::min(bufferFreeSpace, remainingFileSize);

            // Reached EOF?
            if( readSize == 0 )
               break;

            // Start reading into buffer beginning at end of current valid offset
            fread(m_memoryBuffer + bufferValidSize, readSize, 1, m_pMovieSource__fp);

            // We read more data into the buffer, so our buffer valid size increases.
            bufferValidSize += readSize;

            continue;
         }

         mFrameInfos.push_back(SFrameInfo(frameCounter, bufferStartFileOffset + (auAddr - m_memoryBuffer), auSize));
         ++frameCounter;
      }

      // Record the prior movie's length, and then record this one for future use
      mLastMovieLength = mMovieLength;
      mMovieLength = float( frameCounter ) / mFrameRate;
   }
   else if (curPlatform == kPlatform_X360 || curPlatform == kPlatform_Vita)
   {
      mLastMovieLength = mMovieLength;
      GetVideoInformation(mpCurrentReplacementFileName->c_str(), mVideoFrameByteSizes, mFrameRate, mMovieLength);
   }
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick)
{
   if( mHasWrittenOutInfoPackets == false )
   {
      LoadUpNextReplacementFile(curPlatform);

      // Only write out movie driver startup packet if there is actually a movie present.

      if( m_pMovieSource__fp != NULL )
      {
         STREAM_TAG streamTag;
         //Write a stream driver startup packet.
         streamTag._type = CHUNK_TYPE_SYSTEM;
         streamTag._size = sizeof(streamTag);
         streamTag._time = 0;
         streamTag._option = CHUNK_TYPE_MOVIE;

         fwrite(&streamTag, sizeof(streamTag), 1, streamfp);
      }

      mHasWrittenOutInfoPackets = true;
   }

   if( m_startWritingPackets )
   {
      if( m_bufferPos < m_totalBytes )
      {
         if( curPlatform == kPlatform_PS3 )
         {
            float movieTime = (currTick - mMovieDataStartTick) / (float)STREAM_TICKS_PER_SECOND;
            int const frameToDisplay = movieTime * mFrameRate;

            int numberOfBytesThatMustBeWritten = 0;

            for( int i = 0; i < mFrameInfos.size(); ++i )
            {
               if( mFrameInfos[i].mFrame <= frameToDisplay )
               {
                  numberOfBytesThatMustBeWritten = mFrameInfos[i].mStartOffset + mFrameInfos[i].mSize;
                  continue;
               }

               break;
            }

            int numberOfBytesToWrite = numberOfBytesThatMustBeWritten - m_bufferPos;

            while( numberOfBytesToWrite > 0 )
            {
               WriteMoviePacket(streamfp, curPlatform, currTick, kMaxPacketSizePS3);
               numberOfBytesToWrite -= kMaxPacketSizePS3;
            }
         }
         else
         {
            float movieTime = (currTick - mMovieDataStartTick) / (float)STREAM_TICKS_PER_SECOND;
            int frameToDisplay = int(movieTime * mFrameRate);
            if( frameToDisplay > int(mVideoFrameByteSizes.size()-1) )
            {
               frameToDisplay = mVideoFrameByteSizes.size()-1;
            }
            else if( frameToDisplay < 0 )
            {
               frameToDisplay = 0;
            }

            int numberOfBytesThatMustBeWritten = mVideoFrameByteSizes[frameToDisplay];
            int numberOfBytesToWrite = numberOfBytesThatMustBeWritten - m_bufferPos;
            int otherPossibleNumberOfBytesToWrite = m_totalBytes - m_bufferPos;

            if( otherPossibleNumberOfBytesToWrite < numberOfBytesToWrite )
            {
               numberOfBytesToWrite = otherPossibleNumberOfBytesToWrite;
            }

            while( numberOfBytesToWrite > 0 )
            {
               if (curPlatform == kPlatform_X360)
               {
                  WriteMoviePacket(streamfp, curPlatform, currTick, kMaxPacketSizeX360);
                  numberOfBytesToWrite -= kMaxPacketSizeX360;
               }
               else if (curPlatform == kPlatform_Vita)
               {
                  WriteMoviePacket(streamfp, curPlatform, currTick, kMaxPacketSizeVita);
                  numberOfBytesToWrite -= kMaxPacketSizeVita;
               }
            }
         }
      }
      if( m_bufferPos >= m_totalBytes )
      {
         // close the current file increment to next one
         LoadUpNextReplacementFile(curPlatform);
         m_startWritingPackets = false;
      }
   }
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::WriteMoviePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick, int const maxPacketSize)
{
   unsigned int _kMinPacketSize = 0;

   if ( curPlatform == kPlatform_PS3 )
   {
      _kMinPacketSize = kMaxPacketSizePS3;
   }
   else if ( curPlatform == kPlatform_X360 )
   {
      _kMinPacketSize = kMaxPacketSizeX360;
   }
   else
   {
      _kMinPacketSize = kMaxPacketSizeVita;
   }
	  
   unsigned int bytesToWrite = __min(maxPacketSize, m_totalBytes - m_bufferPos);
   unsigned int paddingBytes = ((bytesToWrite + 15) & ~15) - bytesToWrite;
   STREAM_TAG streamTag;

   streamTag._type = CHUNK_TYPE_MOVIE;
   streamTag._size = bytesToWrite + paddingBytes + sizeof(streamTag);
   streamTag._time = currTick;
   streamTag._option = m_totalBytes;
   
   fwrite(&streamTag, sizeof(streamTag), 1, streamfp);

   fseek( m_pMovieSource__fp, m_bufferPos, SEEK_SET );
   fread( &m_memoryBuffer[0], 1, bytesToWrite, m_pMovieSource__fp );
   fwrite( &m_memoryBuffer[0], 1, bytesToWrite, streamfp);

   char c = 0;
   for (unsigned int i = 0; i < paddingBytes; i++)
      fwrite(&c, 1, 1, streamfp);

   m_bufferPos += bytesToWrite;
}

//----------------------------------------------------------------------------
void CStreamWriter_Movies::EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag )
{
   while (m_bufferPos < m_totalBytes)
   {
      switch (curPlatform)
      {
      case kPlatform_X360:
         WriteMoviePacket(streamfp, curPlatform, 0, kMaxPacketSizeX360 );
         break;

      case kPlatform_PS3:
         WriteMoviePacket(streamfp, curPlatform, 0, kMaxPacketSizePS3 );
         break;
      
      case kPlatform_Vita:
         WriteMoviePacket(streamfp, curPlatform, 0, kMaxPacketSizeVita );
         break;

      default:
         throw false;
      }
   }

   if(m_pMovieSource__fp)
   {
      unsigned int remainingBytes = m_totalBytes - m_bufferPos;
      if (remainingBytes != 0)
         System::Console::WriteLine("Buffer underflow detected in movie stream.");

      fclose( m_pMovieSource__fp );
      m_pMovieSource__fp = NULL;
      m_bufferPos = 0;
      m_totalBytes = 0;
   }
   mHasWrittenOutInfoPackets = false;
   mMovieLength = 0.0f;
   mLastMovieLength = 0.0f;
   m_prevTime = 0.0f;

   if(m_memoryBuffer)
   {
      delete [] m_memoryBuffer;
      m_memoryBuffer = NULL;
   }
}

//----------------------------------------------------------------------------
bool CStreamWriter_Movies::ReplacesDataForCurrStream(const unsigned int type)
{
   if(type == STREAM_TYPE_IPU || type == STREAM_TYPE_M2V)
   {
      return m_pMovieSource__fp != NULL;
   }
   else
   {
      return false;
   }
}

#include <windows.h>
//----------------------------------------------------------------------------
void RunProcessWaitForCompletion(char * processName, char * commandLineArg, char const * const workingDir)
{
   STARTUPINFOA siStartupInfo;
   memset(&siStartupInfo, 0, sizeof(siStartupInfo));
   siStartupInfo.cb = sizeof(siStartupInfo);

   PROCESS_INFORMATION piProcessInfo;
   memset(&piProcessInfo, 0, sizeof(piProcessInfo));

   CreateProcessA(processName,
      commandLineArg,
      0,
      0,
      FALSE,
      0,//CREATE_DEFAULT_ERROR_MODE,
      0,
      workingDir,
      &siStartupInfo,
      &piProcessInfo);

   // Wait until application has terminated
   WaitForSingleObject(piProcessInfo.hProcess, INFINITE);

   // Close process and thread handles
   ::CloseHandle(piProcessInfo.hThread);
   ::CloseHandle(piProcessInfo.hProcess);
}
void ChangeFPS_SingleFile()
{
   bool fileExists = BP_FileExists(gCurrInputFilename) == 1;
   if(fileExists)
   {
      int srcWidth, srcHeight;
      GetVideoInformation_WidthHeight( gCurrInputFilename, srcWidth, srcHeight );

      char fullPath[FILENAME_MAX];
      char ffmpegCommandLineArguments[FILENAME_MAX];

      const char * const workingDir = getenv( "BPE_REPOSITORY" );
      sprintf( fullPath, "%s\\..\\bp\\Bin\\Tools\\ffmpeg.exe", workingDir );
      /*
      ffmpeg -i us/demo/p070_01_p01/p070_01_p01.avi -f rawvideo -pix_fmt yuv420p -vcodec rawvideo -s 256x256 -y temp.raw
      ffmpeg -f rawvideo -pix_fmt yuv420p -r 30 -s 256x256 -i temp.raw -y us/demo/p070_01_p01/ok.avi
      */
      sprintf( ffmpegCommandLineArguments, "dummy -i %s -f rawvideo -pix_fmt yuv420p -vcodec rawvideo -s %dx%d -y temp.raw", gCurrInputFilename, srcWidth, srcHeight );
      RunProcessWaitForCompletion( fullPath, ffmpegCommandLineArguments, workingDir );
      sprintf( ffmpegCommandLineArguments, "dummy -f rawvideo -pix_fmt yuv420p -r 30 -s %dx%d -i temp.raw -y %s", srcWidth, srcHeight, gCurrInputFilename );
      RunProcessWaitForCompletion( fullPath, ffmpegCommandLineArguments, workingDir );
   }
}
