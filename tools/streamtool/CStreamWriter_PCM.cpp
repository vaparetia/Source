//----------------------------------------------------------------------------

#include "CStreamWriter_PCM.h"

#include "CStreamWriter_MSF.h"
#include "CStreamWriter_XAudio.h"
#include "CStreamWriter_Vita_AT9.h"

//----------------------------------------------------------------------------

#define SOUND_ONE_READ_SIZE 0x4000
#define VAGST_SPU_SIZE 0x800

namespace
{
   struct SPcmHeader
   {
      unsigned int   mSize;
      unsigned int   mFrequency;
      unsigned int   mChannels;
      unsigned int   mFormat;
   };

   struct SStvStreamHeader
   {
      char           mHeader[4];
      unsigned int   mVersion;
      unsigned int   mUnused0;
      unsigned int   mSize;
      unsigned int   mFrequency;
      unsigned int   mUnused1[3];
      char           mName[16];
      unsigned int   mUnused2[4];
   };
}

//----------------------------------------------------------------------------

static void ByteSwap_Int_Always(int *n)
{
   unsigned int u = *(unsigned int*)n;
   *n = (u >> 24) | ((u >> 8) & 0xff00) | ((u & 0xff00) << 8) | (u << 24);
}

static void ByteSwap_UShort_Always(unsigned short *n)
{
   unsigned short u = *(unsigned short*)n;
   *n = (u >> 8) | (u << 8);
}

static void build_pcm_fullpath( char * const fullPath, EPlatform curPlatform )
{
   sprintf( fullPath, "%s/%s/%s/%s.pcm", gOutputFolder, gCurrStreamName, BP_GetPlatformSubfolder(curPlatform), gCurrStreamName );
}

static bool other_platform_file_exists( EPlatform curPlatform )
{
   if( CStreamWriter_MSF::ReplacementFileExistsForCurrStream_Static( curPlatform ) )
   {
      return true;
   }

   if( CStreamWriter_XAudio::ReplacementFileExistsForCurrStream_Static( curPlatform ) )
   {
      return true;
   }

   if ( CStreamWriter_Vita_AT9::ReplacementFileExistsForCurrStream_Static( curPlatform ) )
   {
      return true;
   }

   //TODO: Vita format checks

   return false;
}

//----------------------------------------------------------------------------

CStreamWriter_PCM::CStreamWriter_PCM()
: mbInitialized( false )
#if MGS_VERSION==2
, mStreamType( CHUNK_TYPE_PCM )
#else
, mStreamType( STREAM_TYPE_STV )
#endif
{
}

bool CStreamWriter_PCM::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   //First, let platform-specific re-encoded sounds take precedence.
   //Don't write out original format data if such sounds exist.
   if( other_platform_file_exists( curPlatform ) )
   {
      return false;
   }

   //Look in platform subfolder for a file that would force a build of this platform due to this writer.
   //Some of these are in the base _bp folder; some are platform-specific and thus in a subfolder.
   char fullPath[FILENAME_MAX];
   build_pcm_fullpath( fullPath, curPlatform );
   return BP_FileExists( fullPath );
}

void CStreamWriter_PCM::DoWritePacket( FILE * const streamfp )
{
   int packetSize = std::min( (int)mSampleBuffer.size() - mSampleBytesWritten, SOUND_ONE_READ_SIZE );

   if( packetSize )
   {
      STREAM_TAG streamTag;
      streamTag._type = mStreamType;
      streamTag._size = sizeof( streamTag ) + packetSize;
      streamTag._time = mTick;
#if MGS_VERSION==2
      streamTag._option = 0;
#else
      streamTag._option = packetSize / ( mChannels * VAGST_SPU_SIZE );
      if( streamTag._option * (mChannels * VAGST_SPU_SIZE) != packetSize )
      {
         printf("Error: unaligned packet size!\n");
         throw false;
      }
#endif

      fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
      fwrite( &mSampleBuffer[mSampleBytesWritten], packetSize, 1, streamfp );

      mSampleBytesWritten += packetSize;
   }
}

void CStreamWriter_PCM::UpdateTick()
{
   if( mFormat == 0 )
   {
      int offset;

      if( mChannels > 1 ){
         offset = SOUND_ONE_READ_SIZE * 2;
      } else {
         offset = SOUND_ONE_READ_SIZE + 0x800 * 2;
      }

      mTick = ( int )( ( float )STREAM_TICKS_PER_SECOND *
         ( ( ( ( float )mSampleBytesWritten - offset ) * 28 / 16 )
         / mChannels ) / ( mFrequency ) );
   }
   else
   {
      printf("TODO: LNR8 format...?\n");
      throw false;
   }
}

