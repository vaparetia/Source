//====================================================================================
//                               Wav File Player
//====================================================================================

#include <algorithm>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/System/CSPURSManager.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Memory/CMemoryAllocator.h"

#include "CWavFile.h"

#include "BP_FileSupport.h"
#include "BP_EndianSupport.h"
#include "BP_SoundSupport.h"

//====================================================================================

namespace
{
   const unsigned int kInvalidStreamHandle = (unsigned int)(-1);
}

//====================================================================================

CWavFile::CWavFile()
: mFileName()
, mRiffChunk()
, mFormatChunk()
, mDataChunkHeader()
, mWaveformSize( 0 )
, mpWaveformData( NULL )
, mWaveformDataOffset( NULL )
, mStreamHandle( kInvalidStreamHandle )
{
   memset( mFileName, 0, sizeof( mFileName ) );
   memset( &mRiffChunk, 0, sizeof( mRiffChunk ) );
   memset( &mFormatChunk, 0, sizeof( mFormatChunk ) );
   memset( &mDataChunkHeader, 0, sizeof( mDataChunkHeader ) );
}

CWavFile::~CWavFile()
{
   Stop();

   delete mpWaveformData;
}

void CWavFile::Load( const char * const filename )
{
   strcpy( mFileName, filename );
   LoadFromFile( &mRiffChunk, 0, sizeof( mRiffChunk ) );

   //Verify & byteswap chunk / header data
   BPE_ASSERT( !strncmp( mRiffChunk.mHeader.mId, "RIFF", 4 ), "Bad wav header" );
   BPE_ASSERT( !strncmp( mRiffChunk.mWavHeader, "WAVE", 4 ), "Bad wav header" );
   BP_LE_SwapSInt_Inp( &mRiffChunk.mHeader.mSize );

   int offset = sizeof( mRiffChunk );
   SIffChunkHeader iffHeader;
   do 
   {
      LoadFromFile( &iffHeader, offset, sizeof( iffHeader ) );
      BP_LE_SwapSInt_Inp( &iffHeader.mSize );
      offset += sizeof( iffHeader );
      if( !strncmp( iffHeader.mId, "fmt ", 4 ) )
      {
         //Load fmt chunk.
         BPE_ASSERT( iffHeader.mSize == sizeof( mFormatChunk ) - sizeof( mFormatChunk.mHeader ), "Bad fmt chunk" );
         mFormatChunk.mHeader = iffHeader;

         LoadFromFile( &mFormatChunk.mHeader + 1, offset, iffHeader.mSize );

         BP_LE_SwapSShort_Inp( &mFormatChunk.mFormatTag );
         BP_LE_SwapUShort_Inp( &mFormatChunk.mChannelCount );
         BP_LE_SwapUInt_Inp( &mFormatChunk.mFrequency );
         BP_LE_SwapUInt_Inp( &mFormatChunk.mAvgBytesPerSec );
         BP_LE_SwapUShort_Inp( &mFormatChunk.mBlockAlign );
         BP_LE_SwapUShort_Inp( &mFormatChunk.mBitsPerSample );
      }
      else if( !strncmp( iffHeader.mId, "data", 4 ) )
      {
         //Load data chunk.
         mDataChunkHeader = iffHeader;
         mWaveformSize = mDataChunkHeader.mSize;
         mWaveformDataOffset = offset;

         mpWaveformData = new unsigned char[mWaveformSize];
         LoadFromFile( mpWaveformData, offset, mWaveformSize );
         //Must byteswap into big-endian for the sake of X360.
         BP_LE_SwapSShortArray_Inp( (short*)mpWaveformData, mWaveformSize / sizeof( short ) );
      }
      else
      {
         printf("CWavFile: skipping chunk %.4s size %d\n", iffHeader.mId, iffHeader.mSize );
      }

      offset += iffHeader.mSize;

   } while ( offset + iffHeader.mSize < mRiffChunk.mHeader.mSize );

   //Make sure we got fmt and data chunks.
   BPE_ASSERT( mFormatChunk.mHeader.mSize > 0, "CWavFile: no fmt chunk found" );
   BPE_ASSERT( mDataChunkHeader.mSize > 0, "CWavFile: no data chunk found" );
}

void CWavFile::PlayOneShot()
{
   Stop();
   mStreamHandle = BP_PlayWaveform( mpWaveformData, mWaveformSize, mFormatChunk.mFrequency, mFormatChunk.mChannelCount );
}

void CWavFile::Stop()
{
   if( mStreamHandle != kInvalidStreamHandle )
   {
      //Must manually close stream to prevent moredata callbacks from producing read requests while shutting down.
      BP_StopWaveform( mStreamHandle );
      mStreamHandle = kInvalidStreamHandle;
   }
}

void CWavFile::SetVolume( const float volume )
{
   BP_SetWaveformVolume( mStreamHandle, volume );
}

void CWavFile::LoadFromFile( void * const pBuf, const int srcOffset, const int sz )
{
   if( sz == 0 )
   {
      return;
   }

   BP_ReadFileByFilename( mFileName, pBuf, srcOffset, sz );
}
