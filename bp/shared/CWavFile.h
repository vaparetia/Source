//====================================================================================
//                               Wav File Player
//====================================================================================

#pragma once

struct SIffChunkHeader
{
   char  mId[4];
   int   mSize;
};

struct SRIffChunk
{
   SIffChunkHeader mHeader;
   char  mWavHeader[4];
};

struct SFmtChunk
{
   SIffChunkHeader   mHeader;
   short             mFormatTag;
   unsigned short    mChannelCount;
   unsigned int      mFrequency;
   unsigned int      mAvgBytesPerSec;
   unsigned short    mBlockAlign;
   unsigned short    mBitsPerSample;
};

class CWavFile
{
public:
   CWavFile();
   ~CWavFile();

   void Load( const char * const filename );
   void Unload();

   void PlayOneShot();
   void Stop();

   void SetVolume( const float volume );

private:

   void LoadFromFile( void * const pBuf, const int srcOffset, const int sz );

public:
   char              mFileName[FILENAME_MAX];
   //Header
   SRIffChunk        mRiffChunk;
   SFmtChunk         mFormatChunk;
   SIffChunkHeader   mDataChunkHeader;

   //Playback data
   int               mWaveformSize;
   unsigned char *   mpWaveformData;
   int               mWaveformDataOffset;

   //Platform-specific playback handle.
   unsigned int      mStreamHandle;
};