void CStreamWriter_PCM::WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick)
{
   if( !mbInitialized )
   {
      mbInitialized = true;
      mSampleBytesWritten = 0;
      mTick = 0;

      if( other_platform_file_exists( curPlatform ) )
      {
         //Don't write anything if there's other platform-specific data going in.
         return;
      }

      char fullPath[FILENAME_MAX];
      build_pcm_fullpath( fullPath, curPlatform );
      if( !BP_FileExists( fullPath ) )
      {
         //Punt to shared / base pcm path if no platform-specific pcm file exists.
         build_pcm_fullpath( fullPath, kPlatform_Base );
      }
      if( FILE * fp = fopen( fullPath, "rb" ) )
      {
         fseek( fp, 0, SEEK_END );
         mNewSize = ftell( fp );
         fseek( fp, 0, SEEK_SET );

         SPcmHeader header;
         fread( &header, sizeof( header ), 1, fp );
         mOriginalSize = header.mSize;
         mFrequency = header.mFrequency;
         mChannels = header.mChannels;
         mFormat = header.mFormat;
         mNewSize -= sizeof( header );

         if( mNewSize & 15 )
         {
            printf("Error: source pcm file must be 16-byte aligned in size!\n");
            fclose( fp );
            throw false;
         }

         mSampleBuffer.assign( mNewSize, 0 );
         fread( &mSampleBuffer[0], mNewSize, 1, fp );

         fclose( fp );

         STREAM_TAG streamTag;

         //Write a stream driver startup packet.
         streamTag._type = CHUNK_TYPE_SYSTEM;
         streamTag._size = sizeof( streamTag );
         streamTag._time = 0;
         streamTag._option = mStreamType;

         fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
      }
      else
      {
         //There's no replacement data for this stream.
      }
   }
   else
   {
      if( mSampleBuffer.size() != 0 )
      {
         //We have data for this stream.

         if( mSampleBytesWritten == 0 )
         {
            //Write a header packet.

#if MGS_VERSION==2
            //PCM / str2 format.
            unsigned char headerBuf[16] = { 0 };

            int size = mNewSize;
            unsigned short frequency = (unsigned short)mFrequency;
            unsigned char channel = (unsigned char)mChannels;
            unsigned char format = (unsigned char)mFormat;
            ByteSwap_Int_Always( &size );
            ByteSwap_UShort_Always( &frequency );
            memcpy( headerBuf, &size, 4 );
            memcpy( headerBuf+6,&frequency, 2 );
            headerBuf[8] = channel;
            headerBuf[10] = format;

            STREAM_TAG streamTag;

            streamTag._type = mStreamType;
            streamTag._size = sizeof( streamTag ) + sizeof(headerBuf);
            streamTag._time = 0;
            streamTag._option = 0;

            fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
            fwrite( &headerBuf, sizeof( headerBuf ), 1, streamfp );

            //N.B. the original insertion logic for this type of stream is in output_sound_chank
            //of tools/stream/generate.c

            //Write out initial body packets.
            for( int i = 0; i < 2; i++ ){
               DoWritePacket( streamfp );
            }
#else
            //We don't have source code for original MGS3 STV insertion (not even in "MGS3" tools drop)...
            //Seems to be the same except that the header is different and they padded the thing with some
            //number of 2048-size pads of 0.  Seems to be required to prevent glitches at the end of vag
            //streams.  I put one out for each stereo channel.
            SStvStreamHeader stvHeader = { 0 };
            strcpy( stvHeader.mHeader, "VAG" );
            stvHeader.mHeader[3] = (char)( '0' + mChannels );
            stvHeader.mFrequency = mFrequency;
            stvHeader.mSize = mNewSize / mChannels;   //size is measured in sample frames not samples
            strncpy( stvHeader.mName, gCurrStreamName, sizeof( stvHeader.mName ) );
            ByteSwap_Int_Always( (int*)&stvHeader.mFrequency );
            ByteSwap_Int_Always( (int*)&stvHeader.mSize );

            STREAM_TAG streamTag;

            streamTag._type = mStreamType;
            streamTag._size = sizeof( streamTag ) + sizeof( stvHeader );
            streamTag._time = 0;
            streamTag._option = 0;

            fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
            fwrite( &stvHeader, sizeof( stvHeader ), 1, streamfp );

            //AFTER header is written out, pad physical size with zeroes.
            int padSize = mChannels * VAGST_SPU_SIZE;
            mSampleBuffer.reserve( mSampleBuffer.size()+padSize );
            while( padSize-- )
            {
               mSampleBuffer.push_back( 0 );
            }

            //Write out initial body packets. (same logic as for MGS2 PCM streams)
            for( int i = 0; i < 2; i++ ){
               DoWritePacket( streamfp );
            }
#endif
         }
         
         if( currTick >= mTick )
         {
            DoWritePacket( streamfp );
            UpdateTick();
         }
      }
      else
      {
         //There's no replacement data for this stream.
      }
   }
}

void CStreamWriter_PCM::EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag)
{
   (void)curPlatform;
   (void)pEndTag;

   if( mbInitialized )
   {
      while( mSampleBytesWritten < (int)mSampleBuffer.size() )
      {
         DoWritePacket( streamfp );
         UpdateTick();
      }
      mbInitialized = false;
      mSampleBuffer.clear();
      mSampleBytesWritten = 0;
   }
}

bool CStreamWriter_PCM::ReplacesDataForCurrStream( const unsigned int type )
{
   if( (type&0xffff) == CHUNK_TYPE_PCM )
   {
      return mSampleBuffer.size() != 0;
   }

   return false;
}

unsigned int CStreamWriter_PCM::GetRebuildPlatformMask() const
{
   return kPlatformMask_Base;
}
