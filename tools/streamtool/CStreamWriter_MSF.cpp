//----------------------------------------------------------------------------

#include "CStreamWriter_MSF.h"

//----------------------------------------------------------------------------

#define CELL_MS_16BIT_BIG  (0) 
#define CELL_MS_16BIT_LITTLE (1) 
#define CELL_MS_32BIT_FLOAT (2) 
#define CELL_MS_ADPCM  (3) 
#define CELL_MS_ATRAC_LOW  (4) 
#define CELL_MS_ATRAC_MEDIUM (5) 
#define CELL_MS_ATRAC_HIGH   (6) 
#define CELL_MS_MP3                 (7) 

//N.B. this struct is big-endian
typedef struct _MSFHeader {
   char header[3];       //Will be 'MSF' 
   char version;  //Revision of tool (Example: 0x32 = Version 3.2) 
   int compressionType; //Will be the compression type of the output  
   int channels;          //Number of channels in the output file 
   int sampleSize; //Number of *bytes* of sample data following the 
   //64 bytes of header data  
   int sampleRate; //Playback sample rate 
   int miscInfo;  //See "More About the MSF miscInfo Field" section. 
   int LoopMarkers[10]; //10 loop markers. See "MSF Loop Markers" section.
} MSFHeader;

typedef enum BP_DOSTREAM_FORMAT
{
   BP_DOSTREAM_FORMAT_PCM,
   BP_DOSTREAM_FORMAT_VAG,

   BP_DOSTREAM_FORMAT_COUNT
} BP_DOSTREAM_FORMAT;

typedef struct _MSFStreamHeader
{
   unsigned int   mCookie;
   int            mFormat;
   int            mChannels;
   int            mFrequency;
   int            mSampleSize;
   int            mPad[3];
} MSFStreamHeader;

//----------------------------------------------------------------------------

static void ByteSwap_Int_Always(int *n)
{
   unsigned int u = *(unsigned int*)n;
   *n = (u >> 24) | ((u >> 8) & 0xff00) | ((u & 0xff00) << 8) | (u << 24);
}

//----------------------------------------------------------------------------

CStreamWriter_MSF::CStreamWriter_MSF()
: mbInitialized( false )
{
}

bool CStreamWriter_MSF::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   return ReplacementFileExistsForCurrStream_Static(curPlatform);
}

bool CStreamWriter_MSF::ReplacementFileExistsForCurrStream_Static(EPlatform curPlatform)
{
   if(curPlatform!=kPlatform_PS3)
   {
      return false;
   }

   char fullPath[FILENAME_MAX];
   sprintf( fullPath, "%s/%s/%s.msf", gOutputFolder, gCurrStreamName, gCurrStreamName );
   return BP_FileExists( fullPath );
}

void CStreamWriter_MSF::WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick)
{
   if(curPlatform!=kPlatform_PS3)
   {
      return;
   }

   if( !mbInitialized )
   {
      mbInitialized = true;
      mSampleBytesWritten = 0;
      char fullPath[FILENAME_MAX];
      sprintf( fullPath, "%s/%s/%s.msf", gOutputFolder, gCurrStreamName, gCurrStreamName );
      if( FILE * fp = fopen( fullPath, "rb" ) )
      {
         MSFHeader msfHeader;
         fread( &msfHeader, sizeof( msfHeader ), 1, fp );
         //Check header and version
         if( strncmp( msfHeader.header, "MSFC", 4 ) != 0 )
         {
            printf("Bad MSF header or new version! (%s)\n", fullPath );
            throw false;
         }
         mCompressionType = msfHeader.compressionType;
         mChannels = msfHeader.channels;
         mFrequency = msfHeader.sampleRate;

         ByteSwap_Int_Always( &mCompressionType );
         ByteSwap_Int_Always( &mChannels );
         ByteSwap_Int_Always( &mFrequency );

         int sampleSize = msfHeader.sampleSize;
         ByteSwap_Int_Always( &sampleSize );

         mSampleBuffer.assign( sampleSize, 0 );
         fread( &mSampleBuffer[0], sampleSize, 1, fp );

         fclose( fp );

         STREAM_TAG streamTag;

         //Write a stream driver startup packet.
         streamTag._type = CHUNK_TYPE_SYSTEM;
         streamTag._size = sizeof( streamTag );
         streamTag._time = 0;
         streamTag._option = STREAM_TYPE_MSF;

         fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );

         //Write a header packet.

         MSFStreamHeader streamHeader = { 0 };
         streamHeader.mCookie = 'MSFC';
         ByteSwap_Int_Always( (int*)&streamHeader.mCookie );

         switch( mCompressionType )
         {
         case CELL_MS_ADPCM:
            {
               streamHeader.mFormat = BP_DOSTREAM_FORMAT_VAG;
            }
            break;
         default:
            {
               printf("CStreamWriter_MSF: TODO: support other audio formats? (%d)\n", mCompressionType );
            }
            throw false;
         }
         ByteSwap_Int_Always( &streamHeader.mFormat );
         streamHeader.mChannels = msfHeader.channels;
         streamHeader.mFrequency = msfHeader.sampleRate;
         streamHeader.mSampleSize = msfHeader.sampleSize;

         streamTag._type = STREAM_TYPE_MSF;
         streamTag._size = sizeof( streamTag ) + sizeof( streamHeader );
         streamTag._time = 0;
         streamTag._option = 0;

         fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );
         fwrite( &streamHeader, sizeof( streamHeader ), 1, streamfp );
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
         //See how much data we would like to write based on the current tick
         //and the amount of data already written.
         const int kMaxAudioLeadTicks = STREAM_TICKS_PER_SECOND / 2;       //allow audio to write packets 1/2 second ahead of current tick.

         const float desiredAudioTime = float( currTick + kMaxAudioLeadTicks ) / STREAM_TICKS_PER_SECOND;
         int samplesWritten = 0;
         switch( mCompressionType )
         {
         case CELL_MS_ADPCM:
            samplesWritten = ( ( __int64 )mSampleBytesWritten ) * 28 / 16;
            break;
         default:
            printf("CStreamWriter_MSF: TODO: support other audio formats? (%d)\n", mCompressionType );
            throw false;
         }
         const int sampleFramesWritten = samplesWritten / mChannels;
         const float currAudioTime = (float)((double)sampleFramesWritten / mFrequency);

         if( currAudioTime < desiredAudioTime )
         {
            //Write a fixed size data packet.
            const int kMaxPacketBodySize = 16 * 1024; //seems pretty standard for audio files.
            int packetBodySize = mSampleBuffer.size() - mSampleBytesWritten;
            if( packetBodySize > kMaxPacketBodySize )
               packetBodySize = kMaxPacketBodySize;

            if( packetBodySize > 0 )
            {
               STREAM_TAG streamTag;

               streamTag._type = STREAM_TYPE_MSF;
               streamTag._size = sizeof( streamTag ) + packetBodySize;
               streamTag._time = (int)(currAudioTime * 300);
               streamTag._option = packetBodySize; //1-byte granularity for this type of stream

               fwrite( &streamTag, sizeof( streamTag ), 1, streamfp );

               const unsigned char * pSampleData = &mSampleBuffer[mSampleBytesWritten];
               fwrite( pSampleData, packetBodySize, 1, streamfp );
               mSampleBytesWritten += packetBodySize;
            }
         }
      }
      else
      {
         //There's no replacement data for this stream.
      }
   }
}

void CStreamWriter_MSF::EndStream(FILE * const /*streamfp*/, EPlatform curPlatform, STREAM_TAG* pEndTag)
{
   (void)curPlatform;
   (void)pEndTag;

   if( mbInitialized )
   {
      if( mSampleBytesWritten < (int)mSampleBuffer.size() )
      {
         printf("TODO: is audio really the last stuff being written out in this stream?\n");
         //More likely, there's a bug and the audio is being written out too slowly
         throw false;
      }
      mbInitialized = false;
      mSampleBuffer.clear();
      mSampleBytesWritten = 0;
   }
}

bool CStreamWriter_MSF::ReplacesDataForCurrStream( const unsigned int type )
{
   if( type == STREAM_TYPE_STV || type == STREAM_TYPE_MTA || type == STREAM_TYPE_VAG )
   {
      return mSampleBuffer.size() != 0;
   }

   return false;
}

unsigned int CStreamWriter_MSF::GetRebuildPlatformMask() const
{
   return kPlatformMask_PS3;
}
